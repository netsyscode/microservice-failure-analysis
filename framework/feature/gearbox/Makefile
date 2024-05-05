BPF_INFRA_PATH:=$(realpath ../../agent)
RELATIVE_BUILD_DIR_NAME:=../../../build/gearbox/

KERNEL_SRC := $(wildcard ./kernel/*.c ./kernel/*.h Makefile)
LOADER_SRC := $(wildcard ./kernel/*.c ./kernel/*.h Makefile)  # This is because loader is relied on kernel source
AGENT_SRC := $(wildcard ./agent/*.c ./agent/*.h Makefile)

.PHONY: all setup_dirs kernel loader agent clean

all: setup_dirs kernel loader agent

setup_dirs:
	@mkdir -p $(RELATIVE_BUILD_DIR_NAME)

kernel: $(KERNEL_SRC) setup_dirs
	@echo
	@echo ">>>>>>>> Building Gearbox eBPF kernel program at $(realpath $(RELATIVE_BUILD_DIR_NAME))/$@/"
	@echo

	@$(MAKE) -C ./kernel \
		BPF_INFRA_PATH=$(BPF_INFRA_PATH) \
		BUILD_DIR_NAME=$(realpath $(RELATIVE_BUILD_DIR_NAME))/$@/ 

	@echo
	@echo "<<<<<<<< Finish building Gearbox eBPF kernel program"
	@echo

loader: $(LOADER_SRC) setup_dirs
	@echo
	@echo ">>>>>>>> Building Gearbox eBPF loader program at $(realpath $(RELATIVE_BUILD_DIR_NAME))/"
	@echo

	@$(MAKE) -C ./loader \
		BPF_INFRA_PATH=$(BPF_INFRA_PATH) \
		BUILD_DIR_NAME=$(realpath $(RELATIVE_BUILD_DIR_NAME))/ \
		SKEL=gearbox \
		SKEL_DIR=$(realpath $(RELATIVE_BUILD_DIR_NAME))/kernel/

	@echo
	@echo "<<<<<<<< Finish building Gearbox eBPF loader program"
	@echo

agent: 
	@echo
	@echo ">>>>>>>> Building Gearbox eBPF agent program at $(realpath $(RELATIVE_BUILD_DIR_NAME))/"
	@echo

	@$(MAKE) -C ./agent \
		BUILD_DIR_NAME=$(realpath $(RELATIVE_BUILD_DIR_NAME))/

	@echo
	@echo "<<<<<<<< Finish building Gearbox eBPF agent program"
	@echo

clean:
	-$(RM) -rf $(RELATIVE_BUILD_DIR_NAME)