#include "../new_common.h"
#include "../cmnds/cmd_public.h"
#include "../logging/logging.h"
#include "drv_local.h"

static float base_v = 120;
static float base_c = 1;
static float base_p = 120;
static bool bAllowRandom = true;

commandResult_t TestPower_Setup(const void* context, const char* cmd, const char* args, int cmdFlags) {
	

	Tokenizer_TokenizeString(args, TOKENIZER_ALLOW_QUOTES | TOKENIZER_DONT_EXPAND);
	// following check must be done after 'Tokenizer_TokenizeString',
	// so we know arguments count in Tokenizer. 'cmd' argument is
	// only for warning display
	if (Tokenizer_CheckArgsCountAndPrintWarning(cmd, 3)) {
		return CMD_RES_NOT_ENOUGH_ARGUMENTS;
	}
	base_v = Tokenizer_GetArgFloat(0);
	base_c = Tokenizer_GetArgFloat(1);
	base_p = Tokenizer_GetArgFloat(2);
	bAllowRandom = Tokenizer_GetArgInteger(3);


	return CMD_RES_OK;
}
//Test Power driver
void Test_Power_Init() {
	BL_Shared_Init();

	//cmddetail:{"name":"SetupTestPower","args":"",
	//cmddetail:"descr":"NULL",
	//cmddetail:"fn":"TestPower_Setup","file":"driver/drv_test_drivers.c","requires":"",
	//cmddetail:"examples":""}
	CMD_RegisterCommand("SetupTestPower", TestPower_Setup, NULL);
}
void Test_Power_RunFrame() {
	float final_v = base_v;
	float final_c = base_c;
	float final_p = base_p;

	if (bAllowRandom) {
		final_c += (rand() % 100) * 0.001f;
		final_v += (rand() % 100) * 0.1f;
		final_p += (rand() % 100) * 0.1f;
	}
	BL_ProcessUpdate(final_v, final_c, final_p);
}


static volatile unsigned long test_tick = 0;
//Test LED driver
void __attribute__((section(".code_IRAM"))) TestFunction(void)
{
    register int i;

    GLOBAL_INT_DECLARATION();
    GLOBAL_INT_DISABLE();

    for(i=0;i<8;i++)
        __asm("nop");
    test_tick = test_tick + 1;

    GLOBAL_INT_RESTORE();
}


void Test_LED_Driver_Init() 
{
    addLogAdv(LOG_INFO, LOG_FEATURE_DRV, "RAM_FUNC: %08lXh\n", (unsigned long)(TestFunction));
    //TestFunction();
}

void Test_LED_Driver_RunFrame() 
{
    addLogAdv(LOG_INFO, LOG_FEATURE_DRV, "RAM_FUNC:  %08lXh\n", (unsigned long)(TestFunction));
    addLogAdv(LOG_INFO, LOG_FEATURE_DRV, "TEST_TICK: %lu\n", (unsigned long)test_tick);
}

void Test_LED_Driver_RunQuick()
{

}

/*
 * Test_LED_Driver_OnChannelChanged function
 *
 * Input:
 *  int ch - channel number
 *  int value - new channel value
 *
 * Output:
 *  int     - ==0 - channel not found
 *          - !=0 - channel value updated.
 *
 */
int Test_LED_Driver_OnChannelChanged(int ch, int value) {
    return 0;
}

