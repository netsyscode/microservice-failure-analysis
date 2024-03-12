PROGRAMS := $(wildcard src/*)

.PHONY: all clean $(PROGRAMS)

all: $(PROGRAMS)

clean: 
	-$(RM) -r ./build

$(PROGRAMS):
	echo $(PROGRAMS)
	$(MAKE) --directory=$@
