MCU     = atmega328p
F_CPU   = 16000000UL
TARGET  = app
BUILD_DIR = build

CC      = avr-gcc
OBJCOPY = avr-objcopy
SIZE    = avr-size
AVRDUDE = avrdude

PROGRAMMER = arduino
PORT       = COM3
BAUD       = 115200

CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall -Wextra -std=c11 \
         -Iinclude -Ihal -Idrivers/gpio -Idrivers/uart -Isrc -Idrivers/millis -Idrivers/keypad

ELF = $(BUILD_DIR)/$(TARGET).elf
HEX = $(BUILD_DIR)/$(TARGET).hex
MAP = $(BUILD_DIR)/$(TARGET).map

OBJS = \
	$(BUILD_DIR)/main.o \
	$(BUILD_DIR)/app.o \
	$(BUILD_DIR)/gpio.o \
	$(BUILD_DIR)/uart.o \
	$(BUILD_DIR)/ring_buffer.o \
	$(BUILD_DIR)/millis.o \
	$(BUILD_DIR)/keypad.o

all: $(HEX)

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	$(SIZE) $<

$(ELF): $(OBJS)
	$(CC) $(CFLAGS) -Wl,-Map=$(MAP) -o $@ $^

$(BUILD_DIR)/main.o: src/main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/app.o: src/app.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/gpio.o: drivers/gpio/gpio.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/uart.o: drivers/uart/uart.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/ring_buffer.o: drivers/uart/ring_buffer.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/millis.o: drivers/millis/millis.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/keypad.o: drivers/keypad/keypad.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"

flash: $(HEX)
	$(AVRDUDE) -c $(PROGRAMMER) -p m328p -P $(PORT) -b $(BAUD) -U flash:w:$(HEX):i

clean:
	@if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)

.PHONY: all clean flash
