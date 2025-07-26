#include "stm32f4xx_ll_adc.h"
#include "stm32f4xx_ll_bus.h"
#include "adc_driver.h"


typedef struct {
	 uint32_t common_clock;
	 uint32_t multimode;
	 uint32_t multi_dma_transf;
	 uint32_t multi_two_sampl_delay;
} sAdcCommonDesc_t;

typedef struct {
	uint32_t resolution;
	uint32_t data_align;
	uint32_t seq_scan_mode;
} sAdcDesc_t;

typedef struct {
	uint32_t triggers_source;
	uint32_t seq_length;
	uint32_t seq_discont;
	uint32_t continuous_mode;
	uint32_t dma_transf;
} sAdcRegDesc_t;



bool ADC_Init (void) {
	LL_ADC_InitTypeDef ADC_InitStruct = {0};
	LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};
	LL_ADC_CommonInitTypeDef ADC_CommonInitStruct = {0};
}

