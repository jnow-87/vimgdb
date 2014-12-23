bin-y := test_gdbparser gdbctrl


# common flags
cppflags-y := -I"$(loc_src_tree)"

#
# gdbctrl
#

# object files
gdbctrl-y := main.o gdb.o log.o pty.o tty.o

# libraries
gdbctrl-ldflags := -lpthread
gdbctrl-cflags := -std=c++11


#
# test_gdbparser
#

# object files
test_gdbparser-y := gdbparser.tab.o gdbparser.lex.o gdb_result.o gdb_value.o xmalloc.o
test_gdbparser-cflags := -std=gnu11

# additional dependencies for generated headers
$(loc_built_tree)/gdbparser.tab.o: $(loc_built_tree)/gdbparser.lex.h
$(loc_built_tree)/gdbparser.lex.h: $(loc_built_tree)/gdbparser.lex.c

# flags
test_gdbparser-cppflags := -I"$(loc_built_tree)" \
						   -DTEST
