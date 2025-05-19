#pragma once

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/time.h"
#include <stdio.h>

// Define sensor types
enum class SensorType { // enum class is recommended
    SENSOR_TYPE_DHT11 = 0,
    SENSOR_TYPE_DHT22_AM2302 = 1
};

// Structure for humidity and temperature readings
struct dht_reading {
    float humidity;
    float temperature_celsius;
};


class Dht { // Class name changed to Dht
public:
    Dht(uint pin, SensorType type = SensorType::SENSOR_TYPE_DHT11);

    bool read(dht_reading *result);

private:
    uint dht_pin;
    SensorType sensor_type;
    // Timeout for waiting for pin state transitions (microseconds)
    const uint DHT_READ_TIMEOUT_US = 100; // For DHT22, 200us might be safer
    // Number of data bits sent by DHT sensor
    const uint DHT_DATA_BITS = 40;
    
    // Function to read data from DHT sensor (private method)
    bool read_from_dht(dht_reading *result);
    // Helper function (private static method or static function in cpp file)
    static bool wait_for_pin_state(uint pin, bool state, uint timeout_us);
};
