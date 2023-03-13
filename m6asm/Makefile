CC = gcc
CFLAGS = -Wall -Wextra -I include

SRCDIR = source
BUILDDIR = build

SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCES))
TARGET = $(BUILDDIR)/m6asm

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)