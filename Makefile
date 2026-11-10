CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -O2 -Iinclude
TARGET  = aerocontrol
SRCDIR  = src
OBJDIR  = obj

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c include/aerocontrol.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(OBJDIR) $(TARGET)
