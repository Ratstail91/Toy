CC=gcc

IDIR +=. ../source ../repl
CFLAGS +=$(addprefix -I,$(IDIR)) -g -Wall -W -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable
LIBS +=-lm
ODIR = obj
TARGETS = $(wildcard ../source/*.c) $(wildcard ../repl/lib_*.c) ../repl/repl_tools.c ../repl/drive_system.c
TESTS = $(wildcard test_*.c)
OBJ = $(addprefix $(ODIR)/,$(TARGETS:../source/%.c=%.o)) $(addprefix $(ODIR)/,$(TESTS:.c=.o))

.PRECIOUS:  $(TESTS:%.c=../$(TOY_OUTDIR)/%.exe)

all: $(OBJ) $(TESTS:%.c=../$(TOY_OUTDIR)/%.exe)

../$(TOY_OUTDIR)/%.exe: $(ODIR)/%.o
	@$(CC) -o $@ $< $(TARGETS:../source/%.c=$(ODIR)/%.o) $(CFLAGS) $(LIBS)
ifeq ($(shell uname)$(DISABLE_VALGRIND),Linux)
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all $@
else
	$@
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
