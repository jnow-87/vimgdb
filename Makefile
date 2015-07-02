YACC := bison
LEX := flex
CC := gcc
CXX := g++

config := ./config

# general include
-include $(config)
include scripts/Makefile.inc

# init source and binary tree
BUILT_TREE ?= built
SRC_TREE ?= .
built_tree := $(BUILT_TREE)
src_tree := $(SRC_TREE)

# init default flags
cflags := $(CFLAGS) -Wall
cxxflags := $(CXXFLAGS) -std=c++11 -Wall
cppflags := $(CPPFLAGS) -g -I"include/" -I"$(built_tree)/"
ldflags := $(LDFLAGS)
asflags := $(ASFLAGS) -g
archflags := $(ARCHFLAGS) -g
yaccflags := $(YACCFLAGS)
lexflags := $(LEXFLAGS)
gperfflags := $(GPERFFLAGS)

# log level
cppflags += -DLOG_FILE=\"$(CONFIG_LOG_FILE)\"
cppflags += -DLOG_LEVEL="(log_level_t)(LOG_ERROR | LOG_DEBUG | LOG_GDB | LOG_VIM | LOG_USER | LOG_TEST | LOG_TODO)"
$(call loglevel,ERROR)
$(call loglevel,DEBUG)
$(call loglevel,GDB)
$(call loglevel,VIM)
$(call loglevel,USER)
$(call loglevel,TEST)
$(call loglevel,TODO)

# gui
$(if $(CONFIG_GUI_CURSES), $(eval cppflags += -DGUI_CURSES),)
$(if $(CONFIG_GUI_VIM), $(eval cppflags += -DGUI_VIM),)
$(if $(CONFIG_VIM_KEEP_USERLOG), $(eval cppflags += -DVIM_KEEP_USERLOG),)
$(if $(CONFIG_VIM_KEEP_GDBLOG), $(eval cppflags += -DVIM_KEEP_GDBLOG),)

# init global variables for list of objects, libraries and executables
obj :=
lib :=
bin :=

subdir-y := scripts/mconf/ main/ gdb/ gui/ user_cmd/ common/ testing/

# fake target as default
.PHONY: fake_all
fake_all: all

# start subdirectory traversal
# 	if subdir-y is empty include '.' (within $(src_tree))
$(if $(subdir-y), \
	$(call dinclude,$(subdir-y)), \
	$(eval loc_dir=) \
	$(eval include $(build)) \
)

# include dependency files
include $(shell find $(built_tree)/ -name \*.d 2>/dev/null)

# main targets
.PHONY: all
all: check_config $(lib) $(bin)

.PHONY: install-user
install-user: all
	mkdir -p ~/.vim/plugin ~/.vim/syntax ~/.vim/doc ~/.vim/autoload/vimgdb ~/bin
	cp -au built/main/vimgdb ~/bin/
	cp -au gui/vim/plugin/* ~/.vim/plugin
	cp -au gui/vim/syntax/* ~/.vim/syntax
	cp -au gui/vim/doc/* ~/.vim/doc
	cp -au gui/vim/autoload/* ~/.vim/autoload/vimgdb/

.PHONY: install-system
install-system: all
	mkdir -p /usr/share/vim/vim74/plugin /usr/share/vim/vim74/syntax /usr/share/vim/vim74/doc /usr/share/vim/vim74/autoload/vimgdb /usr/bin
	cp -au built/main/vimgdb /usr/bin/
	cp -au gui/vim/plugin/* /usr/share/vim/vim74/plugin
	cp -au gui/vim/syntax/* /usr/share/vim/vim74/syntax
	cp -au gui/vim/doc/* /usr/share/vim/vim74/doc
	cp -au gui/vim/autoload/* /usr/share/vim/vim74/autoload/vimgdb/

.PHONY: uninstall
uninstall:
	rm -f /usr/bin/vimgdb
	rm -f /usr/share/vim/vim74/plugin/vimgdb.vim
	rm -f /usr/share/vim/vim74/syntax/vimgdb*.vim
	rm -rf /usr/share/vim/vim74/autoload/vimgdb
	rm -f ~/bin/vimgdb
	rm -f ~/.vim/plugin/vimgdb.vim
	rm -f ~/.vim/syntax/vimgdb*.vim
	rm -rf ~/.vim/autoload/vimgdb

.PHONY: clean
clean:
	$(rm) $(built_tree)

.PHONY: menuconfig
menuconfig: $(built_tree)/scripts/mconf/mconfig
	$< Pconfig
	$(MAKE) clean

.PHONY: check_config
check_config:
ifneq ($(shell test -e $(config) && echo $(config)),$(config))
	$(call error,$(config) does not exist, please run $$make menuconfig or $$make defconfig_<target> first)
endif

# default config targets
cfg_tree = scripts/config
cfg = $(notdir $(wildcard $(cfg_tree)/*))
$(foreach c, $(cfg), \
	$(call gen_rule_basic,cmd_defconfig,defconfig_$(c),$(cfg_tree)/$(c)) \
)

.PHONY: help
help:
	$(printf) "   \033[1m\033[4msub-directory Makefile syntax - available identifiers\033[0m\n"
	$(printf) "\n"
	$(printf) "      \033[1mbuild targets\033[0m\n"
	$(printf) "         %25s\t%s\n" "obj-y" "define target object files, also combined into ./obj.o"
	$(printf) "         %25s\t%s\n" "obj-nobuiltin-y" "define target object files, that are not added to ./obj.o"
	$(printf) "         %25s\t%s\n" "lib-y" "define target libraries"
	$(printf) "         %25s\t%s\n" "bin-y" "define target binaries"
	$(printf) "         %25s\t%s\n" "<target>-y" "specify the objects required to build the compound object <target>"
	$(printf) "\n"
	$(printf) "             - obj-y can take a directory as \"obj-y := <dir>/\"\n"
	$(printf) "                  this builds <dir>/obj.o and adds <dir>/ to subdir-y\n"
	$(printf) "\n"
	$(printf) "      \033[1msub-directories\033[0m\n"
	$(printf) "         %25s\t%s\n" "subdir-y" "define sub-directories to include"
	$(printf) "\n"
	$(printf) "      \033[1mflags\033[0m\n"
	$(printf) "         %25s\t%s\n" "<flags>-y" "specify <flags> to be used for all targets within the current directory"
	$(printf) "         %25s\t%s\n" "subdir-<flags>" "specify <flags> apply to all sub-directories"
	$(printf) "         %25s\t%s\n" "<target>-<flags>" "specify <flags> only applied for that <target>"
	$(printf) "\n"
	$(printf) "         %25s\t%s\n" "cflags" "c compiler flags"
	$(printf) "         %25s\t%s\n" "cxxflags" "c++ compiler flags"
	$(printf) "         %25s\t%s\n" "cppflags" "c pre-processor flags"
	$(printf) "         %25s\t%s\n" "asflags" "assembler flags"
	$(printf) "         %25s\t%s\n" "ldflags" "linker flags"
	$(printf) "         %25s\t%s\n" "archflags" "architecture specific flags"
	$(printf) "         %25s\t%s\n" "yaccflags" "yacc flags"
	$(printf) "         %25s\t%s\n" "lexflags" "lex flags"
	$(printf) "         %25s\t%s\n" "gperfflags" "gperf flags"
	$(printf) "\n\n"
	$(printf) "   \033[1m\033[4muser defineable variables\033[0m\n"
	$(printf) "\n"
	$(printf) "         %25s\t%s\n" "QUIET" "disable echo of commands, except building commands"
	$(printf) "         %25s\t%s\n" "SILENT" "disable echo of all commands"
	$(printf) "         %25s\t%s\n" "DEBUG" "control debug output"
	$(printf) "         %25s\t%s\n" "BUILT_TREE" "define output directory"
	$(printf) "         %25s\t%s\n" "SRC_TREE" "define source directory"
	$(printf) "\n"
	$(printf) "         %25s\t%s\n" "CC"
	$(printf) "         %25s\t%s\n" "CXX"
	$(printf) "         %25s\t%s\n" "AS"
	$(printf) "         %25s\t%s\n" "LD"
	$(printf) "         %25s\t%s\n" "AR"
	$(printf) "         %25s\t%s\n" "LEX"
	$(printf) "         %25s\t%s\n" "YACC"
	$(printf) "         %25s\t%s\n" "GPERF"
	$(printf) "\n"
	$(printf) "         %25s\t%s\n" "CFLAGS"
	$(printf) "         %25s\t%s\n" "CXXFLAGS"
	$(printf) "         %25s\t%s\n" "CPPFLAGS"
	$(printf) "         %25s\t%s\n" "LDFLAGS"
	$(printf) "         %25s\t%s\n" "ASFLAGS"
	$(printf) "         %25s\t%s\n" "ARCHFLAGS"
	$(printf) "         %25s\t%s\n" "LEXFLAGS"
	$(printf) "         %25s\t%s\n" "YACCFLAGS"
	$(printf) "         %25s\t%s\n" "GPERFFLAGS"
	$(printf) "\n\n"
	$(printf) "   \033[1m\033[4mspecial targets\033[0m\n"
	$(printf) "\n"
	$(printf) "         %25s\t%s\n" "$(BUILT_TREE)/<target>.i" "build pre-processed target file"
