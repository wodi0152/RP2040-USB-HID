#include <string.h>
#include "tusb.h"
#include "usb_descriptors.h"

/*
 * Стандартный дескриптор TinyUSB:
 * 6 аналоговых осей, D-pad и до 32 кнопок.
 * Пока будем использовать только четыре кнопки.
 */
static const uint8_t hid_report_descriptor[] =
    {
        TUD_HID_REPORT_DESC_KEYBOARD(
            HID_REPORT_ID(REPORT_ID_KEYBOARD)),

        TUD_HID_REPORT_DESC_CONSUMER(
            HID_REPORT_ID(REPORT_ID_CONSUMER))};

/* Дескриптор USB-устройства. */
static const tusb_desc_device_t device_descriptor =
    {
        .bLength = sizeof(tusb_desc_device_t),
        .bDescriptorType = TUSB_DESC_DEVICE,

        .bcdUSB = 0x0200,

        .bDeviceClass = 0x00,
        .bDeviceSubClass = 0x00,
        .bDeviceProtocol = 0x00,

        .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

        /* Учебные VID/PID. Для коммерческого устройства нужен свой VID/PID. */
        .idVendor = 0xCafe,
        .idProduct = 0x4005,
        .bcdDevice = 0x0100,

        .iManufacturer = 0x01,
        .iProduct = 0x02,
        .iSerialNumber = 0x03,

        .bNumConfigurations = 0x01};

uint8_t const *tud_descriptor_device_cb(void)
{
    return (uint8_t const *)&device_descriptor;
}

/* Интерфейсы USB-конфигурации. */
enum
{
    ITF_NUM_HID,
    ITF_NUM_TOTAL
};

#define CONFIG_TOTAL_LEN \
    (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)

#define EPNUM_HID_IN 0x81

static const uint8_t configuration_descriptor[] =
    {
        TUD_CONFIG_DESCRIPTOR(
            1,                /* Номер конфигурации */
            ITF_NUM_TOTAL,    /* Количество интерфейсов */
            0,                /* Индекс строки конфигурации */
            CONFIG_TOTAL_LEN, /* Общая длина */
            0x00,             /* Атрибуты */
            100               /* Потребление: 100 мА */
            ),

        TUD_HID_DESCRIPTOR(
            ITF_NUM_HID,
            0,
            HID_ITF_PROTOCOL_NONE,
            sizeof(hid_report_descriptor),
            EPNUM_HID_IN,
            CFG_TUD_HID_EP_BUFSIZE,
            5 /* Интервал опроса, мс */
            )};

uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index;
    return configuration_descriptor;
}

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    (void)instance;
    return hid_report_descriptor;
}

/* Строковые USB-дескрипторы. */
static const char *string_descriptors[] =
    {
        (const char[]){0x09, 0x04},
        "RP2040 Project",
        "RP2040 MacroPad",
        "000001"};

static uint16_t string_descriptor_buffer[32];

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void)langid;

    uint8_t character_count;

    if (index == 0)
    {
        memcpy(
            &string_descriptor_buffer[1],
            string_descriptors[0],
            2);

        character_count = 1;
    }
    else
    {
        if (index >=
            (sizeof(string_descriptors) / sizeof(string_descriptors[0])))
        {
            return NULL;
        }

        const char *text = string_descriptors[index];

        character_count = (uint8_t)strlen(text);

        if (character_count > 31)
        {
            character_count = 31;
        }

        for (uint8_t i = 0; i < character_count; i++)
        {
            string_descriptor_buffer[1 + i] = text[i];
        }
    }

    string_descriptor_buffer[0] =
        (uint16_t)((TUSB_DESC_STRING << 8) |
                   (2 * character_count + 2));

    return string_descriptor_buffer;
}

/*
 * Эти callbacks обязательны для HID.
 * Получать команды от ПК пока не требуется.
 */
uint16_t tud_hid_get_report_cb(
    uint8_t instance,
    uint8_t report_id,
    hid_report_type_t report_type,
    uint8_t *buffer,
    uint16_t requested_length)
{
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)requested_length;

    return 0;
}

void tud_hid_set_report_cb(
    uint8_t instance,
    uint8_t report_id,
    hid_report_type_t report_type,
    const uint8_t *buffer,
    uint16_t buffer_size)
{
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)buffer_size;
}