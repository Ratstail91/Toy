#compiler settings
CFLAGS=-g -Wall -Werror -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable
LIBS=-lm

#directories
TEST_ROOTDIR=..
TEST_SOURCEDIR=$(TEST_ROOTDIR)/$(TOY_SOURCEDIR)
TEST_CASESDIR=cases

TEST_OUTDIR=out
TEST_OBJDIR=obj

#file names
TEST_SOURCEFILES=$(addprefix $(TOY_ROOTDIR)/,$(TOY_SOURCEFILES))
TEST_CASESFILES=$(wildcard $(TEST_CASESDIR)/*.c)

#build the object files, compile the test cases, and run
all: clean build-source build-cases $(addprefix $(TEST_OUTDIR)/,$(notdir $(TEST_CASESFILES:%.c=%.exe)))

#targets for each set of source files
.PHONY: build-source
build-source: $(TEST_OUTDIR) $(TEST_OBJDIR) $(addprefix $(TEST_OBJDIR)/,$(notdir $(TEST_SOURCEFILES:.c=.o)))

.PHONY: build-cases
build-cases: $(TEST_OUTDIR) $(TEST_OBJDIR) $(addprefix $(TEST_OBJDIR)/,$(notdir $(TEST_CASESFILES:.c=.o))) 

#util targets
$(TEST_OUTDIR):
	mkdir $(TEST_OUTDIR)

$(TEST_OBJDIR):
	mkdir $(TEST_OBJDIR)

#compilation step
$(TEST_OBJDIR)/%.o: $(TEST_SOURCEDIR)/%.c
	$(CC) -c -o $@ $< $(addprefix -I,$(TEST_SOURCEDIR)) $(CFLAGS) -fdata-sections -ffunction-sections

$(TEST_OBJDIR)/%.o: $(TEST_CASESDIR)/%.c
	$(CC) -c -o $@ $< $(addprefix -I,$(TEST_SOURCEDIR) $(TEST_CASESDIR)) $(CFLAGS) -fdata-sections -ffunction-sections

#final linking step (with extra flags to strip dead code)
$(TEST_OUTDIR)/%.exe: $(TEST_OBJDIR)/%.o
	@$(CC) -o $@ $< $(addprefix $(TEST_OBJDIR)/,$(notdir $(TEST_SOURCEFILES:.c=.o))) $(CFLAGS) $(LIBS) -Wl,--gc-sections
	$@

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
	del *.o *.a *.exe *.dll *.lib *.so *.dylib
	del /s out
	del /s obj
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
