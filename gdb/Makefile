bin-y := test-interface test-parser


## test-interface
# flags
test-interface-cppflags := -I"$(loc_src_tree)"

# object files
test-interface-y := test_interface.o gdb.o parser.tab.o lexer.lex.o result.o value.o variable.o common/ gui/ cmd/

# libraries
test-interface-ldflags := -lpthread -lcurses

## test-parser
# flags
test-parser-cppflags := -I"$(loc_src_tree)"

# object files
test-parser-y := test_parser.o parser.tab.o lexer.lex.o result.o value.o variable.o gdb.o common/ gui/
test-parser-ldflags := -lcurses


## additional dependencies for generated headers
$(loc_built_tree)/test_parser.o: $(loc_built_tree)/parser.tab.h $(loc_built_tree)/lexer.lex.h
$(loc_built_tree)/parser.tab.o: $(loc_built_tree)/lexer.lex.h
$(loc_built_tree)/lexer.lex.o: $(loc_built_tree)/variable.hash.h
$(loc_built_tree)/gdb.o: $(loc_built_tree)/parser.tab.h $(loc_built_tree)/lexer.lex.h $(built_tree)/cmd/cmd.hash.h $(built_tree)/cmd/subcmd.hash.h

