TARGET := snes-meta-gui.so

CXXFILES := $(wildcard *.cpp)
OBJ := $(CXXFILES:.cpp=.o) phoenix/phoenix.o ../inireader.o ../config_file.o
HEADERS = $(wildcard *.hpp)

GTK_CFLAGS = $(shell pkg-config --cflags gtk+-2.0)
GTK_LIBS = $(shell pkg-config --libs gtk+-2.0)

INCLUDES = -Iphoenix -I.

all: $(TARGET)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -DMETA_GUI -fPIC -O3 -std=gnu++0x -c -o $@ $<

phoenix/phoenix.o: $(wildcard phoenix/**/*.cpp) $(wildcard phoenix/**/*.hpp)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -fPIC -O3 -std=gnu++0x -DPHOENIX_GTK $(GTK_CFLAGS) -c -o $@ phoenix/phoenix.cpp

$(TARGET): $(OBJ)
	$(CXX) -o $@ $(OBJ) $(GTK_LIBS) -shared -Wl,--no-undefined -lX11 -ldl

clean:
	rm -f *.o
	rm -f phoenix/*.o
	rm -f *.so

.PHONY: clean
