#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/device.h>   

#include "../inc/lcd_screen_i2c.h"

#define BUTTON1_NODE DT_ALIAS(btn_1)
#define BUZZER_NODE DT_ALIAS(buz)

#define LED_ORANGE_NODE DT_ALIAS(led_orange)
#define AFFICHEUR_NODE DT_ALIAS(afficheur_print)

const struct gpio_dt_spec led_orange_gpio = GPIO_DT_SPEC_GET_OR(LED_ORANGE_NODE, gpios, {0});
const struct i2c_dt_spec afficheur = I2C_DT_SPEC_GET(AFFICHEUR_NODE);
const struct device *const dht11 = DEVICE_DT_GET_ONE(aosong_dht);

volatile bool buzzer_state = false;  // Variable pour le statut du buzzer

#if 0 // Ici, j'ai désactivé la fonction car elle ne semble pas être utilisée pour le moment.
void button1_irq_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    buzzer_state = !buzzer_state;
    //gpio_pin_set(buzzer_gpio.port, buzzer_gpio.pin, buzzer_state);
    //printk("Buzzer state changed to: %s\n", buzzer_state ? "ON" : "OFF");
}
#endif

int main(void) 
{
	gpio_pin_configure_dt(&led_orange_gpio, GPIO_OUTPUT_HIGH);
	
    // Initialize the LCD
    init_lcd(&afficheur);

    // Write "COucou" to the LCD
    write_lcd(&afficheur, HELLO_MSG, LCD_LINE_1);

    k_sleep(K_SECONDS(1));
    struct sensor_value temp, humidity;

    sensor_sample_fetch(dht11);
    sensor_channel_get(dht11, SENSOR_CHAN_AMBIENT_TEMP, &temp);
    sensor_channel_get(dht11, SENSOR_CHAN_HUMIDITY, &humidity);

    double temp_d = sensor_value_to_double(&temp);
    double humidity_d = sensor_value_to_double(&humidity);
    
    printk("Temp: %d ; Humidity %d %% ", temp.val1, humidity.val1);

    
    k_sleep(K_SECONDS(5));
    
}
