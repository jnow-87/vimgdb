obj-y := vimui.o parser.o lexer.o event.o cmd.o reply.o
bin-y := test-nbprotocol


cppflags-y := -DVIM_NB_PORT=$(CONFIG_VIM_NB_PORT)
cppflags-y += -I"$(loc_build_tree)/"


test-nbprotocol-y := test_nbprotocol.o cmd.o fct.o event.o $(build_tree)/gui/gui.o common/
test-nbprotocol-ldflags := -lcurses -lpthread


$(loc_build_tree)/vimui.o: $(loc_build_tree)/parser.tab.h $(loc_build_tree)/cmd.hash.h $(loc_build_tree)/lexer.lex.h
$(loc_build_tree)/test_nbprotocol.o: $(loc_build_tree)/cmd.hash.h $(loc_build_tree)/fct.hash.h $(loc_build_tree)/lexer.lex.h $(loc_build_tree)/parser.tab.h
$(loc_build_tree)/parser.o: $(loc_build_tree)/lexer.lex.h
