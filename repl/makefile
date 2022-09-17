CC=gcc

IDIR+=. ../source
CFLAGS+=$(addprefix -I,$(IDIR)) -g -Wall -W -pedantic -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable
LIBS+=-ltoy

ODIR = obj
SRC = $(wildcard *.c)
OBJ = $(addprefix $(ODIR)/,$(SRC:.c=.o))

OUT=../$(OUTDIR)/toyrepl

all: $(OBJ)
	$(CC) -DTOY_IMPORT $(CFLAGS) -o $(OUT) $(OBJ) -L$(realpath $(shell pwd)/../$(OUTDIR)) $(LIBS)

$(OBJ): | $(ODIR)

$(ODIR):
	mkdir $(ODIR)

$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	$(RM) $(ODIR)
