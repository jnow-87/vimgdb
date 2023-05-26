################
###   init   ###
################

# init build system variables
project_type := cxx
scripts_dir := scripts/build
config := .config
config_tree := scripts/config
use_config_sys := y
config_ftype := Pconfig
githooks_tree := .githooks
tool_deps := gdb

# include config
-include $(config)

# init source and build tree
default_build_tree := build/$(CONFIG_BUILD_TYPE)/
src_dirs := main/ gdb/ gui/ user_cmd/ common/ testing/

# include build system Makefile
include $(scripts_dir)/main.make

# init default flags
cflags := $(CFLAGS) $(CONFIG_CFLAGS) -Wall -O2
cxxflags := $(CXXFLAGS) $(CONFIG_CXXFLAGS) -std=c++11 -Wall -O2
cppflags := $(CPPFLAGS) $(CONFIG_CPPFLAGS) -I"include/" -I"$(build_tree)/"
ldflags := $(LDFLAGS) $(CONFIG_LDFLAGS)
ldrflags := $(LDRFLAGS) $(CONFIG_LDRFLAGS)
asflags := $(ASFLAGS) $(CONFIG_ASFLAGS)
archflags := $(ARCHFLAGS) $(CONFIG_ARCHFLAGS)

yaccflags := $(YACCFLAGS) $(CONFIG_YACCFLAGS)
lexflags := $(LEXFLAGS) $(CONFIG_LEXFLAGS)
gperfflags := $(GPERFFLAGS) $(CONFIG_GPERFFLAGS)


###################
###   targets   ###
###################

####
## build
####

.PHONY: all
ifeq ($(CONFIG_BUILD_DEBUG),y)
all: cflags += -g
all: cxxflags += -g
all: asflags += -g
endif

all: $(lib) $(bin)

####
## cleanup
####

.PHONY: clean
clean:
	$(rm) $(filter-out $(patsubst %/,%,$(dir $(build_tree)/$(scripts_dir))),$(wildcard $(build_tree)/*))

.PHONY: distclean
distclean:
	$(rm) $(config) $(config).old .clang $(build_tree)

####
## install
####

include $(scripts_dir)/install.make

VIM := ~/.vim

.PHONY: install
install: all
	$(call install,$(build_tree)/main/vimgdb)
	$(call install,$(build_tree)/user_cmd/per2h)
	$(call install,gui/vim/doc/vimgdb.txt,$(VIM)/doc)
	$(call install,gui/vim/plugin/vimgdb.vim,$(VIM)/plugin/)
	$(call install,gui/vim/autoload/vimgdb,$(VIM)/autoload/)
	$(call install,gui/vim/syntax/*,$(VIM)/syntax/)

.PHONY: uninstall
uninstall:
	$(call uninstall,$(PREFIX)/vimgdb)
	$(call uninstall,$(PREFIX)/per2h)
	$(call uninstall,$(VIM)/doc/vimgdb.txt)
	$(call uninstall,$(VIM)/plugin/vimgdb.vim)
	$(call uninstall,$(VIM)/syntax/vimgdb_*.vim)
	$(call uninstall,$(VIM)/syntax/per.vim)
	$(call uninstall,$(VIM)/autoload/vimgdb)
