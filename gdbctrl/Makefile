cc = gcc
echo = @echo
rm = @rm -f


OFLAGS = -c -g
LDFLAGS = -g


target = gdbctrl
c_main = $(wildcard *.c)

libs = -lpthread

all: $(target)

$(target): $(c_main:.c=.o)
	$(cc) $(LDFLAGS) $(libs_main) $^ -o $@

clean:
	$(rm) *.o *.d $(target)

-include $(c_main:.c=.d)

%.o: %.c
	$(cc) $(OFLAGS) $< -o $@
	@$(CC) -MM $(CFLAGS) $(OPS) $< > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp
