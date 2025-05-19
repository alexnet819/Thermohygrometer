#include <stdio.h>
#include "pico/stdlib.h"
#include "lcd1602.hpp" // Header for LCD library
#include "dht.hpp"     // Header for DHT sensor library

// DHT sensor configuration
const uint DHT_DATA_PIN = 15; // DHT sensor data pin (e.g. GP15)
// Using DHT11 sensor
const SensorType DHT_SENSOR_TYPE = SensorType::SENSOR_TYPE_DHT11;

int main(){
    stdio_init_all(); // Initialize serial communication

    sleep_ms(5000); // Startup delay
    printf("Thermohygrometer Program Started\n");
    printf("PICO_DEFAULT_I2C_SDA_PIN: %d\n", PICO_DEFAULT_I2C_SDA_PIN);
    printf("PICO_DEFAULT_I2C_SCL_PIN: %d\n", PICO_DEFAULT_I2C_SCL_PIN);

    // LCD initialization
    // Initialize with default SDA(GP4), SCL(GP5) pins, auto-detect I2C address
    Lcd1602 lcd(i2c0); 
    // If SDA/SCL pins are different, specify them explicitly:
    // Example: Lcd1602 lcd(i2c0, -1, YOUR_SDA_PIN, YOUR_SCL_PIN);

    printf("Initializing LCD...\n");
    if (!lcd.init(100 * 1000)) { // Initialize I2C bus and LCD at 100kHz
        printf("LCD initialization FAILED. Check wiring and I2C address.\n");
        lcd.backlight(true);
        lcd.print("Init Failed!");
        while(1) {
            printf("Looping due to LCD init failure.\n");
            sleep_ms(1000);
        }
    }
    
    printf("LCD Initialization SUCCEEDED.\n");
    lcd.print("LCD Initialized SUCCEEDED.");
    sleep_ms(1000); // Wait 1 second
    lcd.clear();
    lcd.set_cursor(0,0);
    lcd.print("LCD OK");
    sleep_ms(2000); // Show "LCD OK" for 2 seconds

    // Initialize DHT sensor
    Dht dht_sensor(DHT_DATA_PIN, DHT_SENSOR_TYPE);
    // DHT sensor doesn't have an explicit init() function,
    // pin is initialized in the constructor
    printf("DHT Sensor Initialized on pin %d\n", DHT_DATA_PIN);

    sleep_ms(2000); // Wait for sensor to stabilize

    lcd.clear();
    lcd.set_cursor(0,0);
    lcd.print("Reading DHT...");
    printf("Ready to read DHT sensor.\n");

    dht_reading current_reading;
    char lcd_buffer_line1[LCD_COLS + 1];
    char lcd_buffer_line2[LCD_COLS + 1];

    while(true){
        if (dht_sensor.read(&current_reading)) {
            printf("Temp: %.1f C, Hum: %.1f %%\n", 
                   current_reading.temperature_celsius, 
                   current_reading.humidity);

            snprintf(lcd_buffer_line1, sizeof(lcd_buffer_line1), "Temp: %5.1fC", current_reading.temperature_celsius);
            snprintf(lcd_buffer_line2, sizeof(lcd_buffer_line2), "Hum:  %5.1f%%", current_reading.humidity);
            
            lcd.clear(); // Clear before display
            lcd.set_cursor(0,0);
            lcd.print(lcd_buffer_line1);
            lcd.set_cursor(1,0);
            lcd.print(lcd_buffer_line2);

        } else {
            printf("Failed to read DHT sensor.\n");
            lcd.clear();
            lcd.set_cursor(0,0);
            lcd.print("DHT Read Error");
        }

        // DHT22 recommends at least 2 seconds between readings
        // DHT11 recommends at least 1 second
        if (DHT_SENSOR_TYPE == SensorType::SENSOR_TYPE_DHT11) {
            sleep_ms(1500); // 1.5 second wait
        } else {
            sleep_ms(2500); // 2.5 second wait
        }
    }

    return 0; // This code is never reached
}