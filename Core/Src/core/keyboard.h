#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <stdint.h>

#include "keymap.h"

#define BIT(x)                  (1 << (x))

#define KEY_ARRAY_ROW_CNT       6
#define KEY_ARRAY_CLM_CNT       16

#define KEY_ACTIVATE_PORT       GPIOA
#define KEY_SCAN_PORT           GPIOB

#define MAX_PRESSED_CNT         16
#define MAX_PRESSED_IN_REPORT   6

#define FN_KEY_ACTIVATE_PORT    GPIO_PIN_6
#define FN_KEY_SCAN_PORT        GPIO_PIN_10

typedef struct key_report {
        uint8_t id;
        uint8_t modifier;
        uint8_t reserved;
        uint8_t keys[MAX_PRESSED_IN_REPORT];
} key_report_st_t;

typedef enum key_state {
        KEY_RELEASED = 0,
        KEY_PRESSED  = 1
} key_state_t;

void key_input_to_usb_report(key_report_st_t *report);

#endif /* __KEYBOARD_H__ */
