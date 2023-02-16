# Building Toy

This tutorial assumes you're using git, GCC, and make.

To embed toy into your program, simply clone the [git repository](https://github.com/Ratstail91/Toy) into a submodule - here we'll assume you called it `Toy`.

Toy's makefile uses the variable `TOY_OUTDIR` to define where the output of the build command will place the result. You MUST set this to a value, relative to the Toy directory.

```make
export LIBDIR = lib
export TOY_OUTDIR = ../$(LIBDIR)
```

Next, you'll want to run make the from within Toy's `source`, assuming the output directory has been created. There are two options for building Toy - `library` (default) or `static`; the former will create a shared library (and a .dll file on windows), while the latter will create a static library.

```make
toy: $(LIBDIR)
	$(MAKE) -C Toy/source

$(LIBDIR):
	mkdir $(LIBDIR)
```

Finally, link to the outputted library, and specify the source directory to access the header files.

```make
all: $(OBJ)
	$(CC) $(CFLAGS) -o $(OUT) $(OBJ) -L../$(LIBDIR) $(LIBS)
```

Here's a quick example makefile template you can use:

```make
CC=gcc

export OUTDIR = out
export LIBDIR = lib
export TOY_OUTDIR = ../$(LIBDIR)

IDIR+=. ./Toy/source
CFLAGS+=$(addprefix -I,$(IDIR))
LIBS+=-ltoy

ODIR=obj
SRC=$(wildcard *.c)
OBJ=$(addprefix $(ODIR)/,$(SRC:.c=.o))

OUT=./$(OUTDIR)/program

all: toy $(OUTDIR) $(ODIR) $(OBJ)
	$(CC) $(CFLAGS) -o $(OUT) $(OBJ) -L$(LIBDIR) $(LIBS)
	cp $(LIBDIR)/*.dll $(OUTDIR) # for shared libraries

toy: $(LIBDIR)
	$(MAKE) -C Toy/source

$(OUTDIR):
	mkdir $(OUTDIR)

$(LIBDIR):
	mkdir $(LIBDIR)

$(ODIR):
	mkdir $(ODIR)

$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)
```