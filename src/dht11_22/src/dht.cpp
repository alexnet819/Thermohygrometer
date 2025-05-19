#include "dht.hpp"

// Define wait_for_pin_state as a static private member of the Dht class
bool Dht::wait_for_pin_state(uint pin, bool state, uint timeout_us) {
    uint64_t start_time = time_us_64();
    while (gpio_get(pin) != state) {
        if (time_us_64() - start_time > timeout_us) {
            return false; // Timeout
        }
        // We could use sleep_us(1) instead of busy waiting,
        // but DHT timing is critical, so we stick with busy waiting here
    }
    return true;
}

Dht::Dht(uint pin, SensorType type) : dht_pin(pin), sensor_type(type) {
    // Check pin number range
    if (pin < 0 || pin > 28) { // Pin numbers range from 0 to 28
        printf("Invalid GPIO pin number: %u. Please specify a number between 0 and 28.\n", pin);
        return;
    }
    gpio_init(this->dht_pin);
}

// Function to read data from DHT sensor
// Returns true on success, false on failure
bool Dht::read_from_dht(dht_reading *result) {
    uint8_t data[5] = {0, 0, 0, 0, 0};

    // Phase 1: Send start signal
    gpio_set_dir(this->dht_pin, GPIO_OUT);
    gpio_put(this->dht_pin, 0); // Set LOW
    if (this->sensor_type == SensorType::SENSOR_TYPE_DHT11) {
        sleep_ms(18); // DHT11 requires at least 18ms LOW pulse
    } else if (this->sensor_type == SensorType::SENSOR_TYPE_DHT22_AM2302) {
        sleep_ms(1);
    } else {
        printf("Invalid sensor type.\n");
        return false;
    }
    
    gpio_put(this->dht_pin, 1);
    gpio_set_dir(this->dht_pin, GPIO_IN); // Switch to input
    gpio_pull_up(this->dht_pin);          // Enable pull-up resistor
    sleep_us(40); // Margin before response (DHT will pull LOW after 20-40us)

    // Phase 2: Wait for DHT sensor response
    // Wait for DHT to pull bus LOW (80us)
    if (!wait_for_pin_state(this->dht_pin, 0, DHT_READ_TIMEOUT_US + 50)) { // Slightly increase timeout value
        printf("DHT response timeout (waiting for LOW)\n");
        return false;
    }
    // Wait for DHT to pull bus HIGH (80us)
    if (!wait_for_pin_state(this->dht_pin, 1, DHT_READ_TIMEOUT_US + 50)) {
        printf("DHT response timeout (waiting for HIGH)\n");
        return false;
    }
    // Wait for DHT to pull bus LOW for next bit (start of data)
    if (!wait_for_pin_state(this->dht_pin, 0, DHT_READ_TIMEOUT_US + 50)) {
        printf("DHT data start timeout (waiting for LOW)\n");
        return false;
    }

    // Phase 3: Read data bits
    for (int i = 0; i < DHT_DATA_BITS; i++) {
        // Start of each bit (50us LOW)
        if (!wait_for_pin_state(this->dht_pin, 1, DHT_READ_TIMEOUT_US)) {
            printf("Data bit %d timeout (waiting for HIGH)\n", i);
            return false;
        }
        uint64_t high_pulse_start_time = time_us_64();
        // End of each bit (HIGH pulse followed by LOW for next bit)
        if (!wait_for_pin_state(this->dht_pin, 0, DHT_READ_TIMEOUT_US)) {
            printf("Data bit %d timeout (waiting for LOW after)\n", i);
            return false;
        }
        uint32_t high_pulse_duration_us = time_us_64() - high_pulse_start_time;

        data[i / 8] <<= 1;
        // If HIGH pulse is longer than ~40-50us, interpret as '1'
        // DHT11/22: 0 bit 26-28us HIGH, 1 bit 70us HIGH
        // Threshold around mid-value (e.g., (28+70)/2 = 49)
        if (high_pulse_duration_us > 48) { // Adjust threshold
            data[i / 8] |= 1;
        }
    }

    // Phase 4: Verify checksum
    uint8_t checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
    if (checksum != data[4]) {
        printf("Checksum error: Calculated %02X, Got %02X\n", checksum, data[4]);
        // printf("Data: %02X %02X %02X %02X Checksum: %02X\n", data[0], data[1], data[2], data[3], data[4]);
        return false;
    }

    // Phase 5: Decode data
    if (this->sensor_type == SensorType::SENSOR_TYPE_DHT22_AM2302) {
        result->humidity = (float)((data[0] << 8) | data[1]) / 10.0f;
        float temp_val = (float)(((data[2] & 0x7F) << 8) | data[3]) / 10.0f;
        if (data[2] & 0x80) { // If MSB of temperature byte is 1, it's negative
            temp_val *= -1;
        }
        result->temperature_celsius = temp_val;
    } else if(this->sensor_type == SensorType::SENSOR_TYPE_DHT11){ // SENSOR_TYPE_DHT11
        result->humidity = (float)data[0]; // Humidity is data[0] (integer part)
        result->humidity += (float)data[1] / 10.0f; // Consider decimal part if needed
        result->temperature_celsius = (float)data[2]; // Temperature is data[2] (integer part)
        result->temperature_celsius += (float)data[3] / 10.0f;
        // DHT11 does not support negative temperatures
    
    } else {
        printf("Invalid sensor type.\n");
        return false;
    }

    return true;
}

bool Dht::read(dht_reading *result) {
    return read_from_dht(result);
}