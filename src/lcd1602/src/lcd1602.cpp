#include "lcd1602.hpp"

Lcd1602::Lcd1602(i2c_inst_t* i2c_instance, int i2c_addr, uint sda_pin, uint scl_pin)
    : _i2c_instance(i2c_instance), _lcd_addr(i2c_addr), 
      _backlight_val(LCD_BACKLIGHT), _initialized(false),
      _sda_pin(sda_pin), _scl_pin(scl_pin) {
}

bool Lcd1602::find_device_address() {
    if (DEBUG_MODE) {
        printf("I2Cデバイススキャン開始 (Lcd1602クラス内部)...\n");
    }
    bool device_found_at_specific_addresses = false;
    for (int addr_scan = 0; addr_scan < 128; addr_scan++) {
        uint8_t rxdata;
        int ret = i2c_read_blocking(_i2c_instance, addr_scan, &rxdata, 1, false);
        if (ret >= 0) {
            if (DEBUG_MODE) {
                printf("I2Cデバイス検出: 0x%02X\n", addr_scan);
            }
            if (addr_scan == 0x27 || addr_scan == 0x3F) {
                _lcd_addr = addr_scan;
                if (DEBUG_MODE) {
                    printf("LCDアドレスをクラス内部で設定: 0x%02X\n", _lcd_addr);
                }
                device_found_at_specific_addresses = true;
                break; // 最初に見つかった一般的なLCDアドレスを使用
            }
        }
    }
    if (!device_found_at_specific_addresses && _lcd_addr == -1) { // アドレスが指定されておらず、スキャンでも見つからなかった場合
         _lcd_addr = DEFAULT_I2C_ADDR; // デフォルトアドレスを試す
        if (DEBUG_MODE) {
            printf("既知のLCDアドレスが見つからず。デフォルトアドレス 0x%02X を試行します。\n", _lcd_addr);
        }
        // デフォルトアドレスで通信可能か簡単なテストを行う (オプション)
        uint8_t dummy_data = _backlight_val; // バックライトの状態を書き込んでみる
        int test_write = i2c_write_blocking(_i2c_instance, _lcd_addr, &dummy_data, 1, false);
        if (test_write < 0) {
            if (DEBUG_MODE) {
                printf("デフォルトアドレス 0x%02X での通信に失敗しました。\n", _lcd_addr);
            }
            return false; // デフォルトアドレスでも通信失敗
        }
         if (DEBUG_MODE) {
            printf("デフォルトアドレス 0x%02X で通信試行成功。\n", _lcd_addr);
        }
    } else if (!device_found_at_specific_addresses && _lcd_addr != -1) {
        // アドレスが指定されていたが、そのアドレスで応答がなかった場合（この関数ではチェックしないが、initでのコマンド送信で判明する）
        if (DEBUG_MODE) {
             printf("指定されたアドレス 0x%02X でスキャン中にデバイスは見つかりませんでした。指定アドレスで続行します。\n", _lcd_addr);
        }
    }


    if (_lcd_addr == -1) { // 結局アドレスが特定できなかった場合
        if (DEBUG_MODE) {
            printf("LCDアドレスを特定できませんでした。\n");
        }
        return false;
    }
    return true;
}


bool Lcd1602::init(uint32_t i2c_baudrate) {
    // I2CバスとGPIOピンの初期化
    if (DEBUG_MODE) {
        printf("I2Cバス初期化 (Lcd1602::init)...\n");
        printf("SDAピン: %d, SCLピン: %d, ボーレート: %lu Hz\n", _sda_pin, _scl_pin, i2c_baudrate);
    }
    i2c_init(_i2c_instance, i2c_baudrate);
    gpio_set_function(_sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(_scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(_sda_pin);
    gpio_pull_up(_scl_pin);
    // I2C初期化後の短い待機
    sleep_ms(10);


    if (_lcd_addr == -1) { // コンストラクタでアドレスが指定されなかった場合
        if (!find_device_address()) {
            if (DEBUG_MODE) {
                printf("LCDデバイスアドレスの特定に失敗しました。初期化を中止します。\n");
            }
            return false;
        }
    }

    // 初期化前の遅延を増加
    sleep_ms(150);
    
    // バックライトをON
    this->backlight(true); // クラスのbacklightメソッドを呼び出す
    sleep_ms(200); // 待機時間を延長
    
    if (DEBUG_MODE) {
        printf("LCD初期化シーケンス開始...\n");
    }
    
    // 初期化シーケンス改良版
    // 8ビットモード指示を3回送信
    write_4bits(0x03 << 4, 0);  // mode_rs = 0 (コマンド) を追加
    sleep_ms(50);  // 待機時間を大幅に延長
    
    write_4bits(0x03 << 4, 0);  // mode_rs = 0 (コマンド) を追加
    sleep_ms(5);
    
    write_4bits(0x03 << 4, 0);  // mode_rs = 0 (コマンド) を追加
    sleep_ms(5);
    
    // 4ビットモードに切り替え
    write_4bits(0x02 << 4, 0);  // mode_rs = 0 (コマンド) を追加
    sleep_ms(10);
    
    if (DEBUG_MODE) {
        printf("4ビットモード設定完了\n");
    }
    
    // ファンクションセット
    command(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS);
    sleep_ms(5);
    
    // ディスプレイコントロール
    command(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
    sleep_ms(5);
    
    // エントリーモードセット
    command(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
    sleep_ms(5);
    
    // ディスプレイクリア
    clear();
    
    // ホームポジションに戻す
    home();
    
    if (DEBUG_MODE) {
        printf("LCD初期化完了\n");
    }
    _initialized = true;
    return true;
}

bool Lcd1602::i2c_write_byte(uint8_t val) {
    int result = i2c_write_blocking(_i2c_instance, _lcd_addr, &val, 1, false);
    
    if (DEBUG_MODE && result < 0) {
        printf("I2C書き込み失敗: 0x%02X, アドレス: 0x%02X, エラー: %d\n", val, _lcd_addr, result);
    }
    return (result > 0);
}

void Lcd1602::write_4bits(uint8_t value_nibble, uint8_t mode_rs) {
    // 元のコードと同様に、単純にデータを送信してからイネーブルを操作する
    uint8_t data_to_send = value_nibble | mode_rs | _backlight_val;
    i2c_write_byte(data_to_send);
    pulse_enable(data_to_send);
}

void Lcd1602::pulse_enable(uint8_t data) {
    // イネーブル信号のパルス幅を正確に制御
    i2c_write_byte(data | LCD_ENABLE);
    sleep_us(1);  // 短いパルスでも認識される（最小1us）
    i2c_write_byte(data & ~LCD_ENABLE);
    sleep_us(50); // コマンド処理のための待機時間を確保
}

void Lcd1602::send_byte(uint8_t value, uint8_t mode) {
    // 元のコードの実装方法と完全に一致させる
    uint8_t high_nibble = (value & 0xF0);
    uint8_t low_nibble = ((value << 4) & 0xF0);
    
    write_4bits(high_nibble, mode); // modeは既にhigh_nibbleに含まれているので0を渡す
    write_4bits(low_nibble, mode); // modeは既にlow_nibbleに含まれているので0を渡す
}

void Lcd1602::command(uint8_t value) {
    // 初期化前でも一部のコマンドは実行できるようにする（特にバックライト関連）
    send_byte(value, 0);
}

void Lcd1602::write(uint8_t value) {
    // 初期化前でも文字を送信できるようにする（デバッグ用）
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
    // バックライト状態だけを単独で送信
    i2c_write_byte(_backlight_val);
    sleep_ms(5); // バックライト状態変更後に少し待機
}

void Lcd1602::set_cursor(uint8_t row, uint8_t col) {
    if (!_initialized) return;
    static uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54}; // 2行用
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