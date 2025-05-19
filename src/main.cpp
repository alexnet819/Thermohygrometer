#include <stdio.h>
#include "pico/stdlib.h"
#include "lcd1602.hpp" // LCDライブラリのヘッダー
#include "dht.hpp"     // DHTライブラリのヘッダー

// DHTセンサーの設定
const uint DHT_DATA_PIN = 15; // DHTセンサーのデータピン (例: GP15)
// dht11を使用する
const SensorType DHT_SENSOR_TYPE = SensorType::SENSOR_TYPE_DHT11; // SENSOR_TYPE_DHT11

int main(){
    stdio_init_all(); // シリアル通信の初期化

    sleep_ms(5000); // 起動待機
    printf("Thermohygrometer Program Started\n");
    printf("PICO_DEFAULT_I2C_SDA_PIN: %d\n", PICO_DEFAULT_I2C_SDA_PIN);
    printf("PICO_DEFAULT_I2C_SCL_PIN: %d\n", PICO_DEFAULT_I2C_SCL_PIN);


    // LCDの初期化
    // デフォルトのSDA(GP4), SCL(GP5)ピン、I2Cアドレス自動検出で初期化
    Lcd1602 lcd(i2c0); 
    // もしSDA/SCLピンが異なる場合は、以下のように明示的に指定してください
    // 例: Lcd1602 lcd(i2c0, -1, YOUR_SDA_PIN, YOUR_SCL_PIN);

    printf("Initializing LCD...\n");
    if (!lcd.init(100 * 1000)) { // 100kHzでI2CバスとLCDを初期化
        printf("LCD initialization FAILED. Check wiring and I2C address.\n");
        lcd.backlight(true);
        lcd.print("Init Failed!");
        while(1) {
            printf("Looping due to LCD init failure.\n");
            sleep_ms(1000);
        }
    }
    
    printf("LCD Initialized SUCCEEDED.\n");
    lcd.print("LCD Initialized SUCCEEDED.");
    sleep_ms(1000); // 1秒待機
    lcd.clear();
    lcd.set_cursor(0,0);
    lcd.print("LCD OK");
    sleep_ms(2000); // "LCD OK" を2秒表示

    // DHTセンサーの初期化
    Dht dht_sensor(DHT_DATA_PIN, DHT_SENSOR_TYPE);
    // DHTセンサーは init() のような明示的な初期化関数は持ちませんが、
    // コンストラクタでピンが初期化されます。
    printf("DHT Sensor Initialized on pin %d\n", DHT_DATA_PIN);

    sleep_ms(2000); // センサー安定待ち

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
            
            lcd.clear(); // 表示前にクリア
            lcd.set_cursor(0,0);
            lcd.print(lcd_buffer_line1);
            lcd.set_cursor(1,0);
            lcd.print(lcd_buffer_line2);

        } else {
            printf("DHTセンサーの読み取りに失敗しました。\n");
            lcd.clear();
            lcd.set_cursor(0,0);
            lcd.print("DHT Read Error");
        }

        // DHT22は2秒以上の間隔で読み取り推奨
        // DHT11は1秒以上の間隔
        if (DHT_SENSOR_TYPE == SensorType::SENSOR_TYPE_DHT11) {
            sleep_ms(1500); // 1.5秒ウェイト
        } else {
            sleep_ms(2500); // 2.5秒ウェイト
        }
    }

    return 0;
}