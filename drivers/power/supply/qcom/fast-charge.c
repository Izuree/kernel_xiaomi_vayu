// SPDX-License-Identifier: GPL-2.0
/*
 * fast-charge.c - Fast charging override driver
 *
 * Exposes /sys/kernel/fast_charge/force_fast_charge
 */

#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/init.h>
#include <linux/pmic-voter.h>

#define FAST_CHARGE_VOTER		"FAST_CHARGE_VOTER"
#define THERMAL_DAEMON_VOTER		"THERMAL_DAEMON_VOTER"
#define RESTRICT_CHG_VOTER		"RESTRICT_CHG_VOTER"

#define FAST_CHARGE_ICL_UA		6000000
#define FAST_CHARGE_FCC_UA		6000000

static struct kobject *fast_charge_kobj;
static int force_fast_charge;
extern bool skip_thermal;

static void fast_charge_apply(bool enable)
{
	skip_thermal = enable; 
	struct votable *fcc_votable     = find_votable("FCC");
	struct votable *usb_icl_votable = find_votable("USB_ICL");

	if (enable) {
		if (fcc_votable) {
			vote(fcc_votable, FAST_CHARGE_VOTER, true,
			     FAST_CHARGE_FCC_UA);
			vote(fcc_votable, THERMAL_DAEMON_VOTER, false, 0);
			vote(fcc_votable, RESTRICT_CHG_VOTER, false, 0);
		}
		if (usb_icl_votable)
			vote(usb_icl_votable, FAST_CHARGE_VOTER, true,
			     FAST_CHARGE_ICL_UA);

		pr_info("fast_charge: enabled — FCC/ICL set to 6A\n");
	} else {
		if (fcc_votable)
			vote(fcc_votable, FAST_CHARGE_VOTER, false, 0);
		if (usb_icl_votable)
			vote(usb_icl_votable, FAST_CHARGE_VOTER, false, 0);

		pr_info("fast_charge: disabled\n");
	}
}

static ssize_t force_fast_charge_show(struct kobject *kobj,
				      struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", force_fast_charge);
}

static ssize_t force_fast_charge_store(struct kobject *kobj,
					struct kobj_attribute *attr,
					const char *buf, size_t count)
{
	int val, ret;

	ret = kstrtoint(buf, 10, &val);
	if (ret)
		return ret;
	if (val < 0 || val > 1)
		return -EINVAL;
	if (val == force_fast_charge)
		return count;

	force_fast_charge = val;
	fast_charge_apply(!!val);

	return count;
}

static struct kobj_attribute force_fast_charge_attr =
	__ATTR(force_fast_charge, 0644,
	       force_fast_charge_show, force_fast_charge_store);

static struct attribute *fast_charge_attrs[] = {
	&force_fast_charge_attr.attr,
	NULL,
};

static struct attribute_group fast_charge_attr_group = {
	.attrs = fast_charge_attrs,
};

static int __init fast_charge_init(void)
{
	int ret;

	fast_charge_kobj = kobject_create_and_add("fast_charge", kernel_kobj);
	if (!fast_charge_kobj)
		return -ENOMEM;

	ret = sysfs_create_group(fast_charge_kobj, &fast_charge_attr_group);
	if (ret)
		kobject_put(fast_charge_kobj);

	return ret;
}

static void __exit fast_charge_exit(void)
{
	if (force_fast_charge)
		fast_charge_apply(false);

	sysfs_remove_group(fast_charge_kobj, &fast_charge_attr_group);
	kobject_put(fast_charge_kobj);
}

module_init(fast_charge_init);
module_exit(fast_charge_exit);

MODULE_DESCRIPTION("Fast charging driver");
MODULE_LICENSE("GPL v2");
