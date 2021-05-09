/*
 * ads1018.h
 *
 * Created: 12.01.2021 10:26:07
 *  Author: gfcwfzkm
 */ 


#ifndef ADS1018_H_
#define ADS1018_H_

#include <inttypes.h>

/* ADS1018 Config Register Bits */
#define ADS1018_SS		0x8000
#define ADS1018_MUX2	0x4000
#define ADS1018_MUX1	0x2000
#define ADS1018_MUX0	0x1000
#define ADS1018_MUX_MASK	(ADS1018_MUX2 | ADS1018_MUX1 | ADS1018_MUX0)
#define ADS1018_PGA2	0x0800
#define ADS1018_PGA1	0x0400
#define ADS1018_PGA0	0x0200
#define ADS1018_PGA_MASK	(ADS1018_PGA2 | ADS1018_PGA1 | ADS1018_PGA0)
#define ADS1018_MODE	0x0100
#define ADS1018_DR2		0x0080
#define ADS1018_DR1		0x0040
#define ADS1018_DR0		0x0020
#define ADS1018_DR_MASK		(ADS1018_DR2 | ADS1018_DR1 | ADS1018_DR0)
#define ADS1018_TSMODE	0x0010
#define ADS1018_PULLEN	0x0008
#define ADS1018_NOP_0	0x0002
#define ADS1018_NOP_1	0x0004
#define ADS1018_NOP_VD	(ADS1018_NOP_0)
#define ADS1018_RES		0x0001
#define ADS1018_MUX_TEMP_MASK	(ADS1018_MUX_MASK | ADS1018_TSMODE)

enum ads_MUX {
	ADS_AIN0P_AIN1N			= 0,
	ADS_AIN0P_AIN3N			= ADS1018_MUX0,
	ADS_AIN1P_AIN3N			= ADS1018_MUX1,
	ADS_AIN2P_AIN3N			= ADS1018_MUX1 | ADS1018_MUX0,
	ADS_AIN0P_GND			= ADS1018_MUX2,
	ADS_AIN1P_GND			= ADS1018_MUX2 | ADS1018_MUX0,
	ADS_AIN2P_GND			= ADS1018_MUX2 | ADS1018_MUX1,
	ADS_AIN3P_GND			= ADS1018_MUX2 | ADS1018_MUX1 | ADS1018_MUX0,
	ADS_TEMPSENSING_MODE	= ADS1018_TSMODE
};

enum ads_FSR {
	ADS_6144mV	= 0,
	ADS_4096mV	= ADS1018_PGA0,
	ADS_2048mV	= ADS1018_PGA1,
	ADS_1024mV	= ADS1018_PGA1 | ADS1018_PGA0,
	ADS_0512mV	= ADS1018_PGA2,
	ADS_0256mV	= ADS1018_PGA2 | ADS1018_PGA0
};

enum ads_SAMPLES {
	ADS_128SPS	= 0,
	ADS_250SPS	= ADS1018_DR0,
	ADS_490SPS	= ADS1018_DR1,
	ADS_920SPS	= ADS1018_DR1 | ADS1018_DR0,
	ADS_1600SPS	= ADS1018_DR2,
	ADS_2400SPS	= ADS1018_DR2 | ADS1018_DR0,
	ADS_3300SPS	= ADS1018_DR2 | ADS1018_DR1
};

enum ads_error {
	ADS_NOERROR		= 0,
	ADS_COMSERROR	= 1,
	ADS_CONFIGERROR	= 2
};

typedef struct
{
	uint16_t config_reg;				// configuration register of the device
	void *ioInterface;					// Pointer to the IO/Peripheral Interface library
	// Any return value by the IO interface functions have to return zero when successful or
	// non-zero when not successful.
	uint8_t (*startTransaction)(void*);	// Prepare the IO/Peripheral Interface for a transaction
	uint8_t (*transceiveBytes)(void*,	// Send and receive Bytes from the buffer (SPI only)
						uint8_t,		// Address of the PortExpander (8-Bit Address Format!),
						uint8_t*,		// Pointer to send buffer,
						uint16_t);		// Amount of bytes to send
	uint8_t (*readMISOPin)(void*);		// Return one if MISO is high, zero if low
	uint8_t (*endTransaction)(void*);	// Finish the transaction / Release IO/Peripheral
} ads1018_t;

void ads_initStruct(ads1018_t *adc, void *ioComs, uint8_t (*startTransaction)(void*),
				uint8_t (*transceiveBytes)(void*,uint8_t,uint8_t*,uint16_t),
				uint8_t (*readMISOPin)(void*), uint8_t (*endTransaction)(void*));

// Returns 1 if config-readback is either 0x0000 or 0xFFFF, 2 if it's not the default config.
enum ads_error ads_init(ads1018_t *adc);

// Settings only updated when ADC is started / restartet.
void ads_setMux(ads1018_t *adc, enum ads_MUX ads_mux);

// Settings only updated when ADC is started / restartet.
void ads_setFSR(ads1018_t *adc, enum ads_FSR ads_pga);

// Settings only updated when ADC is started / restartet.
void ads_setSampleRate(ads1018_t *adc, enum ads_SAMPLES ads_sps);

// Start the conversion, simple as that 
void ads_startConversion(ads1018_t *adc);

// Checks if the ADC is busy (needs MISO pin access, basically reads it's status)
uint8_t ads_isBusy(ads1018_t *adc);

// Result is left-justified, signed 12-bit value
int16_t ads_getADCResult(ads1018_t *adc);

// Result is left-justified, signed 12-bit value
int16_t ads_restartADCResult(ads1018_t *adc);

// Returns the converted temperature (0.125°C per bit) as a float
float ads_convertTemperature(int16_t raw_temp);

// Returns a right-justified, signed result
int16_t ads_convertRightJustified(int16_t raw_adc);

// Read back the config from the adc
uint16_t ads_getConfig(ads1018_t *adc);

#endif /* ADS1018_H_ */