TARGETS = rarch_reverb.so rarch_wah.so rarch_phaser.so rarch_iir.so rarch_echo_sse.so rarch_volume.so rarch_eq.so

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
RARCH_REVERB_OBJ = rarch_reverb.o freeverb.o $(INIREAD_OBJ)
RARCH_WAH_OBJ = rarch_wah.o wahwah.o $(INIREAD_OBJ)
RARCH_PHASER_OBJ = rarch_phaser.o phaser.o $(INIREAD_OBJ)
RARCH_IIR_OBJ = rarch_iir.o iirfilters.o $(INIREAD_OBJ)
RARCH_ECHO_SSE_OBJ = rarch_echo_sse.o $(INIREAD_OBJ)
RARCH_VOLUME_OBJ = rarch_volume.o $(INIREAD_OBJ)
RARCH_EQ_OBJ = rarch_eq.o eq.o

LDFLAGS += -shared -Wl,--no-undefined

rarch_reverb.so: $(RARCH_REVERB_OBJ)
	$(CXX) -o $@ $(RARCH_REVERB_OBJ) $(LDFLAGS)

rarch_wah.so: $(RARCH_WAH_OBJ)
	$(CXX) -o $@ $(RARCH_WAH_OBJ) $(LDFLAGS)

rarch_phaser.so: $(RARCH_PHASER_OBJ)
	$(CXX) -o $@ $(RARCH_PHASER_OBJ) $(LDFLAGS)

rarch_iir.so: $(RARCH_IIR_OBJ)
	$(CXX) -o $@ $(RARCH_IIR_OBJ) $(LDFLAGS)

rarch_echo_sse.so: $(RARCH_ECHO_SSE_OBJ)
	$(CXX) -o $@ $(RARCH_ECHO_SSE_OBJ) $(LDFLAGS)

rarch_volume.so: $(RARCH_VOLUME_OBJ)
	$(CXX) -o $@ $(RARCH_VOLUME_OBJ) $(LDFLAGS)

rarch_eq.so: $(RARCH_EQ_OBJ)
	$(CXX) -o $@ $(RARCH_EQ_OBJ) $(LDFLAGS)

%.o: %.cpp $(HEADERS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.so
	rm -f *.o
	rm -f meta/*.o

.PHONY: clean
