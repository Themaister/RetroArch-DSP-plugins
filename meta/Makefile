TARGET := rarch-meta-gui.so

TEST_TARGET := meta-test
TEST_TARGET_SRC := $(wildcard test/*.cpp) utils.cpp
TEST_TARGET_OBJ := $(TEST_TARGET_SRC:.cpp=.o)

CXXFILES := $(wildcard *.cpp)
MOCFILES := $(wildcard *.moc.hpp)
OBJ := $(CXXFILES:.cpp=.o) $(MOCFILES:.moc.hpp=.moc.o) ../inireader.o ../config_file.o
HEADERS := $(wildcard *.hpp)

QT_CXXFLAGS := $(shell pkg-config --cflags QtCore QtGui)
QT_LIBS := $(shell pkg-config --libs QtCore QtGui)

MOC = moc-qt4

INCLUDES := -I.

all: $(TARGET)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) $(INCLUDES) -DMETA_GUI -fPIC -O3 -std=gnu++0x -c -o $@ $<

test/%.o: test/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -O3 -std=gnu++0x -c -o $@ $<

%.moc.o: %.moc.cxx $(HEADERS)
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) $(INCLUDES) -DMETA_GUI -fPIC -O3 -std=gnu++0x -c -o $@ $<

%.moc.cxx: %.moc.hpp
	$(MOC) -o $@ $< $(INCLUDES)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $(OBJ) $(QT_LIBS) -shared -Wl,--no-undefined -ldl

$(TEST_TARGET): $(TEST_TARGET_OBJ)
	$(CXX) -o $@ $(TEST_TARGET_OBJ) -ldl

test: $(TEST_TARGET)

clean:
	rm -f *.o
	rm -f *.so
	rm -f *.moc.cxx

.PHONY: clean
