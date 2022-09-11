CC=gcc

IDIR +=.
CFLAGS +=$(addprefix -I,$(IDIR)) -g -Wall -W -pedantic -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable
LIBS +=

ODIR = obj
SRC = $(filter-out $(wildcard *main.c),$(wildcard *.c))
OBJ = $(addprefix $(ODIR)/,$(SRC:.c=.o))

OUTNAME=toy

ifeq ($(findstring CYGWIN, $(shell uname)),CYGWIN)
	LIBLINE =-Wl,--out-implib=../$(OUTDIR)/lib$(OUTNAME).dll.a -Wl,--export-all-symbols -Wl,--enable-auto-import -Wl,--whole-archive $(OBJ) -Wl,--no-whole-archive
	OUT=../$(OUTDIR)/$(OUTNAME).dll
else ifeq ($(shell uname),Linux)
	#I can't get this working as a shared lib
	LIBLINE=-Wl,--out-implib=../$(OUTDIR)/lib$(OUTNAME).a -Wl,--whole-archive $(OBJ) -Wl,--no-whole-archive
	OUT=../$(OUTDIR)/$(OUTNAME)
	CFLAGS += -fPIC
else ifeq ($(OS),Windows_NT)
	LIBLINE =-Wl,--out-implib=../$(OUTDIR)/lib$(OUTNAME).dll.a -Wl,--export-all-symbols -Wl,--enable-auto-import -Wl,--whole-archive $(OBJ) -Wl,--no-whole-archive
	OUT=../$(OUTDIR)/$(OUTNAME).dll
else
	@echo "Platform test failed - what platform is this?"
	exit 1
endif

REPLSRC = $(wildcard repl_main.c)
REPLOUT = $(OUTNAME)repl.exe

all: library $(addprefix $(ODIR)/,$(REPLSRC:.c=.o))
	$(CC) -DTOY_IMPORT $(CFLAGS) -o ../$(OUTDIR)/$(REPLOUT) $(addprefix $(ODIR)/,$(REPLSRC:.c=.o)) $(LIBS) -L$(realpath $(shell pwd)/../$(OUTDIR)) -l$(OUTNAME)

library: $(OBJ)
	$(CC) -DTOY_EXPORT $(CFLAGS) -shared -o $(OUT) $(LIBLINE)

static: $(OBJ) $(addprefix $(ODIR)/,$(REPLSRC:.c=.o))
	$(CC) $(CFLAGS) -o ../$(OUTDIR)/$(REPLOUT) $(addprefix $(ODIR)/,$(REPLSRC:.c=.o)) $(OBJ) $(LIBS)

$(OBJ): | $(ODIR)

$(ODIR):
	mkdir $(ODIR)

$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	$(RM) $(ODIR)
