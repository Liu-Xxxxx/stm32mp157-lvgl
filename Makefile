#
# Makefile
#
CC = arm-buildroot-linux-gnueabihf-gcc 
LVGL_DIR_NAME ?= lvgl
LVGL_DIR ?= ${shell pwd}

CFLAGS = -O3 -g0 -I$(LVGL_DIR)/

BIN = demo


CFLAGS = -I$(LVGL_DIR)/ $(DEFINES) $(WARNINGS) $(OPTIMIZATION) -I$(LVGL_DIR)  -I.

LDFLAGS = -lm  -lpthread  -lSDL2  -lpng -linput 

#Collect the files to compile
MAINSRC = ./main.c

include ./lvgl/lvgl.mk
include ./lv_drivers/lv_drivers.mk
include ./lv_demos/lv_demo.mk

OBJEXT ?= .o

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))

MAINOBJ = $(MAINSRC:.c=$(OBJEXT))

SRCS = $(ASRCS) $(CSRCS) $(MAINSRC)
OBJS = $(AOBJS) $(COBJS)

## MAINOBJ -> OBJFILES

all: default

%.o: %.c
	@$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"
    
default: $(AOBJS) $(COBJS) $(MAINOBJ)
	$(CC) -o $(BIN) $(MAINOBJ) $(AOBJS) $(COBJS) $(LDFLAGS)
	cp $(BIN) ~/nfs_rootfs

clean: 
	rm -f $(BIN) $(AOBJS) $(COBJS) $(MAINOBJ)

