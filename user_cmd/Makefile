obj-y := cmd.o subcmd.o test.o help.o inferior.o break.o exec.o variable.o callstack.o register.o memory.o evaluate.o per.lex.o per.tab.o pertypes.o per.o
bin-y := test-perparser


test-perparser-y := test_perparser.o per.tab.o per.lex.o pertypes.o common/ gui/

# flags
test-perparser-cppflags := -I"$(loc_src_tree)"
test-perparser-ldflags := -lpthread



file.o: $(loc_built_tree)/subcmd.hash.h
help.o: $(loc_built_tree)/cmd.hash.h

$(loc_built_tree)/test_perparser.o: $(loc_built_tree)/per.tab.h $(loc_built_tree)/per.lex.h
$(loc_built_tree)/per.lex.o: $(loc_built_tree)/per.tab.h
$(loc_built_tree)/per.tab.o: $(loc_built_tree)/per.lex.h
$(loc_built_tree)/per.o: $(loc_built_tree)/per.tab.h $(loc_built_tree)/per.lex.h
