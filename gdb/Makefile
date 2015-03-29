obj-y := gdb.o parser.tab.o lexer.lex.o result.o value.o identifier.o convert.o breakpoint.o location.o frame.o
bin-y := test-parser

## test-parser
# object files
test-parser-y := test_parser.o gdb/ common/ gui/

# flags
test-parser-cppflags := -I"$(loc_src_tree)"
test-parser-ldflags := -lpthread -lcurses


## additional dependencies for generated headers
$(loc_built_tree)/test_parser.o: $(loc_built_tree)/parser.tab.h $(loc_built_tree)/lexer.lex.h
$(loc_built_tree)/parser.tab.o: $(loc_built_tree)/lexer.lex.h
$(loc_built_tree)/lexer.lex.o: $(loc_built_tree)/identifier.hash.h
$(loc_built_tree)/gdb.o: $(built_tree)/user_cmd/cmd.hash.h $(built_tree)/user_cmd/subcmd.hash.h $(loc_built_tree)/convert.hash.h $(built_tree)/gdb//parser.tab.h
