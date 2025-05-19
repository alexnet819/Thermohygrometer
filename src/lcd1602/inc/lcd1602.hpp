#pragma once

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"

// LCD commands
#define LCD_CLEARDISPLAY   0x01
#define LCD_RETURNHOME     0x02
#define LCD_ENTRYMODESET   0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT    0x10
#define LCD_FUNCTIONSET    0x20
#define LCD_SETCGRAMADDR   0x40
#define LCD_SETDDRAMADDR   0x80

// Display entry mode flags
#define LCD_ENTRYRIGHT          0x00
#define LCD_ENTRYLEFT           0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// Display control flags
#define LCD_DISPLAYON  0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON   0x02
#define LCD_CURSOROFF  0x00
#define LCD_BLINKON    0x01
#define LCD_BLINKOFF   0x00

// Function set flags
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE    0x08
#define LCD_1LINE    0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS  0x00

// I2C backpack pin assignments
#define LCD_BACKLIGHT   0x08
#define LCD_NOBACKLIGHT 0x00
#define LCD_ENABLE      0x04
#define LCD_RW          0x02
#define LCD_RS          0x01

// Default I2C address for TC1602B01
const int DEFAULT_I2C_ADDR = 0x27; // Used within the class

// LCD configuration
#define LCD_ROWS 2
#define LCD_COLS 16

// Debug mode (outputs detailed logs)
#define DEBUG_MODE 1


class Lcd1602 {
public:
    // Constructor accepts I2C instance and optionally address
    // Default SDA/SCL pins are used based on i2c_instance
    Lcd1602(i2c_inst_t* i2c_instance, int i2c_addr = -1, uint sda_pin = PICO_DEFAULT_I2C_SDA_PIN, uint scl_pin = PICO_DEFAULT_I2C_SCL_PIN);
    bool init(uint32_t i2c_baudrate = 100 * 1000); // Also initializes I2C bus. Baud rate can be specified.
    void clear();
    void home();
    void backlight(bool on);
    void set_cursor(uint8_t row, uint8_t col);
    void print(const char *str);
    void command(uint8_t value);
    void write(uint8_t value);

private:
    bool i2c_write_byte(uint8_t val);
    void pulse_enable(uint8_t data);
    void write_4bits(uint8_t value_nibble, uint8_t mode_rs = 0); // Explicitly added mode argument
    void send_byte(uint8_t value, uint8_t mode);
    bool find_device_address(); // Scan and set I2C device address

    i2c_inst_t* _i2c_instance;
    int _lcd_addr;
    uint8_t _backlight_val;
    bool _initialized;
    uint _sda_pin;
    uint _scl_pin;
};
