# init source and binary tree
BUILD_TREE ?= build
SRC_TREE ?= .
build_tree := $(BUILD_TREE)
src_tree := $(SRC_TREE)


# init default flags
cflags := $(CFLAGS) dc
cxxflags := $(CXXFLAGS) dcxx
cppflags := $(CPPFLAGS) dcpp
ldflags := $(LDFLAGS) dld
asflags := $(ASFLAGS) das
archflags := $(ARCHFLAGS) dharch
hostcflags := $(HOSTCFLAGS) dhc
hostcxxflags := $(HOSTCXXFLAGS) dhcxx
hostcppflags := $(HOSTCPPFLAGS) dhcpp
hostldflags := $(HOSTLDFLAGS) dhld
hostasflags := $(HOSTASFLAGS) dhas
hostarchflags := $(HOSTARCHFLAGS) dharch
yaccflags := $(YACCFLAGS) dyacc
lexflags := $(LEXFLAGS) dlex


# init global variables for list of objects, libraries and executables
obj :=
lib :=
bin :=


# define subdirectories to visit
subdir-y := example

# fake target as default
.PHONY: fake_all
fake_all: all


# general include
-include config
include scripts/Makefile.inc


# start subdirectory traversal
$(call dinclude,$(subdir-y))

# include dependency files
include $(shell find $(build_tree)/ -name \*.d 2>/dev/null)


# main targets
.PHONY: all
all: $(obj) $(lib) $(bin)


.PHONY: clean
clean:
	$(rm) $(build_tree)

.PHONY: help
help:
	$(printf) "   \033[1m\033[4msub-directory Makefile syntax - available identifiers\033[0m\n"
	$(printf) "\n"
	$(printf) "      \033[1mbuild targets\033[0m\n"
	$(printf) "         %20s\t%s\n" "[host]obj-y" "define target object files"
	$(printf) "         %20s\t%s\n" "[host]lib-y" "define target libraries"
	$(printf) "         %20s\t%s\n" "[host]bin-y" "define target binaries"
	$(printf) "         %20s\t%s\n" "<target>-y" "specify the objects required to build the compound object <target>"
	$(printf) "\n"
	$(printf) "             - prefixed \"host\" indicates to use host compiler tools\n"
	$(printf) "\n"
	$(printf) "      \033[1mflags\033[0m\n"
	$(printf) "         %20s\t%s\n" "[host]<flags>-y" "specify <flags> to be used for all targets within the current directory"
	$(printf) "         %20s\t%s\n" "subdir-<flags>" "specify <flags> apply to all sub-directories"
	$(printf) "         %20s\t%s\n" "<target>-[host]<flags>" "specify <flags> only applied for that <target>"
	$(printf) "\n"
	$(printf) "         %20s\t%s\n" "cflags" "c compiler flags"
	$(printf) "         %20s\t%s\n" "cxxflags" "c++ compiler flags"
	$(printf) "         %20s\t%s\n" "cppflags" "c pre-processor flags"
	$(printf) "         %20s\t%s\n" "asflags" "assembler flags"
	$(printf) "         %20s\t%s\n" "ldflags" "linker flags"
	$(printf) "         %20s\t%s\n" "archflags" "architecture specific flags"
	$(printf) "         %20s\t%s\n" "yaccflags" "yacc flags"
	$(printf) "         %20s\t%s\n" "lexflags" "lex flags"
	$(printf) "\n"
	$(printf) "      \033[1msub-directories\033[0m\n"
	$(printf) "         %20s\t%s\n" "subdir-y" "define sub-directories to include"
	$(printf) "\n\n"
	$(printf) "   \033[1m\033[4muser defineable variables\033[0m\n"
	$(printf) "\n"
	$(printf) "         %20s\t%s\n" "QUIET" "disable echo of commands, except building commands"
	$(printf) "         %20s\t%s\n" "SILENT" "disable echo of all commands"
	$(printf) "         %20s\t%s\n" "DEBUG" "control debug output"
	$(printf) "         %20s\t%s\n" "BUILD_TREE" "define output directory"
	$(printf) "         %20s\t%s\n" "SRC_TREE" "define source directory"
	$(printf) "\n"
	$(printf) "         %20s\t%s\n" "[HOST]CC"
	$(printf) "         %20s\t%s\n" "[HOST]CXX"
	$(printf) "         %20s\t%s\n" "[HOST]AS"
	$(printf) "         %20s\t%s\n" "[HOST]LD"
	$(printf) "         %20s\t%s\n" "[HOST]AR"
	$(printf) "         %20s\t%s\n" "LEX"
	$(printf) "         %20s\t%s\n" "YACC"
	$(printf) "\n"
	$(printf) "         %20s\t%s\n" "[HOST]CFLAGS"
	$(printf) "         %20s\t%s\n" "[HOST]CXXFLAGS"
	$(printf) "         %20s\t%s\n" "[HOST]CPPFLAGS"
	$(printf) "         %20s\t%s\n" "[HOST]LDFLAGS"
	$(printf) "         %20s\t%s\n" "[HOST]ASFLAGS"
	$(printf) "         %20s\t%s\n" "[HOST]ARCHFLAGS"
	$(printf) "         %20s\t%s\n" "YACCFLAGS"
	$(printf) "         %20s\t%s\n" "LEXFLAGS"
	$(printf) "\n\n"
	$(printf) "   \033[1m\033[4mspecial targets\033[0m\n"
	$(printf) "\n"
	$(printf) "         %20s\t%s\n" "$(BUILD_TREE)/<target>.i" "build pre-processed target file"
