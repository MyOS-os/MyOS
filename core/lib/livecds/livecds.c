#include <kernel/lib/livecds.h>
#include <kernel/lib/string.h>

static livecds_device_t devices[LIVECDS_MAX_DEVICES];
static const livecds_driver_t *drivers[LIVECDS_MAX_DRIVERS];
static u32 device_count = 0;
static u32 driver_count = 0;
static bool initialized = false;

static bool name_equals(const char *a, const char *b) {
    if (!a || !b) {
        return false;
    }
    return strcmp(a, b) == 0;
}

void livecds_init(void) {
    if (initialized) {
        return;
    }
    kmemset(devices, 0, sizeof(devices));
    kmemset(drivers, 0, sizeof(drivers));
    device_count = 0;
    driver_count = 0;
    initialized = true;
}

livecds_status_t livecds_register_driver(const livecds_driver_t *driver) {
    if (!driver || !driver->name) {
        return LIVECDS_ERR_INVALID;
    }
    if (!initialized) {
        livecds_init();
    }
    if (driver_count >= LIVECDS_MAX_DRIVERS) {
        return LIVECDS_ERR_FULL;
    }
    for (u32 i = 0; i < driver_count; i++) {
        if (drivers[i] == driver || name_equals(drivers[i]->name, driver->name)) {
            return LIVECDS_ERR_BUSY;
        }
    }
    drivers[driver_count++] = driver;
    return LIVECDS_OK;
}

livecds_status_t livecds_unregister_driver(const livecds_driver_t *driver) {
    if (!driver) {
        return LIVECDS_ERR_INVALID;
    }
    for (u32 i = 0; i < driver_count; i++) {
        if (drivers[i] == driver) {
            for (u32 j = i + 1; j < driver_count; j++) {
                drivers[j - 1] = drivers[j];
            }
            drivers[driver_count - 1] = NULL;
            driver_count--;
            return LIVECDS_OK;
        }
    }
    return LIVECDS_ERR_NOT_FOUND;
}

static livecds_device_t *alloc_device_slot(void) {
    if (device_count >= LIVECDS_MAX_DEVICES) {
        return NULL;
    }
    livecds_device_t *dev = &devices[device_count];
    device_count++;
    return dev;
}

livecds_status_t livecds_register_device(const livecds_device_info_t *info, livecds_device_t **out_device) {
    if (!info || !info->name) {
        return LIVECDS_ERR_INVALID;
    }
    if (!initialized) {
        livecds_init();
    }

    livecds_device_t *dev = alloc_device_slot();
    if (!dev) {
        return LIVECDS_ERR_FULL;
    }

    dev->id = device_count - 1;
    dev->type = info->type;
    dev->info = *info;
    dev->driver = NULL;
    dev->driver_data = NULL;
    dev->online = false;
    dev->ops.read = NULL;
    dev->ops.write = NULL;
    dev->ops.ioctl = NULL;

    if (out_device) {
        *out_device = dev;
    }

    return LIVECDS_OK;
}

livecds_status_t livecds_unregister_device(livecds_device_t *dev) {
    if (!dev) {
        return LIVECDS_ERR_INVALID;
    }
    if (dev->online) {
        livecds_detach_device(dev);
    }

    bool found = false;
    for (u32 i = 0; i < device_count; i++) {
        if (&devices[i] == dev) {
            found = true;
        }
        if (found && i + 1 < device_count) {
            devices[i] = devices[i + 1];
            devices[i].id = i;
        }
    }

    if (!found) {
        return LIVECDS_ERR_NOT_FOUND;
    }

    if (device_count > 0) {
        device_count--;
        kmemset(&devices[device_count], 0, sizeof(livecds_device_t));
    }

    return LIVECDS_OK;
}

livecds_device_t *livecds_find_device(u32 id) {
    if (id >= device_count) {
        return NULL;
    }
    return &devices[id];
}

const livecds_driver_t *livecds_find_driver(const char *name) {
    if (!name) {
        return NULL;
    }
    for (u32 i = 0; i < driver_count; i++) {
        if (name_equals(drivers[i]->name, name)) {
            return drivers[i];
        }
    }
    return NULL;
}

static const livecds_driver_t *find_driver_for_device(const livecds_device_info_t *info) {
    for (u32 i = 0; i < driver_count; i++) {
        const livecds_driver_t *driver = drivers[i];
        if (!driver) {
            continue;
        }
        if (driver->type != LIVECDS_DEV_UNKNOWN && driver->type != info->type) {
            continue;
        }
        if (!driver->probe || driver->probe(info)) {
            return driver;
        }
    }
    return NULL;
}

livecds_status_t livecds_attach_device(livecds_device_t *dev) {
    if (!dev) {
        return LIVECDS_ERR_INVALID;
    }
    if (dev->online) {
        return LIVECDS_OK;
    }

    const livecds_driver_t *driver = find_driver_for_device(&dev->info);
    if (!driver || !driver->attach) {
        return LIVECDS_ERR_NOT_FOUND;
    }

    livecds_status_t status = driver->attach(dev);
    if (status != LIVECDS_OK) {
        return status;
    }

    dev->driver = driver;
    dev->online = true;
    return LIVECDS_OK;
}

livecds_status_t livecds_detach_device(livecds_device_t *dev) {
    if (!dev) {
        return LIVECDS_ERR_INVALID;
    }
    if (!dev->online) {
        return LIVECDS_OK;
    }

    if (dev->driver && dev->driver->detach) {
        dev->driver->detach(dev);
    }

    dev->driver = NULL;
    dev->online = false;
    return LIVECDS_OK;
}

livecds_status_t livecds_read(livecds_device_t *dev, u64 offset, void *buffer, u64 size) {
    if (!dev || !buffer || size == 0) {
        return LIVECDS_ERR_INVALID;
    }
    if (!dev->online || !dev->ops.read) {
        return LIVECDS_ERR_NOT_FOUND;
    }
    return dev->ops.read(dev, offset, buffer, size);
}

livecds_status_t livecds_write(livecds_device_t *dev, u64 offset, const void *buffer, u64 size) {
    if (!dev || !buffer || size == 0) {
        return LIVECDS_ERR_INVALID;
    }
    if (!dev->online || !dev->ops.write) {
        return LIVECDS_ERR_NOT_FOUND;
    }
    return dev->ops.write(dev, offset, buffer, size);
}

livecds_status_t livecds_ioctl(livecds_device_t *dev, u64 request, void *arg) {
    if (!dev || !dev->ops.ioctl) {
        return LIVECDS_ERR_INVALID;
    }
    if (!dev->online) {
        return LIVECDS_ERR_NOT_FOUND;
    }
    return dev->ops.ioctl(dev, request, arg);
}
