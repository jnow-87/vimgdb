# general include
-include config
include scripts/Makefile.inc

# init source and binary tree
BUILT_TREE ?= built
SRC_TREE ?= .
built_tree := $(BUILT_TREE)
src_tree := $(SRC_TREE)

# init default flags
cflags := $(CFLAGS)
cxxflags := $(CXXFLAGS)
cppflags := $(CPPFLAGS)
ldflags := $(LDFLAGS)
ldrflags := $(LDRFLAGS)
asflags := $(ASFLAGS)
archflags := $(ARCHFLAGS)
hostcflags := $(HOSTCFLAGS)
hostcxxflags := $(HOSTCXXFLAGS)
hostcppflags := $(HOSTCPPFLAGS)
hostldflags := $(HOSTLDFLAGS)
hostasflags := $(HOSTASFLAGS)
hostarchflags := $(HOSTARCHFLAGS)
yaccflags := $(YACCFLAGS)
lexflags := $(LEXFLAGS)
gperfflags := $(GPERFFLAGS)

# init global variables for list of objects, libraries and executables
obj :=
lib :=
bin :=

subdir-y :=

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
all: $(lib) $(bin)

.PHONY: clean
clean:
	$(rm) $(built_tree)

.PHONY: debug
debug: cflags += -g
debug: cxxflags += -g
debug: asflags += -g
debug: hostcflags += -g
debug: hostcxxflags += -g
debug: hostasflags += -g
debug: all

.PHONY: help
help:
	$(printf) "   \033[1m\033[4msub-directory Makefile syntax - available identifiers\033[0m\n"
	$(printf) "\n"
	$(printf) "      \033[1mbuild targets\033[0m\n"
	$(printf) "         %25s\t%s\n" "[host]obj-y" "define target object files, also combined into ./obj.o"
	$(printf) "         %25s\t%s\n" "[host]obj-nobuiltin-y" "define target object files, that are not added to ./obj.o"
	$(printf) "         %25s\t%s\n" "[host]lib-y" "define target libraries"
	$(printf) "         %25s\t%s\n" "[host]bin-y" "define target binaries"
	$(printf) "         %25s\t%s\n" "<target>-y" "specify the objects required to build the compound object <target>"
	$(printf) "\n"
	$(printf) "             - prefixed \"host\" indicates to use host compiler tools\n"
	$(printf) "             - obj-y can take a directory as \"obj-y := <dir>/\"\n"
	$(printf) "                  this builds <dir>/obj.o and adds <dir>/ to subdir-y\n"
	$(printf) "\n"
	$(printf) "      \033[1msub-directories\033[0m\n"
	$(printf) "         %25s\t%s\n" "subdir-y" "define sub-directories to include"
	$(printf) "\n"
	$(printf) "      \033[1mflags\033[0m\n"
	$(printf) "         %25s\t%s\n" "[host]<flags>-y" "specify <flags> to be used for all targets within the current directory"
	$(printf) "         %25s\t%s\n" "subdir-<flags>" "specify <flags> apply to all sub-directories"
	$(printf) "         %25s\t%s\n" "<target>-[host]<flags>" "specify <flags> only applied for that <target>"
	$(printf) "\n"
	$(printf) "         %25s\t%s\n" "cflags" "c compiler flags"
	$(printf) "         %25s\t%s\n" "cxxflags" "c++ compiler flags"
	$(printf) "         %25s\t%s\n" "cppflags" "c pre-processor flags"
	$(printf) "         %25s\t%s\n" "asflags" "assembler flags"
	$(printf) "         %25s\t%s\n" "ldflags" "linker flags"
	$(printf) "         %25s\t%s\n" "ldrflags" "linker flags for creating relocatable output (ld -r)"
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
	$(printf) "         %25s\t%s\n" "[HOST]CC"
	$(printf) "         %25s\t%s\n" "[HOST]CXX"
	$(printf) "         %25s\t%s\n" "[HOST]AS"
	$(printf) "         %25s\t%s\n" "[HOST]LD"
	$(printf) "         %25s\t%s\n" "[HOST]AR"
	$(printf) "         %25s\t%s\n" "LEX"
	$(printf) "         %25s\t%s\n" "YACC"
	$(printf) "         %25s\t%s\n" "GPERF"
	$(printf) "\n"
	$(printf) "         %25s\t%s\n" "[HOST]CFLAGS"
	$(printf) "         %25s\t%s\n" "[HOST]CXXFLAGS"
	$(printf) "         %25s\t%s\n" "[HOST]CPPFLAGS"
	$(printf) "         %25s\t%s\n" "[HOST]LDFLAGS"
	$(printf) "         %25s\t%s\n" "[HOST]ASFLAGS"
	$(printf) "         %25s\t%s\n" "[HOST]ARCHFLAGS"
	$(printf) "         %25s\t%s\n" "LEXFLAGS"
	$(printf) "         %25s\t%s\n" "YACCFLAGS"
	$(printf) "         %25s\t%s\n" "GPERFFLAGS"
	$(printf) "\n\n"
	$(printf) "   \033[1m\033[4mspecial targets\033[0m\n"
	$(printf) "\n"
	$(printf) "         %25s\t%s\n" "$(BUILT_TREE)/<target>.i" "build pre-processed target file"
