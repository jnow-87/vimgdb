obj-y := cmd.o subcmd.o test.o help.o inferior.o break.o exec.o variable.o callstack.o register.o memory.o evaluate.o per.lex.o per.tab.o pertypes.o per.o ui.o
bin-y := test-perparser per2h


# test_perparser
test-perparser-y := test_perparser.o per.tab.o per.lex.o pertypes.o common/ gui/

test-perparser-cppflags := -I"$(loc_src_tree)"
test-perparser-ldlibs := -lpthread

# per2h
per2h-y := per2h.o per.tab.o per.lex.o pertypes.o common/ gui/
per2h-cppflags := -I"$(loc_src_tree)"
per2h-ldlibs := -lpthread

# additional dependencies
file.o: $(loc_build_tree)/subcmd.hash.h
help.o: $(loc_build_tree)/cmd.hash.h

$(loc_build_tree)/test_perparser.o: $(loc_build_tree)/per.tab.h $(loc_build_tree)/per.lex.h
$(loc_build_tree)/per.lex.o: $(loc_build_tree)/per.tab.h
$(loc_build_tree)/per.tab.o: $(loc_build_tree)/per.lex.h
$(loc_build_tree)/per.o: $(loc_build_tree)/per.tab.h $(loc_build_tree)/per.lex.h
