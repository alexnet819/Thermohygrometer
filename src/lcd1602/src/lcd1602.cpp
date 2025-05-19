#include "lcd1602.hpp"

Lcd1602::Lcd1602(i2c_inst_t* i2c_instance, int i2c_addr, uint sda_pin, uint scl_pin)
    : _i2c_instance(i2c_instance), _lcd_addr(i2c_addr), 
      _backlight_val(LCD_BACKLIGHT), _initialized(false),
      _sda_pin(sda_pin), _scl_pin(scl_pin) {
}

bool Lcd1602::find_device_address() {
    if (DEBUG_MODE) {
        printf("Starting I2C device scan (inside Lcd1602 class)...\n");
    }
    bool device_found_at_specific_addresses = false;
    for (int addr_scan = 0; addr_scan < 128; addr_scan++) {
        uint8_t rxdata;
        int ret = i2c_read_blocking(_i2c_instance, addr_scan, &rxdata, 1, false);
        if (ret >= 0) {
            if (DEBUG_MODE) {
                printf("I2C device detected: 0x%02X\n", addr_scan);
            }
            if (addr_scan == 0x27 || addr_scan == 0x3F) {
                _lcd_addr = addr_scan;
                if (DEBUG_MODE) {
                    printf("LCD address set internally: 0x%02X\n", _lcd_addr);
                }
                device_found_at_specific_addresses = true;
                break; // Use the first common LCD address found
            }
        }
    }
    if (!device_found_at_specific_addresses && _lcd_addr == -1) { // If no address specified and not found during scan
         _lcd_addr = DEFAULT_I2C_ADDR; // Try default address
        if (DEBUG_MODE) {
            printf("No known LCD address found. Trying default address 0x%02X.\n", _lcd_addr);
        }
        // Simple test to check if communication is possible with default address (optional)
        uint8_t dummy_data = _backlight_val; // Try writing backlight state
        int test_write = i2c_write_blocking(_i2c_instance, _lcd_addr, &dummy_data, 1, false);
        if (test_write < 0) {
            if (DEBUG_MODE) {
                printf("Communication failed with default address 0x%02X.\n", _lcd_addr);
            }
            return false; // Communication failed even with default address
        }
         if (DEBUG_MODE) {
            printf("Communication attempt succeeded with default address 0x%02X.\n", _lcd_addr);
        }
    } else if (!device_found_at_specific_addresses && _lcd_addr != -1) {
        // If address was specified but no response during scan (will be determined when sending commands in init)
        if (DEBUG_MODE) {
             printf("No device found at specified address 0x%02X during scan. Continuing with specified address.\n", _lcd_addr);
        }
    }


    if (_lcd_addr == -1) { // If address couldn't be determined
        if (DEBUG_MODE) {
            printf("Could not determine LCD address.\n");
        }
        return false;
    }
    return true;
}


bool Lcd1602::init(uint32_t i2c_baudrate) {
    // Initialize I2C bus and GPIO pins
    if (DEBUG_MODE) {
        printf("Initializing I2C bus (Lcd1602::init)...\n");
        printf("SDA pin: %d, SCL pin: %d, Baud rate: %lu Hz\n", _sda_pin, _scl_pin, i2c_baudrate);
    }
    i2c_init(_i2c_instance, i2c_baudrate);
    gpio_set_function(_sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(_scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(_sda_pin);
    gpio_pull_up(_scl_pin);
    // Short wait after I2C initialization
    sleep_ms(10);


    if (_lcd_addr == -1) { // If address wasn't specified in constructor
        if (!find_device_address()) {
            if (DEBUG_MODE) {
                printf("Failed to identify LCD device address. Aborting initialization.\n");
            }
            return false;
        }
    }

    // Increased delay before initialization
    sleep_ms(150);
    
    // Turn backlight ON
    this->backlight(true); // Call the class backlight method
    sleep_ms(200); // Extended wait time
    
    if (DEBUG_MODE) {
        printf("Starting LCD initialization sequence...\n");
    }
    
    // Improved initialization sequence
    // Send 8-bit mode command 3 times
    write_4bits(0x03 << 4, 0);  // mode_rs = 0 (command)
    sleep_ms(50);  // Extended delay time
    
    write_4bits(0x03 << 4, 0);  // mode_rs = 0 (command)
    sleep_ms(5);
    
    write_4bits(0x03 << 4, 0);  // mode_rs = 0 (command)
    sleep_ms(5);
    
    // Switch to 4-bit mode
    write_4bits(0x02 << 4, 0);  // mode_rs = 0 (command)
    sleep_ms(10);
    
    if (DEBUG_MODE) {
        printf("4-bit mode configuration complete\n");
    }
    
    // Function set
    command(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS);
    sleep_ms(5);
    
    // Display control
    command(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
    sleep_ms(5);
    
    // Entry mode set
    command(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
    sleep_ms(5);
    
    // Clear display
    clear();
    
    // Return to home position
    home();
    
    if (DEBUG_MODE) {
        printf("LCD initialization complete\n");
    }
    _initialized = true;
    return true;
}

bool Lcd1602::i2c_write_byte(uint8_t val) {
    int result = i2c_write_blocking(_i2c_instance, _lcd_addr, &val, 1, false);
    
    if (DEBUG_MODE && result < 0) {
        printf("I2C write failed: 0x%02X, Address: 0x%02X, Error: %d\n", val, _lcd_addr, result);
    }
    return (result > 0);
}

void Lcd1602::write_4bits(uint8_t value_nibble, uint8_t mode_rs) {
    // Similar to the original code, simply send data and then operate enable
    uint8_t data_to_send = value_nibble | mode_rs | _backlight_val;
    i2c_write_byte(data_to_send);
    pulse_enable(data_to_send);
}

void Lcd1602::pulse_enable(uint8_t data) {
    // Precisely control enable signal pulse width
    i2c_write_byte(data | LCD_ENABLE);
    sleep_us(1);  // Even short pulses are recognized (minimum 1us)
    i2c_write_byte(data & ~LCD_ENABLE);
    sleep_us(50); // Wait time for command processing
}

void Lcd1602::send_byte(uint8_t value, uint8_t mode) {
    // Match the original code's implementation method completely
    uint8_t high_nibble = (value & 0xF0);
    uint8_t low_nibble = ((value << 4) & 0xF0);
    
    write_4bits(high_nibble, mode); // Pass 0 for mode as it's already included in high_nibble
    write_4bits(low_nibble, mode); // Pass 0 for mode as it's already included in low_nibble
}

void Lcd1602::command(uint8_t value) {
    // Allow some commands to be executed even before initialization (especially backlight related)
    send_byte(value, 0);
}

void Lcd1602::write(uint8_t value) {
    // Allow characters to be sent even before initialization (for debugging)
    send_byte(value, LCD_RS);
}

void Lcd1602::clear() {
    if (!_initialized) return;
    command(LCD_CLEARDISPLAY);
    sleep_ms(5);
}

void Lcd1602::home() {
    if (!_initialized) return;
    command(LCD_RETURNHOME);
    sleep_ms(5);
}

void Lcd1602::backlight(bool on) {
    _backlight_val = on ? LCD_BACKLIGHT : LCD_NOBACKLIGHT;
    // Send backlight state alone
    i2c_write_byte(_backlight_val);
    sleep_ms(5); // Wait a bit after changing backlight state
}

void Lcd1602::set_cursor(uint8_t row, uint8_t col) {
    if (!_initialized) return;
    static uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54}; // For 2 lines
    if (row >= LCD_ROWS) {
        row = LCD_ROWS - 1; 
    }
    if (col >= LCD_COLS) {
        col = LCD_COLS - 1;
    }
    command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

void Lcd1602::print(const char *str) {
    if (!_initialized) return;
    while (*str) {
        write(*str++);
    }
}