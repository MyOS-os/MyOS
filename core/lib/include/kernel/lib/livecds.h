#pragma once

#include <kernel/lib/types.h>

#define LIVECDS_NAME_MAX 32
#define LIVECDS_MAX_DEVICES 64
#define LIVECDS_MAX_DRIVERS 64

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
    LIVECDS_ERR_BUSY = -4
} livecds_status_t;

typedef struct livecds_device livecds_device_t;

typedef struct {
    const char *name;
    u32 vendor_id;
    u32 device_id;
    livecds_device_type_t type;
} livecds_device_info_t;

typedef struct {
    const char *name;
    livecds_device_type_t type;
    bool (*probe)(const livecds_device_info_t *info);
    livecds_status_t (*attach)(livecds_device_t *dev);
    void (*detach)(livecds_device_t *dev);
} livecds_driver_t;

typedef struct {
    livecds_status_t (*read)(livecds_device_t *dev, u64 offset, void *buffer, u64 size);
    livecds_status_t (*write)(livecds_device_t *dev, u64 offset, const void *buffer, u64 size);
    livecds_status_t (*ioctl)(livecds_device_t *dev, u64 request, void *arg);
} livecds_device_ops_t;

struct livecds_device {
    u32 id;
    livecds_device_type_t type;
    livecds_device_info_t info;
    const livecds_driver_t *driver;
    livecds_device_ops_t ops;
    void *driver_data;
    bool online;
};

// Инициализация подсистемы
void livecds_init(void);

// Регистрация драйверов
livecds_status_t livecds_register_driver(const livecds_driver_t *driver);
livecds_status_t livecds_unregister_driver(const livecds_driver_t *driver);

// Регистрация устройств
livecds_status_t livecds_register_device(const livecds_device_info_t *info, livecds_device_t **out_device);
livecds_status_t livecds_unregister_device(livecds_device_t *dev);

// Поиск
livecds_device_t *livecds_find_device(u32 id);
const livecds_driver_t *livecds_find_driver(const char *name);

// Управление устройствами
livecds_status_t livecds_attach_device(livecds_device_t *dev);
livecds_status_t livecds_detach_device(livecds_device_t *dev);

// Запросы ввода/вывода
livecds_status_t livecds_read(livecds_device_t *dev, u64 offset, void *buffer, u64 size);
livecds_status_t livecds_write(livecds_device_t *dev, u64 offset, const void *buffer, u64 size);
livecds_status_t livecds_ioctl(livecds_device_t *dev, u64 request, void *arg);
