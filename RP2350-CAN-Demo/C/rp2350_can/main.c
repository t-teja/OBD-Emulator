#include <stdio.h>
#include "pico/stdlib.h"
#include "xl2515.h"

#define LED_PIN   25
int main()
{
    uint32_t id = 0x123;
    uint8_t data[8] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
    uint8_t recv_data[8];
    uint8_t recv_len = 0;
    bool led_state = false;
    stdio_init_all(); 
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, led_state);
    xl2515_init(KBPS125);
    while (true) {
        printf("Hello, world!\n");
        xl2515_send(id, data, 8);
        if (xl2515_recv(id, recv_data, &recv_len))
        {
            printf("recv: ");
            for (uint8_t i = 0; i < recv_len; i++)
            {
                printf("%02x ", recv_data[i]);
            }
            printf("\r\n");
        }
        led_state = !led_state;
        gpio_put(LED_PIN, led_state);
        sleep_ms(1000);
    }
}
