#compiler settings
CC=gcc
CFLAGS+=-std=c17 -pedantic -Werror
LIBS=-lm

#directories
export TOY_SOURCEDIR=source
export TOY_OUTDIR=out
export TOY_OBJDIR=obj

#file names
export TOY_SOURCEFILES=$(wildcard $(TOY_SOURCEDIR)/*.c)

#targets
all: clean tests
	@echo no targets ready

.PHONY: tests
tests:
	$(MAKE) -C tests

#util targets
$(TOY_OUTDIR):
	mkdir $(TOY_OUTDIR)

$(TOY_OBJDIR):
	mkdir $(TOY_OBJDIR)

$(TOY_OBJDIR)/%.o: $(TOY_SOURCEDIR)/%.c
	$(CC) -c -o $@ $< $(addprefix -I,$(TOY_SOURCEDIR)) $(CFLAGS)

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

.PHONY: rebuild
rebuild: clean all
