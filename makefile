# ------------------------------------------------
# Makefile for Win32 console
#
# Author: cedricfrancoys@gmail.com
# Date  : 2015-12-19
#
# ------------------------------------------------

# project name (generate executable with this name)
TARGET   = tagger.exe

CC       = gcc
# compiling flags here
CFLAGS   = -IC:\TDM-GCC-32\include

LINKER   = gcc -o
# linking flags here
LFLAGS   = -lm

# change these to set the proper directories where each files shoould be
SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin
LIBDIR   = C:\TDM-GCC-32\lib

SOURCES  := $(wildcard $(SRCDIR)/*.c)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
LIBS	 = $(LIBDIR)/libgw32c.a $(LIBDIR)/libole32.a $(LIBDIR)/libuuid.a $(LIBDIR)/libiconv.a

FixPath  = $(subst /,\,$1)
rm       = del /Q


$(BINDIR)/$(TARGET): $(OBJECTS)
	@echo Linking binary:
	$(LINKER) $@ $(LFLAGS) $(OBJECTS) $(LIBS)
	@echo "$@" successfuly generated

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@echo Compiling "$<"
	@$(CC) $(CFLAGS) -c $< -o $@
	

.PHONEY: clean
clean:
	$(rm) $(call FixPath,$(OBJECTS))
	$(rm) $(call FixPath,$(BINDIR)/$(TARGET))
	@echo Cleanup complete

.PHONEY: remove
remove: clean
	@$(rm) $(call FixPath,$(BINDIR)/$(TARGET))
	@echo Binary removed