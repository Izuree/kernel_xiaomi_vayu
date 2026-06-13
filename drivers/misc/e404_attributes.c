// E404 kernel helper by Project 113 (kvsnr113)

#include <linux/e404_attributes.h>

#ifdef CONFIG_E404_EFFCPU_DEFAULT
bool early_effcpu = 1;
#else
bool early_effcpu = 0;
#endif
#ifdef CONFIG_E404_MIUI
int early_rom_type = 2;
#else
int early_rom_type = 1;
#endif
#ifdef CONFIG_E404_MIUI
int early_dtbo_type = 2;
#else
int early_dtbo_type = 1;
#endif
bool early_ksu = 1;
bool early_dtbo_130 = 0;

int early_lyb_override = 2;
bool early_lyb_pressure = false;

extern unsigned long sysctl_sched_features;

enum {
#define SCHED_FEAT(name, enabled) __SCHED_FEAT_##name ,
#include "../../kernel/sched/features.h"
#undef SCHED_FEAT
	__SCHED_FEAT_NR,
};

struct e404_attributes e404_data = {
    .effcpu                     = 0,
    .rom_type                   = 1,
    .dtbo_type                  = 0,
    .kgsl_skip_zeroing          = 0,
    .file_sync                  = 1,
    .panel_width                = 70,
    .panel_height               = 155,
    .bg_blocklist               = "com.shopee.id,com.lazada.android,com.tokopedia.tkpd",
    .effcpu                     = 1,
};

static int  blocked_cnt;
static u8   blocked_len[E404_MAX_BLOCKED];
static char blocked[E404_MAX_BLOCKED][TASK_COMM_LEN];

static struct kobject *e404_kobj;

static int __init parse_e404_args(char *str)
{
    char *arg;

    while ((arg = strsep(&str, " ,")) != NULL) {
        if (!*arg) continue;

        pr_alert("E404: Parsing flag: %s\n", arg);

        if (strcmp(arg, "dtb_effcpu") == 0)
            early_effcpu = 1;
        else if (strcmp(arg, "dtb_def") == 0)
            early_effcpu = 0;
        else if (strcmp(arg, "rom_port") == 0)
            early_rom_type = 3;
        else if (strcmp(arg, "rom_oem") == 0)
            early_rom_type = 2;
        else if (strcmp(arg, "rom_aosp") == 0)
            early_rom_type = 1;
        else if (strcmp(arg, "dtbo_120") == 0)
            early_dtbo_130 = 0;
        else if (strcmp(arg, "dtbo_130") == 0)
            early_dtbo_130 = 1;
        else if (strcmp(arg, "ksu") == 0)
            early_ksu = 1;
        else if (strcmp(arg, "noksu") == 0)
            early_ksu = 0;
        else if (strcmp(arg, "dtbo_def") == 0)
            early_dtbo_type = 1;
        else if (strcmp(arg, "dtbo_oem") == 0)
            early_dtbo_type = 2;
        else if (strcmp(arg, "lyb0") == 0)
            early_lyb_override = 0;
        else if (strcmp(arg, "lyb1") == 0)
            early_lyb_override = 1;
        else if (strcmp(arg, "lyb2") == 0) {
            early_lyb_override = 2;
            early_lyb_pressure = true;
        }
        else 
            pr_alert("E404: Unknown flag: %s\n", arg);
    }

    return 0;
}
early_param("e404_args", parse_e404_args);

bool e404_comm_blocked(const char *comm)
{
    int i;

    for (i = 0; i < blocked_cnt; i++) {
        if (!strncmp(comm,
                     blocked[i],
                     blocked_len[i]))
            return true;
    }

    return false;
}
EXPORT_SYMBOL_GPL(e404_comm_blocked);

static void e404_rebuild_blocklist(char *buf)
{
    char *p = buf;
    char *token;

    blocked_cnt = 0;
    while ((token = strsep(&p, ",")) &&
           blocked_cnt < E404_MAX_BLOCKED) {

        if (!*token)
            continue;

        strscpy(blocked[blocked_cnt],
                token,
                TASK_COMM_LEN);

        blocked_len[blocked_cnt] =
            strlen(blocked[blocked_cnt]);

        pr_alert("E404: blocking '%s'\n", blocked[blocked_cnt]);
        blocked_cnt++;
    }
    pr_alert("E404: total blocked apps = %d\n", blocked_cnt);
}

static ssize_t bg_blocklist_show(struct kobject *kobj,
                                 struct kobj_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%s\n",
                     e404_data.bg_blocklist);
}

static ssize_t bg_blocklist_store(struct kobject *kobj,
                                  struct kobj_attribute *attr,
                                  const char *buf, size_t count)
{
    char tmp[E404_BLOCKLIST_STRLEN];

    strscpy(tmp, buf, sizeof(tmp));
    strreplace(tmp, '\n', '\0');
    strscpy(e404_data.bg_blocklist,
            tmp,
            sizeof(e404_data.bg_blocklist));

    e404_rebuild_blocklist(tmp);

    return count;
}

static struct kobj_attribute bg_blocklist_attr =
    __ATTR(bg_blocklist, 0664, bg_blocklist_show, bg_blocklist_store);

#define E404_ATTR_RO(name) \
static ssize_t name##_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) { \
    return sprintf(buf, "%d\n", e404_data.name); \
} \
static struct kobj_attribute name##_attr = __ATTR(name, 0444, name##_show, NULL);

#define E404_ATTR_RW(name) \
static ssize_t name##_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) { \
    return sprintf(buf, "%d\n", e404_data.name); \
} \
static ssize_t name##_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) { \
    int ret, val, old_val; \
    ret = kstrtoint(buf, 10, &val); \
    if (ret) return ret; \
    old_val = e404_data.name; \
    e404_data.name = val; \
    pr_alert("E404: %s changed from %d to %d\n", #name, old_val, val); \
    sysfs_notify(e404_kobj, NULL, #name); \
    return count; \
} \
static struct kobj_attribute name##_attr = __ATTR(name, 0664, name##_show, name##_store);

E404_ATTR_RO(effcpu);
E404_ATTR_RO(rom_type);
E404_ATTR_RO(dtbo_type);
E404_ATTR_RO(panel_width);
E404_ATTR_RO(panel_height);
E404_ATTR_RO(ksu);
E404_ATTR_RW(kgsl_skip_zeroing);
E404_ATTR_RW(file_sync);

static struct attribute *e404_attrs[] = {
    &kgsl_skip_zeroing_attr.attr,
    &file_sync_attr.attr,
    &bg_blocklist_attr.attr,
    NULL,
};

static struct attribute_group e404_group = {
    .attrs = e404_attrs,
};

static struct attribute *e404_prop_attrs[] = {
    &effcpu_attr.attr,
    &rom_type_attr.attr,
    &dtbo_type_attr.attr,
    &panel_width_attr.attr,
    &panel_height_attr.attr,
    &ksu_attr.attr,
    NULL,
};

static struct attribute_group e404_prop_group = {
    .name  = "prop",
    .attrs = e404_prop_attrs,
};

static void e404_parse_attributes(void) {
    e404_data.effcpu      = early_effcpu;
    e404_data.rom_type    = early_rom_type;
    e404_data.dtbo_type   = early_dtbo_type;
    e404_data.ksu = early_ksu;
    e404_data.dtbo130 = early_dtbo_130;
}

#define LYB_ATTR_RW(name) \
static ssize_t lyb_##name##_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) { \
    return sprintf(buf, "%d\n", lyb_##name); \
} \
static ssize_t lyb_##name##_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) { \
    int ret, val; \
    ret = kstrtoint(buf, 10, &val); \
    if (ret) return ret; \
    lyb_##name = val; \
    return count; \
} \
static struct kobj_attribute lyb_##name##_attr = __ATTR(name, 0664, lyb_##name##_show, lyb_##name##_store);

LYB_ATTR_RW(override);
LYB_ATTR_RW(angle_callback);
LYB_ATTR_RW(touch_game_mode);
LYB_ATTR_RW(touch_active_mode);
LYB_ATTR_RW(touch_up_thresh);
LYB_ATTR_RW(touch_tolerance);
LYB_ATTR_RW(touch_edge);
LYB_ATTR_RW(touch_resist_rf);

static struct attribute *lyb_attrs[] = {
    &lyb_override_attr.attr,
    &lyb_angle_callback_attr.attr,
    &lyb_touch_game_mode_attr.attr,
    &lyb_touch_active_mode_attr.attr,
    &lyb_touch_up_thresh_attr.attr,
    &lyb_touch_tolerance_attr.attr,
    &lyb_touch_edge_attr.attr,
    &lyb_touch_resist_rf_attr.attr,
    NULL,
};

static struct attribute_group lyb_group = {
    .name  = "lyb",
    .attrs = lyb_attrs,
};

#define EEVDF_FEAT_ATTR_RW(name, file) \
static ssize_t eevdf_##file##_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) { \
    return sprintf(buf, "%d\n", !!(sysctl_sched_features & (1UL << __SCHED_FEAT_##name))); \
} \
static ssize_t eevdf_##file##_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) { \
    int ret, val; \
    ret = kstrtoint(buf, 10, &val); \
    if (ret) return ret; \
    if (val) \
        sysctl_sched_features |= (1UL << __SCHED_FEAT_##name); \
    else \
        sysctl_sched_features &= ~(1UL << __SCHED_FEAT_##name); \
    return count; \
} \
static struct kobj_attribute eevdf_##file##_attr = __ATTR(file, 0664, eevdf_##file##_show, eevdf_##file##_store);

EEVDF_FEAT_ATTR_RW(PLACE_LAG, place_lag);
EEVDF_FEAT_ATTR_RW(DELAY_DEQUEUE, delay_dequeue);
EEVDF_FEAT_ATTR_RW(RUN_TO_PARITY, run_to_parity);
EEVDF_FEAT_ATTR_RW(PREEMPT_SHORT, preempt_short);
EEVDF_FEAT_ATTR_RW(PICK_BUDDY, pick_buddy);
EEVDF_FEAT_ATTR_RW(HRTICK, hrtick);

static struct attribute *eevdf_attrs[] = {
    &eevdf_place_lag_attr.attr,
    &eevdf_delay_dequeue_attr.attr,
    &eevdf_run_to_parity_attr.attr,
    &eevdf_preempt_short_attr.attr,
    &eevdf_pick_buddy_attr.attr,
    &eevdf_hrtick_attr.attr,
    NULL,
};

static struct attribute_group eevdf_group = {
    .name  = "eevdf",
    .attrs = eevdf_attrs,
};

static int __init e404_init(void) {
    int ret;
    char tmp[E404_BLOCKLIST_STRLEN];

    e404_parse_attributes();

    /* EEVDF init values */
    sysctl_sched_features |= (0UL << __SCHED_FEAT_DELAY_DEQUEUE);
    sysctl_sched_features |= (0UL << __SCHED_FEAT_HRTICK);
    sysctl_sched_features |= (1UL << __SCHED_FEAT_PICK_BUDDY);
    sysctl_sched_features |= (0UL << __SCHED_FEAT_PREEMPT_SHORT);
    sysctl_sched_features |= (1UL << __SCHED_FEAT_RUN_TO_PARITY);

    if (e404_data.bg_blocklist[0]) {
        strscpy(tmp, e404_data.bg_blocklist, sizeof(tmp));
        e404_rebuild_blocklist(tmp);
    }

    e404_kobj = kobject_create_and_add("e404", kernel_kobj);
    if (!e404_kobj)
        return -ENOMEM;

    ret = sysfs_create_group(e404_kobj, &e404_group);
    if (ret)
        goto fail_kobj;

    ret = sysfs_create_group(e404_kobj, &e404_prop_group);
    if (ret)
        goto fail_group;

    ret = sysfs_create_group(e404_kobj, &lyb_group);
    if (ret)
        goto fail_prop_group;

    ret = sysfs_create_group(e404_kobj, &eevdf_group);
    if (ret)
        goto fail_lyb_group;

    pr_alert("E404: Helper Init !\n");
    return 0;

fail_lyb_group:
    sysfs_remove_group(e404_kobj, &lyb_group);

fail_prop_group:
    sysfs_remove_group(e404_kobj, &e404_prop_group);

fail_group:
    sysfs_remove_group(e404_kobj, &e404_group);
fail_kobj:
    kobject_put(e404_kobj);
    return ret;
}

static void __exit e404_exit(void) {
    sysfs_remove_group(e404_kobj, &eevdf_group);
    sysfs_remove_group(e404_kobj, &lyb_group);
    sysfs_remove_group(e404_kobj, &e404_prop_group);
    sysfs_remove_group(e404_kobj, &e404_group);
    kobject_put(e404_kobj);
    pr_alert("E404: Helper Exit !\n");
}

core_initcall(e404_init);
module_exit(e404_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("kvsnr113");
MODULE_DESCRIPTION("E404 kernel helper for features & stuff");
MODULE_VERSION("1.6");