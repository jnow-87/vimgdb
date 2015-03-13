bin-y := vimgdb

## vimgdb
# object files
vimgdb-y := main.o common/ gdb/ gui/ user_cmd/

# flags
vimgdb-cppflags := -I"$(loc_src_tree)"
vimgdb-ldflags := -lpthread -lcurses


## additional dependencies for generated headers
$(loc_built_tree)/main.o: $(built_tree)/gdb//parser.tab.h

