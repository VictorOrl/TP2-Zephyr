// main.c
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/device.h>
#include "../inc/lcd_screen_i2c.h"

#define BUZZER_PIN DT_ALIAS(buzzer)
#define LED_ORANGE_NODE DT_ALIAS(led_orange)
#define AFFICHEUR_NODE DT_ALIAS(afficheur_print)
// #define BUTTON1_NODE DT_ALIAS(btn1)
// #define BUTTON2_NODE DT_ALIAS(btn2)
#define BUZZER_TOGGLE_PERIOD K_MSEC(1)
#define ALARM_NODE DT_ALIAS(alarme)
#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
    ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

static bool alarmeOK = false;
static bool alarmeNOK = false;

const struct gpio_dt_spec presence_sensor_gpio = GPIO_DT_SPEC_GET_OR(ALARM_NODE, gpios, {0});
// const struct gpio_dt_spec button_gpio = GPIO_DT_SPEC_GET_OR(BUTTON_NODE, gpios, {0});
const struct gpio_dt_spec buzzer_gpio = GPIO_DT_SPEC_GET_OR(BUZZER_PIN, gpios, {0});
const struct gpio_dt_spec led_orange_gpio = GPIO_DT_SPEC_GET_OR(LED_ORANGE_NODE, gpios, {0});
const struct i2c_dt_spec afficheur = I2C_DT_SPEC_GET(AFFICHEUR_NODE);
const struct device *const dht11 = DEVICE_DT_GET_ONE(aosong_dht);

static struct k_work button_work;
// static const struct adc_dt_spec adc_channels[]={
//  DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user),io_channels,DT_SPEC_AND_COMMA);}
static struct gpio_callback button_callback_data;
static struct gpio_callback button_callback_data2;

void button_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    k_work_submit(&button_work);
}

/*void button_pressed_handler(struct k_work *work)
{
    write_lcd(&afficheur,"", LCD_LINE_1);
    write_lcd(&afficheur,"", LCD_LINE_2);
    write_lcd(&afficheur, "MONNNSTREEE !!", LCD_LINE_1);
}*/

void button_pressed()
{
    printf("bouton 16 pressé !\n");
    alarmeOK = true;
}
void button_pressed2()
{
    printf("bouton 27 pressé !\n");
    alarmeNOK = true;
}
////////////////////////////////

int main(void)
{

    gpio_pin_configure_dt(&led_orange_gpio, GPIO_OUTPUT_HIGH);

    // gpio_pin_configure_dt(&button_gpio, GPIO_INPUT);
    // gpio_pin_interrupt_configure_dt(&button_gpio, GPIO_INT_EDGE_TO_ACTIVE);

    // static struct gpio_callback button_cb_data;
    // gpio_init_callback(&button_cb_data, button_isr, BIT(button_gpio.pin));
    // gpio_add_callback(button_gpio.port, &button_cb_data);

    // k_work_init(&button_work, button_pressed_handler);

    ///////////////////////////////////////////
    while (5)
    {
        k_sleep(K_SECONDS(1));
        struct sensor_value temp, humidity;

        sensor_sample_fetch(dht11);
        sensor_channel_get(dht11, SENSOR_CHAN_AMBIENT_TEMP, &temp);
        sensor_channel_get(dht11, SENSOR_CHAN_HUMIDITY, &humidity);

        printk("temp: %d.%06d; humidity: %d.%06d\n", temp.val1, temp.val2, humidity.val1, humidity.val2);
        // double temp_d = sensor_value_to_double(&temp);
        // double humidity_d = sensor_value_to_double(&humidity);
        // printf('test');
        // printf("Temp: %d ; Humidity %d %% ",temp.val1,humidity.val1);
    }
}

////////////////////

void alarm_button_thread()
{
    int count = 0;

    const struct gpio_dt_spec button_gpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(btn1), gpios, {0});

    gpio_pin_configure(button_gpio.port, button_gpio.pin, GPIO_INPUT | button_gpio.dt_flags);
    gpio_init_callback(&button_callback_data, button_pressed, BIT(button_gpio.pin));
    gpio_add_callback(button_gpio.port, &button_callback_data);
    gpio_pin_interrupt_configure(button_gpio.port, button_gpio.pin, GPIO_INT_EDGE_TO_ACTIVE);

    const struct gpio_dt_spec button_gpio2 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(btn2), gpios, {0});

    gpio_pin_configure(button_gpio2.port, button_gpio2.pin, GPIO_INPUT | button_gpio2.dt_flags);
    gpio_init_callback(&button_callback_data2, button_pressed2, BIT(button_gpio2.pin));
    gpio_add_callback(button_gpio2.port, &button_callback_data2);
    gpio_pin_interrupt_configure(button_gpio2.port, button_gpio2.pin, GPIO_INT_EDGE_TO_ACTIVE);

    gpio_pin_configure_dt(&buzzer_gpio, GPIO_OUTPUT_LOW);
    init_lcd(&afficheur);
    while (1)
    {
        gpio_pin_configure_dt(&presence_sensor_gpio, GPIO_INPUT);
        // Lire la valeur du capteur de présence
        int presence = gpio_pin_get(presence_sensor_gpio.port, presence_sensor_gpio.pin);
        if (alarmeOK)
        {
            // write_lcd(&afficheur, MODE_ALARME, LCD_LINE_1);

            if (presence)
            {
                printf("Présence détectée\n");
                gpio_pin_set_dt(&buzzer_gpio, 1);
                k_sleep(BUZZER_TOGGLE_PERIOD);K_THREAD_DEFINE(alarm_button_id, 521, alarm_button_thread, NULL, NULL, NULL, 9, 0, 0);

                gpio_pin_set_dt(&buzzer_gpio, 0);
                k_sleep(BUZZER_TOGGLE_PERIOD);

                count++;

                if (count == 5)K_THREAD_DEFINE(alarm_button_id, 521, alarm_button_thread, NULL, NULL, NULL, 9, 0, 0);

                {
                    write_lcd(&afficheur, " ATTENTION !!!! ", LCD_LINE_1);
                    write_lcd(&afficheur, "DANGER INTRU !!!", LCD_LINE_2);
                }
            }
            else
            {
                printf("Pas de présence\n");
                write_lcd(&afficheur, "   Mode Alarme  ", LCD_LINE_1);
                write_lcd(&afficheur, "     Active !   ", LCD_LINE_2);
                count = 0;
            }
        }

        if (alarmeNOK)
        {
            write_lcd(&afficheur, " BIENVENUE CHEZ ", LCD_LINE_1);
            write_lcd(&afficheur, " NATH ET VICO ! ", LCD_LINE_2);
            alarmeOK = false;
            alarmeNOK = false;
        }

    }
}

K_THREAD_DEFINE(alarm_button_id, 521, alarm_button_thread, NULL, NULL, NULL, 9, 0, 0);

////////////////////////////////
