CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = editor

all: $(TARGET)

$(TARGET): editor.c
	$(CC) $(CFLAGS) -o $(TARGET) editor.c

clean:
	del /f /q $(TARGET).exe 2>nul || rm -f $(TARGET) $(TARGET).exe
