TARGETS = snes_reverb.so snes_wah.so snes_phaser.so snes_iir.so snes_echo.so

CXXFLAGS += -O3 -g -fPIC -Wall -pedantic -ansi

all: $(TARGETS)

INIREAD_OBJ = inireader.o config_file.o
SNES_REVERB_OBJ = snes_reverb.o freeverb.o $(INIREAD_OBJ)
SNES_WAH_OBJ = snes_wah.o wahwah.o $(INIREAD_OBJ)
SNES_PHASER_OBJ = snes_phaser.o phaser.o $(INIREAD_OBJ)
SNES_IIR_OBJ = snes_iir.o iirfilters.o $(INIREAD_OBJ)
SNES_ECHO_OBJ = snes_echo.o echo.o $(INIREAD_OBJ)

snes_reverb.so: $(SNES_REVERB_OBJ)
	$(CXX) -o $@ $(SNES_REVERB_OBJ) -shared -Wl,--no-undefined

snes_wah.so: $(SNES_WAH_OBJ)
	$(CXX) -o $@ $(SNES_WAH_OBJ) -shared -Wl,--no-undefined

snes_phaser.so: $(SNES_PHASER_OBJ)
	$(CXX) -o $@ $(SNES_PHASER_OBJ) -shared -Wl,--no-undefined

snes_iir.so: $(SNES_IIR_OBJ)
	$(CXX) -o $@ $(SNES_IIR_OBJ) -shared -Wl,--no-undefined

snes_echo.so: $(SNES_ECHO_OBJ)
	$(CXX) -o $@ $(SNES_ECHO_OBJ) -shared -Wl,--no-undefined

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

clean:
	rm -f *.so
	rm -f *.o

.PHONY: clean
