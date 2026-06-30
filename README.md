# RobotArm

---

## 📋 Descrição

Projeto de sistemas embarcados desenvolvido com **ESP32-WROVER** e **ESP-IDF**, implementando controle em tempo real de 4 servos motores através de 2 joysticks analógicos. Cada eixo do joystick controla independentemente um servo, com zona morta aplicada para eliminar jitter nas leituras ADC.

---

## 🛠️ Hardware

| Componente | Quantidade | Observação |
|---|---|---|
| ESP32-WROVER | 1 | Chip ESP32 |
| Servo SG90 / MG995 / MG996R | 4 | Alimentação 5V externa |
| Joystick analógico KY-023 | 2 | Saída 0–3.3V |
| Fonte 5V 4.1A | 1 | Para alimentação dos servos |

### Pinagem

**Servos (saída PWM):**

| Servo | GPIO | Canal LEDC |
|---|---|---|
| servo_am (amarelo) | 14 | LEDC_CHANNEL_0 |
| servo_roxo | 25 | LEDC_CHANNEL_1 |
| servo1 | 26 | LEDC_CHANNEL_2 |
| servo2 | 27 | LEDC_CHANNEL_3 |

**Joysticks (entrada ADC1):**

| Joystick | Eixo | GPIO | Canal ADC |
|---|---|---|---|
| Joystick 1 | X → servo_am | 33 | ADC_CHANNEL_5 |
| Joystick 1 | Y → servo_roxo | 32 | ADC_CHANNEL_4 |
| Joystick 2 | X → servo1 | 39 | ADC_CHANNEL_3 |
| Joystick 2 | Y → servo2 | 36 | ADC_CHANNEL_0 |

### Diagrama de alimentação

```
Fonte 5V (+) ──┬── VCC servo_am
               ├── VCC servo_roxo
               ├── VCC servo1
               └── VCC servo2

Fonte 5V (-) ──┬── GND servo_am
               ├── GND servo_roxo
               ├── GND servo1
               ├── GND servo2
               └── GND ESP32     
```

---

## 💻 Software

### Ambiente

- **Framework:** ESP-IDF v5.5.4
- **IDE:** VS Code + extensão ESP-IDF (Espressif Systems)
- **Linguagem:** C (C99)
- **Build system:** CMake

### Dependências

Componentes nativos do ESP-IDF, sem bibliotecas externas:

- `driver/ledc.h` — geração de PWM para controle de servos
- `esp_adc/adc_oneshot.h` — leitura ADC para os joysticks
- `freertos/FreeRTOS.h` + `freertos/task.h` — temporização não-bloqueante

### Arquitetura do código

```
main/
└── main.c
    ├── Servo (struct)          — abstração de um servo motor
    ├── graus_para_duty()       — converte ângulo 0–180° para duty cycle LEDC
    ├── servo_init()            — configura timer LEDC e canal PWM
    ├── servo_set_graus()       — aplica ângulo com clamp 0–180°
    ├── joystick_init()         — configura ADC1 para os 4 canais
    ├── ler_eixo_com_zona_morta() — lê ADC e aplica zona morta central
    └── app_main()              — loop principal de controle
```

### Parâmetros PWM

| Parâmetro | Valor |
|---|---|
| Frequência | 50 Hz (período 20ms) |
| Resolução | 14 bits (0–16383) |
| Duty min (0°) | 819 (~1ms de pulso) |
| Duty max (180°) | 1638 (~2ms de pulso) |

### Zona morta do joystick

O centro analógico do joystick (valor ADC ~2048) possui ruído elétrico natural. Uma zona morta de ±500 counts é aplicada para travar o servo quando o stick está solto:

```
---

## 🚀 Como compilar e gravar

```bash
# 1. Clonar o repositório
git clone https://github.com/ZionSants/RobotArm.git
cd RobotArm

# 2. Configurar target (se necessário)
idf.py set-target esp32

# 3. Compilar
idf.py build

# 4. Gravar e monitorar
idf.py -p COMx flash monitor
```

Ou pelo VS Code: botão **🔨⚡🔌 (Build Flash Monitor)** na barra inferior.

---

## 📊 Monitor Serial

Durante a execução, o firmware imprime os ângulos em tempo real (115200 baud):

```
J1 X:90 Y:45 | J2 X:120 Y:90
J1 X:91 Y:44 | J2 X:119 Y:91
```

---

## 📁 Estrutura do repositório

```
RobotArm/
├── main/
│   ├── CMakeLists.txt
│   └── main.c
├── CMakeLists.txt
├── sdkconfig
└── README.md
```

## 📄 Licença

MIT License — veja o arquivo [LICENSE](LICENSE) para detalhes.

---

## 👤 Autor

**Zion Santos** — [@ZionSants](https://github.com/ZionSants)
