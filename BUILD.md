# Сборка и запуск

Документ описывает базовую сборку ядра и запуск в эмуляторе.

## Требования
- `gcc` / `clang`
- `make`
- `cmake`
- `ninja` (опционально)
- `qemu-system-x86_64` (для запуска)

## Быстрый старт
```bash
mkdir -p build
cmake -S . -B build
cmake --build build
```

Если используется `Makefile` напрямую:
```bash
make
```

## Запуск в QEMU
```bash
qemu-system-x86_64 -kernel build/MyOS.elf
```

> Примечание: путь к бинарнику обновляй под актуальную структуру сборки.

## Очистка
```bash
rm -rf build
```

## Docker
Если требуется использовать Docker:
```bash
docker build -t myos .
```

