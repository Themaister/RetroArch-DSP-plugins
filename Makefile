TARGETS = snes_volume.so snes_eq.so

CXXFLAGS += -O3 -g -fPIC -Wall -pedantic -std=gnu++0x
CFLAGS += -O3 -g -fPIC -Wall -pedantic -std=gnu99

ifneq ($(PERF_TEST),)
   CXXFLAGS += -DPERF_TEST
   CFLAGS += -DPERF_TEST
   LDFLAGS += -lrt
endif

HEADERS = $(wildcard *.h) $(wildcard *.hpp) $(wildcard */*.h) $(wildcard */*.hpp)

all: $(TARGETS)

INIREAD_OBJ = inireader.o config_file.o
SNES_VOLUME_OBJ = snes_volume.o
SNES_EQ_OBJ = snes_eq.o eq.o

LDFLAGS += -shared -Wl,--no-undefined

snes_volume.so: $(SNES_VOLUME_OBJ)
	$(CXX) -o $@ $(SNES_VOLUME_OBJ) $(LDFLAGS)

snes_eq.so: $(SNES_EQ_OBJ)
	$(CXX) -o $@ $(SNES_EQ_OBJ) $(LDFLAGS)

%.o: %.cpp $(HEADERS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.so
	rm -f *.o
	rm -f meta/*.o

.PHONY: clean
