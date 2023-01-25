CC=gcc

TOY_OUTDIR=out

IDIR+=. ../../source
CFLAGS+=$(addprefix -I,$(IDIR)) -g -Wall -W -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable
LIBS+=-ltoy

ODIR = obj
SRC = $(wildcard *.c)
OBJ = $(addprefix $(ODIR)/,$(SRC:.c=.o))
OUTNAME=toy
OUT=../../$(TOY_OUTDIR)/memusage

all:
	cp $(shell find ../../repl/repl_tools*) .
	cp $(shell find ../../repl/lib*) .
	$(MAKE) build

build: $(OBJ)
	echo $(OBJ)
ifeq ($(shell uname),Darwin)
	cp $(PWD)/$(TOY_OUTDIR)/lib$(OUTNAME).dylib /usr/local/lib/
	$(CC) -DTOY_IMPORT $(CFLAGS) -o $(OUT) $(OBJ) $(LIBS)
else
	$(CC) -DTOY_IMPORT $(CFLAGS) -o $(OUT) $(OBJ) -Wl,-rpath,. -L$(realpath $(shell pwd)/../../$(TOY_OUTDIR)) $(LIBS)
endif

$(OBJ): | $(ODIR)

$(ODIR):
	mkdir $(ODIR)

$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	$(RM) -r $(ODIR)
	$(RM) $(shell find ./repl_tools*)
	$(RM) $(shell find ./lib*)