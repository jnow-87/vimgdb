obj-y := vimui.o parser.o lexer.o event.o cmd.o reply.o
bin-y := test-nbprotocol


cppflags-y := -DVIM_NB_PORT=$(CONFIG_VIM_NB_PORT)
cppflags-y += -I"$(loc_built_tree)/"


test-nbprotocol-y := test_nbprotocol.o cmd.o fct.o event.o $(built_tree)/gui/gui.o common/
test-nbprotocol-ldflags := -lcurses -lpthread


$(loc_built_tree)/vimui.o: $(loc_built_tree)/parser.tab.h $(loc_built_tree)/cmd.hash.h $(loc_built_tree)/lexer.lex.h
$(loc_built_tree)/test_nbprotocol.o: $(loc_built_tree)/cmd.hash.h $(loc_built_tree)/fct.hash.h $(loc_built_tree)/lexer.lex.h $(loc_built_tree)/parser.tab.h
$(loc_built_tree)/parser.o: $(loc_built_tree)/lexer.lex.h
