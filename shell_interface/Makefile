bin-y := shellif forkee


#
# shell_interface
#

# objects
shellif-y := main.o pty.o

# flags
shellif-ldflags := -lpthread
shellif-cppflags := -DFORKEE_PATH="\"$(loc_built_tree)/forkee\""


#
# forkee
#

# objects
forkee-y := forkee.o

# flags
forkee-ldflags := -lpthread
