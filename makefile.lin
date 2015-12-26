# ------------------------------------------------
# Makefile for Linux
#
# Author: cedricfrancoys@gmail.com
# Date  : 2015-12-19
#
# ------------------------------------------------

# project name (generate executable with this name)
TARGET   = tagger.o

CC       = c99
# compiling flags here
CFLAGS   = -I/usr/include -D_BSD_SOURCE

LINKER   = gcc -o
# linking flags here
LFLAGS   = -lm

# change these to set the proper directories where each files shoould be
SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin

SOURCES  := $(wildcard $(SRCDIR)/*.c)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

rm       = rm -f

$(BINDIR)/$(TARGET): $(OBJECTS)
	@echo Linking binary:
	$(LINKER) $@ $(LFLAGS) $(OBJECTS)
	@echo "$@" successfuly generated

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@echo Compiling "$<"
	@$(CC) $(CFLAGS) -c $< -o $@

.PHONEY: clean
clean:
	@$(rm) $(OBJECTS)
	@$(rm) $(BINDIR)/$(TARGET)
	@echo Cleanup complete

.PHONEY: remove
remove: clean
	@$(rm) $(BINDIR)/$(TARGET)
	@echo Binary removed
