#include <stddef.h>
#include "stm32f4xx_ll_adc.h"
#include "stm32f4xx_ll_bus.h"
#include "adc_driver.h"
#include "dma_driver.h"

typedef struct {
	 uint32_t common_clock;
} sAdcCommonDesc_t;

typedef void (*EnableClock_t)(uint32_t periph);

typedef struct {
    uint16_t value;
} sAdcValue_t;

typedef struct {
	ADC_TypeDef *adc;
	uint32_t resolution;
	uint32_t data_align;
	uint32_t seq_scan_mode;
	uint32_t clock;
	EnableClock_t enable_clock;
	uint32_t channel;
	uint32_t rank;
	uint32_t triggers_source;
	uint32_t seq_length;
	uint32_t seq_discont;
	uint32_t continuous_mode;
	uint32_t dma_transf;
	bool dma_enabled;
    IRQn_Type irqn;
    uint32_t irqn_priority;
} sAdcDesc_t;

typedef struct {
	eAdc_t adc;
    uint32_t rank;
    uint32_t channel;
    uint32_t sampling_time;
} sAdcChannel_t;

static sAdcValue_t dyn_adc_val[eAdcChannel_Last];

static sAdcCommonDesc_t static_adc_common_lut = {
	.common_clock = LL_ADC_CLOCK_SYNC_PCLK_DIV4,
};

static sAdcChannel_t static_adc_channel_lut[eAdcChannel_Last] = {
	[eAdcChannel_1] = {
		.adc = eAdc_1,
		.channel = LL_ADC_CHANNEL_0,
		.rank = LL_ADC_REG_RANK_1,
		.sampling_time = LL_ADC_SAMPLINGTIME_480CYCLES,
	}
};

static sAdcDesc_t static_adc_lut[eAdc_Last] = {
	[eAdc_1] = {
		.adc = ADC1,
		.resolution = LL_ADC_RESOLUTION_12B,
		.data_align = LL_ADC_DATA_ALIGN_RIGHT,
		.seq_scan_mode = LL_ADC_SEQ_SCAN_DISABLE,
		.clock = LL_APB2_GRP1_PERIPH_ADC1,
		.enable_clock = LL_APB2_GRP1_EnableClock,
		.channel = LL_ADC_CHANNEL_0,
		.rank = LL_ADC_REG_RANK_1,
		.triggers_source = LL_ADC_REG_TRIG_SOFTWARE,
		.seq_length = LL_ADC_REG_SEQ_SCAN_DISABLE,
		.seq_discont = LL_ADC_REG_SEQ_DISCONT_DISABLE,
		.continuous_mode = LL_ADC_REG_CONV_SINGLE,
		.dma_transf = LL_ADC_REG_DMA_TRANSFER_UNLIMITED,
		.dma_enabled = true,
	}
};

bool ADC_Driver_Init (eAdc_t adc) {
	if ((eAdc_Last <= adc) || (eAdc_First > adc)) {
		return false;
	}

	LL_ADC_InitTypeDef ADC_InitStruct = {0};
	LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};

    static_adc_lut[adc].enable_clock(static_adc_lut[adc].clock);

    LL_ADC_SetCommonClock(__LL_ADC_COMMON_INSTANCE(static_adc_lut[adc].adc), static_adc_common_lut.common_clock);

	ADC_InitStruct.Resolution = static_adc_lut[adc].resolution;
	ADC_InitStruct.DataAlignment = static_adc_lut[adc].data_align;
	ADC_InitStruct.SequencersScanMode = static_adc_lut[adc].seq_scan_mode;

	if (LL_ADC_Init(static_adc_lut[adc].adc, &ADC_InitStruct) != SUCCESS) {
		return false;
	}

	ADC_REG_InitStruct.TriggerSource = static_adc_lut[adc].triggers_source;
	ADC_REG_InitStruct.SequencerLength = static_adc_lut[adc].seq_length;
	ADC_REG_InitStruct.SequencerDiscont = static_adc_lut[adc].seq_discont;
	ADC_REG_InitStruct.ContinuousMode = static_adc_lut[adc].continuous_mode;
	ADC_REG_InitStruct.DMATransfer = static_adc_lut[adc].dma_transf;

	if (LL_ADC_REG_Init(static_adc_lut[eAdc_1].adc, &ADC_REG_InitStruct) != SUCCESS) {
		return false;
	}

	for (eAdcChannel_t adc_ch = eAdcChannel_First; adc_ch < eAdcChannel_Last; adc_ch++) {
		if (static_adc_channel_lut[adc_ch].adc == adc) {
			LL_ADC_REG_SetSequencerRanks(static_adc_lut[adc].adc, static_adc_channel_lut[adc_ch].rank, static_adc_channel_lut[adc_ch].channel);
			LL_ADC_SetChannelSamplingTime(static_adc_lut[adc].adc, static_adc_channel_lut[adc_ch].channel, static_adc_channel_lut[adc_ch].sampling_time);
		}
	}

	if (static_adc_lut[adc].dma_enabled) {
		sDmaInit_t dma_init = {0};
		dma_init.data_amount = eAdcChannel_Last;
		dma_init.dest_addr = &dyn_adc_val;
		dma_init.periph_or_src_addr = (void*) LL_ADC_DMA_GetRegAddr(static_adc_lut[adc].adc, LL_ADC_DMA_REG_REGULAR_DATA);
		dma_init.dma_stream = eDmaStream_1;
		DMA_Driver_Init(&dma_init);
	}

	LL_ADC_Enable(static_adc_lut[adc].adc);
	DMA_Driver_EnableStream(eDmaStream_1);

    NVIC_SetPriority(static_adc_lut[adc].irqn, static_adc_lut[adc].irqn_priority);
    NVIC_EnableIRQ(static_adc_lut[adc].irqn);

	return true;
}

bool ADC_Driver_ReadChannels (eAdc_t adc) {
	if ((eAdc_Last <= adc) || (eAdc_First > adc)) {
		return false;
	}

    LL_ADC_REG_StartConversionSWStart(static_adc_lut[adc].adc);

    return true;
}

bool ADC_Driver_GetChannelValue (eAdcChannel_t channel, uint16_t *value) {
    if (channel >= eAdcChannel_Last) {
        return false;
    }

    if (value == NULL) {
        return false;
    }

    *value = dyn_adc_val[channel].value;

    return true;
}


