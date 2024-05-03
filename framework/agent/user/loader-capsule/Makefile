THIS_MAKEFILE := $(lastword $(MAKEFILE_LIST))
SRC_DIR := $(realpath $(dir $(THIS_MAKEFILE)))

CLANG := clang
BPFSO := -lbpf
ELFSO := -lelf

LIBS = $(BPFSO) $(ELFSO)
LDFLAGS = -Wl,-rpath,/usr/lib64
CFLAGS = -I"$(SKEL_DIR)" -DBPF_KERNEL_SKELETON=$(SKEL)

DEBUG ?= 0
ifeq ($(DEBUG), 1)
CFLAGS += -DDEBUG
endif

TARGET = loader-capsule
SRCS = $(wildcard $(SRC_DIR)/*.c)
DEPS = $(wildcard $(SRC_DIR)/*.c $(SRC_DIR)/*.h Makefile) $(SKEL_DIR)/$(SKEL).skel.h

BUILD_DIR_NAME ?= ../build/$(SKEL)

.PHONY: all check-params clean

all: check-params $(TARGET)

check-params:
ifndef SKEL
	$(error SKEL is not set)
endif
ifndef SKEL_DIR
	$(error SKEL_DIR is not set)
endif
	@echo "SKEL set to" $(SKEL)
	@echo "SKEL_DIR set to" $(SKEL_DIR)

$(TARGET): $(DEPS) check-params
	@echo $(MAKEFILE_LIST)
	@mkdir -p "$(BUILD_DIR_NAME)"
	$(CLANG) $(LDFLAGS) $(SRCS) -o "$(BUILD_DIR_NAME)/$(TARGET)" $(CFLAGS) $(LIBS)

clean:
	-$(RM) -r "$(BUILD_DIR_NAME)"