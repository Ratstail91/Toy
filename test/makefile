CC=gcc

IDIR +=. ../source
CFLAGS +=$(addprefix -I,$(IDIR)) -g -Wall -W -pedantic -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable
LIBS +=

ODIR = obj
TARGETS = $(filter-out $(wildcard ../source/*main.c),$(wildcard ../source/*.c))
TESTS = $(wildcard *.c)
OBJ = $(addprefix $(ODIR)/,$(TARGETS:../source/%.c=%.o)) $(addprefix $(ODIR)/,$(TESTS:.c=.o))

.PRECIOUS:  $(TESTS:%.c=../$(OUTDIR)/%.exe)

all: $(OBJ) $(TESTS:%.c=../$(OUTDIR)/%.exe)

../$(OUTDIR)/%.exe: $(ODIR)/%.o
	@$(CC) -o $@ $< $(TARGETS:../source/%.c=$(ODIR)/%.o) $(CFLAGS) $(LIBS)
ifeq ($(shell uname),Linux)
	valgrind $@
else
	@echo please run these tests with valgrind on linux
endif

$(OBJ): | $(ODIR)

$(ODIR):
	mkdir $(ODIR)

$(ODIR)/%.o: %.c
	@$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: ../source/%.c
	@$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	$(RM) $(ODIR)
