bin-y := test-interface test-parser


## test-interface
# flags
test-interface-cppflags := -I"$(loc_src_tree)"

# object files
test-interface-y := test_interface.o gdb.o common/

# libraries
test-interface-ldflags := -lpthread
test-interface-cflags := -std=c++11


## test-parser
# flags
test-parser-cppflags := -I"$(loc_src_tree)"

# object files
test-parser-y := test_parser.o parser.tab.o lexer.lex.o result.o value.o common/

# additional dependencies for generated headers
$(loc_built_tree)/test_parser.o: $(loc_built_tree)/parser.tab.h $(loc_built_tree)/lexer.lex.h
$(loc_built_tree)/parser.tab.o: $(loc_built_tree)/lexer.lex.h
