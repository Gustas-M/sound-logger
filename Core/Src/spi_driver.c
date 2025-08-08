#include "stm32f4xx_ll_spi.h"
#include "stm32f4xx_ll_bus.h"
#include "spi_driver.h"
#include "gpio_driver.h"

typedef void (*EnableClock_t)(uint32_t periph);

typedef struct {
	SPI_TypeDef *spi;
	eGpioPin_t select_pin;
	uint32_t transfer_direction;
	uint32_t mode;
	uint32_t data_width;
	uint32_t clock_polarity;
	uint32_t clock_phase;
	uint32_t nss;
	uint32_t baudrate;
	uint32_t bit_order;
	uint32_t crc_calculation;
	uint32_t crc_poly;
	uint32_t standard;
	EnableClock_t enable_clock;
	uint32_t clock;
} sSpiDriver_t;

static sSpiDriver_t static_spi_driver_lut[eSpi_Last] = {
	[eSpi_SdCardReader] = {
		.spi = SPI2,
		.select_pin = eGpioPin_SdCardSpiCs,
		.transfer_direction = LL_SPI_FULL_DUPLEX,
		.mode = LL_SPI_MODE_MASTER,
		.data_width = LL_SPI_DATAWIDTH_8BIT,
		.clock_polarity = LL_SPI_POLARITY_LOW,
		.clock_phase = LL_SPI_PHASE_1EDGE,
		.nss = LL_SPI_NSS_SOFT,
		.baudrate = LL_SPI_BAUDRATEPRESCALER_DIV2,
		.bit_order = LL_SPI_MSB_FIRST,
		.crc_calculation = LL_SPI_CRCCALCULATION_DISABLE,
		.crc_poly = 10,
		.standard = LL_SPI_PROTOCOL_MOTOROLA,
		.enable_clock = LL_APB1_GRP1_EnableClock,
		.clock = LL_APB1_GRP1_PERIPH_SPI2
	}
};

bool SPI_Driver_Init (eSpi_t spi) {
	if ((eSpi_Last <= spi) || (eSpi_First > spi)) {
		return false;
	}

	LL_SPI_InitTypeDef spi_init_struct = {0};

	static_spi_driver_lut[spi].enable_clock(static_spi_driver_lut[spi].clock);

	spi_init_struct.TransferDirection = static_spi_driver_lut[spi].transfer_direction;
	spi_init_struct.Mode = static_spi_driver_lut[spi].mode;
	spi_init_struct.DataWidth = static_spi_driver_lut[spi].data_width;
	spi_init_struct.ClockPolarity = static_spi_driver_lut[spi].clock_polarity;
	spi_init_struct.ClockPhase = static_spi_driver_lut[spi].clock_phase;
	spi_init_struct.NSS = static_spi_driver_lut[spi].nss;
	spi_init_struct.BaudRate = static_spi_driver_lut[spi].baudrate;
	spi_init_struct.BitOrder = static_spi_driver_lut[spi].bit_order;
	spi_init_struct.CRCCalculation = static_spi_driver_lut[spi].crc_calculation;
	spi_init_struct.CRCPoly = static_spi_driver_lut[spi].crc_poly;

	if (LL_SPI_Init(static_spi_driver_lut[spi].spi, &spi_init_struct) != SUCCESS) {
		return false;
	}

	LL_SPI_SetStandard(static_spi_driver_lut[spi].spi, static_spi_driver_lut[spi].standard);

	return true;
}

bool SPI_Driver_Select (eSpi_t spi) {
	if ((eSpi_Last <= spi) || (eSpi_First > spi)) {
		return false;
	}

	GPIO_Driver_WritePin(static_spi_driver_lut[spi].select_pin, false);
	return true;
}

bool SPI_Driver_Deselect (eSpi_t spi) {
	if ((eSpi_Last <= spi) || (eSpi_First > spi)) {
		return false;
	}

	GPIO_Driver_WritePin(static_spi_driver_lut[spi].select_pin, true);
	return true;
}

bool SPI_Driver_Write (eSpi_t spi, uint8_t *buffer, size_t byte_count) {
	if ((eSpi_Last <= spi) || (eSpi_First > spi)) {
		return false;
	}

	for (uint32_t i = 0; i < byte_count; i++) {
	        while (!LL_SPI_IsActiveFlag_TXE(static_spi_driver_lut[spi].spi));

	        if (LL_SPI_GetDataWidth(static_spi_driver_lut[spi].spi) == LL_SPI_DATAWIDTH_8BIT) {
	            LL_SPI_TransmitData8(static_spi_driver_lut[spi].spi, buffer[i]);
	        } else {
	            uint16_t value = (buffer[i] << 8) | (i + 1 < byte_count ? buffer[i+1] : 0xFF);
	            LL_SPI_TransmitData16(static_spi_driver_lut[spi].spi, value);
	            i++;
	        }

	        while (!LL_SPI_IsActiveFlag_RXNE(static_spi_driver_lut[spi].spi));
	        if (LL_SPI_GetDataWidth(static_spi_driver_lut[spi].spi) == LL_SPI_DATAWIDTH_8BIT) {
	            LL_SPI_ReceiveData8(static_spi_driver_lut[spi].spi);
	        } else {
	            LL_SPI_ReceiveData16(static_spi_driver_lut[spi].spi);
	        }
	    }

	return true;
}

bool SPI_Driver_Read (eSpi_t spi, uint8_t *buffer, size_t byte_count) {
	if ((eSpi_Last <= spi) || (eSpi_First > spi)) {
		return false;
	}

	for (uint32_t i = 0; i < byte_count; i++) {
	        while (!LL_SPI_IsActiveFlag_TXE(static_spi_driver_lut[spi].spi));

	        if (LL_SPI_GetDataWidth(static_spi_driver_lut[spi].spi) == LL_SPI_DATAWIDTH_8BIT) {
	            LL_SPI_TransmitData8(static_spi_driver_lut[spi].spi, 0xFF);
	        } else {
	            LL_SPI_TransmitData16(static_spi_driver_lut[spi].spi, 0xFFFF);
	            i++;
	        }

	        while (!LL_SPI_IsActiveFlag_RXNE(static_spi_driver_lut[spi].spi));

	        if (LL_SPI_GetDataWidth(static_spi_driver_lut[spi].spi) == LL_SPI_DATAWIDTH_8BIT) {
	            buffer[i] = LL_SPI_ReceiveData8(static_spi_driver_lut[spi].spi);
	        } else {
	            uint16_t value = LL_SPI_ReceiveData16(static_spi_driver_lut[spi].spi);

	            buffer[i] = (value >> 8) & 0xFF;
	            if (i+1 < byte_count) {
	                buffer[i+1] = value & 0xFF;
	            }
	        }
	    }

	return true;
}
