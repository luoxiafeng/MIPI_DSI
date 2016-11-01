#include <common.h>
#include <irq.h>
#include <regcheck.h>
#include <access.h>
#include <ids/ids.h>
#include <ids/hdmi.h>
#include <api.h>
#include <malloc.h>
#include <access.h>

static struct reg_check reg[] = {
	REG_CHECK_INIT(0x180,	0xff),
	REG_CHECK_INIT(0x181,   0xff),
};

int hdmi_reg_check(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(reg); i++)
		reg[i].addr = reg[i].addr * 4 + HDMI_TX_BASE_ADDR;
		   
	if (common_reg_check(reg, ARRAY_SIZE(reg), 0xffffffff) || common_reg_check(reg, ARRAY_SIZE(reg), 0))
			return -1;
	return 0;
}

#define CEC_NUM	16
static int cec_data[] = {16, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
int cec_test(void)
{
	int val;
	int i;
	
	val = access_Read(0x4001);
	val &= ~(0x1 << 5);
	access_Write(val, 0x4001);

	access_Write(0x0, 0x7d06);
	access_Write(0x1, 0x7D05);
	access_Write(0x2, 0x7D00);
	access_Write(0x10, 0x7D10);	
	
	for (i = 1;i < CEC_NUM;i++)
		access_Write(i, 0x7d10 + i);

	access_Write(CEC_NUM, 0x7D07);
	access_Write(0x3, 0x7d00);
	while (1) {
		val = access_Read(0x7d00);
		if (!(val & 0x1)) {
			printf("cec write completely\n");
			break;
		}	
	}
	access_Write(0x0, 0x7d30);
	while (1) {
		val = access_Read(0x7D08);
		if (val)
			break;
	}
	//printf("read num is %d\n", val);
	for (i = 0; i < val; i++)
		if (access_Read(0x7d20 + i) != cec_data[i]) {
			printf("cec read err: [%d, %d]\n", access_Read(0x7d20 + i), cec_data[i]);
			return -1;
		}
	return 0;
}

static struct hdmi_dev *hdmi = NULL;

int hdmi_set_hdcp(void)
{
	if (!hdmi->hdcp)
		hdmi->hdcp = (hdcpParams_t *)malloc(sizeof(hdcpParams_t));

	if (!hdmi->hdcp)
		return -1;

	hdcpParams_Reset(hdmi->hdcp);
	hdcpParams_SetEnable11Feature(hdmi->hdcp, 0);
	hdcpParams_SetEnhancedLinkVerification(hdmi->hdcp, 1);
	hdcpParams_SetI2cFastMode(hdmi->hdcp, 0);
	hdcpParams_SetRiCheck(hdmi->hdcp, 1);

	return 0;
}

#if 0
static int hdmi_set_name(void)
{
	const u8 vName[] = "Synopsys";
	const u8 pName[] = "Infotm";

	if (!hdmi->product)
		hdmi->product = (productParams_t *)malloc(sizeof(productParams_t));

	if (!hdmi->product)
		return -1;

	productParams_Reset(hdmi->product);
	productParams_SetVendorName(hdmi->product, vName, sizeof(vName) - 1);
	productParams_SetProductName(hdmi->product, pName, sizeof(pName) - 1);
	productParams_SetSourceType(hdmi->product, 0x0D);

	return 0;
}

static int hdmi_set_audio(void)
{
	if (!hdmi->audio)
		hdmi->audio = (audioParams_t *)malloc(sizeof(audioParams_t));

	if (!hdmi->audio)
		return -1;

	audioParams_Reset(hdmi->audio);
	audioParams_SetInterfaceType(hdmi->audio, I2S);
	audioParams_SetCodingType(hdmi->audio, PCM);
	audioParams_SetChannelAllocation(hdmi->audio, 0);
	audioParams_SetPacketType(hdmi->audio, AUDIO_SAMPLE);
	audioParams_SetSampleSize(hdmi->audio, 16);
	audioParams_SetSamplingFrequency(hdmi->audio, 48000);
	audioParams_SetLevelShiftValue(hdmi->audio, 0);
	audioParams_SetDownMixInhibitFlag(hdmi->audio, 0);
	audioParams_SetClockFsFactor(hdmi->audio, 64);

	return 0;
}

static int hdmi_set_video(void)
{
	dtd_t dtd;

	if (!hdmi->video)
		hdmi->video = (videoParams_t *)malloc(sizeof(videoParams_t));

	if (!hdmi->video)
		return -1;
	
	dtd_Fill(&dtd, 4, 60000);
	videoParams_Reset(hdmi->video);
	videoParams_SetHdmi(hdmi->video, 1);
	videoParams_SetEncodingIn(hdmi->video, RGB);
	videoParams_SetEncodingOut(hdmi->video, RGB);
	videoParams_SetColorResolution(hdmi->video, 8);
	videoParams_SetDtd(hdmi->video, &dtd);
	videoParams_SetPixelRepetitionFactor(hdmi->video, 0);
	videoParams_SetPixelPackingDefaultPhase(hdmi->video, 0);
	videoParams_SetColorimetry(hdmi->video, 1);
	videoParams_SetHdmiVideoFormat(hdmi->video, 0);
	videoParams_Set3dStructure(hdmi->video, 0);
	videoParams_Set3dExtData(hdmi->video, 0);
	return 0;
}

static void hdmi_set_cfg(void)
{
	hdmi_set_name();
	hdmi_set_audio();
	hdmi_set_video();
	//hdmi_set_hdcp();

	if (!api_Configure(hdmi->video, hdmi->audio, hdmi->product, hdmi->hdcp)) {
		ids_err("api set cfg err\n");
		return;
	}
	printf("hdmi_set_cfg ok\n");
}
#endif

void hdmi_set_output(enum hdmi_type format)
{
	if (hdmi && hdmi->video)
		videoParams_SetEncodingOut(hdmi->video, format);
}

static void hdmi_edid_callback(void *param)
{
	struct hdmi_priv *data = &hdmi->data;

	ids_dbg("read edid complete\n");
	data->edidstate = TRUE;
	
	//hdmi_set_cfg();
}

int hdmi_wait_edid(void)
{
	return hdmi->data.edidstate;
}

static void hdmi_hpd_callback(void *param)
{
	struct hdmi_priv *data = &hdmi->data;
	
	data->hpd = *(u8 *)param; 

	if (data->hpd == TRUE) {
		if (!api_EdidRead(hdmi_edid_callback)) {
			ids_err("read Edid err\n");
			return;
		}
	} else {
		if (!api_Standby()) {
			ids_err("Api standby err\n");
			return;
		}

		if (!api_Initialize(0, 1, 2500, 0)) {
			ids_err("api init err\n");
			return;
		}
		data->edidstate = FALSE;
	}
}
int hdmi_open(int mode)
{
	int ret;
	
	hdmi = (struct hdmi_dev *)malloc(sizeof(struct hdmi_dev));
	if (!hdmi)
		return -1;
	
	memset(hdmi, 0, sizeof(struct hdmi_dev));
	/* set hdmi base addr */
	access_Initialize((u8 *)HDMI_TX_BASE_ADDR);
	
	//log_SetVerbose(VERBOSE_TRACE);
	//log_SetVerboseWrite(1);
	pads_set_mode(85, PADS_MODE_OUT, 0, CPU_TYPE_AP);
	if (!api_Initialize(0, 1, 2500, 0)) {
		ids_err("hdmi api err\n");
		return -1;
	}

	api_EventEnable(HPD_EVENT, hdmi_hpd_callback, FALSE);
	if (mode) {
		ret = request_irq(HDMI_IRQ, (irq_handler_t)api_EventHandler, "hdmi hpd");
		if (ret < 0) {
			ids_err("request irq err\n");
			return -1;
		}
	}
	pads_set_mode(85, PADS_MODE_OUT, 1, CPU_TYPE_AP);
	return 0;
}

void hdmi_close(void)
{
	if (hdmi) {
		if (hdmi->video)
			free(hdmi->video);

		if (hdmi->audio)
			free(hdmi->audio);

		if (hdmi->hdcp)
			free(hdmi->hdcp);

		if (hdmi->product)
			free(hdmi->product);
	}
}

void hdmi_ctrl_switch(int flags)
{
	if (flags) {
		if (!api_Configure(hdmi->video, hdmi->audio, hdmi->product, hdmi->hdcp)) {
			ids_err("api set cfg err\n");
			return;
		}
	} else
		api_Standby();
}

void hdmi_init(int flags)
{
	int i;
	/* 
	 * Maybe hdmi do not need module power on
	 * but In x9 core, hdmi power ctrl in sysm
	 * is In ids1 sysm
	 */
	if (flags)
		module_enable("hdmi");

	/* In ids1 sysm ctrl: gate enable the iis and spdif */
	writel(0x3, 0x21E2C01C);
	/* set hdmi clk 24MHZ */
	module_set_clock("hdmi", "dpll", 24000000, 0);
	/* set gpio:
	 * function mode for hdmi_scl, hdmi_sda, hdmi_cec,
	 * other gpio (hdmi_dp*, hdmi_dn*, hdmi_ihpd) need 
	 * not be set.
	 */
	for (i = 94; i <= 96; i++)
		pads_set_mode(i, PADS_MODE_CTRL, 1, CPU_TYPE_AP);
}
