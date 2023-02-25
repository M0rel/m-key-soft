#include <stdbool.h>

#include "stm32f4xx_hal.h"

#include "keyboard.h"
#include "keymap.h"

/* GPIOB */
uint16_t gpios_to_scan[KEY_ARRAY_CLM_CNT] = {
        GPIO_PIN_0,
        GPIO_PIN_1,
        GPIO_PIN_2,
        GPIO_PIN_3,
        GPIO_PIN_4,
        GPIO_PIN_5,
        GPIO_PIN_6,
        GPIO_PIN_7,
        GPIO_PIN_8,
        GPIO_PIN_9,
        GPIO_PIN_10,
        GPIO_PIN_12,
        GPIO_PIN_13,
        GPIO_PIN_14,
        GPIO_PIN_15
};

/* GPIOA */
uint16_t gpios_to_activate[KEY_ARRAY_ROW_CNT] = {
        GPIO_PIN_1,
        GPIO_PIN_2,
        GPIO_PIN_3,
        GPIO_PIN_4,
        GPIO_PIN_5,
        GPIO_PIN_6
};

uint8_t key_array[KEY_ARRAY_ROW_CNT][KEY_ARRAY_CLM_CNT] = {
        {
                KEY_ESC, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7,
                KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12, KEY_PRT_SCRN,
                KEY_SCRL_LOCK, KEY_DELETE_FORW
        },
        {
                KEY_TILDE, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7,
                KEY_8, KEY_9, KEY_0, KEY_UNDERSCORE, KEY_EQUAL, NO_KEY,
                KEY_DELETE, KEY_HOME
        },
        {
                KEY_TAB, KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I,
                KEY_O, KEY_P, KEY_OPEN_BRACE, KEY_CLOSE_BRACE, NO_KEY,
                KEY_BACKSLASH, KEY_PAGE_UP
        },
        {
                KEY_CAPS_LOCK, KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J,
                KEY_K, KEY_L, KEY_SEMICOLON, KEY_SINGLE_QUOTE, NO_KEY, NO_KEY,
                KEY_ENTER, KEY_PAGE_DOWN
        },
        {
                KEY_LEFT_SHIFT, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M,
                KEY_LESS_THAN, KEY_GREATER_THAN, KEY_SLASH, KEY_RIGHT_SHIFT,
                NO_KEY, NO_KEY, KEY_UP_ARROW, KEY_END
        },
        {
                KEY_LEFT_CTRL, KEY_LEFT_GUI, KEY_LEFT_ALT, KEY_SPACE, NO_KEY,
                NO_KEY, NO_KEY, NO_KEY, NO_KEY, KEY_RIGHT_ALT, FN_KEY,
                KEY_LEFT_ARROW, NO_KEY, NO_KEY, KEY_DOWN_ARROW, KEY_RIGHT_ARROW
        }
};

static
uint8_t get_modifier_key_bit(uint8_t keycode)
{
	switch(keycode) {
                case KEY_LEFT_CTRL:
			return BIT(0);
                case KEY_LEFT_SHIFT:
			return BIT(1);
                case KEY_LEFT_ALT:
			return BIT(2);
                case KEY_LEFT_GUI:
			return BIT(3);
                case KEY_RIGHT_CTRL:
			return BIT(4);
                case KEY_RIGHT_SHIFT:
			return BIT(5);
                case KEY_RIGHT_ALT:
			return BIT(6);
                case KEY_RIGHT_GUI:
			return BIT(7);
                default:
			return 0;
	}
}

static
uint8_t normalize_buffer(uint8_t *array, uint8_t size)
{
        uint8_t free_position = 0;
        uint8_t index = 0;
        uint8_t i = 0;

        for (i = 0; i < size; i++) {
                index = i;

                while((index != size)) {
                        if (array[index] == NO_KEY) {
                                index++;

                                continue;
                        }

                        if (i == index) {
                                free_position += 1;

                                break;
                        }

                        array[i] = array[index];
                        array[index] = NO_KEY;

                        free_position = i;

                        break;
                }
        }

        return free_position;
}

static
bool is_fn_pressed(void)
{
        GPIO_PinState state = false;

        HAL_GPIO_WritePin(KEY_ACTIVATE_PORT, FN_KEY_ACTIVATE_PORT,
                          GPIO_PIN_SET);

        state = HAL_GPIO_ReadPin(KEY_SCAN_PORT, FN_KEY_SCAN_PORT);

        HAL_GPIO_WritePin(KEY_ACTIVATE_PORT, FN_KEY_ACTIVATE_PORT,
                          GPIO_PIN_RESET);

        return state;
}

/* poll matrix */
uint8_t poll_keys_matrix(uint8_t *array, uint8_t size)
{
        GPIO_PinState state = false;
        uint8_t pressed_cnt = 0;
        bool fn_pressed = false;
        volatile int i = 0;
        volatile int j = 0;

        fn_pressed = is_fn_pressed();

        for (i = 0; i < KEY_ARRAY_ROW_CNT; i++) {
                HAL_GPIO_WritePin(KEY_ACTIVATE_PORT, gpios_to_activate[i],
                                  GPIO_PIN_SET);

                for (j = 0; j < KEY_ARRAY_CLM_CNT; j++) {
                        state = HAL_GPIO_ReadPin(KEY_SCAN_PORT,
                                                 gpios_to_scan[j]);
                        if (GPIO_PIN_RESET == state) {
                                continue;
                        }

                        array[pressed_cnt]= key_array[i][j];
                        pressed_cnt++;
                }

                HAL_GPIO_WritePin(KEY_ACTIVATE_PORT, gpios_to_activate[i],
                                  GPIO_PIN_RESET);
        }

        return pressed_cnt;
}

static
uint8_t set_modifier_keys(uint8_t *pressed_keys, uint8_t *pressed_cnt)
{
        uint8_t modifier_keys = 0;
        uint8_t tmp_size = 0;
        int j = 0;

        for (j = 0; j < *pressed_cnt; j++) {
                if (KEY_LEFT_CTRL <= pressed_keys[j] &&
                    KEY_RIGHT_GUI >= pressed_keys[j]) {
                        modifier_keys |= get_modifier_key_bit(pressed_keys[j]);

                        pressed_keys[j] = NO_KEY;
                }
        }

        tmp_size = normalize_buffer(pressed_keys, *pressed_cnt);
        *pressed_cnt = tmp_size;

        return modifier_keys;
}

static
void is_pressed_in_buffer(uint8_t *pressed_keys, uint8_t *pressed_cnt,
        key_report_st_t *report, uint8_t report_cnt)
{
        bool key_not_found = true;
        int i = 0;
        int j = 0;

        for (i = 0; i < report_cnt; i++) {
                for (j = 0; j < *pressed_cnt; j++) {
                        if (pressed_keys[j]== report->keycodes[i]) {
                                pressed_keys[j] = NO_KEY;

                                key_not_found = false;

                                break;
                        }

                }

                if (true == key_not_found) {
                        report->keycodes[i] = NO_KEY;
                }

                key_not_found = true;
        }

        *pressed_cnt = normalize_buffer(pressed_keys, *pressed_cnt);
}

static
void fill_buffer(uint8_t *pressed_keys, uint8_t pressed_cnt,
        uint8_t *report, uint8_t report_size)
{
        volatile int start_idx = 0;
        volatile int key_idx = 0;

        start_idx = normalize_buffer(report, MAX_PRESSED_IN_REPORT);

        for (; start_idx < report_size; start_idx++) {
                if (NO_KEY == pressed_keys[key_idx]) {
                        /* Buffer is empty. Nothing to write */
                        break;
                }

                report[start_idx] = pressed_keys[key_idx];
                key_idx++;
        }
}

static
void prepare_send_buffer(uint8_t *pressed_keys, uint8_t *pressed_cnt,
        key_report_st_t *report, uint8_t report_cnt)
{
        report->modifier = set_modifier_keys(pressed_keys, pressed_cnt);

        is_pressed_in_buffer(pressed_keys, pressed_cnt, report, report_cnt);

        fill_buffer(pressed_keys, *pressed_cnt, report->keycodes,
                    MAX_PRESSED_IN_REPORT);
}

void key_input_to_usb_report(key_report_st_t *report)
{
        uint8_t pressed_keys[MAX_PRESSED_CNT] = {0};
        uint8_t pressed_cnt = 0;

        pressed_cnt = poll_keys_matrix(pressed_keys, MAX_PRESSED_CNT);

        prepare_send_buffer(pressed_keys, &pressed_cnt, report,
                            MAX_PRESSED_IN_REPORT);
}

