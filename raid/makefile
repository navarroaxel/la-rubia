TOPDIR  := ./
SRCDIR  := $(TOPDIR)src/
OBJDIR  := $(TOPDIR)obj/
BINDIR  := $(TOPDIR)bin/
NAME    := raid
EXE     := $(BINDIR)$(NAME)

SFILES  := c
OFILES  := o
CC      := gcc
CFLAGS  := -I/usr/include/fuse -O0 -g3 -Wall -c -fmessage-length=0 -g -D_FILE_OFFSET_BITS=64 -licui18n -licuuc -licudata -MMD -MP -pthread -lpthread -lm -lrt
LDFLAGS := -licui18n -licuuc -licudata -MMD -MP -pthread -lpthread -lm -lrt -lfuse
SOURCES := $(shell find $(SRCDIR) -name "*.$(SFILES)")
OBJECTS := $(patsubst $(SRCDIR)%.$(SFILES), $(OBJDIR)%.$(OFILES), $(SOURCES))

ALLFILES := $(SOURCES)

.PHONY: all clean

all:    $(EXE)

$(EXE): $(OBJECTS)
	@$(CC) $^ -o $@ $(LDFLAGS)

$(OBJDIR)%$(OFILES):    $(SRCDIR)%$(SFILES)
	@$(CC) $(CFLAGS) $< -o $@

clean:
	@rm -f $(OBJECTS) $(EXE)
