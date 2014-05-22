cc = gcc
cpp = g++
echo = @echo
rm = @rm -f


OFLAGS = -c -g -std=c++11
LDFLAGS = -g


target = gdbctrl
c_main = $(wildcard *.c)
cc_main = $(wildcard *.cc)

libs = -lpthread

all: $(target)

$(target): $(c_main:.c=.o) $(cc_main:.cc=.o)
	$(cpp) $(LDFLAGS) $(libs_main) $^ -o $@

clean:
	$(rm) *.o *.d $(target)

-include $(c_main:.c=.d)
-include $(cc_main:.cc=.d)

%.o: %.c
	$(cpp) $(OFLAGS) $< -o $@
	@$(cpp) -MM $(CFLAGS) $< > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

%.o: %.cc
	$(cpp) $(OFLAGS) $< -o $@
	@$(cpp) -MM $(CFLAGS) $< > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp
