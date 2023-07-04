#include "usb/hid_ps3_driver.h"
#include "usb/usb_driver.h"

#include "class/hid/hid_device.h"
#include "pico/unique_id.h"

#include "tusb.h"

const tusb_desc_device_t ds3_desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = TUSB_CLASS_UNSPECIFIED,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0x054C,  // Sny
    .idProduct = 0x0268, // Dualshock3
    .bcdDevice = 0x0100,
    .iManufacturer = USBD_STR_MANUFACTURER,
    .iProduct = USBD_STR_PRODUCT,
    .iSerialNumber = USBD_STR_SERIAL,
    .bNumConfigurations = 1,
};

enum {
    USBD_ITF_HID,
    USBD_ITF_MAX,
};

#define USBD_PS3_DESC_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_INOUT_DESC_LEN)
const uint8_t ps3_desc_cfg[] = {
    TUD_CONFIG_DESCRIPTOR(1, USBD_ITF_MAX, USBD_STR_LANGUAGE, USBD_PS3_DESC_LEN, 0, USBD_MAX_POWER_MAX),
    TUD_HID_INOUT_DESCRIPTOR(USBD_ITF_HID, USBD_STR_PS3, 0, 148, 0x02, 0x81, CFG_TUD_HID_EP_BUFSIZE, 1),
};

const uint8_t ps3_desc_hid_report[] = {
    0x05, 0x01,       // Usage Page (Generic Desktop Ctrls)
    0x09, 0x04,       // Usage (Joystick)
    0xA1, 0x01,       // Collection (Application)
    0xA1, 0x02,       //   Collection (Logical)
    0x85, 0x01,       //     Report ID (1)
    0x75, 0x08,       //     Report Size (8)
    0x95, 0x01,       //     Report Count (1)
    0x15, 0x00,       //     Logical Minimum (0)
    0x26, 0xFF, 0x00, //     Logical Maximum (255)
    0x81, 0x03,       //     Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x75, 0x01,       //     Report Size (1)
    0x95, 0x13,       //     Report Count (19)
    0x15, 0x00,       //     Logical Minimum (0)
    0x25, 0x01,       //     Logical Maximum (1)
    0x35, 0x00,       //     Physical Minimum (0)
    0x45, 0x01,       //     Physical Maximum (1)
    0x05, 0x09,       //     Usage Page (Button)
    0x19, 0x01,       //     Usage Minimum (0x01)
    0x29, 0x13,       //     Usage Maximum (0x13)
    0x81, 0x02,       //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x75, 0x01,       //     Report Size (1)
    0x95, 0x0D,       //     Report Count (13)
    0x06, 0x00, 0xFF, //     Usage Page (Vendor Defined 0xFF00)
    0x81, 0x03,       //     Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x15, 0x00,       //     Logical Minimum (0)
    0x26, 0xFF, 0x00, //     Logical Maximum (255)
    0x05, 0x01,       //     Usage Page (Generic Desktop Ctrls)
    0x09, 0x01,       //     Usage (Pointer)
    0xA1, 0x00,       //     Collection (Physical)
    0x75, 0x08,       //       Report Size (8)
    0x95, 0x04,       //       Report Count (4)
    0x35, 0x00,       //       Physical Minimum (0)
    0x46, 0xFF, 0x00, //       Physical Maximum (255)
    0x09, 0x30,       //       Usage (X)
    0x09, 0x31,       //       Usage (Y)
    0x09, 0x32,       //       Usage (Z)
    0x09, 0x35,       //       Usage (Rz)
    0x81, 0x02,       //       Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,             //     End Collection
    0x05, 0x01,       //     Usage Page (Generic Desktop Ctrls)
    0x75, 0x08,       //     Report Size (8)
    0x95, 0x27,       //     Report Count (39)
    0x09, 0x01,       //     Usage (Pointer)
    0x81, 0x02,       //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x75, 0x08,       //     Report Size (8)
    0x95, 0x30,       //     Report Count (48)
    0x09, 0x01,       //     Usage (Pointer)
    0x91, 0x02,       //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x75, 0x08,       //     Report Size (8)
    0x95, 0x30,       //     Report Count (48)
    0x09, 0x01,       //     Usage (Pointer)
    0xB1, 0x02,       //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,             //   End Collection
    0xA1, 0x02,       //   Collection (Logical)
    0x85, 0x02,       //     Report ID (2)
    0x75, 0x08,       //     Report Size (8)
    0x95, 0x30,       //     Report Count (48)
    0x09, 0x01,       //     Usage (Pointer)
    0xB1, 0x02,       //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,             //   End Collection
    0xA1, 0x02,       //   Collection (Logical)
    0x85, 0xEE,       //     Report ID (-18)
    0x75, 0x08,       //     Report Size (8)
    0x95, 0x30,       //     Report Count (48)
    0x09, 0x01,       //     Usage (Pointer)
    0xB1, 0x02,       //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,             //   End Collection
    0xA1, 0x02,       //   Collection (Logical)
    0x85, 0xEF,       //     Report ID (-17)
    0x75, 0x08,       //     Report Size (8)
    0x95, 0x30,       //     Report Count (48)
    0x09, 0x01,       //     Usage (Pointer)
    0xB1, 0x02,       //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,             //   End Collection
    0xC0,             // End Collection
};

static hid_ps3_report_t last_report = {};

bool send_hid_ps3_report(usb_report_t report) {
    bool result = false;
    if (tud_hid_ready()) {
        result = tud_hid_report(0, report.data, report.size);
    }

    memcpy(&last_report, report.data, tu_min16(report.size, sizeof(hid_ps3_report_t)));

    return result;
}

static uint8_t ps3_report_0xf2[] = {
    0xff, 0xff, 0x00,                        // Unknown
    0x00, 0x07, 0x04,                        // MAC address OUI (ALPS Co.)
    0x39, 0x39, 0x39,                        // MAC manufacturer specific
    0x00, 0x03, 0x50, 0x81, 0xd8, 0x01, 0x8a // Unknown
};
static const uint8_t ps3_report_0xf5[] = {0x00, 0xf0, 0xf0, 0x02, 0x5e, 0x16, 0x26}; // Unknown

uint16_t hid_ps3_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t reqlen) {
    (void)itf;
    (void)reqlen;

    static bool do_init_mac = true;

    if (report_type == HID_REPORT_TYPE_INPUT) {
        memcpy(buffer, &last_report, sizeof(hid_ps3_report_t));
        return sizeof(hid_ps3_report_t);
    } else if (report_type == HID_REPORT_TYPE_FEATURE) {
        switch (report_id) {
        case 0xf2:
            if (do_init_mac) {
                pico_unique_board_id_t uid;
                pico_get_unique_board_id(&uid);

                // Genrate manufacturer specific using pico board id
                for (uint8_t i = 0; i < PICO_UNIQUE_BOARD_ID_SIZE_BYTES; ++i) {
                    ps3_report_0xf2[6 + (i % 3)] ^= uid.id[i];
                }

                do_init_mac = false;
            }

            memcpy(buffer, ps3_report_0xf2, sizeof(ps3_report_0xf2));
            return sizeof(ps3_report_0xf2);
        case 0xf5:
            memcpy(buffer, ps3_report_0xf5, sizeof(ps3_report_0xf5));
            return sizeof(ps3_report_0xf5);
        default:
        }
    }
    return 0;
}

typedef struct __attribute((packed, aligned(1))) {
    uint8_t rumble[4];
    uint8_t padding[4];
    uint8_t leds_bitmap;
    uint8_t leds[5][5];
} hid_ps3_ouput_report_t;

void hid_ps3_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize) {
    (void)itf;

    if (report_type == HID_REPORT_TYPE_INVALID) {
        if (bufsize > 0) {
            report_id = buffer[0];
            buffer = &buffer[1];
            bufsize--;
        }
    }

    switch (report_id) {
    case 0x01:
        if (bufsize == sizeof(hid_ps3_ouput_report_t)) {
            hid_ps3_ouput_report_t *report = (hid_ps3_ouput_report_t *)buffer;

            usb_player_led_t player_led = {.type = USB_PLAYER_LED_ID, .id = 0};
            player_led.id = 0 | ((report->leds_bitmap & 0x02) ? (1 << 0) : 0) //
                            | ((report->leds_bitmap & 0x04) ? (1 << 1) : 0)   //
                            | ((report->leds_bitmap & 0x08) ? (1 << 2) : 0)   //
                            | ((report->leds_bitmap & 0x10) ? (1 << 3) : 0);

            usb_driver_get_player_led_cb()(player_led);
        }
        break;
    default:
    }
}
