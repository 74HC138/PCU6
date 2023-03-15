CC = gcc
CFLAGS = -Wall -Wextra -I include

SRCDIR = source
BUILDDIR = build
TESTDIR = test

SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCES))
TARGET = $(BUILDDIR)/m6asm

.PHONY: all copy clean test

all: $(TARGET) copy

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

copy:
	cp $(TARGET) .

test:
	for file in $(TESTDIR)/*.m6a; do \
		name=$$(basename $$file .m6a); \
		./$(TARGET) -i $$file -o $(TESTDIR)/$$name.hex -l $(TESTDIR)/$$name.lst --ihex -v; \
	done

clean:
	rm -rf $(BUILDDIR)
	rm -f $(TESTDIR)/*.hex
	rm -f $(TESTDIR)/*.bin
	rm -f $(TESTDIR)/*.lst