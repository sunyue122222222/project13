CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
LDFLAGS = -lm
TARGET = six_bar_mechanism
SOURCE = six_bar_mechanism.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE) $(LDFLAGS)

clean:
	rm -f $(TARGET) *.txt

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run