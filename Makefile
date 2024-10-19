CXX = g++

CXXFLAGS = -g -O0

TARGET = lunarwm

LIBS = -lX11 -lxcb

SRC = main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LIBS)

chmod: $(TARGET)
	chmod +x $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

install:
	cp lunarwm /usr/bin/lunarwm
	chmod +x /usr/bin/lunarwm

.PHONY: all chmod run clean