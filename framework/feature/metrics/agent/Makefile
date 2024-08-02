CLANG := clang
BPFSO := -lbpf
ELFSO := -lelf
CJSON := -lcjson

LIBS = $(BPFSO) $(ELFSO) $(CJSON)
LDFLAGS = -Wl,-rpath,/usr/lib64 -Wl,-rpath,/usr/local/lib

TARGET = agent
SRCS = $(wildcard ./*.c)
DEPS = $(wildcard ./*.c ./*.h Makefile)

BUILD_DIR_NAME ?= ./build

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(DEPS)
	@mkdir -p "$(BUILD_DIR_NAME)"
	@$(CLANG) $(LDFLAGS) $(SRCS) -o "$(BUILD_DIR_NAME)/$(TARGET)" $(CFLAGS) $(LIBS)

clean:
	-$(RM) -r "$(BUILD_DIR_NAME)"