#compiler settings reference
#CC=gcc
#CFLAGS+=-std=c17 -g -Wall -Werror -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wformat=2
#LIBS+=-lm
#LDFLAGS+=

#directories
export TOY_SOURCEDIR=source
export TOY_REPLDIR=repl
export TOY_CASESDIR=tests/cases
export TOY_INTEGRATIONSDIR=tests/integrations
export TOY_BENCHMARKSDIR=tests/benchmarks
export TOY_OUTDIR=out
export TOY_OBJDIR=obj

#targets
#all:

.PHONY: source
source:
	$(MAKE) -C source -k

.PHONY: repl
repl: source
	$(MAKE) -C repl -k

#various kinds of available tests
.PHONY: tests
tests: clean test-cases test-integrations test-benchmarks

.PHONY: test-cases
test-cases:
	$(MAKE) -C $(TOY_CASESDIR) -k

.PHONY: test-integrations
test-integrations:
	$(MAKE) -C $(TOY_INTEGRATIONSDIR) -k

.PHONY: test-benchmarks
test-benchmarks:
	$(MAKE) -C $(TOY_BENCHMARKSDIR) -k

#same as above, but with GDB
.PHONY: test-gdb
test-gdb: clean test-cases-gdb test-integrations-gdb test-benchmarks-gdb

.PHONY: test-cases-gdb
test-cases-gdb:
	$(MAKE) -C $(TOY_CASESDIR) gdb -k

.PHONY: test-integrations-gdb
test-integrations-gdb:
	$(MAKE) -C $(TOY_INTEGRATIONSDIR) gdb -k

.PHONY: test-benchmarks-gdb
test-benchmarks-gdb:
	$(MAKE) -C $(TOY_BENCHMARKSDIR) gdb -k

#TODO: mustfail tests

#util targets
$(TOY_OUTDIR):
	mkdir $(TOY_OUTDIR)

$(TOY_OBJDIR):
	mkdir $(TOY_OBJDIR)

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

