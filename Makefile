# ========================
# Environment
# ========================
ifneq (,$(wildcard .env))
	include .env
	export
endif

BUILD_DIR ?= build
CONFIG_FILE ?= config/.config
DOCKER_IMAGE ?= myos-build
USE_DOCKER ?= false

# ========================
# Targets
# ========================
.PHONY: all menuconfig build docker shell clean

all: build

menuconfig:
	@echo "🛠  Configuring MyOS"
	@./tools/menuconfig config/Kconfig config/defconfig $(CONFIG_FILE)

build: $(CONFIG_FILE)
ifeq ($(USE_DOCKER),true)
	$(MAKE) docker
else
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake ..
	cd $(BUILD_DIR) && make
endif

docker:
	docker build -t $(DOCKER_IMAGE) .
	docker run --rm -it \
		-v $(PWD):/myos \
		$(DOCKER_IMAGE) \
		make build

shell:
	docker run --rm -it \
		-v $(PWD):/myos \
		$(DOCKER_IMAGE)

clean:
	rm -rf $(BUILD_DIR)
