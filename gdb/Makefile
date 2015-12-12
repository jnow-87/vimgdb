obj-y := gdb.o parser.tab.o lexer.lex.o strlist.o result.o event.o breakpoint.o location.o frame.o variable.o memory.o
bin-y := test-parser

## test-parser
# object files
test-parser-y := test_parser.o gdb/ common/ gui/

# flags
test-parser-cppflags := -I"$(loc_src_tree)"
test-parser-ldflags := -lpthread -lcurses


## additional dependencies for generated headers
$(loc_build_tree)/test_parser.o: $(loc_build_tree)/parser.tab.h $(loc_build_tree)/lexer.lex.h
$(loc_build_tree)/parser.tab.o: $(loc_build_tree)/lexer.lex.h
$(loc_build_tree)/gdb.o: $(build_tree)/user_cmd/cmd.hash.h $(build_tree)/user_cmd/subcmd.hash.h $(build_tree)/gdb/parser.tab.h $(build_tree)/gdb/lexer.lex.h
