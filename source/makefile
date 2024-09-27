#compiler settings
CC=gcc
CFLAGS+=-g -Wall -Werror -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable
LIBS+=-lm
LDFLAGS+=

#directories
SRC_ROOTDIR=..
SRC_SOURCEDIR=.

SRC_OUTDIR=$(SRC_ROOTDIR)/out
SRC_OBJDIR=obj

#file names
SRC_SOURCEFILES=$(wildcard $(SRC_SOURCEDIR)/*.c)
SRC_OBJFILES=$(addprefix $(SRC_OBJDIR)/,$(notdir $(SRC_SOURCEFILES:.c=.o)))
SRC_TARGETNAME=Toy

#TODO: fix windows & macos
#SRC_LIBLINE is a fancy way of making the linker work correctly
ifeq ($(shell uname),Linux)
	SRC_TARGETEXT=.so
	SRC_LIBLINE=-Wl,-rpath,. -Wl,--out-implib=$(SRC_OUTDIR)/lib$(SRC_TARGETNAME).a -Wl,--whole-archive $(SRC_OBJFILES) -Wl,--no-whole-archive
	CFLAGS+=-fPIC
# else ifeq ($(OS),Windows_NT)
# 	SRC_TARGETEXT=.dll
# 	SRC_LIBLINE=-Wl,-rpath,. -Wl,--out-implib=$(SRC_OUTDIR)/lib$(OUTNAME).dll.a -Wl,--export-all-symbols -Wl,--enable-auto-import -Wl,--whole-archive $(SRC_OBJFILES) -Wl,--no-whole-archive
# else ifeq ($(shell uname),Darwin)
# 	SRC_TARGETEXT=.dylib
# 	SRC_LIBLINE=$(SRC_OBJFILES)
else
	@echo "Platform test failed - what platform is this?"
	exit 1
endif

#build the object files, compile the test cases, and run
all: clean build link

#targets for each step
.PHONY: build
build: $(SRC_OUTDIR) $(SRC_OBJDIR) $(SRC_OBJFILES)

.PHONY: link
link: $(SRC_OUTDIR)
	$(CC) -DTOY_EXPORT $(CFLAGS) -shared -o $(SRC_OUTDIR)/lib$(SRC_TARGETNAME)$(SRC_TARGETEXT) $(SRC_LIBLINE)

#util targets
$(SRC_OUTDIR):
	mkdir $(SRC_OUTDIR)

$(SRC_OBJDIR):
	mkdir $(SRC_OBJDIR)

#compilation steps
$(SRC_OBJDIR)/%.o: $(SRC_SOURCEDIR)/%.c
	$(CC) -c -o $@ $< $(addprefix -I,$(SRC_SOURCEDIR)) $(CFLAGS)	

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
