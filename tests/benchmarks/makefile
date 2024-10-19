#compiler settings
CC=gcc
CFLAGS+=-std=c17 -g -Wall -Werror -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wformat=2
LIBS+=-lm
LDFLAGS+=

ifeq ($(shell uname),Linux)
LDFLAGS=-Wl,--gc-sections
else ifeq ($(OS),Windows_NT)
LDFLAGS=-Wl,--gc-sections
else ifeq ($(shell uname),Darwin)
LDFLAGS=-Wl,-dead_strip
else
	@echo "LDFLAGS set failed - what platform is this?"
endif

#patch in these for compatability
TOY_SOURCEDIR=source

#directories
TEST_ROOTDIR=../..
TEST_SOURCEDIR=$(TEST_ROOTDIR)/$(TOY_SOURCEDIR)
TEST_CASESDIR=.

TEST_OUTDIR=out
TEST_OBJDIR=obj

#file names
TEST_SOURCEFILES=$(wildcard $(TEST_SOURCEDIR)/*.c)
TEST_CASESFILES=$(wildcard $(TEST_CASESDIR)/*.c)

#utils
UC=$(shell echo '$1' | tr '[:lower:]' '[:upper:]')

#kick off
all: $(TEST_OBJDIR) $(TEST_OUTDIR) build-files run-all
 
run-all:
	for exe in $(wildcard $(TEST_OUTDIR)/*.exe) ; do \
		i=1 ; \
		exp=10 ; \
		while [ $$i -le 8 ] ; do \
			/usr/bin/time --format "%C: %U %W" $$exe $$exp ; \
			exp=$$((exp * 10)); \
			i=$$(( i + 1 )) ; \
		done ; \
	done


#			gdb $$exe -ix gdb_init -ex=run -ex=bt --batch --return-child-result --args "$$exe" $$exp ; \

#build the source files
build-files:
	for src in $(TEST_CASESFILES) ; do \
		$(MAKE) SRC=$$src build-parameters ; \
	done

build-parameters:
	initial=8 ; \
	while [ $$initial -le 256 ] ; do \
		expansion=2 ; \
		while [ $$expansion -le 8 ] ; do \
			$(MAKE) INITIAL=$$initial EXPANSION=$$expansion build-executable ; \
			expansion=$$((expansion + 1)) ; \
		done ; \
		initial=$$((initial + initial)) ; \
	done

build-executable:
	$(MAKE) UCSRC=$(call UC,$(basename $(notdir $(SRC)))) build-source
	$(MAKE) UCSRC=$(call UC,$(basename $(notdir $(SRC)))) build-src
	$(MAKE) UCSRC=$(call UC,$(basename $(notdir $(SRC)))) build-exe

.PHONY: build-source
build-source: $(TEST_OUTDIR) $(TEST_OBJDIR) $(addprefix $(TEST_OBJDIR)/,$(notdir $(TEST_SOURCEFILES:.c=.o)))

$(TEST_OBJDIR)/%.o: $(TEST_SOURCEDIR)/%.c
		$(CC) -DTOY_$(UCSRC)_INITIAL_CAPACITY=$(INITIAL) -DTOY_$(UCSRC)_EXPANSION_RATE=$(EXPANSION) -c -o $@ $< $(addprefix -I,$(TEST_SOURCEDIR)) $(CFLAGS) -fdata-sections -ffunction-sections

.PHONY: build-src
build-src:
	$(CC) -DTOY_$(UCSRC)_INITIAL_CAPACITY=$(INITIAL) -DTOY_$(UCSRC)_EXPANSION_RATE=$(EXPANSION) -c -o $(TEST_OBJDIR)/$(SRC:.c=.o) $(SRC) $(addprefix -I,$(TEST_SOURCEDIR)) $(CFLAGS) -fdata-sections -ffunction-sections

.PHONY: build-exe
build-exe:
	$(CC) -o $(TEST_OUTDIR)/$(SRC:.c=)-$(INITIAL)-$(EXPANSION).exe $(TEST_OBJDIR)/$(SRC:.c=.o) $(addprefix $(TEST_OBJDIR)/,$(notdir $(TEST_SOURCEFILES:.c=.o))) $(CFLAGS) $(LIBS) $(LDFLAGS)

#util targets
$(TEST_OUTDIR):
	mkdir $(TEST_OUTDIR)

$(TEST_OBJDIR):
	mkdir $(TEST_OBJDIR)

