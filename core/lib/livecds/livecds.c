#include <kernel/lib/livecds.h>
#include <kernel/lib/memory.h>
#include <kernel/lib/string.h>

// ==============================================
// LiveCDS: простая подсистема драйверов устройств
// ==============================================
//
// Цели:
// - Регистрация устройств и драйверов.
// - Поиск совместимого драйвера по типу/ID.
// - Поддержка attach/detach/suspend/resume.
// - Минимальная очередь событий для ядра.
// - Возможность регистрации шин (bus) для
//   автоматического перечисления устройств.
//
// Важно: это базовая инфраструктура без блокировок.
// В дальнейшем можно добавить синхронизацию.
//

static livecds_device_t devices[LIVECDS_MAX_DEVICES];
static const livecds_driver_t *drivers[LIVECDS_MAX_DRIVERS];
static const livecds_bus_t *buses[LIVECDS_MAX_BUSES];
static livecds_event_t events[LIVECDS_MAX_EVENTS];

static u32 device_count = 0;
static u32 driver_count = 0;
static u32 bus_count = 0;
static u32 event_head = 0;
static u32 event_tail = 0;
static bool initialized = false;

// ========================
// Вспомогательные функции
// ========================

static bool name_equals(const char *a, const char *b) {
    if (!a || !b) {
        return false;
    }
    return strcmp(a, b) == 0;
}

static void copy_name(char *dest, const char *src) {
    if (!dest) {
        return;
    }
    if (!src) {
        dest[0] = '\0';
        return;
    }
    strncpy(dest, src, LIVECDS_NAME_MAX - 1);
    dest[LIVECDS_NAME_MAX - 1] = '\0';
}

static void push_event(livecds_event_type_t type, livecds_device_t *dev) {
    livecds_event_t *slot = &events[event_tail % LIVECDS_MAX_EVENTS];
    slot->type = type;
    slot->device_id = dev ? dev->id : 0;
    slot->dev_type = dev ? dev->type : LIVECDS_DEV_UNKNOWN;
    event_tail = (event_tail + 1) % LIVECDS_MAX_EVENTS;
    if (event_tail == event_head) {
        event_head = (event_head + 1) % LIVECDS_MAX_EVENTS;
    }
}

static bool event_queue_empty(void) {
    return event_head == event_tail;
}

static bool event_queue_full(void) {
    return ((event_tail + 1) % LIVECDS_MAX_EVENTS) == event_head;
}

// ========================
// Инициализация
// ========================

void livecds_init(void) {
    if (initialized) {
        return;
    }
    kmemset(devices, 0, sizeof(devices));
    kmemset(drivers, 0, sizeof(drivers));
    kmemset(buses, 0, sizeof(buses));
    kmemset(events, 0, sizeof(events));
    device_count = 0;
    driver_count = 0;
    bus_count = 0;
    event_head = 0;
    event_tail = 0;
    initialized = true;
}

static void ensure_init(void) {
    if (!initialized) {
        livecds_init();
    }
}

// ========================
// Регистрация драйверов
// ========================

livecds_status_t livecds_register_driver(const livecds_driver_t *driver) {
    if (!driver || !driver->name) {
        return LIVECDS_ERR_INVALID;
    }
    ensure_init();
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

// ========================
// Регистрация шин
// ========================

livecds_status_t livecds_register_bus(const livecds_bus_t *bus) {
    if (!bus || !bus->name) {
        return LIVECDS_ERR_INVALID;
    }
    ensure_init();
    if (bus_count >= LIVECDS_MAX_BUSES) {
        return LIVECDS_ERR_FULL;
    }
    for (u32 i = 0; i < bus_count; i++) {
        if (buses[i] == bus || name_equals(buses[i]->name, bus->name)) {
            return LIVECDS_ERR_BUSY;
        }
    }
    buses[bus_count++] = bus;
    return LIVECDS_OK;
}

livecds_status_t livecds_unregister_bus(const livecds_bus_t *bus) {
    if (!bus) {
        return LIVECDS_ERR_INVALID;
    }
    for (u32 i = 0; i < bus_count; i++) {
        if (buses[i] == bus) {
            for (u32 j = i + 1; j < bus_count; j++) {
                buses[j - 1] = buses[j];
            }
            buses[bus_count - 1] = NULL;
            bus_count--;
            return LIVECDS_OK;
        }
    }
    return LIVECDS_ERR_NOT_FOUND;
}

const livecds_bus_t *livecds_find_bus(const char *name) {
    if (!name) {
        return NULL;
    }
    for (u32 i = 0; i < bus_count; i++) {
        if (name_equals(buses[i]->name, name)) {
            return buses[i];
        }
    }
    return NULL;
}

static void enumerate_buses(void) {
    for (u32 i = 0; i < bus_count; i++) {
        if (buses[i] && buses[i]->enumerate) {
            buses[i]->enumerate();
        }
    }
}

// ========================
// Регистрация устройств
// ========================

static livecds_device_t *alloc_device_slot(void) {
    if (device_count >= LIVECDS_MAX_DEVICES) {
        return NULL;
    }
    livecds_device_t *dev = &devices[device_count];
    device_count++;
    return dev;
}

static void init_device(livecds_device_t *dev, const livecds_device_info_t *info) {
    kmemset(dev, 0, sizeof(livecds_device_t));
    dev->type = info->type;
    dev->info = *info;
    dev->state = LIVECDS_STATE_REGISTERED;
    copy_name(dev->name, info->name);
}

livecds_status_t livecds_register_device(const livecds_device_info_t *info, livecds_device_t **out_device) {
    if (!info || !info->name) {
        return LIVECDS_ERR_INVALID;
    }
    ensure_init();

    livecds_device_t *dev = alloc_device_slot();
    if (!dev) {
        return LIVECDS_ERR_FULL;
    }

    dev->id = device_count - 1;
    init_device(dev, info);

    if (out_device) {
        *out_device = dev;
    }

    push_event(LIVECDS_EVENT_DEVICE_ADD, dev);
    return LIVECDS_OK;
}

livecds_status_t livecds_unregister_device(livecds_device_t *dev) {
    if (!dev) {
        return LIVECDS_ERR_INVALID;
    }
    if (dev->state & LIVECDS_STATE_ATTACHED) {
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

    push_event(LIVECDS_EVENT_DEVICE_REMOVE, dev);
    return LIVECDS_OK;
}

// ========================
// Поиск
// ========================

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
        if (drivers[i] && name_equals(drivers[i]->name, name)) {
            return drivers[i];
        }
    }
    return NULL;
}

// ========================
// Подбор драйвера
// ========================

static bool driver_matches(const livecds_driver_t *driver, const livecds_device_info_t *info) {
    if (!driver) {
        return false;
    }
    if (driver->type != LIVECDS_DEV_UNKNOWN && driver->type != info->type) {
        return false;
    }
    if (driver->probe && !driver->probe(info)) {
        return false;
    }
    return true;
}

static const livecds_driver_t *find_driver_for_device(const livecds_device_info_t *info) {
    for (u32 i = 0; i < driver_count; i++) {
        const livecds_driver_t *driver = drivers[i];
        if (driver_matches(driver, info)) {
            return driver;
        }
    }
    return NULL;
}

// ========================
// Управление устройствами
// ========================

livecds_status_t livecds_attach_device(livecds_device_t *dev) {
    if (!dev) {
        return LIVECDS_ERR_INVALID;
    }
    if (dev->state & LIVECDS_STATE_ATTACHED) {
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
    dev->state |= LIVECDS_STATE_ATTACHED | LIVECDS_STATE_ONLINE;
    push_event(LIVECDS_EVENT_DEVICE_ATTACH, dev);
    return LIVECDS_OK;
}

livecds_status_t livecds_detach_device(livecds_device_t *dev) {
    if (!dev) {
        return LIVECDS_ERR_INVALID;
    }
    if (!(dev->state & LIVECDS_STATE_ATTACHED)) {
        return LIVECDS_OK;
    }

    if (dev->driver && dev->driver->detach) {
        dev->driver->detach(dev);
    }

    dev->driver = NULL;
    dev->state &= ~(LIVECDS_STATE_ATTACHED | LIVECDS_STATE_ONLINE);
    push_event(LIVECDS_EVENT_DEVICE_DETACH, dev);
    return LIVECDS_OK;
}

livecds_status_t livecds_suspend_device(livecds_device_t *dev) {
    if (!dev) {
        return LIVECDS_ERR_INVALID;
    }
    if (!(dev->state & LIVECDS_STATE_ATTACHED)) {
        return LIVECDS_ERR_NOT_FOUND;
    }
    if (dev->state & LIVECDS_STATE_SUSPENDED) {
        return LIVECDS_OK;
    }

    if (dev->driver && dev->driver->suspend) {
        livecds_status_t status = dev->driver->suspend(dev);
        if (status != LIVECDS_OK) {
            return status;
        }
    }

    dev->state |= LIVECDS_STATE_SUSPENDED;
    push_event(LIVECDS_EVENT_DEVICE_SUSPEND, dev);
    return LIVECDS_OK;
}

livecds_status_t livecds_resume_device(livecds_device_t *dev) {
    if (!dev) {
        return LIVECDS_ERR_INVALID;
    }
    if (!(dev->state & LIVECDS_STATE_ATTACHED)) {
        return LIVECDS_ERR_NOT_FOUND;
    }
    if (!(dev->state & LIVECDS_STATE_SUSPENDED)) {
        return LIVECDS_OK;
    }

    if (dev->driver && dev->driver->resume) {
        livecds_status_t status = dev->driver->resume(dev);
        if (status != LIVECDS_OK) {
            return status;
        }
    }

    dev->state &= ~LIVECDS_STATE_SUSPENDED;
    push_event(LIVECDS_EVENT_DEVICE_RESUME, dev);
    return LIVECDS_OK;
}

// ========================
// Работа с устройствами
// ========================

void livecds_set_ops(livecds_device_t *dev, const livecds_device_ops_t *ops) {
    if (!dev || !ops) {
        return;
    }
    dev->ops = *ops;
}

void livecds_set_driver_data(livecds_device_t *dev, void *data) {
    if (!dev) {
        return;
    }
    dev->driver_data = data;
}

void *livecds_get_driver_data(livecds_device_t *dev) {
    if (!dev) {
        return NULL;
    }
    return dev->driver_data;
}

// ========================
// Запросы ввода/вывода
// ========================

livecds_status_t livecds_read(livecds_device_t *dev, u64 offset, void *buffer, u64 size) {
    if (!dev || !buffer || size == 0) {
        return LIVECDS_ERR_INVALID;
    }
    if (!(dev->state & LIVECDS_STATE_ONLINE) || !dev->ops.read) {
        return LIVECDS_ERR_NOT_FOUND;
    }
    return dev->ops.read(dev, offset, buffer, size);
}

livecds_status_t livecds_write(livecds_device_t *dev, u64 offset, const void *buffer, u64 size) {
    if (!dev || !buffer || size == 0) {
        return LIVECDS_ERR_INVALID;
    }
    if (!(dev->state & LIVECDS_STATE_ONLINE) || !dev->ops.write) {
        return LIVECDS_ERR_NOT_FOUND;
    }
    return dev->ops.write(dev, offset, buffer, size);
}

livecds_status_t livecds_ioctl(livecds_device_t *dev, u64 request, void *arg) {
    if (!dev || !dev->ops.ioctl) {
        return LIVECDS_ERR_INVALID;
    }
    if (!(dev->state & LIVECDS_STATE_ONLINE)) {
        return LIVECDS_ERR_NOT_FOUND;
    }
    return dev->ops.ioctl(dev, request, arg);
}

// ========================
// События
// ========================

livecds_status_t livecds_push_event(livecds_event_type_t type, livecds_device_t *dev) {
    if (!dev) {
        return LIVECDS_ERR_INVALID;
    }
    if (event_queue_full()) {
        return LIVECDS_ERR_FULL;
    }
    push_event(type, dev);
    return LIVECDS_OK;
}

livecds_status_t livecds_pop_event(livecds_event_t *event) {
    if (!event) {
        return LIVECDS_ERR_INVALID;
    }
    if (event_queue_empty()) {
        return LIVECDS_ERR_NOT_FOUND;
    }
    *event = events[event_head % LIVECDS_MAX_EVENTS];
    event_head = (event_head + 1) % LIVECDS_MAX_EVENTS;
    return LIVECDS_OK;
}

// ========================
// Итерация по устройствам
// ========================

void livecds_for_each_device(void (*callback)(livecds_device_t *dev, void *ctx), void *ctx) {
    if (!callback) {
        return;
    }
    for (u32 i = 0; i < device_count; i++) {
        callback(&devices[i], ctx);
    }
}

// ========================
// Автопоиск устройств
// ========================

void livecds_autodetect(void) {
    ensure_init();
    enumerate_buses();
}

// ========================
// Диагностика
// ========================

u32 livecds_device_count(void) {
    return device_count;
}

u32 livecds_driver_count(void) {
    return driver_count;
}

u32 livecds_bus_count(void) {
    return bus_count;
}

bool livecds_device_online(const livecds_device_t *dev) {
    if (!dev) {
        return false;
    }
    return (dev->state & LIVECDS_STATE_ONLINE) != 0;
}

bool livecds_device_attached(const livecds_device_t *dev) {
    if (!dev) {
        return false;
    }
    return (dev->state & LIVECDS_STATE_ATTACHED) != 0;
}

// ========================
// Примитивные проверки
// ========================

bool livecds_device_has_cap(const livecds_device_t *dev, u64 cap) {
    if (!dev) {
        return false;
    }
    return (dev->info.caps & cap) != 0;
}

livecds_status_t livecds_require_caps(const livecds_device_t *dev, u64 caps) {
    if (!dev) {
        return LIVECDS_ERR_INVALID;
    }
    if ((dev->info.caps & caps) != caps) {
        return LIVECDS_ERR_UNSUPPORTED;
    }
    return LIVECDS_OK;
}

// ========================
// Отладочные хелперы
// ========================

static u64 count_devices_by_type(livecds_device_type_t type) {
    u64 count = 0;
    for (u32 i = 0; i < device_count; i++) {
        if (devices[i].type == type) {
            count++;
        }
    }
    return count;
}

u64 livecds_count_block_devices(void) {
    return count_devices_by_type(LIVECDS_DEV_BLOCK);
}

u64 livecds_count_char_devices(void) {
    return count_devices_by_type(LIVECDS_DEV_CHAR);
}

u64 livecds_count_network_devices(void) {
    return count_devices_by_type(LIVECDS_DEV_NETWORK);
}

u64 livecds_count_display_devices(void) {
    return count_devices_by_type(LIVECDS_DEV_DISPLAY);
}

u64 livecds_count_input_devices(void) {
    return count_devices_by_type(LIVECDS_DEV_INPUT);
}
