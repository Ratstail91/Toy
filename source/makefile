CC=gcc

IDIR+=.
CFLAGS+=$(addprefix -I,$(IDIR)) -g -Wall -W -pedantic -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable
LIBS+=

ODIR = obj
SRC = $(wildcard *.c)
OBJ = $(addprefix $(ODIR)/,$(SRC:.c=.o))

OUTNAME=toy

ifeq ($(findstring CYGWIN, $(shell uname)),CYGWIN)
	LIBLINE =-Wl,--out-implib=../$(TOY_OUTDIR)/lib$(OUTNAME).dll.a -Wl,--export-all-symbols -Wl,--enable-auto-import -Wl,--whole-archive $(OBJ) -Wl,--no-whole-archive
	OUT=../$(TOY_OUTDIR)/$(OUTNAME).dll
else ifeq ($(shell uname),Linux)
	#I can't get this working as a shared lib
	LIBLINE=-Wl,--out-implib=../$(TOY_OUTDIR)/lib$(OUTNAME).a -Wl,--whole-archive $(OBJ) -Wl,--no-whole-archive
	OUT=../$(TOY_OUTDIR)/$(OUTNAME)
	CFLAGS += -fPIC
else ifeq ($(OS),Windows_NT)
	LIBLINE =-Wl,--out-implib=../$(TOY_OUTDIR)/lib$(OUTNAME).dll.a -Wl,--export-all-symbols -Wl,--enable-auto-import -Wl,--whole-archive $(OBJ) -Wl,--no-whole-archive
	OUT=../$(TOY_OUTDIR)/$(OUTNAME).dll
else ifeq ($(shell uname),Darwin)
	LIBLINE = $(OBJ)
	OUT=../$(TOY_OUTDIR)/lib$(OUTNAME).dylib
else
	@echo "Platform test failed - what platform is this?"
	exit 1
endif

library: $(OBJ)
	$(CC) -DTOY_EXPORT $(CFLAGS) -shared -o $(OUT) $(LIBLINE)

static: $(OBJ)
	ar crs ../$(TOY_OUTDIR)/lib$(OUTNAME).a $(OBJ)

$(OBJ): | $(ODIR)

$(ODIR):
	mkdir $(ODIR)

$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	$(RM) $(ODIR)
