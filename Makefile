CC = cc
CFLAGS = -Wall -O2 `pkg-config --cflags libpng`

SRCS = beta.c raw.c writepng.c beta2png.c
OBJS = $(SRCS:.c=.o)

TARGET = beta2png

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

beta.o: beta.c beta.h raw.h
	$(CC) $(CFLAGS) -c beta.c -o beta.o

beta2png.o: beta2png.c beta.h writepng.h
	$(CC) $(CFLAGS) -c beta2png.c -o beta2png.o

raw.o: raw.c raw.h
	$(CC) $(CFLAGS) -c raw.c -o raw.o

writepng.o: writepng.c writepng.h
	$(CC) $(CFLAGS) -c writepng.c -o writepng.o

clean:
	rm -f $(OBJS) $(TARGET)
