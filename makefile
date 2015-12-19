# project name (generate executable with this name)
TARGET   = tagger.exe

CC       = gcc
# compiling flags here
CFLAGS   = -I../include

LINKER   = gcc -o
# linking flags here
LFLAGS   = -lm

# change these to set the proper directories where each files shoould be
SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin

SOURCES  := $(wildcard $(SRCDIR)/*.c)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) 
LIBS	 = ../lib/libgw32c.a ../lib/libole32.a ../lib/libuuid.a ../lib/libiconv.a
rm       = rm -f


$(BINDIR)/$(TARGET): $(OBJECTS)
	@echo Linking binary
	$(LINKER) $@ $(LFLAGS) $(OBJECTS) $(LIBS)
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
