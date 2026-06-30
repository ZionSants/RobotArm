#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"

#define SERVO_FREQ_HZ    50
#define SERVO_RESOLUTION LEDC_TIMER_14_BIT
#define SERVO_DUTY_MIN   819
#define SERVO_DUTY_MAX   1638

// Joystick 1
#define JOY1_X_CHANNEL   ADC_CHANNEL_5   // GPIO 33
#define JOY1_Y_CHANNEL   ADC_CHANNEL_4   // GPIO 32

// Joystick 2
#define JOY2_X_CHANNEL   ADC_CHANNEL_3   // GPIO 39
#define JOY2_Y_CHANNEL   ADC_CHANNEL_0   // GPIO 36

#define ZONA_MORTA       500
#define JOY_CENTRO       2048

typedef struct {
    int gpio;
    ledc_channel_t canal;
    int angulo_atual;
} Servo;

Servo servo_am   = { .gpio = 14, .canal = LEDC_CHANNEL_0, .angulo_atual = 90 };
Servo servo_roxo = { .gpio = 25, .canal = LEDC_CHANNEL_1, .angulo_atual = 90 };
Servo servo1     = { .gpio = 26, .canal = LEDC_CHANNEL_2, .angulo_atual = 90 };
Servo servo2     = { .gpio = 27, .canal = LEDC_CHANNEL_3, .angulo_atual = 90 };

adc_oneshot_unit_handle_t adc_handle;

// -------------------------------------------------------

uint32_t graus_para_duty(int graus) {
    return SERVO_DUTY_MIN + (graus * (SERVO_DUTY_MAX - SERVO_DUTY_MIN) / 180);
}

void servo_init(Servo *s) {
    ledc_timer_config_t timer = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,
        .timer_num       = LEDC_TIMER_0,
        .duty_resolution = SERVO_RESOLUTION,
        .freq_hz         = SERVO_FREQ_HZ,
        .clk_cfg         = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer);

    ledc_channel_config_t canal = {
        .gpio_num   = s->gpio,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = s->canal,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = graus_para_duty(s->angulo_atual),
        .hpoint     = 0
    };
    ledc_channel_config(&canal);
}

void servo_set_graus(Servo *s, int graus) {
    if (graus < 0)   graus = 0;
    if (graus > 180) graus = 180;
    s->angulo_atual = graus;
    uint32_t duty = graus_para_duty(graus);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, s->canal, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, s->canal);
}

// --- ADC ---
void joystick_init(void) {
    adc_oneshot_unit_init_cfg_t init_config = { .unit_id = ADC_UNIT_1 };
    adc_oneshot_new_unit(&init_config, &adc_handle);

    adc_oneshot_chan_cfg_t chan_config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten    = ADC_ATTEN_DB_12,
    };
    adc_oneshot_config_channel(adc_handle, JOY1_X_CHANNEL, &chan_config);
    adc_oneshot_config_channel(adc_handle, JOY1_Y_CHANNEL, &chan_config);
    adc_oneshot_config_channel(adc_handle, JOY2_X_CHANNEL, &chan_config);
    adc_oneshot_config_channel(adc_handle, JOY2_Y_CHANNEL, &chan_config);
}

int ler_eixo_com_zona_morta(adc_channel_t canal) {
    int raw = 0;
    adc_oneshot_read(adc_handle, canal, &raw);

    if (raw > (JOY_CENTRO - ZONA_MORTA) && raw < (JOY_CENTRO + ZONA_MORTA)) {
        raw = JOY_CENTRO;
    }

    return (raw * 180) / 4095;
}

// -------------------------------------------------------

void app_main(void) {
    joystick_init();

    servo_init(&servo_am);
    servo_init(&servo_roxo);
    servo_init(&servo1);
    servo_init(&servo2);

    while (1) {
        // Joystick 1 -> servo_am (X) e servo_roxo (Y)
        int graus_j1x = ler_eixo_com_zona_morta(JOY1_X_CHANNEL);
        int graus_j1y = ler_eixo_com_zona_morta(JOY1_Y_CHANNEL);

        // Joystick 2 -> servo1 (X) e servo2 (Y)
        int graus_j2x = ler_eixo_com_zona_morta(JOY2_X_CHANNEL);
        int graus_j2y = ler_eixo_com_zona_morta(JOY2_Y_CHANNEL);

        servo_set_graus(&servo_am,   graus_j1x);
        servo_set_graus(&servo_roxo, graus_j1y);
        servo_set_graus(&servo1,     graus_j2x);
        servo_set_graus(&servo2,     graus_j2y);

        printf("J1 X:%d Y:%d | J2 X:%d Y:%d\n",
               graus_j1x, graus_j1y, graus_j2x, graus_j2y);

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}