#ifndef INC_DMA_DRIVER_H_
#define INC_DMA_DRIVER_H_

typedef enum {
	eDmaStream_First = 0,
	eDmaStream_1 = eDmaStream_First,
	eDmaStream_Last
} eDmaStream_t;

typedef struct {
	eDmaStream_t dma_stream;
	void *periph_or_src_addr;
	void *dest_addr;
	uint32_t data_amount;
	void (*IT_cb) (eDmaStream_t);
} sDmaInit_t;

bool DMA_Driver_Init (sDmaInit_t *dma_init_data);
bool DMA_Driver_EnableStream (eDmaStream_t dma_stream);
bool DMA_Driver_DisableStream (eDmaStream_t dma_stream);

#endif /* INC_DMA_DRIVER_H_ */
