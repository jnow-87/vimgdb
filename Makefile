################
###   init   ###
################

# init source and built tree
scripts_dir := scripts
default_built_tree := built/
src_dirs := main/ gdb/ gui/ user_cmd/ common/ testing/

# init build system variables
project_type := cxx
config := ./config
config_tree := $(scripts_dir)/config
mconfig := $(scripts_dir)/mconf/mconfig
mconfig_ftype := Pconfig

# include build system Makefile
include $(scripts_dir)/Makefile.inc

# init default flags
cflags := $(CFLAGS) $(CONFIG_CFLAGS) -Wall
cxxflags := $(CXXFLAGS) $(CONFIG_CXXFLAGS) -std=c++11 -Wall
cppflags := $(CPPFLAGS) $(CONFIG_CPPFLAGS) -I"include/" -I"$(built_tree)/"
ldflags := $(LDFLAGS) $(CONFIG_LDFLAGS)
ldrflags := $(LDRFLAGS) $(CONFIG_LDRFLAGS)
asflags := $(ASFLAGS) $(CONFIG_ASFLAGS)
archflags := $(ARCHFLAGS) $(CONFIG_ARCHFLAGS)

yaccflags := $(YACCFLAGS) $(CONFIG_YACCFLAGS)
lexflags := $(LEXFLAGS) $(CONFIG_LEXFLAGS)
gperfflags := $(GPERFFLAGS) $(CONFIG_GPERFFLAGS)

# log level
define loglevel
$(if $(CONFIG_LOG_$(1)), \
  $(eval cppflags += -DLOG_$(1)=$(1)), \
)
endef

cppflags += -DLOG_LEVEL="(log_level_t)(LOG_ERROR | LOG_DEBUG | LOG_GDB | LOG_VIM | LOG_USER | LOG_TEST | LOG_TODO)"
$(call loglevel,ERROR)
$(call loglevel,DEBUG)
$(call loglevel,GDB)
$(call loglevel,VIM)
$(call loglevel,USER)
$(call loglevel,TEST)
$(call loglevel,TODO)

###################
###   targets   ###
###################

####
## build
####
.PHONY: all
all: check_tools check_config $(lib) $(bin)

.PHONY: debug
debug: cflags += -g
debug: cxxflags += -g
debug: asflags += -g
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
	$(mkdir) -p ~/.vim/plugin ~/.vim/syntax ~/.vim/doc ~/.vim/autoload/vimgdb ~/bin
	$(cp) -au built/main/vimgdb ~/bin/
	$(cp) -au gui/vim/plugin/* ~/.vim/plugin
	$(cp) -au gui/vim/syntax/* ~/.vim/syntax
	$(cp) -au gui/vim/doc/* ~/.vim/doc
	$(cp) -au gui/vim/autoload/* ~/.vim/autoload/vimgdb/

.PHONY: install-system
install-system: all
	$(mkdir) -p /usr/share/vim/vim74/plugin /usr/share/vim/vim74/syntax /usr/share/vim/vim74/doc /usr/share/vim/vim74/autoload/vimgdb /usr/bin
	$(cp) -au built/main/vimgdb /usr/bin/
	$(cp) -au gui/vim/plugin/* /usr/share/vim/vim74/plugin
	$(cp) -au gui/vim/syntax/* /usr/share/vim/vim74/syntax
	$(cp) -au gui/vim/doc/* /usr/share/vim/vim74/doc
	$(cp) -au gui/vim/autoload/* /usr/share/vim/vim74/autoload/vimgdb/

.PHONY: uninstall
uninstall:
	$(rm) -f /usr/bin/vimgdb
	$(rm) -f /usr/share/vim/vim74/plugin/vimgdb.vim
	$(rm) -f /usr/share/vim/vim74/syntax/vimgdb*.vim
	$(rm) -rf /usr/share/vim/vim74/autoload/vimgdb
	$(rm) -f ~/bin/vimgdb
	$(rm) -f ~/.vim/plugin/vimgdb.vim
	$(rm) -f ~/.vim/syntax/vimgdb*.vim
	$(rm) -rf ~/.vim/autoload/vimgdb

####
## help
####

.PHONY: help
help:
