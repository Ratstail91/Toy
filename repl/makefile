#compiler settings
CC=gcc
CFLAGS+=-g -Wall -Werror -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable
LIBS+=-lm -lToy
LDFLAGS+=-Wl,-R -Wl,'$$ORIGIN'

#directories
REPL_ROOTDIR=..
REPL_REPLDIR=.
REPL_SOURCEDIR=$(REPL_ROOTDIR)/source

REPL_OUTDIR=$(REPL_ROOTDIR)/out
REPL_OBJDIR=obj

#file names
REPL_REPLFILES=$(wildcard $(REPL_REPLDIR)/*.c)
REPL_OBJFILES=$(addprefix $(REPL_OBJDIR)/,$(notdir $(REPL_REPLFILES:.c=.o)))
REPL_TARGETNAME=repl.exe

#linker fix
LDFLAGS+=-L$(realpath $(shell pwd)/$(REPL_OUTDIR))

#build the object files, compile the test cases, and run
all: clean build link

#targets for each step
.PHONY: build
build: $(REPL_OBJDIR) $(REPL_OBJFILES)

.PHONY: link
link: $(REPL_OUTDIR) $(REPL_OUTDIR)/$(REPL_TARGETNAME)

#util targets
$(REPL_OUTDIR):
	mkdir $(REPL_OUTDIR)

$(REPL_OBJDIR):
	mkdir $(REPL_OBJDIR)

#compilation steps
$(REPL_OBJDIR)/%.o: $(REPL_REPLDIR)/%.c
	$(CC) -c -o $@ $< $(addprefix -I,$(REPL_REPLDIR)) $(addprefix -I,$(REPL_SOURCEDIR)) $(CFLAGS)

$(REPL_OUTDIR)/$(REPL_TARGETNAME): $(REPL_OBJFILES)
	$(CC) -DTOY_IMPORT $(CFLAGS) -o $@ $(REPL_OBJFILES) $(LDFLAGS) $(LIBS)

#util commands
.PHONY: clean
clean:
ifeq ($(shell uname),Linux)
	find . -type f -name '*.o' -delete
	find . -type f -name '*.a' -delete
	find . -type f -name '*.exe' -delete
	find . -type f -name '*.dll' -delete
	find . -type f -name '*.lib' -delete
	find . -type f -name '*.so' -delete
	find . -type f -name '*.dylib' -delete
	find . -type d -name 'out' -delete
	find . -type d -name 'obj' -delete
else ifeq ($(OS),Windows_NT)
	$(RM) *.o *.a *.exe *.dll *.lib *.so *.dylib
	$(RM) out
	$(RM) obj
else ifeq ($(shell uname),Darwin)
	find . -type f -name '*.o' -delete
	find . -type f -name '*.a' -delete
	find . -type f -name '*.exe' -delete
	find . -type f -name '*.dll' -delete
	find . -type f -name '*.lib' -delete
	find . -type f -name '*.so' -delete
	find . -type f -name '*.dylib' -delete
	find . -type d -name 'out' -delete
	find . -type d -name 'obj' -delete
else
	@echo "Deletion failed - what platform is this?"
endif
