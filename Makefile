# Compiler settings
CC = arm-none-linux-gnueabihf-gcc
#CC = gcc
LVGL_DIR_NAME ?= lvgl
LVGL_DIR ?= .
OUTPUT_DIR = output

CFLAGS = -O3 -g0 -I$(LVGL_DIR)/ -I.

LDFLAGS = -lm -lpthread -lSDL2 -lpng -linput -lpaho-mqtt3a

# Collect the files to compile
MAINSRC = ./main.c

include ./lvgl/lvgl.mk
include ./lv_drivers/lv_drivers.mk
# include ./lv_demos/lv_demo.mk
include ./squareLine/squareLine.mk
include ./user/user.mk
include ./mqtt/mqtt.mk

OBJEXT ?= .o

# Update object file paths to include the output directory
AOBJS = $(patsubst %,$(OUTPUT_DIR)/%,$(ASRCS:.S=$(OBJEXT)))
COBJS = $(patsubst %,$(OUTPUT_DIR)/%,$(CSRCS:.c=$(OBJEXT)))
MAINOBJ = $(patsubst %,$(OUTPUT_DIR)/%,$(MAINSRC:.c=$(OBJEXT)))

OBJS = $(AOBJS) $(COBJS) $(MAINOBJ)

BIN = $(OUTPUT_DIR)/demo

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

# Rule to compile C source files
$(OUTPUT_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to clean the build
clean:
	rm -rf $(OUTPUT_DIR)

# Additional dependencies or rules can go here
