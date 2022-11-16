CC=gcc

IDIR+=. ../source
CFLAGS+=$(addprefix -I,$(IDIR)) -g -Wall -W -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable
LIBS+=-ltoy

ODIR = obj
SRC = $(wildcard *.c)
OBJ = $(addprefix $(ODIR)/,$(SRC:.c=.o))
OUTNAME=toy
OUT=../$(TOY_OUTDIR)/toyrepl

all: $(OBJ)
ifeq ($(shell uname),Darwin)
	cp $(PWD)/$(TOY_OUTDIR)/lib$(OUTNAME).dylib /usr/local/lib/
	$(CC) -DTOY_IMPORT $(CFLAGS) -o $(OUT) $(OBJ) $(LIBS)
else
	$(CC) -DTOY_IMPORT $(CFLAGS) -o $(OUT) $(OBJ) -Wl,-rpath,. -L$(realpath $(shell pwd)/../$(TOY_OUTDIR)) $(LIBS)
endif

release: all
	strip $(OUT)

$(OBJ): | $(ODIR)

$(ODIR):
	mkdir $(ODIR)

$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	$(RM) $(ODIR)
	rm /usr/local/lib/lib$(OUTNAME).dylib
