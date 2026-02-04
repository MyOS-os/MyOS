FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Базовые инструменты
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    nasm \
    gcc \
    g++ \
    xorriso \
    grub-pc-bin \
    grub-common \
    qemu-system-x86 \
    git \
    python3 \
    python3-pip \
    ncurses-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /myos

CMD ["/bin/bash"]
