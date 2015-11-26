CFLAGS = -g -O0 -Wall

TARGET = tun2

.PHONY: all clean

all: $(TARGET)

clean:
	rm -f $(TARGET)
