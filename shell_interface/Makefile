cc = gcc
echo = @echo
rm = @rm -f


OFLAGS = -c -g
LDFLAGS = -g

c_forkee = forkee.c
c_main = main.c pty.c log.c

libs_forkee = -lpthread
libs_main = -lpthread

all: main forkee

main: $(c_main:.c=.o)
	$(cc) $(LDFLAGS) $(libs_main) $^ -o $@

forkee: $(c_forkee:.c=.o)
	$(cc) $(LDFLAGS) $(libs_forkee) $^ -o $@

clean:
	$(rm) *.o *.d main forkee

-include $(c_main:.c=.d)

%.o: %.c
	$(cc) $(OFLAGS) $< -o $@
	@$(CC) -MM $(CFLAGS) $(OPS) $< > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp
