#include <gpiod.h>     // libgpiod 函式庫頭文件
#include <stdio.h>     // 標準輸入輸出函式庫
#include <unistd.h>    // 用於 usleep 函式
#include <string.h>    // 用於 strerror 函式
#include <errno.h>     // 用於 errno 變數

// 定義 GPIO 相關參數
#define GPIO_CHIP_NAME "gpiochip0" // 樹莓派上 GPIO 控制器的名稱，通常是 gpiochip0
#define LED_PIN_BCM 18             // 你要控制的 LED 連接的 BCM GPIO 腳位，請根據你的實際連接修改！
#define BLINK_FREQUENCY_HZ 5       // 閃爍頻率 (Hz)
#define PERIOD_MS (1000 / BLINK_FREQUENCY_HZ) // 每個週期毫秒數
#define HALF_PERIOD_US (PERIOD_MS * 1000 / 2) // 半個週期微秒數 (亮或滅的時間)

int main() {
    struct gpiod_chip *chip;   // GPIO 晶片物件
    struct gpiod_line *line;   // GPIO 線路物件
    int ret;                   // 函式回傳值
    int value = 0;             // LED 狀態 (0: 滅, 1: 亮)

    printf("Starting LED blink program (5Hz) on BCM GPIO %d...\n", LED_PIN_BCM);

    // 1. 打開 GPIO 晶片
    chip = gpiod_chip_open_by_name(GPIO_CHIP_NAME);
    if (!chip) {
        fprintf(stderr, "Error opening GPIO chip %s: %s\n", GPIO_CHIP_NAME, strerror(errno));
        return 1;
    }
    printf("GPIO chip '%s' opened successfully.\n", GPIO_CHIP_NAME);

    // 2. 取得 GPIO 線路
    line = gpiod_chip_get_line(chip, LED_PIN_BCM);
    if (!line) {
        fprintf(stderr, "Error getting line %d from chip %s: %s\n", LED_PIN_BCM, GPIO_CHIP_NAME, strerror(errno));
        gpiod_chip_close(chip);
        return 1;
    }
    printf("GPIO line BCM %d acquired.\n", LED_PIN_BCM);

    // 3. 請求 GPIO 線路作為輸出
    // "led_blink" 是請求此 GPIO 的名稱，你可以自定義
    ret = gpiod_line_request_output(line, "led_blink", 0); // 初始設定為低電位 (LED 滅)
    if (ret < 0) {
        fprintf(stderr, "Error requesting line as output: %s\n", strerror(errno));
        gpiod_line_release(line);
        gpiod_chip_close(chip);
        return 1;
    }
    printf("GPIO line BCM %d requested as output.\n", LED_PIN_BCM);

    // 4. LED 閃爍迴圈
    printf("LED blinking at %d Hz. Press Ctrl+C to stop.\n", BLINK_FREQUENCY_HZ);
    while (1) {
        // 設定 GPIO 輸出高電位 (LED 亮)
        ret = gpiod_line_set_value(line, 1);
        if (ret < 0) {
            fprintf(stderr, "Error setting line value to 1: %s\n", strerror(errno));
            break; // 發生錯誤則跳出迴圈
        }
        usleep(HALF_PERIOD_US); // 亮半個週期

        // 設定 GPIO 輸出低電位 (LED 滅)
        ret = gpiod_line_set_value(line, 0);
        if (ret < 0) {
            fprintf(stderr, "Error setting line value to 0: %s\n", strerror(errno));
            break; // 發生錯誤則跳出迴圈
        }
        usleep(HALF_PERIOD_US); // 滅半個週期
    }

    // 5. 釋放 GPIO 線路和晶片 (此段程式碼通常因無限迴圈而無法執行，
    //    但良好的習慣是包含清理資源的部分)
    printf("Releasing GPIO line and chip...\n");
    gpiod_line_release(line);
    gpiod_chip_close(chip);

    printf("Program terminated.\n");
    return 0;
}

