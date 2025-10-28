CC = gcc
CFLAGS = -Wall -Wextra -O2

TARGET = meu_servidor

SRC = servidor.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) *.o