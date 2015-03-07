obj-y := vimui.o
bin-y := test-nbprotocol


cppflags-y := -DVIM_NB_PORT=$(CONFIG_VIM_NB_PORT)

test-nbprotocol-y := test_nbprotocol.o cmd.o fct.o event.o parser.o lexer.o gui/ common/
test-nbprotocol-ldflags := -lcurses -lpthread

test-nbprotocol-cppflags := -I"$(loc_built_tree)/" -I"$(loc_src_tree)"

$(loc_built_tree)/test_nbprotocol.o: $(loc_built_tree)/cmd.hash.h $(loc_built_tree)/fct.hash.h $(loc_built_tree)/event.hash.h $(loc_built_tree)/lexer.lex.h $(loc_built_tree)/parser.tab.h
$(loc_built_tree)/parser.o: $(loc_built_tree)/lexer.lex.h
$(loc_built_tree)/lexer.o: $(loc_built_tree)/event.hash.h
