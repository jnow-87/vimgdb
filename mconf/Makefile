hostbin-y := mconfig genconfig

$(loc_built_tree)/zconf.tab.host.o: $(loc_built_tree)/zconf.lex.c $(loc_built_tree)/zconf.hash.c $(loc_built_tree)/zconf.tab.c


mconfig-y := zconf.tab.o mconf.o lxdialog/

mconfig-hostldflags := -lncurses
mconfig-hostcppflags := -I"$(loc_src_tree)" -I"$(loc_built_tree)" -DYYDEBUG -DCURSES_LOC="<ncurses.h>"
mconfig-hostcflags := -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer
mconfig-yaccflags := --name-prefix=zconf
mconfig-lexflags := --prefix=zconf
mconfig-gperfflags := -a -C -E -g -k '1,3,$$$$' -p -t

genconfig-y := genconf.o zconf.tab.o

genconfig-hostcppflags := -I"$(loc_src_tree)" -I"$(loc_built_tree)" -DYYDEBUG
genconfig-hostcflags := -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer
genconfig-yaccflags := --name-prefix=zconf
genconfig-lexflags := --prefix=zconf
genconfig-gperfflags := -a -C -E -g -k '1,3,$$$$' -p -t
