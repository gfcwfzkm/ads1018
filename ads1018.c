/*
 * ads1018.c
 *
 * Created: 12.01.2021 10:25:52
 *  Author: gfcwfzkm
 */ 

#include "ads1018.h"

#define ADS_DEFAULTCONFIG	( ADS_2048mV | ADS1018_MODE | \
						ADS_1600SPS | ADS1018_PULLEN | ADS1018_NOP_VD | ADS1018_RES)

#define ADS_TEMPERATUREBIT	0.125f

void ads_initStruct(ads1018_t *adc, void *ioComs, uint8_t (*startTransaction)(void*),
				uint8_t (*transceiveBytes)(void*,uint8_t,uint8_t*,uint16_t),
				uint8_t (*readMISOPin)(void*), uint8_t (*endTransaction)(void*))
{
	adc->ioInterface = ioComs;
	adc->startTransaction = startTransaction;
	adc->transceiveBytes = transceiveBytes;
	adc->readMISOPin = readMISOPin;
	adc->endTransaction = endTransaction;
}

enum ads_error ads_init(ads1018_t *adc)
{
	uint16_t configReg;
	enum ads_error err_check = ADS_NOERROR;
	
	adc->config_reg = ADS_DEFAULTCONFIG;	
	configReg = ads_getConfig(adc);
	
	if ( (configReg == 0) || (configReg == 0xFF) )
	{
		err_check = ADS_COMSERROR;
	}
	else if (configReg != adc->config_reg)
	{
		err_check = ADS_CONFIGERROR;	// Data not according to the default. 
	}
	
	return err_check;
}

void ads_setMux(ads1018_t *adc, enum ads_MUX ads_mux)
{
	uint16_t temp_reg = adc->config_reg & ~ADS1018_MUX_TEMP_MASK;
	temp_reg |= (ads_mux & ADS1018_MUX_TEMP_MASK);
	adc->config_reg = temp_reg;
}

enum ads_MUX ads_getMux(ads1018_t *adc)
{
	enum ads_MUX tempMux;
	
	tempMux = adc->config_reg & ADS1018_MUX_TEMP_MASK;
	
	return tempMux;
}

void ads_setFSR(ads1018_t *adc, enum ads_FSR ads_fsr)
{
	uint16_t temp_reg = adc->config_reg & ~ADS1018_PGA_MASK;
	temp_reg |= (ads_fsr & ADS1018_PGA_MASK);
	adc->config_reg = temp_reg;
}

void ads_setSampleRate(ads1018_t *adc, enum ads_SAMPLES ads_sps)
{
	uint16_t temp_reg = adc->config_reg & ~ADS1018_DR_MASK;
	temp_reg |= (ads_sps & ADS1018_DR_MASK);
	adc->config_reg = temp_reg;
}

void ads_startConversion(ads1018_t *adc)
{
	uint8_t _databuf[2];
	uint16_t temp_reg = adc->config_reg | ADS1018_SS;
	
	_databuf[0] = (uint8_t)(temp_reg >> 8);
	_databuf[1] = (uint8_t)(temp_reg);
	
	adc->startTransaction(adc->ioInterface);
	adc->transceiveBytes(adc->ioInterface, 0, _databuf, 2);
	adc->endTransaction(adc->ioInterface);
}

uint8_t ads_isBusy(ads1018_t *adc)
{
	uint8_t _readMISObit;
	
	adc->startTransaction(adc->ioInterface);
	_readMISObit = adc->readMISOPin(adc->ioInterface);
	adc->endTransaction(adc->ioInterface);
	
	return (_readMISObit ? 1 : 0);
}

int16_t ads_getADCResult(ads1018_t *adc)
{
	uint8_t _databuf[2];
	uint16_t temp_reg = adc->config_reg;
	
	_databuf[0] = (uint8_t)(temp_reg >> 8);
	_databuf[1] = (uint8_t)(temp_reg);
	
	adc->startTransaction(adc->ioInterface);
	adc->transceiveBytes(adc->ioInterface, 0, _databuf, 2);
	adc->endTransaction(adc->ioInterface);
	
	temp_reg = (uint16_t)(_databuf[0] << 8) | (uint16_t)_databuf[1];
	
	return temp_reg;
}

int16_t ads_restartADCResult(ads1018_t *adc)
{
	uint8_t _databuf[2];
	uint16_t temp_reg = adc->config_reg | ADS1018_SS;
	
	_databuf[0] = (uint8_t)(temp_reg >> 8);
	_databuf[1] = (uint8_t)(temp_reg);
	
	adc->startTransaction(adc->ioInterface);
	adc->transceiveBytes(adc->ioInterface, 0, _databuf, 2);
	adc->endTransaction(adc->ioInterface);
	
	temp_reg = (uint16_t)((_databuf[0] << 8) | _databuf[1]);
	
	return temp_reg;
}

float ads_convertTemperature(int16_t raw_temp)
{
	float temperature;
	
	raw_temp = (raw_temp >> 4) | ((raw_temp < 0) ? 0xF000 : 0);
		
	temperature = (float)raw_temp * ADS_TEMPERATUREBIT;
	
	return temperature;
}

int16_t ads_convertRightJustified(int16_t raw_adc)
{
	int16_t rjust_adc = (raw_adc >> 4) | ((raw_adc < 0) ? 0xF000 : 0);
	
	return rjust_adc;
}

uint16_t ads_getConfig(ads1018_t *adc)
{
	uint8_t _databuf[4];
	uint16_t ads_config;
	_databuf[0] = (uint8_t)(adc->config_reg >> 8);
	_databuf[1] = (uint8_t)(adc->config_reg);
	_databuf[2] = 0xff;
	_databuf[3] = 0xff;
		
	adc->startTransaction(adc->ioInterface);
	adc->transceiveBytes(adc->ioInterface, 0, _databuf, 4);
	adc->endTransaction(adc->ioInterface);
	
	ads_config = (uint16_t)((_databuf[2] << 8) | _databuf[3]);
	
	return ads_config;
}