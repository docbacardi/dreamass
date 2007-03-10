

.PHONY: all clean

# select debug or release version
#DREAMASS = ../dreamass
DREAMASS = ../dreamass_dbg

# uncomment next line to enable valgrind
CHECK = valgrind --leak-check=yes --log-file=$(subst src,log,$<) --show-reachable=yes -v

# uncomment next line to enable callgrind
#CHECK = callgrind

#---------------------------------------

TESTS = $(basename $(wildcard *.src))
OBJS  = $(addsuffix .o64,$(TESTS))

all: $(OBJS)

%.o64: %.src $(DREAMASS)
	@echo "----------------------------------------"
	@echo " Assembling testfile " $<
	@echo "----------------------------------------"
	$(CHECK) $(DREAMASS) -v -d $(subst src,html,$<) -e $(subst src,err,$<) -l $(subst src,lab,$<) -o $@ $<
	@echo

#---------------------------------------

clean:
	rm -f $(addsuffix .err,$(TESTS))
	rm -f $(addsuffix .lab,$(TESTS))
	rm -f $(addsuffix .html,$(TESTS))
	rm -f $(addsuffix .o64,$(TESTS))
	rm -f $(addsuffix .log*,$(TESTS))


