DEVKITPRO  ?= /opt/devkitpro
DEVKITARM  ?= $(DEVKITPRO)/devkitARM

ifeq ($(strip $(DEVKITARM)),)
$(error "DEVKITARM environment variable is not set. Please point it to your devkitARM directory")
endif

include $(DEVKITARM)/base_tools

TARGET_NAME ?= rat_test
SRC_DIR     ?= src
BUILD_DIR   ?= build

# --- TONC PATHS ---
LIBTONC     := $(DEVKITPRO)/libtonc

ifeq ($(OS),Windows_NT)
    RM    = del /q /f
    FIX_PATH = $(subst /,\,$1)
    MKDIR = if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
    
    RUN_CMD = start mgba-qt $(call FIX_PATH,$(TARGET))
else
    RM    = rm -rf
    MKDIR = mkdir -p $(BUILD_DIR)
    
    RUN_CMD = mgba-qt $(TARGET) &
endif

ARCH    := -mthumb -mthumb-interwork
# 1. Added -I$(LIBTONC)/include so GCC can find <tonc.h>
CFLAGS  := -g -Wall -O2 $(ARCH) -mcpu=arm7tdmi -mtune=arm7tdmi -I$(SRC_DIR) -I$(LIBTONC)/include

# 2. Added -L$(LIBTONC)/lib -ltonc so the linker includes the library functions
LDFLAGS := -g $(ARCH) -specs=gba.specs -L$(LIBTONC)/lib -ltonc

TARGET  := $(BUILD_DIR)/$(TARGET_NAME).gba
ELF     := $(BUILD_DIR)/$(TARGET_NAME).elf

SRCS    := $(wildcard $(SRC_DIR)/*.c)
OBJS    := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	$(MKDIR)

$(TARGET): $(ELF)
	$(OBJCOPY) -O binary $< $@
	@echo "ROM Built: $@"

$(ELF): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
ifeq ($(OS),Windows_NT)
	-$(RM) $(call FIX_PATH,$(OBJS)) $(call FIX_PATH,$(ELF)) $(call FIX_PATH,$(TARGET))
	-rmdir $(BUILD_DIR) 2>nul || exit 0
else
	$(RM) $(BUILD_DIR)
endif

run: all
	$(RUN_CMD)

rebuild:
	$(MAKE) clean
	$(MAKE) run

.PHONY: all clean run rebuild
