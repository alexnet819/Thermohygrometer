#pragma once

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/time.h"
#include <stdio.h>

// 使用するセンサーのタイプを定義
enum class SensorType { // enum class を推奨
    SENSOR_TYPE_DHT11 = 0,
    SENSOR_TYPE_DHT22_AM2302 = 1
};

// 湿度と温度の構造体
struct dht_reading {
    float humidity;
    float temperature_celsius;
};


class Dht { // クラス名を Dht に変更
public:
    Dht(uint pin, SensorType type = SensorType::SENSOR_TYPE_DHT11);

    bool read(dht_reading *result);

private:
    uint dht_pin;
    SensorType sensor_type;
    // パルスの遷移を待つタイムアウト時間 (マイクロ秒)
    const uint DHT_READ_TIMEOUT_US = 100; // DHT22では200us程度が安全な場合もある
    // DHTセンサーが送信するデータビット数
    const uint DHT_DATA_BITS = 40;
    
    // DHTセンサーからデータを読み取る関数 (プライベートメソッド)
    bool read_from_dht(dht_reading *result);
    // ヘルパー関数 (プライベート静的メソッドまたはcppファイルのstatic関数)
    static bool wait_for_pin_state(uint pin, bool state, uint timeout_us);
};
