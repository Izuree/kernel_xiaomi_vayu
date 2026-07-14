#ifndef _E404_ATTRIBUTES_H
#define _E404_ATTRIBUTES_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/sched.h>

#define E404_BLOCKLIST_STRLEN 256
#define E404_MAX_BLOCKED 16

bool e404_comm_blocked(const char *comm);

struct e404_attributes {
    bool effcpu;
    int rom_type;
    int dtbo_type;
    bool kgsl_skip_zeroing;
    bool file_sync;
    int panel_width;
    int panel_height;
    char bg_blocklist[E404_BLOCKLIST_STRLEN];
    bool dtbo130;
    bool ir;
    bool fas;
};

extern struct e404_attributes e404_data;

extern bool early_effcpu;
extern int early_rom_type;
extern int early_dtbo_type;
extern bool early_dtbo_130;

extern int early_lyb_override;
extern bool early_lyb_pressure;
extern int lyb_override;
extern int lyb_angle_callback;
extern int lyb_touch_game_mode;
extern int lyb_touch_active_mode;
extern int lyb_touch_up_thresh;
extern int lyb_touch_tolerance;
extern int lyb_touch_edge;
extern int lyb_touch_resist_rf;

#endif /* _E404_ATTRIBUTES_H */
