ifeq ($(OS),Windows_NT)
    DEVKITPRO ?= C:/devkitPro
else
    DEVKITPRO ?= /opt/devkitpro
endif

DEVKITARM ?= $(DEVKITPRO)/devkitARM

ifeq ($(wildcard $(DEVKITARM)/base_tools),)
$(error Could not find $(DEVKITARM)/base_tools. Set DEVKITPRO/DEVKITARM correctly)
endif

include $(DEVKITARM)/base_tools

TARGET_NAME ?= rat_test
SRC_DIR     ?= src
BUILD_DIR   ?= build

LIBTONC     := $(DEVKITPRO)/libtonc
GBAFIX      := $(DEVKITPRO)/tools/bin/gbafix

ifneq ($(findstring msys,$(MAKE_HOST))$(findstring cygwin,$(MAKE_HOST)),)
    CMD_SHELL := 0
else ifeq ($(OS),Windows_NT)
    CMD_SHELL := 1
else
    CMD_SHELL := 0
endif

ifeq ($(CMD_SHELL),1)
    SHELL       := cmd.exe
    .SHELLFLAGS := /C

    BSLASH   := \$(strip)
    FIX_PATH  = $(subst /,$(BSLASH),$(1))

    MKDIR_CMD = if not exist "$(call FIX_PATH,$(BUILD_DIR))" mkdir "$(call FIX_PATH,$(BUILD_DIR))"
    CLEAN_CMD = if exist "$(call FIX_PATH,$(BUILD_DIR))" rmdir /s /q "$(call FIX_PATH,$(BUILD_DIR))"

    EMULATOR ?= mGBA.exe
    RUN_CMD   = start "" "$(EMULATOR)" "$(call FIX_PATH,$(TARGET))"
else
    FIX_PATH  = $(1)

    MKDIR_CMD = mkdir -p $(BUILD_DIR)
    CLEAN_CMD = rm -rf $(BUILD_DIR)

    EMULATOR ?= mgba-qt
    RUN_CMD   = $(EMULATOR) $(TARGET) &
endif

ARCH    := -mthumb -mthumb-interwork
CFLAGS  := -g -Wall -O2 $(ARCH) -mcpu=arm7tdmi -mtune=arm7tdmi \
           -fomit-frame-pointer -ffast-math \
           -I$(SRC_DIR) -I$(LIBTONC)/include
LDFLAGS := -g $(ARCH) -specs=gba.specs
LIBS    := -L$(LIBTONC)/lib -ltonc

TARGET  := $(BUILD_DIR)/$(TARGET_NAME).gba
ELF     := $(BUILD_DIR)/$(TARGET_NAME).elf

SRCS    := $(wildcard $(SRC_DIR)/*.c)
OBJS    := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPS    := $(OBJS:.o=.d)

all: $(TARGET)

$(BUILD_DIR):
	@$(MKDIR_CMD)

$(TARGET): $(ELF)
	$(OBJCOPY) -O binary $< $@
	@$(call FIX_PATH,$(GBAFIX)) $@ -t$(TARGET_NAME)
	@echo ROM built: $@

$(ELF): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(LIBS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	-@$(CLEAN_CMD)

run: all
	$(RUN_CMD)

rebuild:
	$(MAKE) clean
	$(MAKE) run

-include $(DEPS)

.PHONY: all clean run rebuild