################
###   init   ###
################

# init source and built tree
default_built_tree := built/
src_dirs := example

# init build system variables
project_type := cxx
config := ./config
config_tree := scripts/config
mconfig := scripts/mconf/mconfig
mconfig_ftype := Pconfig

# include build system Makefile
include scripts/Makefile.inc

# init default flags
cflags := $(CFLAGS) $(CONFIG_CFLAGS)
cxxflags := $(CXXFLAGS) $(CONFIG_CXXFLAGS)
cppflags := $(CPPFLAGS) $(CONFIG_CPPFLAGS)
ldflags := $(LDFLAGS) $(CONFIG_LDFLAGS)
ldrflags := $(LDRFLAGS) $(CONFIG_LDRFLAGS)
asflags := $(ASFLAGS) $(CONFIG_ASFLAGS)
archflags := $(ARCHFLAGS) $(CONFIG_ARCHFLAGS)

hostcflags := $(HOSTCFLAGS) $(CONFIG_HOSTCFLAGS)
hostcxxflags := $(HOSTCXXFLAGS) $(CONFIG_HOSTCCCFLAGS)
hostcppflags := $(HOSTCPPFLAGS) $(CONFIG_HOSTCPPFLAGS)
hostldflags := $(HOSTLDFLAGS) $(CONFIG_HOSTLDFLAGS)
hostldrflags := $(HOSTLDRFLAGS) $(CONFIG_HOSTLDRFLAGS)
hostasflags := $(HOSTASFLAGS) $(CONFIG_HOSTASFLAGS)
hostarchflags := $(HOSTARCHFLAGS) $(CONFIG_HOSTARCHFLAGS)

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
all: check_tools check_config $(lib) $(hostlib) $(bin) $(hostbin)

.PHONY: debug
debug: cflags += -g
debug: cxxflags += -g
debug: asflags += -g
debug: hostcflags += -g
debug: hostcxxflags += -g
debug: hostasflags += -g
debug: all

####
## cleanup
####
.PHONY: clean
clean:
	$(rm) $(built_tree)

.PHONY: distclean
distclean:
	$(rm) $(config) $(built_tree)

####
## install
####
.PHONY: install-user
install-user: all

.PHONY: install-system
install-system: all

.PHONY: uninstall
uninstall:

####
## help
####

.PHONY: help
help:
