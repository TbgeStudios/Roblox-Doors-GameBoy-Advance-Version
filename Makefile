DEVKITPRO := /opt/devkitpro
DEVKITARM := $(DEVKITPRO)/devkitARM

CC := $(DEVKITARM)/bin/arm-none-eabi-gcc
CFLAGS := -mthumb -mthumb-interwork -O2 -Wall -fomit-frame-pointer
CFLAGS += -I$(DEVKITPRO)/libgba/include

LDFLAGS := -specs=gba.specs -Wl,-Map,build/doorsgba.map
LDFLAGS += -L$(DEVKITPRO)/libgba/lib -lgba -lm   # <- Added -lm here

SOURCES := main.c
OBJECTS := $(patsubst %.c, build/%.o, $(SOURCES))

OUTPUT := doorsgba.gba

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

build/%.o: %.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build *.gba *.elf *.map
