bin-y := vimgdb

## vimgdb
# object files
vimgdb-y := main.o common/ gdb/ gui/ user_cmd/

# flags
vimgdb-cppflags := -I"$(loc_src_tree)"
vimgdb-ldflags := -lpthread -lcurses -lm

$(loc_build_tree)/main.o: $(build_tree)/user_cmd/cmd.hash.h
