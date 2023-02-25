#include <stdbool.h>

#include "usbd_hid.h"
#include "usb_device.h"

#include "core.h"
#include "keyboard.h"

key_report_st_t report;
extern USBD_HandleTypeDef hUsbDeviceFS;

int user_main(void)
{
        HAL_Delay (3000);
        while (true) {
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

                key_input_to_usb_report(&report);

                USBD_HID_SendReport(&hUsbDeviceFS, &report,
                                    sizeof(key_report_st_t));
        };

        return 0;
}

