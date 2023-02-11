#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <stdint.h>

#define KEYCODES_COUNT  6

typedef struct key_report {
        uint8_t modifier;
        uint8_t reserved;
        uint8_t keycode[KEYCODES_COUNT];
} key_report_st_t;


#endif /* __KEYBOARD_H__ */
