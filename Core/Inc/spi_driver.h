#ifndef INC_SPI_DRIVER_H_
#define INC_SPI_DRIVER_H_

#include <stdbool.h>

typedef enum {
	eSpi_First = 0,
	eSpi_SdCardReader = eSpi_First,
	eSpi_Last
} eSpi_t;

bool SPI_Driver_Init (eSpi_t spi);
bool SPI_Driver_Select (eSpi_t spi);
bool SPI_Driver_Deselect (eSpi_t spi);
bool SPI_Driver_Write (eSpi_t spi, uint8_t *buffer, size_t byte_count);
bool SPI_Driver_Read (eSpi_t spi, uint8_t *buffer, size_t byte_count);

#endif /* INC_SPI_DRIVER_H_ */
