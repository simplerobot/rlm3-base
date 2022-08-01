GITHUB_DEPS += simplerobot/build-scripts
GITHUB_DEPS += simplerobot/logger
GITHUB_DEPS += simplerobot/test
include ../build-scripts/build/release/include.make

CPU_CC = g++
CPU_CFLAGS = -Wall -Werror -pthread -DTEST -fsanitize=address -static-libasan -g -Og

MCU_TOOLCHAIN_PATH = /opt/gcc-arm-none-eabi-7-2018-q2-update/bin/arm-none-eabi-
MCU_CC = $(MCU_TOOLCHAIN_PATH)gcc
MCU_AS = $(MCU_TOOLCHAIN_PATH)gcc -x assembler-with-cpp
MCU_SZ = $(MCU_TOOLCHAIN_PATH)size
MCU_HX = $(MCU_TOOLCHAIN_PATH)objcopy -O ihex
MCU_BN = $(MCU_TOOLCHAIN_PATH)objcopy -O binary -S
MCU_CFLAGS = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -fdata-sections -ffunction-sections -Wall -Werror -DUSE_HAL_DRIVER -DSTM32F427xx -DTEST -DUSE_FULL_ASSERT=1 -fexceptions -g -Og -gdwarf-2
MCU_CLIBS = -lc -lm -lstdc++

SOURCE_DIR = source
MAIN_SOURCE_DIR = $(SOURCE_DIR)/main
CPU_TEST_SOURCE_DIR = $(SOURCE_DIR)/test-cpu
MCU_TEST_SOURCE_DIR = $(SOURCE_DIR)/test-mcu

BUILD_DIR = build
LIBRARY_BUILD_DIR = $(BUILD_DIR)/library
CPU_TEST_BUILD_DIR = $(BUILD_DIR)/test-cpu
MCU_TEST_BUILD_DIR = $(BUILD_DIR)/test-mcu
RELEASE_DIR = $(BUILD_DIR)/release

CPU_INCLUDES = \
	-I$(MAIN_SOURCE_DIR) \
	-I$(PKG_LOGGER_DIR) \
	-I$(PKG_TEST_DIR) \
	-I$(PKG_MOCK_DIR) \
	-I$(PKG_RLM3_MOCK_BASE_DIR) \

MCU_INCLUDES = \
	-I$(MAIN_SOURCE_DIR) \
	-I$(PKG_LOGGER_DIR) \
	-I$(PKG_TEST_STM32_DIR) \
	-I$(PKG_RLM3_HARDWARE_DIR) \
	-I$(PKG_RLM3_DRIVER_BASE_DIR) \

LIBRARY_FILES = $(notdir $(wildcard $(MAIN_SOURCE_DIR)/*))

CPU_TEST_SOURCE_DIRS = $(MAIN_SOURCE_DIR) $(CPU_TEST_SOURCE_DIR) $(PKG_LOGGER_DIR) $(PKG_TEST_DIR)
CPU_TEST_SOURCE_FILES = $(notdir $(wildcard $(CPU_TEST_SOURCE_DIRS:%=%/*.c) $(CPU_TEST_SOURCE_DIRS:%=%/*.cpp)))
CPU_TEST_O_FILES = $(addsuffix .o,$(basename $(CPU_TEST_SOURCE_FILES)))

VPATH = $(CPU_TEST_SOURCE_DIRS)

.PHONY: default all library test-cpu test-mcu release clean

default : all

all : release

library : $(LIBRARY_FILES:%=$(LIBRARY_BUILD_DIR)/%)

$(LIBRARY_BUILD_DIR)/% : $(MAIN_SOURCE_DIR)/% | $(LIBRARY_BUILD_DIR)
	cp $< $@

$(LIBRARY_BUILD_DIR) :
	mkdir -p $@

test-cpu : library $(CPU_TEST_BUILD_DIR)/a.out
	$(CPU_TEST_BUILD_DIR)/a.out

$(CPU_TEST_BUILD_DIR)/a.out : $(CPU_TEST_O_FILES:%=$(CPU_TEST_BUILD_DIR)/%)
	$(CPU_CC) $(CPU_CFLAGS) $^ -o $@

$(CPU_TEST_BUILD_DIR)/%.o : %.cpp Makefile | $(CPU_TEST_BUILD_DIR)
	$(CPU_CC) -c $(CPU_CFLAGS) $(CPU_INCLUDES) -MMD $< -o $@
	
$(CPU_TEST_BUILD_DIR)/%.o : %.c Makefile | $(CPU_TEST_BUILD_DIR)
	$(CPU_CC) -c $(CPU_CFLAGS) $(CPU_INCLUDES) -MMD $< -o $@

$(CPU_TEST_BUILD_DIR) :
	mkdir -p $@

release : test-cpu $(LIBRARY_FILES:%=$(RELEASE_DIR)/%)

$(RELEASE_DIR)/% : $(LIBRARY_BUILD_DIR)/% | $(RELEASE_DIR)
	cp $< $@
	
$(RELEASE_DIR) :
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

-include $(wildcard $(CPU_TEST_BUILD_DIR)/*.d)


