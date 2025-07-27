/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include <stddef.h>
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_bus.h"
#include "gpio_driver.h"
/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/
/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/
 typedef struct {
     GPIO_TypeDef *port;
     uint32_t pin;
     uint32_t mode;
     uint32_t speed;
     uint32_t output;
     uint32_t pull;
     uint32_t clock;
     uint32_t alternate;
 } sGpioDesc_t;
/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
 const static sGpioDesc_t g_static_gpio_lut[eGpioPin_Last] = {
    [eGpioPin_DebugTx] = {
        .port = GPIOA,
        .pin = LL_GPIO_PIN_2,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .pull = LL_GPIO_PULL_NO,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOA,
        .alternate = LL_GPIO_AF_7
    },
    [eGpioPin_DebugRx] = {
        .port = GPIOA,
        .pin = LL_GPIO_PIN_3,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .pull = LL_GPIO_PULL_NO,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOA,
        .alternate = LL_GPIO_AF_7
    },
	[eGpioPin_ADC1_CH0] = {
		.port = GPIOA,
		.pin = LL_GPIO_PIN_0,
		.mode = LL_GPIO_MODE_ANALOG,
		.speed = LL_GPIO_SPEED_FREQ_LOW,
		.output = LL_GPIO_OUTPUT_OPENDRAIN,
		.pull = LL_GPIO_PULL_NO,
		.clock = LL_AHB1_GRP1_PERIPH_GPIOA,
		.alternate = LL_GPIO_AF_0
	}
};
/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/
bool GPIO_Driver_Init (void) {
    LL_GPIO_InitTypeDef gpio_init_struct = {0};
    bool is_init_successful = true;

    for (eGpioPin_t pin = eGpioPin_First; pin < eGpioPin_Last; pin++) {
        LL_AHB1_GRP1_EnableClock(g_static_gpio_lut[pin].clock);
        LL_GPIO_ResetOutputPin(g_static_gpio_lut[pin].port, g_static_gpio_lut[pin].pin);

        gpio_init_struct.Pin = g_static_gpio_lut[pin].pin;
        gpio_init_struct.Mode = g_static_gpio_lut[pin].mode;
        gpio_init_struct.Speed = g_static_gpio_lut[pin].speed;
        gpio_init_struct.OutputType = g_static_gpio_lut[pin].output;
        gpio_init_struct.Pull = g_static_gpio_lut[pin].pull;
        gpio_init_struct.Alternate = g_static_gpio_lut[pin].alternate;

        if (LL_GPIO_Init(g_static_gpio_lut[pin].port, &gpio_init_struct) != SUCCESS) {
            is_init_successful = false;
        }
    }

    return is_init_successful;
}

bool GPIO_Driver_TogglePin (eGpioPin_t pin) {
    if ((pin < eGpioPin_First) || (pin >= eGpioPin_Last) || (g_static_gpio_lut[pin].mode != LL_GPIO_MODE_OUTPUT)) {
        return false;
    }

    LL_GPIO_TogglePin(g_static_gpio_lut[pin].port, g_static_gpio_lut[pin].pin);

    return true;
}

bool GPIO_Driver_ReadPin (eGpioPin_t pin, bool *state) {
    if ((pin < eGpioPin_First) || (pin >= eGpioPin_Last) || (state == NULL)) {
        return false;
    }

    switch (g_static_gpio_lut[pin].mode) {
        case LL_GPIO_MODE_OUTPUT:
            *state = LL_GPIO_IsOutputPinSet(g_static_gpio_lut[pin].port, g_static_gpio_lut[pin].pin);
            return true;
        case LL_GPIO_MODE_INPUT:
            *state = LL_GPIO_IsInputPinSet(g_static_gpio_lut[pin].port, g_static_gpio_lut[pin].pin);
            return true;
        default:
            return false;
    }
}

bool GPIO_Driver_WritePin (eGpioPin_t pin, bool state) {
    if ((pin < eGpioPin_First) || (pin >= eGpioPin_Last)) {
        return false;
    }

    if (state == false) {
        LL_GPIO_ResetOutputPin(g_static_gpio_lut[pin].port, g_static_gpio_lut[pin].pin);
    } else {
        LL_GPIO_SetOutputPin(g_static_gpio_lut[pin].port, g_static_gpio_lut[pin].pin);
    }

    return true;
}
