<<<<<<< HEAD
=======
# SPDX-License-Identifier: GPL-2.0

>>>>>>> a33b14d... build(Makefile): add pre configured for ARM platform
PWD   := $(shell pwd)
PHONY :=

# ------------------------------------------------------------------------------

KRELEASE ?= $(shell uname -r)
KDIR     ?= /lib/modules/$(KRELEASE)/build

# ------------------------------------------------------------------------------

BUILDDIR := ./build
SRCDIR   := ./src
INCDIR   := ./include

SRC := $(shell find $(SRCDIR) -name "*.c")
INC := $(shell find $(INCDIR) -name "*.h")

# ------------------------------------------------------------------------------

CROSS_COMPILE := arm-linux-gnueabihf-
EXTRA_CFLAGS  :=

# ------------------------------------------------------------------------------

CP     := cp -rf
FORMAT := clang-format -i -style=file
MKDIR  := mkdir -p
MV     := mv -f
RM     := rm -rf

# ------------------------------------------------------------------------------

help: ## Display this help and exit
	@grep -E "^[a-zA-Z_-]+:.*?## .*$$" $(MAKEFILE_LIST) \
		| awk 'BEGIN {FS = ":.*?## "}; { \
			printf "\033[36m%-30s\033[0m %s\n", $$1, $$2 \
		}'

PHONY += help

# ------------------------------------------------------------------------------

modules: ## Build this kernel module
	@$(MKDIR) $(BUILDDIR)
	@echo "  DIR  $(BUILDDIR)"

	@$(MAKE) \
		-C $(KDIR) \
		ARCH=arm \
		CROSS_COMPILE=$(CROSS_COMPILE) \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
		M=$(PWD) \
		modules

	@$(MV) *.symvers *.order *.ko *.o .*.cmd *.mod.c *.mod $(BUILDDIR)
	@$(MV) $(SRCDIR)/*.o $(SRCDIR)/.*.cmd $(BUILDDIR)
	@echo "  MOVE  *.symvers *.order *.ko *.o .*.cmd *.mod.c *.mod"
	@echo "  MOVE  $(SRCDIR)/*.o $(SRCDIR)/.*.cmd"

modules_release: EXTRA_CFLAGS += -std=gnu89 -Wall -Winline ## Build this kernel module with release flags
modules_release: EXTRA_CFLAGS += -O3
modules_release: modules

modules_debug: EXTRA_CFLAGS += -std=gnu89 -Wall -Winline ## Build this kernel module with debug flags
modules_debug: EXTRA_CFLAGS += -g -DDEBUG
modules_debug: modules

modules_install: ## Install this kernel module
	@$(MAKE) -C $(KDIR) M=$(PWD) modules_install

format:  ## Format sources with clang-format
ifneq ($(SRC),)
	@$(FORMAT) $(SRC)
	@$(foreach file,$(SRC),echo "  FORMAT  $(file)";)
else
	@echo "  SKIP  $(SRCDIR)"
endif

ifneq ($(INC),)
	@$(FORMAT) $(INC)
	@$(foreach file,$(INC),echo "  FORMAT  $(file)";)
else
	@echo "  SKIP  $(INCDIR)"
endif

clean: ## Remove build artifacts
	@$(RM) $(BUILDDIR)
	@echo "  CLEAN  $(BUILDDIR)"
	@$(MAKE) -C $(KDIR) M=$(PWD) clean

PHONY += modules format clean

# ------------------------------------------------------------------------------

.PHONY: $(PHONY)
.DEFAULT_GOAL := help
