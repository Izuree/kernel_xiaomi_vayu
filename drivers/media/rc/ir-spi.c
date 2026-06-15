// SPDX-License-Identifier: GPL-2.0
// SPI driven IR LED device driver
//
// Copyright (c) 2016 Samsung Electronics Co., Ltd.
// Copyright (c) Andi Shyti <andi@etezian.org>
// Modified to support both legacy miscdevice and rc-core

#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/spi/spi.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <uapi/linux/lirc.h>
#include <media/rc-core.h>

#define IR_SPI_DRIVER_NAME		"ir-spi"

/* rc-core defaults */
#define IR_SPI_RC_DEFAULT_FREQ		38000
#define IR_SPI_RC_MAX_BUFSIZE		32768

/* legacy defaults */
#define IR_SPI_LEGACY_DEFAULT_FREQ	1920000
#define IR_SPI_LEGACY_BUFSIZE		150000

struct ir_spi_data {
	/* common */
	struct spi_device *spi;
	struct regulator *regulator;
	struct mutex lock;

	/* rc-core specific */
	struct rc_dev *rc;
	u32 rc_freq;
	u16 rc_tx_buf[IR_SPI_RC_MAX_BUFSIZE];
	u16 pulse;
	u16 space;
	bool negated;

	/* legacy specific */
	struct miscdevice misc;
	u32 legacy_freq;
	u8 *legacy_buffer;
	size_t legacy_buf_len;
	int nusers;
};

static struct ir_spi_data *ir_spi_global;

/* --- rc-core implementation --- */

static int ir_spi_rc_tx(struct rc_dev *dev,
		     unsigned int *buffer, unsigned int count)
{
	int i, ret;
	unsigned int len = 0;
	struct ir_spi_data *idata = dev->priv;
	struct spi_transfer xfer;

	mutex_lock(&idata->lock);

	/* convert the pulse/space signal to raw binary signal */
	for (i = 0; i < count; i++) {
		unsigned int periods;
		int j;
		u16 val;

		periods = DIV_ROUND_CLOSEST(buffer[i] * idata->rc_freq, 1000000);

		if (len + periods >= IR_SPI_RC_MAX_BUFSIZE) {
			mutex_unlock(&idata->lock);
			return -EINVAL;
		}

		val = (i % 2) ? idata->space : idata->pulse;
		for (j = 0; j < periods; j++)
			idata->rc_tx_buf[len++] = val;
	}

	memset(&xfer, 0, sizeof(xfer));
	xfer.speed_hz = idata->rc_freq * 16;
	xfer.len = len * sizeof(*idata->rc_tx_buf);
	xfer.tx_buf = idata->rc_tx_buf;

	ret = regulator_enable(idata->regulator);
	if (ret) {
		mutex_unlock(&idata->lock);
		return ret;
	}

	ret = spi_sync_transfer(idata->spi, &xfer, 1);
	if (ret)
		dev_err(&idata->spi->dev, "rc-core: unable to deliver the signal\n");

	regulator_disable(idata->regulator);
	mutex_unlock(&idata->lock);

	return ret ? ret : count;
}

static int ir_spi_set_tx_carrier(struct rc_dev *dev, u32 carrier)
{
	struct ir_spi_data *idata = dev->priv;

	if (!carrier)
		return -EINVAL;

	idata->rc_freq = carrier;
	return 0;
}

static int ir_spi_set_duty_cycle(struct rc_dev *dev, u32 duty_cycle)
{
	struct ir_spi_data *idata = dev->priv;
	int bits = (duty_cycle * 15) / 100;

	idata->pulse = GENMASK(bits, 0);

	if (idata->negated) {
		idata->pulse = ~idata->pulse;
		idata->space = 0xffff;
	} else {
		idata->space = 0;
	}

	return 0;
}

/* --- legacy implementation --- */

static ssize_t ir_spi_legacy_write(struct file *file,
					const char __user *buffer,
					size_t length, loff_t *offset)
{
	struct ir_spi_data *idata = ir_spi_global;
	struct spi_transfer xfer;
	int ret = 0;

	if (!idata)
		return -ENODEV;

	if (length > IR_SPI_LEGACY_BUFSIZE)
		return -EINVAL;

	mutex_lock(&idata->lock);

	if (copy_from_user(idata->legacy_buffer, buffer, length)) {
		ret = -EFAULT;
		goto out_unlock;
	}

	memset(&xfer, 0, sizeof(xfer));
	xfer.speed_hz = idata->legacy_freq;
	xfer.len = length;
	xfer.tx_buf = idata->legacy_buffer;

	ret = regulator_enable(idata->regulator);
	if (ret)
		goto out_unlock;

	ret = spi_sync_transfer(idata->spi, &xfer, 1);
	if (ret)
		dev_err(&idata->spi->dev, "legacy: unable to deliver the signal\n");

	regulator_disable(idata->regulator);

out_unlock:
	mutex_unlock(&idata->lock);

	return ret ? ret : length;
}

static int ir_spi_legacy_open(struct inode *inode, struct file *file)
{
	struct ir_spi_data *idata = ir_spi_global;

	if (!idata)
		return -ENODEV;

	mutex_lock(&idata->lock);
	idata->nusers++;
	mutex_unlock(&idata->lock);

	return 0;
}

static int ir_spi_legacy_release(struct inode *inode, struct file *file)
{
	struct ir_spi_data *idata = ir_spi_global;

	if (!idata)
		return -ENODEV;

	mutex_lock(&idata->lock);
	idata->nusers--;
	if (!idata->nusers) {
		idata->legacy_freq = IR_SPI_LEGACY_DEFAULT_FREQ;
	}
	mutex_unlock(&idata->lock);

	return 0;
}

static long ir_spi_legacy_ioctl(struct file *file, unsigned int cmd,
						unsigned long arg)
{
	__u32 p;
	int ret;
	struct ir_spi_data *idata = ir_spi_global;

	if (!idata)
		return -ENODEV;

	switch (cmd) {
	case LIRC_SET_SEND_MODE:
		ret = get_user(p, (__u32 __user *) arg);
		if (ret)
			return ret;
		/* Legacy implementation used this to set buffer length? 
		 * In legacy it just updated idata->xfer.len. 
		 * We'll just return success as we use a fixed max buffer.
		 */
		return 0;
	case LIRC_SET_SEND_CARRIER:
		ret = get_user(p, (__u32 __user *) arg);
		if (ret)
			return ret;
		mutex_lock(&idata->lock);
		idata->legacy_freq = p;
		mutex_unlock(&idata->lock);
		return 0;
	}

	return -EINVAL;
}

static const struct file_operations ir_spi_legacy_fops = {
	.owner   = THIS_MODULE,
	.write   = ir_spi_legacy_write,
	.open    = ir_spi_legacy_open,
	.release = ir_spi_legacy_release,
	.llseek  = noop_llseek,
	.unlocked_ioctl = ir_spi_legacy_ioctl,
	.compat_ioctl   = ir_spi_legacy_ioctl,
};

/* --- common probe/remove --- */

static int ir_spi_probe(struct spi_device *spi)
{
	int ret;
	u8 dc;
	struct ir_spi_data *idata;

	idata = devm_kzalloc(&spi->dev, sizeof(*idata), GFP_KERNEL);
	if (!idata)
		return -ENOMEM;

	idata->spi = spi;
	mutex_init(&idata->lock);
	ir_spi_global = idata;

	idata->regulator = devm_regulator_get(&spi->dev, "irda_regulator");
	if (IS_ERR(idata->regulator)) {
		/* Fallback to non-managed if necessary, but devm is better */
		return PTR_ERR(idata->regulator);
	}

	/* Legacy buffer allocation */
	idata->legacy_buffer = devm_kmalloc(&spi->dev, IR_SPI_LEGACY_BUFSIZE, GFP_KERNEL | GFP_DMA);
	if (!idata->legacy_buffer)
		return -ENOMEM;
	idata->legacy_freq = IR_SPI_LEGACY_DEFAULT_FREQ;

	/* rc-core allocation */
	idata->rc = devm_rc_allocate_device(&spi->dev, RC_DRIVER_IR_RAW_TX);
	if (!idata->rc)
		return -ENOMEM;

	idata->rc->tx_ir           = ir_spi_rc_tx;
	idata->rc->s_tx_carrier    = ir_spi_set_tx_carrier;
	idata->rc->s_tx_duty_cycle = ir_spi_set_duty_cycle;
	idata->rc->device_name	   = "IR SPI";
	idata->rc->driver_name     = IR_SPI_DRIVER_NAME;
	idata->rc->priv            = idata;

	idata->negated = of_property_read_bool(spi->dev.of_node, "led-active-low");
	ret = of_property_read_u8(spi->dev.of_node, "duty-cycle", &dc);
	if (ret)
		dc = 50;

	ir_spi_set_duty_cycle(idata->rc, dc);
	idata->rc_freq = IR_SPI_RC_DEFAULT_FREQ;

	/* Register rc-core device */
	ret = devm_rc_register_device(&spi->dev, idata->rc);
	if (ret) {
		dev_err(&spi->dev, "failed to register rc-core device\n");
		return ret;
	}

	/* Register legacy miscdevice */
	idata->misc.minor = MISC_DYNAMIC_MINOR;
	idata->misc.name = "ir_spi";
	idata->misc.fops = &ir_spi_legacy_fops;
	idata->misc.parent = &spi->dev;

	ret = misc_register(&idata->misc);
	if (ret) {
		dev_err(&spi->dev, "failed to register legacy misc device\n");
		/* We don't fail probe if legacy fails, but rc-core worked */
	}

	return 0;
}

static int ir_spi_remove(struct spi_device *spi)
{
	struct ir_spi_data *idata = ir_spi_global;

	if (idata) {
		misc_deregister(&idata->misc);
		ir_spi_global = NULL;
	}
	return 0;
}

static const struct of_device_id ir_spi_of_match[] = {
	{ .compatible = "ir-spi-led" },
	{ .compatible = "ir-spi" },
	{},
};
MODULE_DEVICE_TABLE(of, ir_spi_of_match);

static struct spi_driver ir_spi_driver = {
	.probe = ir_spi_probe,
	.remove = ir_spi_remove,
	.driver = {
		.name = IR_SPI_DRIVER_NAME,
		.of_match_table = ir_spi_of_match,
	},
};

module_spi_driver(ir_spi_driver);

MODULE_AUTHOR("Andi Shyti <andi@etezian.org>");
MODULE_DESCRIPTION("SPI IR LED Dual Mode");
MODULE_LICENSE("GPL v2");
