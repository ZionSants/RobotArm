#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"

#define SERVO_FREQ_HZ 50
#define SERVO_RESOLUTION LEDC_TIMER_14_BIT
#define SERVO_DUTY_MIN 819
#define SERVO_DUTY_MAX 1638

// Joystick 1
#define joy1X ADC_CHANNEL_5   // GPIO 33
#define joy1Y ADC_CHANNEL_4   // GPIO 32

// Joystick 2
#define joy2X ADC_CHANNEL_3   // GPIO 39
#define joy2Y ADC_CHANNEL_0   // GPIO 36

#define zonaMorta 700
#define joyCentro 2048

typedef struct {
    int gpio;
    ledc_channel_t canal;
    int angulo_atual;
} Servo;

Servo servoBase = { .gpio = 14, .canal = LEDC_CHANNEL_0, .angulo_atual = 90 };
Servo servoOmbro = { .gpio = 25, .canal = LEDC_CHANNEL_1, .angulo_atual = 90 };
// Servo servoCotovelo = { .gpio = 26, .canal = LEDC_CHANNEL_2, .angulo_atual = 90 };
Servo servoPulso = { .gpio = 26, .canal = LEDC_CHANNEL_2, .angulo_atual = 90 };
Servo servoGarra = { .gpio = 27, .canal = LEDC_CHANNEL_3, .angulo_atual = 90 };

adc_oneshot_unit_handle_t adc_handle;

// -------------------------------------------------------

uint32_t graus_para_duty(int graus) {
    return SERVO_DUTY_MIN + (graus * (SERVO_DUTY_MAX - SERVO_DUTY_MIN) / 180);
}

void servo_init(Servo *s) {
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = SERVO_RESOLUTION,
        .freq_hz = SERVO_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer);

    ledc_channel_config_t canal = {
        .gpio_num = s->gpio,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = s->canal,
        .timer_sel = LEDC_TIMER_0,
        .duty = graus_para_duty(s->angulo_atual),
        .hpoint = 0
    };
    ledc_channel_config(&canal);
}

// Controladores de velocidade dos servos (Temporário)
#define baseMin 30
#define baseMax 150
#define ombroMin 30
#define ombroMax 150
#define pulsoMin 0
#define pulsoMax 180
#define garraMin 0    // fechada
#define garraMax 180    // aberta

void servo_set_graus(Servo *s, int graus, int min, int max) {
    if (graus < min) graus = min;
    if (graus > max) graus = max;
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
        .atten = ADC_ATTEN_DB_12,
    };
    adc_oneshot_config_channel(adc_handle, joy1X, &chan_config);
    adc_oneshot_config_channel(adc_handle, joy1Y, &chan_config);
    adc_oneshot_config_channel(adc_handle, joy2X, &chan_config);
    adc_oneshot_config_channel(adc_handle, joy2Y, &chan_config);
}

int joystickRange(adc_channel_t canal) {
    int raw = 0;
    adc_oneshot_read(adc_handle, canal, &raw);

    if (raw > (joyCentro - zonaMorta) && raw < (joyCentro + zonaMorta)) {
        raw = joyCentro;
    }
    return (raw * 180) / 4095;
}

// -------------------------------------------------------

void app_main(void) {
    joystick_init();

    servo_init(&servoBase);
    servo_init(&servoOmbro);
    servo_init(&servoPulso);
    servo_init(&servoGarra);

    while (1) {
        // Joystick 1 -> servoBase (X) e servoOmbro (Y)
        int graus_j1x = joystickRange(joy1X);
        int graus_j1y = joystickRange(joy1Y);

        // Joystick 2 -> servoPulso (X) e servoGarra (Y)
        int graus_j2x = joystickRange(joy2X);
        int graus_j2y = joystickRange(joy2Y);

        servo_set_graus(&servoBase,   graus_j1y, baseMin,  baseMax);
        servo_set_graus(&servoOmbro,  graus_j1x, ombroMin, ombroMax);
        servo_set_graus(&servoPulso,  graus_j2x, pulsoMin, pulsoMax);
        servo_set_graus(&servoGarra,  graus_j2y, garraMin, garraMax);

        printf("J1 X:%d Y:%d | J2 X:%d Y:%d\n",
               graus_j1x, graus_j1y, graus_j2x, graus_j2y);

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}