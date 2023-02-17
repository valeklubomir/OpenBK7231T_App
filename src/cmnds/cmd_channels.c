
#include "../logging/logging.h"
#include "../new_pins.h"
#include "../new_cfg.h"
#include "../obk_config.h"
#include "../driver/drv_public.h"
#include <ctype.h>
#include "cmd_local.h"

// bit mask telling which channels are hidden from HTTP
// If given bit is set, then given channel is hidden
int g_hiddenChannels = 0;
static char *g_channelLabels[CHANNEL_MAX] = { 0 };

void CHANNEL_SetLabel(int ch, const char *s) {
	if (ch < 0)
		return;
	if (ch >= CHANNEL_MAX)
		return;
	if (g_channelLabels[ch])
		free(g_channelLabels[ch]);
	g_channelLabels[ch] = strdup(s);
}
const char *CHANNEL_GetLabel(int ch) {
	if (ch >= 0 && ch < CHANNEL_MAX) {
		if (g_channelLabels[ch])
			return g_channelLabels[ch];
	}
	static char tmp[8];
	sprintf(tmp, "%i", ch);
	return tmp;
}

static commandResult_t CMD_SetChannelLabel(const void *context, const char *cmd, const char *args, int cmdFlags) {
	int ch;
	const char *s;

	Tokenizer_TokenizeString(args, TOKENIZER_ALLOW_QUOTES);
	// following check must be done after 'Tokenizer_TokenizeString',
	// so we know arguments count in Tokenizer. 'cmd' argument is
	// only for warning display
	if (Tokenizer_CheckArgsCountAndPrintWarning(cmd, 2)) {
		return CMD_RES_NOT_ENOUGH_ARGUMENTS;
	}

	ch = Tokenizer_GetArgInteger(0);
	s = Tokenizer_GetArg(1);

	CHANNEL_SetLabel(ch, s);

	return CMD_RES_OK;
}
static commandResult_t CMD_SetChannel(const void *context, const char *cmd, const char *args, int cmdFlags){
	int ch, val;

	Tokenizer_TokenizeString(args,0);
	// following check must be done after 'Tokenizer_TokenizeString',
	// so we know arguments count in Tokenizer. 'cmd' argument is
	// only for warning display
	if (Tokenizer_CheckArgsCountAndPrintWarning(cmd, 2)) {
		return CMD_RES_NOT_ENOUGH_ARGUMENTS;
	}

	ch = Tokenizer_GetArgInteger(0);
	val = Tokenizer_GetArgInteger(1);

	CHANNEL_Set(ch,val, false);

	return CMD_RES_OK;
}
static commandResult_t CMD_AddChannel(const void *context, const char *cmd, const char *args, int cmdFlags){
	int ch, val;
	int bWrapInsteadOfClamp;

	Tokenizer_TokenizeString(args,0);
	// following check must be done after 'Tokenizer_TokenizeString',
	// so we know arguments count in Tokenizer. 'cmd' argument is
	// only for warning display
	if (Tokenizer_CheckArgsCountAndPrintWarning(cmd, 2)) {
		return CMD_RES_NOT_ENOUGH_ARGUMENTS;
	}


	ch = Tokenizer_GetArgInteger(0);
	val = Tokenizer_GetArgInteger(1);
	if(Tokenizer_GetArgsCount() >= 4) {
		int min, max;

		min = Tokenizer_GetArgInteger(2);
		max = Tokenizer_GetArgInteger(3);
		if (Tokenizer_GetArgsCount() > 4) {
			bWrapInsteadOfClamp = Tokenizer_GetArgInteger(4);
		}
		else {
			bWrapInsteadOfClamp = 0;
		}

		CHANNEL_AddClamped(ch,val,min,max, bWrapInsteadOfClamp);
	} else {
		CHANNEL_Add(ch,val);
	}


	return CMD_RES_OK;
}
static commandResult_t CMD_ToggleChannel(const void *context, const char *cmd, const char *args, int cmdFlags){
	int ch;

	Tokenizer_TokenizeString(args,0);
	// following check must be done after 'Tokenizer_TokenizeString',
	// so we know arguments count in Tokenizer. 'cmd' argument is
	// only for warning display
	if (Tokenizer_CheckArgsCountAndPrintWarning(cmd, 1)) {
		return CMD_RES_NOT_ENOUGH_ARGUMENTS;
	}

	ch = Tokenizer_GetArgInteger(0);

	CHANNEL_Toggle(ch);

	return CMD_RES_OK;
}
static commandResult_t CMD_ClampChannel(const void *context, const char *cmd, const char *args, int cmdFlags){
	int ch, max, min;
	int bWrapInsteadOfClamp;

	Tokenizer_TokenizeString(args,0);
	// following check must be done after 'Tokenizer_TokenizeString',
	// so we know arguments count in Tokenizer. 'cmd' argument is
	// only for warning display
	if (Tokenizer_CheckArgsCountAndPrintWarning(cmd, 3)) {
		return CMD_RES_NOT_ENOUGH_ARGUMENTS;
	}

	ch = Tokenizer_GetArgInteger(0);
	min = Tokenizer_GetArgInteger(1);
	max = Tokenizer_GetArgInteger(2);
	bWrapInsteadOfClamp = 0;

	CHANNEL_AddClamped(ch,0, min, max, bWrapInsteadOfClamp);

	return CMD_RES_OK;
}

static commandResult_t CMD_SetPinRole(const void *context, const char *cmd, const char *args, int cmdFlags){
	int pin, roleIndex;
	const char *role;

	Tokenizer_TokenizeString(args,0);
	// following check must be done after 'Tokenizer_TokenizeString',
	// so we know arguments count in Tokenizer. 'cmd' argument is
	// only for warning display
	if (Tokenizer_CheckArgsCountAndPrintWarning(cmd, 2)) {
		return CMD_RES_NOT_ENOUGH_ARGUMENTS;
	}

	pin = Tokenizer_GetArgInteger(0);
	role = Tokenizer_GetArg(1);

	roleIndex = PIN_ParsePinRoleName(role);
	if(roleIndex == IOR_Total_Options) {
		ADDLOG_INFO(LOG_FEATURE_CMD, "Unknown role");
	} else {
		PIN_SetPinRoleForPinIndex(pin,roleIndex);
	}

	return CMD_RES_OK;
}
static commandResult_t CMD_SetPinChannel(const void *context, const char *cmd, const char *args, int cmdFlags){
	int pin, ch;

	Tokenizer_TokenizeString(args,0);
	// following check must be done after 'Tokenizer_TokenizeString',
	// so we know arguments count in Tokenizer. 'cmd' argument is
	// only for warning display
	if (Tokenizer_CheckArgsCountAndPrintWarning(cmd, 2)) {
		return CMD_RES_NOT_ENOUGH_ARGUMENTS;
	}

	pin = Tokenizer_GetArgInteger(0);
	ch = Tokenizer_GetArgInteger(1);

	PIN_SetPinChannelForPinIndex(pin,ch);

	return CMD_RES_OK;
}
static commandResult_t CMD_GetChannel(const void *context, const char *cmd, const char *args, int cmdFlags){
	int ch, val;

	Tokenizer_TokenizeString(args,0);
	// following check must be done after 'Tokenizer_TokenizeString',
	// so we know arguments count in Tokenizer. 'cmd' argument is
	// only for warning display
	if (Tokenizer_CheckArgsCountAndPrintWarning(cmd, 1)) {
		return CMD_RES_NOT_ENOUGH_ARGUMENTS;
	}

	ch = Tokenizer_GetArgInteger(0);
	val = CHANNEL_Get(ch);

	if(cmdFlags & COMMAND_FLAG_SOURCE_TCP) {
		ADDLOG_INFO(LOG_FEATURE_RAW, "%i", val);
	} else {
		ADDLOG_INFO(LOG_FEATURE_CMD, "Channel %i is %i",ch, val);
	}

	return CMD_RES_OK;
}
// See self test for usage
// MapRanges 10 0.15 0.2 0.4 0.8 1
static commandResult_t CMD_MapRanges(const void *context, const char *cmd, const char *args, int cmdFlags) {
	int targetCH;
	float useVal;
	int res = 0;
	int i;

	Tokenizer_TokenizeString(args, 0);
	// following check must be done after 'Tokenizer_TokenizeString',
	// so we know arguments count in Tokenizer. 'cmd' argument is
	// only for warning display
	if (Tokenizer_CheckArgsCountAndPrintWarning(cmd, 3)) {
		return CMD_RES_NOT_ENOUGH_ARGUMENTS;
	}

	targetCH = Tokenizer_GetArgInteger(0);
	useVal = Tokenizer_GetArgFloat(1);
	for (i = 2; i < Tokenizer_GetArgsCount(); i++) {
		float argVal = Tokenizer_GetArgFloat(i);
		if (argVal >= useVal) {
			break;
		}
		res++;
	}

	ADDLOG_INFO(LOG_FEATURE_CMD, "CMD_MapRanges: will set %i to %i",targetCH,res);
	CHANNEL_Set(targetCH, res, 0);

	return CMD_RES_OK;
}
// hide/show channel from gui
static commandResult_t CMD_SetChannelVisible(const void *context, const char *cmd, const char *args, int cmdFlags) {
	int targetCH;
	int bOn;

	Tokenizer_TokenizeString(args, 0);
	// following check must be done after 'Tokenizer_TokenizeString',
	// so we know arguments count in Tokenizer. 'cmd' argument is
	// only for warning display
	if (Tokenizer_CheckArgsCountAndPrintWarning(cmd, 2)) {
		return CMD_RES_NOT_ENOUGH_ARGUMENTS;
	}

	targetCH = Tokenizer_GetArgInteger(0);
	bOn = Tokenizer_GetArgInteger(1);

	if (bOn) {
		BIT_CLEAR(g_hiddenChannels, targetCH);
	}
	else {
		BIT_SET(g_hiddenChannels, targetCH);
	}

	return CMD_RES_OK;
}
static commandResult_t CMD_GetReadings(const void *context, const char *cmd, const char *args, int cmdFlags){
#ifndef OBK_DISABLE_ALL_DRIVERS
	char tmp[96];
	float v, c, p;
    float e, elh;

	v = DRV_GetReading(OBK_VOLTAGE);
	c = DRV_GetReading(OBK_CURRENT);
	p = DRV_GetReading(OBK_POWER);
    e = DRV_GetReading(OBK_CONSUMPTION_TOTAL);
    elh = DRV_GetReading(OBK_CONSUMPTION_LAST_HOUR);

	snprintf(tmp, sizeof(tmp), "%f %f %f %f %f",v,c,p,e,elh);

	if(cmdFlags & COMMAND_FLAG_SOURCE_TCP) {
		ADDLOG_INFO(LOG_FEATURE_RAW, tmp);
	} else {
		ADDLOG_INFO(LOG_FEATURE_CMD, "Readings are %s",tmp);
	}
#endif
	return CMD_RES_OK;
}
static commandResult_t CMD_ShortName(const void *context, const char *cmd, const char *args, int cmdFlags) {
	const char *s;
	Tokenizer_TokenizeString(args, TOKENIZER_ALLOW_QUOTES);

	s = CFG_GetShortDeviceName();
	if (Tokenizer_GetArgsCount() == 0) {
		if (cmdFlags & COMMAND_FLAG_SOURCE_TCP) {
			ADDLOG_INFO(LOG_FEATURE_RAW, s);
		}
		else {
			ADDLOG_INFO(LOG_FEATURE_CMD, "Name is %s", s);
		}
	}
	else {
		CFG_SetShortDeviceName(Tokenizer_GetArg(0));
	}
	return CMD_RES_OK;
}
static commandResult_t CMD_FriendlyName(const void *context, const char *cmd, const char *args, int cmdFlags) {
	const char *s;
	Tokenizer_TokenizeString(args, TOKENIZER_ALLOW_QUOTES);

	s = CFG_GetDeviceName();
	if (Tokenizer_GetArgsCount() == 0) {
		if (cmdFlags & COMMAND_FLAG_SOURCE_TCP) {
			ADDLOG_INFO(LOG_FEATURE_RAW, s);
		}
		else {
			ADDLOG_INFO(LOG_FEATURE_CMD, "FriendlyName is %s", s);
		}
	}
	else {
		CFG_SetDeviceName(Tokenizer_GetArg(0));
	}
	return CMD_RES_OK;
}
static commandResult_t CMD_FullBootTime(const void *context, const char *cmd, const char *args, int cmdFlags) {
	int v;
	Tokenizer_TokenizeString(args, 0);

	// following check must be done after 'Tokenizer_TokenizeString',
	// so we know arguments count in Tokenizer. 'cmd' argument is
	// only for warning display
	if (Tokenizer_CheckArgsCountAndPrintWarning(cmd, 1)) {
		return CMD_RES_NOT_ENOUGH_ARGUMENTS;
	}
	v = Tokenizer_GetArgInteger(0);

	CFG_SetBootOkSeconds(v);
	ADDLOG_INFO(LOG_FEATURE_CMD, "Boot time to %i",v);

	return CMD_RES_OK;
}
static commandResult_t CMD_SetFlag(const void *context, const char *cmd, const char *args, int cmdFlags) {
	const char *s;
	int flag;
	int bOn;
	Tokenizer_TokenizeString(args, 0);
	// following check must be done after 'Tokenizer_TokenizeString',
	// so we know arguments count in Tokenizer. 'cmd' argument is
	// only for warning display
	if (Tokenizer_CheckArgsCountAndPrintWarning(cmd, 2)) {
		return CMD_RES_NOT_ENOUGH_ARGUMENTS;
	}
	s = CFG_GetDeviceName();
	flag = Tokenizer_GetArgInteger(0);
	bOn = Tokenizer_GetArgInteger(1);

	CFG_SetFlag(flag, bOn);
	ADDLOG_INFO(LOG_FEATURE_CMD, "Flag %i set to %i",flag,bOn);

	return CMD_RES_OK;
}
extern int g_bWantPinDeepSleep;
static commandResult_t CMD_PinDeepSleep(const void *context, const char *cmd, const char *args, int cmdFlags){
	g_bWantPinDeepSleep = 1;
	return CMD_RES_OK;
}
void CMD_InitChannelCommands(){
	//cmddetail:{"name":"SetChannel","args":"[ChannelIndex][ChannelValue]",
	//cmddetail:"descr":"Sets a raw channel to given value. Relay channels are using 1 and 0 values. PWM channels are within [0,100] range. Do not use this for LED control, because there is a better and more advanced LED driver with dimming and configuration memory (remembers setting after on/off), LED driver commands has 'led_' prefix.",
	//cmddetail:"fn":"CMD_SetChannel","file":"cmnds/cmd_channels.c","requires":"",
	//cmddetail:"examples":""}
    CMD_RegisterCommand("SetChannel", CMD_SetChannel, NULL);
	//cmddetail:{"name":"ToggleChannel","args":"[ChannelIndex]",
	//cmddetail:"descr":"Toggles given channel value. Non-zero becomes zero, zero becomes 1.",
	//cmddetail:"fn":"CMD_ToggleChannel","file":"cmnds/cmd_channels.c","requires":"",
	//cmddetail:"examples":""}
    CMD_RegisterCommand("ToggleChannel", CMD_ToggleChannel, NULL);
	//cmddetail:{"name":"AddChannel","args":"[ChannelIndex][ValueToAdd][ClampMin][ClampMax][bWrapInsteadOfClamp]",
	//cmddetail:"descr":"Adds a given value to the channel. Can be used to change PWM brightness. Clamp min and max arguments are optional.",
	//cmddetail:"fn":"CMD_AddChannel","file":"cmnds/cmd_channels.c","requires":"",
	//cmddetail:"examples":""}
    CMD_RegisterCommand("AddChannel", CMD_AddChannel, NULL);
	//cmddetail:{"name":"ClampChannel","args":"[ChannelIndex][Min][Max]",
	//cmddetail:"descr":"Clamps given channel value to a range.",
	//cmddetail:"fn":"CMD_ClampChannel","file":"cmnds/cmd_channels.c","requires":"",
	//cmddetail:"examples":""}
    CMD_RegisterCommand("ClampChannel", CMD_ClampChannel, NULL);
	//cmddetail:{"name":"SetPinRole","args":"[PinRole][RoleIndexOrName]",
	//cmddetail:"descr":"This allows you to set a pin role, for example a Relay role, or Button, etc. Usually it's easier to do this through WWW panel, so you don't have to use this command.",
	//cmddetail:"fn":"CMD_SetPinRole","file":"cmnds/cmd_channels.c","requires":"",
	//cmddetail:"examples":""}
    CMD_RegisterCommand("SetPinRole", CMD_SetPinRole, NULL);
	//cmddetail:{"name":"SetPinChannel","args":"[PinRole][ChannelIndex]",
	//cmddetail:"descr":"This allows you to set a channel linked to pin from console. Usually it's easier to do this through WWW panel, so you don't have to use this command.",
	//cmddetail:"fn":"CMD_SetPinChannel","file":"cmnds/cmd_channels.c","requires":"",
	//cmddetail:"examples":""}
    CMD_RegisterCommand("SetPinChannel", CMD_SetPinChannel, NULL);
	//cmddetail:{"name":"GetChannel","args":"[ChannelIndex]",
	//cmddetail:"descr":"Prints given channel value to console.",
	//cmddetail:"fn":"CMD_GetChannel","file":"cmnds/cmd_channels.c","requires":"",
	//cmddetail:"examples":""}
    CMD_RegisterCommand("GetChannel", CMD_GetChannel, NULL);
	//cmddetail:{"name":"GetReadings","args":"",
	//cmddetail:"descr":"Prints voltage etc readings to console.",
	//cmddetail:"fn":"CMD_GetReadings","file":"cmnds/cmd_channels.c","requires":"",
	//cmddetail:"examples":""}
    CMD_RegisterCommand("GetReadings", CMD_GetReadings, NULL);
	//cmddetail:{"name":"ShortName","args":"[Name]",
	//cmddetail:"descr":"Sets the short name of the device.",
	//cmddetail:"fn":"CMD_ShortName","file":"cmnds/cmd_channels.c","requires":"",
	//cmddetail:"examples":""}
    CMD_RegisterCommand("ShortName", CMD_ShortName, NULL);
	//cmddetail:{"name":"FriendlyName","args":"[Name]",
	//cmddetail:"descr":"Sets the full name of the device",
	//cmddetail:"fn":"CMD_FriendlyName","file":"cmnds/cmd_channels.c","requires":"",
	//cmddetail:"examples":""}
	CMD_RegisterCommand("FriendlyName", CMD_FriendlyName, NULL);
	//cmddetail:{"name":"PinDeepSleep","args":"",
	//cmddetail:"descr":"Starts a pin deep sleep (deep sleep that can be interrupted by external IO events like a button press)",
	//cmddetail:"fn":"CMD_PinDeepSleep","file":"cmnds/cmd_channels.c","requires":"",
	//cmddetail:"examples":""}
	CMD_RegisterCommand("PinDeepSleep", CMD_PinDeepSleep, NULL);
	//cmddetail:{"name":"SetFlag","args":"[FlagIndex][1or0]",
	//cmddetail:"descr":"Enables/disables given flag.",
	//cmddetail:"fn":"CMD_SetFlag","file":"cmnds/cmd_channels.c","requires":"",
	//cmddetail:"examples":""}
	CMD_RegisterCommand("SetFlag", CMD_SetFlag, NULL);
	//cmddetail:{"name":"FullBootTime","args":"[Value]",
	//cmddetail:"descr":"Sets time in seconds after which boot is marked as valid. This is related to emergency AP mode which is enabled by powering on/off device 5 times quickly.",
	//cmddetail:"fn":"CMD_FullBootTime","file":"cmnds/cmd_channels.c","requires":"",
	//cmddetail:"examples":""}
	CMD_RegisterCommand("FullBootTime", CMD_FullBootTime, NULL);
	//cmddetail:{"name":"SetChannelLabel","args":"[ChannelIndex][Str]",
	//cmddetail:"descr":"Sets a channel label for UI.",
	//cmddetail:"fn":"CMD_SetChannelLabel","file":"cmnds/cmd_channels.c","requires":"",
	//cmddetail:"examples":""}
	CMD_RegisterCommand("SetChannelLabel", CMD_SetChannelLabel, NULL);
	//cmddetail:{"name":"MapRanges","args":"[TargetChannel][InputValue][RangeVal0][RangeVal1][RangeValN]",
	//cmddetail:"descr":"This will set given channel to an index showing where given input value is within given range sections. For example, MapRanges 10 0.5 0.3 0.6 0.9 will set channel 10 to 1 because 0.5 value is between 0.3 and 0.6",
	//cmddetail:"fn":"CMD_MapRanges","file":"cmnds/cmd_channels.c","requires":"",
	//cmddetail:"examples":""}
	CMD_RegisterCommand("MapRanges", CMD_MapRanges, NULL);
	//cmddetail:{"name":"SetChannelVisible","args":"",
	//cmddetail:"descr":"NULL",
	//cmddetail:"fn":"CMD_SetChannelVisible","file":"cmnds/cmd_channels.c","requires":"",
	//cmddetail:"examples":""}
	CMD_RegisterCommand("SetChannelVisible", CMD_SetChannelVisible, NULL);

}
