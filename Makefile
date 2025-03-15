CC = cc
CFLAGS = -Wall -O2 `pkg-config --cflags libpng`

SRCS = beta.c raw.c writepng.c beta2png.c
OBJS = $(SRCS:.c=.o)

TARGET = beta2png

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
