TARGET = chess

CFLAGS = -std=c++17
# CFLAGS += -g #debug
CFLAGS += -O3 #release

all: $(TARGET).cpp
	g++ $(CFLAGS) $(TARGET).cpp -o $(TARGET) -lsfml-graphics -lsfml-window -lsfml-system

clean:
	rm -f $(TARGET)