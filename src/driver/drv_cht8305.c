#include "../new_common.h"
#include "../new_pins.h"
#include "../new_cfg.h"
// Commands register, execution API and cmd tokenizer
#include "../cmnds/cmd_public.h"
#include "../mqtt/new_mqtt.h"
#include "../logging/logging.h"
#include "drv_local.h"
#include "drv_uart.h"
#include "../httpserver/new_http.h"
#include "../hal/hal_pins.h"

#include "drv_cht8305.h"


#define CHT8305_I2C_ADDR (0x40 << 1)

static byte channel_temp = 0, channel_humid = 0;
static float g_temp = 0.0, g_humid = 0.0;



//static commandResult_t CHT8305_GETENV(const void* context, const char* cmd, const char* args, int flags){
//	const char* c = args;
//
//	ADDLOG_DEBUG(LOG_FEATURE_CMD, "CHT8305_GETENV");
//
//	return CMD_RES_OK;
//}

static void CHT8305_ReadEnv(float* temp, float* hum)
	{
	uint8_t buff[4];
	unsigned int th, tl, hh, hl;

	Soft_I2C_Start(CHT8305_I2C_ADDR);
	Soft_I2C_WriteByte(0x00);
	Soft_I2C_Stop();

	rtos_delay_milliseconds(20);	//give the sensor time to do the conversion

	Soft_I2C_Start(CHT8305_I2C_ADDR | 1);
	Soft_I2C_ReadBytes(buff, 4);
	Soft_I2C_Stop();

	th = buff[0];
	tl = buff[1];
	hh = buff[2];
	hl = buff[3];

	(*temp) = (th << 8 | tl) * 165.0 / 65535.0 - 40.0;

	(*hum) = (hh << 8 | hl) * 100.0 / 65535.0;

	}

// startDriver CHT8305
void CHT8305_Init() {

	uint8_t buff[4];


	g_i2c_pin_clk = 9;
	g_i2c_pin_data = 14;
	g_i2c_pin_clk = PIN_FindPinIndexForRole(IOR_CHT8305_CLK, g_i2c_pin_clk);
	g_i2c_pin_data = PIN_FindPinIndexForRole(IOR_CHT8305_DAT, g_i2c_pin_data);

	Soft_I2C_PreInit();

	Soft_I2C_Start(CHT8305_I2C_ADDR);
	Soft_I2C_WriteByte(0xfe);			//manufacturer ID
	Soft_I2C_Stop();
	Soft_I2C_Start(CHT8305_I2C_ADDR | 1);
	Soft_I2C_ReadBytes(buff, 2);
	Soft_I2C_Stop();

	addLogAdv(LOG_INFO, LOG_FEATURE_SENSOR, "DRV_CHT8304_init: ID: %02X %02X", buff[0], buff[1]);

}

OBK_Publish_Result CHT8305_OnChannelChanged(int ch, int value) 
{
    return OBK_PUBLISH_WAS_NOT_REQUIRED;
}

void CHT8305_OnEverySecond() {

	CHT8305_ReadEnv(&g_temp, &g_humid);

	channel_temp = g_cfg.pins.channels[g_i2c_pin_data];
	channel_humid = g_cfg.pins.channels2[g_i2c_pin_data];
	// don't want to loose accuracy, so multiply by 10
	// We have a channel types to handle that
	CHANNEL_Set(channel_temp, (int)(g_temp * 10), 0);
	CHANNEL_Set(channel_humid, (int)(g_humid), 0);

	addLogAdv(LOG_INFO, LOG_FEATURE_SENSOR, "DRV_CHT8304_readEnv: Temperature:%fC Humidity:%f%%", g_temp, g_humid);
}

void CHT8305_AppendInformationToHTTPIndexPage(http_request_t* request)
{
	hprintf255(request, "<h2>CHT8305 Temperature=%f, Humidity=%f</h2>", g_temp, g_humid);
	if (channel_humid == channel_temp) {
		hprintf255(request, "WARNING: You don't have configured target channels for temp and humid results, set the first and second channel index in Pins!");
	}
}

