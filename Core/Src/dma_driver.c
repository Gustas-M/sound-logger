#include <stdbool.h>
#include <stddef.h>
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_dma.h"
#include "dma_driver.h"

typedef void (*EnableClock_t)(uint32_t periph);

typedef struct {
	DMA_TypeDef *dma;
	uint32_t dma_stream;
	uint32_t dma_channel;
	uint32_t direction;
	uint32_t priority;
	uint32_t mode;
	uint32_t periph_inc_mode;
	uint32_t mem_inc_mode;
	uint32_t periph_size;
	uint32_t mem_size;
	bool fifo;
	bool dma_interrupt;
	uint32_t dma_irq;
	uint32_t irq_prio;
	EnableClock_t enable_clock;
	uint32_t clock;
} sDmaDesc_t;

typedef struct {
	uint16_t buf_size;
	void *periph_or_src_addr;
	void *dst_addr;
	void (*IT_cb)(eDmaStream_t);
} sDmaDynamic_t;

static const sDmaDesc_t static_dma_stream_lut[eDmaStream_Last] = {
	[eDmaStream_1] = {
		.dma = DMA2,
		.dma_stream = LL_DMA_STREAM_0,
		.dma_channel = LL_DMA_CHANNEL_0,
		.direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY,
		.priority = LL_DMA_PRIORITY_MEDIUM,
		.mode = LL_DMA_MODE_CIRCULAR,
		.periph_inc_mode = LL_DMA_PERIPH_NOINCREMENT,
		.mem_inc_mode = LL_DMA_MEMORY_INCREMENT,
		.periph_size = LL_DMA_PDATAALIGN_HALFWORD,
		.mem_size = LL_DMA_MDATAALIGN_HALFWORD,
		.fifo = false,
		.dma_interrupt = true,
		.dma_irq = DMA2_Stream0_IRQn,
		.irq_prio = 0,
		.enable_clock = LL_AHB1_GRP1_EnableClock,
		.clock = LL_AHB1_GRP1_PERIPH_DMA2
	}
};

static sDmaDynamic_t dyn_dma_lut[eDmaStream_Last] = {
	[eDmaStream_1] = {
		.IT_cb = NULL,
	}
};

bool DMA_Driver_Init (sDmaInit_t *dma_init_data) {
	if ((eDmaStream_Last <= dma_init_data->dma_stream) || (eDmaStream_First > dma_init_data->dma_stream)) {
		return false;
	}

	LL_DMA_InitTypeDef DMA_InitStruct = {0};

	eDmaStream_t dma_stream = dma_init_data->dma_stream;
    static_dma_stream_lut[dma_stream].enable_clock(static_dma_stream_lut[dma_stream].clock);

    dyn_dma_lut[dma_stream].buf_size = dma_init_data->data_amount;
    dyn_dma_lut[dma_stream].periph_or_src_addr = dma_init_data->periph_or_src_addr;
    dyn_dma_lut[dma_stream].dst_addr = dma_init_data->dest_addr;

    DMA_InitStruct.Channel = static_dma_stream_lut[dma_stream].dma_channel;
    DMA_InitStruct.Direction = static_dma_stream_lut[dma_stream].direction;
    DMA_InitStruct.Priority = static_dma_stream_lut[dma_stream].priority;
    DMA_InitStruct.Mode = static_dma_stream_lut[dma_stream].mode;
    DMA_InitStruct.PeriphOrM2MSrcIncMode = static_dma_stream_lut[dma_stream].periph_inc_mode;
    DMA_InitStruct.MemoryOrM2MDstIncMode = static_dma_stream_lut[dma_stream].mem_inc_mode;
    DMA_InitStruct.PeriphOrM2MSrcDataSize = static_dma_stream_lut[dma_stream].periph_size;
    DMA_InitStruct.MemoryOrM2MDstDataSize = static_dma_stream_lut[dma_stream].mem_size;
    DMA_InitStruct.NbData = dyn_dma_lut[dma_stream].buf_size;
    DMA_InitStruct.PeriphOrM2MSrcAddress = (uint32_t)dyn_dma_lut[dma_stream].periph_or_src_addr;
    DMA_InitStruct.MemoryOrM2MDstAddress = (uint32_t)dyn_dma_lut[dma_stream].dst_addr;

    if (LL_DMA_Init(static_dma_stream_lut[dma_stream].dma, static_dma_stream_lut[dma_stream].dma_stream, &DMA_InitStruct) != SUCCESS) {
    	return false;
    }

    if (static_dma_stream_lut[dma_stream].fifo) {
    	LL_DMA_EnableFifoMode(static_dma_stream_lut[dma_stream].dma, static_dma_stream_lut[dma_stream].dma_stream);
    } else {
    	LL_DMA_DisableFifoMode(static_dma_stream_lut[dma_stream].dma, static_dma_stream_lut[dma_stream].dma_stream);
    }

    if (static_dma_stream_lut[dma_stream].dma_interrupt) {
    	LL_DMA_EnableIT_TC(static_dma_stream_lut[dma_stream].dma, static_dma_stream_lut[dma_stream].dma_stream);
    	NVIC_SetPriority(static_dma_stream_lut[dma_stream].dma_irq, static_dma_stream_lut[dma_stream].irq_prio);
		NVIC_EnableIRQ(static_dma_stream_lut[dma_stream].dma_irq);
    } else {
    	LL_DMA_DisableIT_TC(static_dma_stream_lut[dma_stream].dma, static_dma_stream_lut[dma_stream].dma_stream);
    }

    return true;
}

bool DMA_Driver_EnableStream (eDmaStream_t dma_stream) {
	if ((eDmaStream_Last <= dma_stream) || (eDmaStream_First > dma_stream)) {
		return false;
	}

    LL_DMA_EnableStream(static_dma_stream_lut[dma_stream].dma, static_dma_stream_lut[dma_stream].dma_stream);
    return true;
}

bool DMA_Driver_DisableStream (eDmaStream_t dma_stream) {
	if ((eDmaStream_Last <= dma_stream) || (eDmaStream_First > dma_stream)) {
		return false;
	}

    LL_DMA_DisableStream(static_dma_stream_lut[dma_stream].dma, static_dma_stream_lut[dma_stream].dma_stream);
    return true;
}

