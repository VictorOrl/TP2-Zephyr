//main.c
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/device.h>   
#include "../inc/lcd_screen_i2c.h"


#define BUZZER_NODE DT_ALIAS(buz)
#define LED_ORANGE_NODE DT_ALIAS(led_orange)
#define AFFICHEUR_NODE DT_ALIAS(afficheur_print)
#define BUTTON1_NODE DT_ALIAS(btn1)

const struct gpio_dt_spec button_gpio = GPIO_DT_SPEC_GET_OR(BUTTON1_NODE, gpios, {0});
const struct gpio_dt_spec buz_gpio = GPIO_DT_SPEC_GET_OR(BUTTON1_NODE, gpios, {0});
const struct gpio_dt_spec led_orange_gpio = GPIO_DT_SPEC_GET_OR(LED_ORANGE_NODE, gpios, {0});
const struct i2c_dt_spec afficheur = I2C_DT_SPEC_GET(AFFICHEUR_NODE);
const struct device *const dht11 = DEVICE_DT_GET_ONE(aosong_dht);

static struct k_work button_work;

void button_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins) 
{
    k_work_submit(&button_work);
}

void button_pressed_handler(struct k_work *work) 
{
    write_lcd(&afficheur,"", LCD_LINE_1);
    write_lcd(&afficheur,"", LCD_LINE_2);
    write_lcd(&afficheur, "MONNNSTREEE !!", LCD_LINE_1);
}


int main(void) 
{
    init_lcd(&afficheur);
	gpio_pin_configure_dt(&led_orange_gpio, GPIO_OUTPUT_HIGH);

    gpio_pin_configure_dt(&button_gpio, GPIO_INPUT);
    gpio_pin_interrupt_configure_dt(&button_gpio, GPIO_INT_EDGE_TO_ACTIVE);

    static struct gpio_callback button_cb_data;
    gpio_init_callback(&button_cb_data, button_isr, BIT(button_gpio.pin));
    gpio_add_callback(button_gpio.port, &button_cb_data);

    k_work_init(&button_work, button_pressed_handler);
    
    while (5)
    {
    write_lcd(&afficheur, HELLO_MSG, LCD_LINE_1);
    k_sleep(K_SECONDS(1));
    struct sensor_value temp, humidity;

    sensor_sample_fetch(dht11);
    sensor_channel_get(dht11, SENSOR_CHAN_AMBIENT_TEMP, &temp);
    sensor_channel_get(dht11, SENSOR_CHAN_HUMIDITY, &humidity);

    double temp_d = sensor_value_to_double(&temp);
    double humidity_d = sensor_value_to_double(&humidity);
    
    printk("Temp: %d ; Humidity %d %% ",temp.val1,humidity.val1);
    }
    

}


