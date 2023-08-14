CC=gcc

IDIR+=.
CFLAGS+=$(addprefix -I,$(IDIR)) -g -Wall -W -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable
LIBS+=

ODIR = obj
SRC = $(wildcard *.c)
OBJ = $(addprefix $(ODIR)/,$(SRC:.c=.o))
OUTNAME=
OUT=disassembler

all: $(OBJ)
	$(CC) $(CFLAGS) -o $(OUT) $(OBJ) $(LIBS)

$(OBJ): | $(ODIR)

$(ODIR):
	mkdir $(ODIR)

$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	$(RM) -r $(ODIR)
