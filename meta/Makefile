TARGET := snes-meta-gui.so

CXXFILES := $(wildcard *.cpp)
MOCFILES := $(wildcard *.moc.hpp)
OBJ := $(CXXFILES:.cpp=.o) $(MOCFILES:.moc.hpp=.moc.o) ../inireader.o ../config_file.o
HEADERS := $(wildcard *.hpp)

QT_CXXFLAGS := $(shell pkg-config --cflags QtCore QtGui)
QT_LIBS := $(shell pkg-config --libs QtCore QtGui)

INCLUDES := -I.

all: $(TARGET)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) $(INCLUDES) -DMETA_GUI -fPIC -O3 -std=gnu++0x -c -o $@ $<

%.moc.o: %.moc.cxx $(HEADERS)
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) $(INCLUDES) -DMETA_GUI -fPIC -O3 -std=gnu++0x -c -o $@ $<

%.moc.cxx: %.moc.hpp
	moc -o $@ $< $(INCLUDES)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $(OBJ) $(QT_LIBS) -shared -Wl,--no-undefined -ldl

clean:
	rm -f *.o
	rm -f *.so

.PHONY: clean
