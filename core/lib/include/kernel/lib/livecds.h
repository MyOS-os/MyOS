#pragma once

#include <kernel/lib/types.h>

#define LIVECDS_NAME_MAX 32
#define LIVECDS_MAX_DEVICES 64
#define LIVECDS_MAX_DRIVERS 64
#define LIVECDS_MAX_BUSES 16
#define LIVECDS_MAX_EVENTS 128

// Базовые типы устройства/драйвера

typedef enum {
    LIVECDS_DEV_UNKNOWN = 0,
    LIVECDS_DEV_BLOCK,
    LIVECDS_DEV_CHAR,
    LIVECDS_DEV_NETWORK,
    LIVECDS_DEV_DISPLAY,
    LIVECDS_DEV_INPUT
} livecds_device_type_t;

typedef enum {
    LIVECDS_OK = 0,
    LIVECDS_ERR_FULL = -1,
    LIVECDS_ERR_NOT_FOUND = -2,
    LIVECDS_ERR_INVALID = -3,
    LIVECDS_ERR_BUSY = -4,
    LIVECDS_ERR_UNSUPPORTED = -5
} livecds_status_t;

typedef enum {
    LIVECDS_STATE_NONE = 0,
    LIVECDS_STATE_REGISTERED = 1 << 0,
    LIVECDS_STATE_ATTACHED = 1 << 1,
    LIVECDS_STATE_ONLINE = 1 << 2,
    LIVECDS_STATE_SUSPENDED = 1 << 3
} livecds_device_state_t;

typedef enum {
    LIVECDS_CAP_DMA = 1 << 0,
    LIVECDS_CAP_IRQ = 1 << 1,
    LIVECDS_CAP_HOTPLUG = 1 << 2,
    LIVECDS_CAP_BOOT = 1 << 3
} livecds_device_caps_t;

typedef enum {
    LIVECDS_EVENT_DEVICE_ADD = 0,
    LIVECDS_EVENT_DEVICE_REMOVE,
    LIVECDS_EVENT_DEVICE_ATTACH,
    LIVECDS_EVENT_DEVICE_DETACH,
    LIVECDS_EVENT_DEVICE_SUSPEND,
    LIVECDS_EVENT_DEVICE_RESUME
} livecds_event_type_t;

typedef struct livecds_device livecds_device_t;

typedef struct {
    const char *name;
    u32 vendor_id;
    u32 device_id;
    livecds_device_type_t type;
    u64 caps;
} livecds_device_info_t;

typedef struct {
    livecds_event_type_t type;
    u32 device_id;
    livecds_device_type_t dev_type;
} livecds_event_t;

typedef struct {
    const char *name;
    livecds_device_type_t type;
    bool (*probe)(const livecds_device_info_t *info);
    livecds_status_t (*attach)(livecds_device_t *dev);
    void (*detach)(livecds_device_t *dev);
    livecds_status_t (*suspend)(livecds_device_t *dev);
    livecds_status_t (*resume)(livecds_device_t *dev);
} livecds_driver_t;

typedef struct {
    livecds_status_t (*read)(livecds_device_t *dev, u64 offset, void *buffer, u64 size);
    livecds_status_t (*write)(livecds_device_t *dev, u64 offset, const void *buffer, u64 size);
    livecds_status_t (*ioctl)(livecds_device_t *dev, u64 request, void *arg);
} livecds_device_ops_t;

typedef struct {
    const char *name;
    livecds_device_type_t type;
    livecds_status_t (*enumerate)(void);
} livecds_bus_t;

struct livecds_device {
    u32 id;
    livecds_device_type_t type;
    livecds_device_info_t info;
    char name[LIVECDS_NAME_MAX];
    const livecds_driver_t *driver;
    livecds_device_ops_t ops;
    void *driver_data;
    u64 state;
};

// Инициализация подсистемы
void livecds_init(void);

// Регистрация драйверов
livecds_status_t livecds_register_driver(const livecds_driver_t *driver);
livecds_status_t livecds_unregister_driver(const livecds_driver_t *driver);

// Регистрация шин
livecds_status_t livecds_register_bus(const livecds_bus_t *bus);
livecds_status_t livecds_unregister_bus(const livecds_bus_t *bus);

// Регистрация устройств
livecds_status_t livecds_register_device(const livecds_device_info_t *info, livecds_device_t **out_device);
livecds_status_t livecds_unregister_device(livecds_device_t *dev);

// Поиск
livecds_device_t *livecds_find_device(u32 id);
const livecds_driver_t *livecds_find_driver(const char *name);
const livecds_bus_t *livecds_find_bus(const char *name);

// Управление устройствами
livecds_status_t livecds_attach_device(livecds_device_t *dev);
livecds_status_t livecds_detach_device(livecds_device_t *dev);
livecds_status_t livecds_suspend_device(livecds_device_t *dev);
livecds_status_t livecds_resume_device(livecds_device_t *dev);

// Запросы ввода/вывода
livecds_status_t livecds_read(livecds_device_t *dev, u64 offset, void *buffer, u64 size);
livecds_status_t livecds_write(livecds_device_t *dev, u64 offset, const void *buffer, u64 size);
livecds_status_t livecds_ioctl(livecds_device_t *dev, u64 request, void *arg);

// Работа с устройством
void livecds_set_ops(livecds_device_t *dev, const livecds_device_ops_t *ops);
void livecds_set_driver_data(livecds_device_t *dev, void *data);
void *livecds_get_driver_data(livecds_device_t *dev);

// События
livecds_status_t livecds_push_event(livecds_event_type_t type, livecds_device_t *dev);
livecds_status_t livecds_pop_event(livecds_event_t *event);

// Итерация
void livecds_for_each_device(void (*callback)(livecds_device_t *dev, void *ctx), void *ctx);

// Автодетект устройств на зарегистрированных шинах
void livecds_autodetect(void);

// Диагностика
u32 livecds_device_count(void);
u32 livecds_driver_count(void);
u32 livecds_bus_count(void);
bool livecds_device_online(const livecds_device_t *dev);
bool livecds_device_attached(const livecds_device_t *dev);

// Проверка возможностей
bool livecds_device_has_cap(const livecds_device_t *dev, u64 cap);
livecds_status_t livecds_require_caps(const livecds_device_t *dev, u64 caps);

// Счетчики по типу
u64 livecds_count_block_devices(void);
u64 livecds_count_char_devices(void);
u64 livecds_count_network_devices(void);
u64 livecds_count_display_devices(void);
u64 livecds_count_input_devices(void);
