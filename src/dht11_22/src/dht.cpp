#include "dht.hpp"

// wait_for_pin_state を Dht クラスの static private member として定義
bool Dht::wait_for_pin_state(uint pin, bool state, uint timeout_us) {
    uint64_t start_time = time_us_64();
    while (gpio_get(pin) != state) {
        if (time_us_64() - start_time > timeout_us) {
            return false; // タイムアウト
        }
        // 短いビジーウェイトの代わりに sleep_us(1) を挟むことも検討できるが、
        // DHTのタイミングはシビアなので、ここではビジーウェイトのままにする
    }
    return true;
}

Dht::Dht(uint pin, SensorType type) : dht_pin(pin), sensor_type(type) {
    // ピン番号の範囲チェック
    if (pin < 0 || pin > 28) { // ピン番号は0から28の範囲
        printf("無効なGPIOピン番号です: %u。0から28の範囲で指定してください。\n", pin);
        return;
    }
    gpio_init(this->dht_pin);
}


// DHTセンサーからデータを読み取る関数
// 成功した場合は true、失敗した場合は false を返す
bool Dht::read_from_dht(dht_reading *result) {
    uint8_t data[5] = {0, 0, 0, 0, 0};

    // フェーズ1: スタート信号を送信
    gpio_set_dir(this->dht_pin, GPIO_OUT);
    gpio_put(this->dht_pin, 0); // LOWに設定
    if (this->sensor_type == SensorType::SENSOR_TYPE_DHT11) {
        sleep_ms(18); // DHT11は最低18msのLOWパルスが必要
    } else if (this->sensor_type == SensorType::SENSOR_TYPE_DHT22_AM2302) {
        sleep_ms(1);
    } else {
        printf("無効なセンサータイプです。\n");
        return false;
    }
    
    gpio_put(this->dht_pin, 1);
    gpio_set_dir(this->dht_pin, GPIO_IN); // 入力に戻す
    gpio_pull_up(this->dht_pin);          // プルアップを有効にする
    sleep_us(40); // 応答までのマージン (20-40us後にDHTがLOWにする)


    // フェーズ2: DHTセンサーの応答を待機
    // DHTがバスをLOWにするのを待つ (80us)
    if (!wait_for_pin_state(this->dht_pin, 0, DHT_READ_TIMEOUT_US + 50)) { // タイムアウト値を少し増やす
        printf("DHT応答タイムアウト (LOW待ち)\n");
        return false;
    }
    // DHTがバスをHIGHにするのを待つ (80us)
    if (!wait_for_pin_state(this->dht_pin, 1, DHT_READ_TIMEOUT_US + 50)) {
        printf("DHT応答タイムアウト (HIGH待ち)\n");
        return false;
    }
    // DHTがデータ送信のためにバスをLOWにするのを待つ (次のビットの開始)
    if (!wait_for_pin_state(this->dht_pin, 0, DHT_READ_TIMEOUT_US + 50)) {
        printf("DHTデータ開始タイムアウト (LOW待ち)\n");
        return false;
    }

    // フェーズ3: データビットを読み取る
    for (int i = 0; i < DHT_DATA_BITS; i++) {
        // 各ビットの開始 (50us LOW)
        if (!wait_for_pin_state(this->dht_pin, 1, DHT_READ_TIMEOUT_US)) {
            printf("データビット %d タイムアウト (HIGH待ち)\n", i);
            return false;
        }
        uint64_t high_pulse_start_time = time_us_64();
        // 各ビットの終了 (HIGHパルスの後、次のビットのためにLOWになる)
        if (!wait_for_pin_state(this->dht_pin, 0, DHT_READ_TIMEOUT_US)) {
            printf("データビット %d タイムアウト (LOW待ち後)\n", i);
            return false;
        }
        uint32_t high_pulse_duration_us = time_us_64() - high_pulse_start_time;

        data[i / 8] <<= 1;
        // HIGHパルスが約40-50usより長い場合、'1'と判断
        // DHT11/22: 0ビット 26-28us HIGH, 1ビット 70us HIGH
        // 閾値は中間値 (例: (28+70)/2 = 49) あたり
        if (high_pulse_duration_us > 48) { // 閾値を調整
            data[i / 8] |= 1;
        }
    }

    // フェーズ4: チェックサムを検証
    uint8_t checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
    if (checksum != data[4]) {
        printf("Checksum error: Calculated %02X, Got %02X\n", checksum, data[4]);
        // printf("Data: %02X %02X %02X %02X Checksum: %02X\n", data[0], data[1], data[2], data[3], data[4]);
        return false;
    }

    // フェーズ5: データをデコード
    if (this->sensor_type == SensorType::SENSOR_TYPE_DHT22_AM2302) {
        result->humidity = (float)((data[0] << 8) | data[1]) / 10.0f;
        float temp_val = (float)(((data[2] & 0x7F) << 8) | data[3]) / 10.0f;
        if (data[2] & 0x80) { // 温度データの上位バイトのMSBが1の場合、負の温度
            temp_val *= -1;
        }
        result->temperature_celsius = temp_val;
    } else if(this->sensor_type == SensorType::SENSOR_TYPE_DHT11){ // SENSOR_TYPE_DHT11
        result->humidity = (float)data[0]; // 湿度は data[0] (整数部)
        result->humidity += (float)data[1] / 10.0f; // 必要であれば小数部も考慮
        result->temperature_celsius = (float)data[2]; // 温度は data[2] (整数部)
        result->temperature_celsius += (float)data[3] / 10.0f;
        // DHT11は負の温度をサポートしません
    
    } else {
        printf("無効なセンサータイプです。\n");
        return false;
    }

    return true;
}

bool Dht::read(dht_reading *result) {
    return read_from_dht(result);
}