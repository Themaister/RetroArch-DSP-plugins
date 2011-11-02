TARGETS = snes_reverb.so snes_wah.so snes_phaser.so snes_iir.so snes_echo.so snes_volume.so snes_eq.so

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
SNES_REVERB_OBJ = snes_reverb.o freeverb.o $(INIREAD_OBJ)
SNES_WAH_OBJ = snes_wah.o wahwah.o $(INIREAD_OBJ)
SNES_PHASER_OBJ = snes_phaser.o phaser.o $(INIREAD_OBJ)
SNES_IIR_OBJ = snes_iir.o iirfilters.o $(INIREAD_OBJ)
SNES_ECHO_OBJ = snes_echo.o echo.o $(INIREAD_OBJ)
SNES_VOLUME_OBJ = snes_volume.o
SNES_EQ_OBJ = snes_eq.o eq.o

LDFLAGS += -shared -Wl,--no-undefined

snes_reverb.so: $(SNES_REVERB_OBJ)
	$(CXX) -o $@ $(SNES_REVERB_OBJ) $(LDFLAGS)

snes_wah.so: $(SNES_WAH_OBJ)
	$(CXX) -o $@ $(SNES_WAH_OBJ) $(LDFLAGS)

snes_phaser.so: $(SNES_PHASER_OBJ)
	$(CXX) -o $@ $(SNES_PHASER_OBJ) $(LDFLAGS)

snes_iir.so: $(SNES_IIR_OBJ)
	$(CXX) -o $@ $(SNES_IIR_OBJ) $(LDFLAGS)

snes_echo.so: $(SNES_ECHO_OBJ)
	$(CXX) -o $@ $(SNES_ECHO_OBJ) $(LDFLAGS)

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
