#include "disp_hdmi.h"

struct disp_device_private_data {
	u32 enabled;

	disp_tv_mode mode;

	disp_hdmi_func hdmi_func;
	disp_video_timings *video_info;

	struct disp_clk_info lcd_clk;
};

static u32 hdmi_used = 0;

#if defined(__LINUX_PLAT__)
static spinlock_t hdmi_data_lock;
#else
static int hdmi_data_lock;
#endif

static struct disp_device *hdmis = NULL;
static struct disp_device_private_data *hdmi_private = NULL;
s32 disp_hdmi_set_mode(struct disp_device* hdmi, u32 mode);
s32 disp_hdmi_enable(struct disp_device* hdmi);

static struct disp_device_private_data *disp_hdmi_get_priv(struct disp_device *hdmi)
{
	if(NULL == hdmi) {
		DE_WRN("NULL hdl!\n");
		return NULL;
	}

	if(!bsp_disp_feat_is_supported_output_types(hdmi->disp, DISP_OUTPUT_TYPE_HDMI)) {
	    DE_WRN("screen %d do not support HDMI TYPE!\n", hdmi->disp);
	    return NULL;
	}

	return &hdmi_private[hdmi->disp];
}

static s32 hdmi_clk_init(struct disp_device *hdmi)
{
	struct disp_device_private_data *hdmip = disp_hdmi_get_priv(hdmi);
	if(!hdmi || !hdmip) {
	    DE_WRN("hdmi clk init null hdl!\n");
	    return DIS_FAIL;
	}

	return 0;
}

static s32 hdmi_clk_exit(struct disp_device *hdmi)
{
	struct disp_device_private_data *hdmip = disp_hdmi_get_priv(hdmi);
	if(!hdmi || !hdmip) {
		DE_WRN("hdmi clk init null hdl!\n");
		return DIS_FAIL;
	}

	return 0;
}

static s32 hdmi_clk_config(struct disp_device *hdmi)
{
	struct disp_device_private_data *hdmip = disp_hdmi_get_priv(hdmi);
	if(!hdmi || !hdmip) {
	    DE_WRN("hdmi clk init null hdl!\n");
	    return DIS_FAIL;
	}

	return 0;
}

static s32 hdmi_clk_enable(struct disp_device *hdmi)
{
	struct disp_device_private_data *hdmip = disp_hdmi_get_priv(hdmi);
	if(!hdmi || !hdmip) {
	    DE_WRN("hdmi clk init null hdl!\n");
	    return DIS_FAIL;
	}

	hdmi_clk_config(hdmi);

	return 0;
}

static s32 hdmi_clk_disable(struct disp_device *hdmi)
{
	struct disp_device_private_data *hdmip = disp_hdmi_get_priv(hdmi);
	if(!hdmi || !hdmip) {
	    DE_WRN("hdmi clk init null hdl!\n");
	    return DIS_FAIL;
	}

	return 0;
}

//--------------------------------
//----hdmi interface functions----
//--------------------------------

static s32 disp_hdmi_set_func(struct disp_device*  hdmi, disp_hdmi_func * func)
{
	struct disp_device_private_data *hdmip = disp_hdmi_get_priv(hdmi);
	if((NULL == hdmi) || (NULL == hdmip)) {
		DE_WRN("hdmi set func null  hdl!\n");
		return DIS_FAIL;
	}

	hdmip->hdmi_func.hdmi_open = func->hdmi_open;
	hdmip->hdmi_func.hdmi_close= func->hdmi_close;
	hdmip->hdmi_func.hdmi_set_mode= func->hdmi_set_mode;
	hdmip->hdmi_func.hdmi_mode_support= func->hdmi_mode_support;
	hdmip->hdmi_func.hdmi_get_input_csc= func->hdmi_get_input_csc;
	hdmip->hdmi_func.hdmi_set_pll = func->hdmi_set_pll;
	hdmip->hdmi_func.hdmi_get_video_timing_info = func->hdmi_get_video_timing_info;
	hdmip->hdmi_func.hdmi_suspend = func->hdmi_suspend;
	hdmip->hdmi_func.hdmi_resume = func->hdmi_resume;
	hdmip->hdmi_func.hdmi_get_HPD_status = func->hdmi_get_HPD_status;

	return 0;
}

static s32 disp_hdmi_init(struct disp_device*  hdmi)
{
	struct disp_device_private_data *hdmip = disp_hdmi_get_priv(hdmi);
	if(!hdmi || !hdmip) {
	    DE_WRN("hdmi init null hdl!\n");
	    return DIS_FAIL;
	}

	hdmi_clk_init(hdmi);
	return 0;
}

static s32 disp_hdmi_exit(struct disp_device* hdmi)
{
	struct disp_device_private_data *hdmip = disp_hdmi_get_priv(hdmi);
	if(!hdmi || !hdmip) {
	    DE_WRN("hdmi init null hdl!\n");
	    return DIS_FAIL;
	}

	hdmi_clk_exit(hdmi);

  return 0;
}

s32 disp_hdmi_enable(struct disp_device* hdmi)
{
	unsigned long flags;
	struct disp_device_private_data *hdmip = disp_hdmi_get_priv(hdmi);
	struct disp_manager *mgr = NULL;
	if((NULL == hdmi) || (NULL == hdmip)) {
		DE_WRN("hdmi set func null  hdl!\n");
		return DIS_FAIL;
	}
	mgr = hdmi->manager;
	if(!mgr) {
		DE_WRN("hdmi%d's mgr is NULL\n", hdmi->disp);
		return DIS_FAIL;
	}

	if(hdmip->hdmi_func.hdmi_get_video_timing_info == NULL) {
		DE_WRN("hdmi_get_video_timing_info func is null\n");
		return DIS_FAIL;
	}

	hdmip->hdmi_func.hdmi_get_video_timing_info(&(hdmip->video_info));

	if(hdmip->video_info == NULL) {
		DE_WRN("video info is null\n");
		return DIS_FAIL;
	}
	memcpy(&hdmi->timings, hdmip->video_info, sizeof(disp_video_timings));
	if(mgr->enable)
		mgr->enable(mgr);

	hdmi_clk_enable(hdmi);
	disp_al_hdmi_cfg(hdmi->disp, hdmip->video_info);
	disp_al_hdmi_enable(hdmi->disp);

	if(NULL != hdmip->hdmi_func.hdmi_open)
		hdmip->hdmi_func.hdmi_open();
	else
		DE_WRN("hdmi_open is NULL\n");

	disp_sys_irqlock((void*)&hdmi_data_lock, &flags);
	hdmip->enabled = 1;
	disp_sys_irqunlock((void*)&hdmi_data_lock, &flags);

	return 0;
}

static s32 disp_hdmi_disable(struct disp_device* hdmi)
{
	struct disp_device_private_data *hdmip = disp_hdmi_get_priv(hdmi);
	unsigned long flags;
	struct disp_manager *mgr = NULL;

	if((NULL == hdmi) || (NULL == hdmip)) {
	    DE_WRN("hdmi set func null  hdl!\n");
	    return DIS_FAIL;
	}

	mgr = hdmi->manager;
	if(!mgr) {
		DE_WRN("hdmi%d's mgr is NULL\n", hdmi->disp);
		return DIS_FAIL;
	}

	if(hdmip->enabled == 0) {
		DE_WRN("hdmi%d is already closed\n", hdmi->disp);
		return DIS_FAIL;
	}

	disp_al_hdmi_disable(hdmi->disp);
	hdmi_clk_disable(hdmi);

	if(hdmip->hdmi_func.hdmi_close == NULL)
	    return -1;

	hdmip->hdmi_func.hdmi_close();

	if(mgr->disable)
		mgr->disable(mgr);

	disp_sys_irqlock((void*)&hdmi_data_lock, &flags);
	hdmip->enabled = 0;
	disp_sys_irqunlock((void*)&hdmi_data_lock, &flags);

	return 0;
}

static s32 disp_hdmi_is_enabled(struct disp_device* hdmi)
{
	struct disp_device_private_data *hdmip = disp_hdmi_get_priv(hdmi);
	if((NULL == hdmi) || (NULL == hdmip)) {
		DE_WRN("hdmi set func null  hdl!\n");
		return DIS_FAIL;
	}

	return hdmip->enabled;
}


s32 disp_hdmi_set_mode(struct disp_device* hdmi, u32 mode)
{
	s32 ret = 0;
	struct disp_device_private_data *hdmip = disp_hdmi_get_priv(hdmi);
	if((NULL == hdmi) || (NULL == hdmip)) {
		DE_WRN("hdmi set func null  hdl!\n");
		return DIS_FAIL;
	}

	if(hdmip->hdmi_func.hdmi_set_mode == NULL) {
		DE_WRN("hdmi_set_mode is null!\n");
		return -1;
	}

	ret = hdmip->hdmi_func.hdmi_set_mode((disp_tv_mode)mode);

	if(ret == 0)
		hdmip->mode = mode;

	return ret;
}

static s32 disp_hdmi_get_mode(struct disp_device* hdmi)
{
	struct disp_device_private_data *hdmip = disp_hdmi_get_priv(hdmi);
	if((NULL == hdmi) || (NULL == hdmip)) {
		DE_WRN("hdmi set func null  hdl!\n");
		return DIS_FAIL;
	}

	return hdmip->mode;
}

static s32 disp_hdmi_detect(struct disp_device* hdmi)
{
	struct disp_device_private_data *hdmip = disp_hdmi_get_priv(hdmi);
	if((NULL == hdmi) || (NULL == hdmip)) {
		DE_WRN("hdmi set func null  hdl!\n");
		return DIS_FAIL;
	}
	if (hdmip->hdmi_func.hdmi_get_HPD_status)
		return hdmip->hdmi_func.hdmi_get_HPD_status();
	return DIS_FAIL;
}

static s32 disp_hdmi_check_support_mode(struct disp_device* hdmi, u32 mode)
{
	struct disp_device_private_data *hdmip = disp_hdmi_get_priv(hdmi);
	if((NULL == hdmi) || (NULL == hdmip)) {
		DE_WRN("hdmi set func null  hdl!\n");
		return DIS_FAIL;
	}

	if(hdmip->hdmi_func.hdmi_mode_support == NULL)
		return -1;

	return hdmip->hdmi_func.hdmi_mode_support(mode);
}

static s32 disp_hdmi_get_input_csc(struct disp_device* hdmi)
{
	struct disp_device_private_data *hdmip = disp_hdmi_get_priv(hdmi);
	if((NULL == hdmi) || (NULL == hdmip)) {
		DE_WRN("hdmi set func null  hdl!\n");
		return DIS_FAIL;
	}

	if(hdmip->hdmi_func.hdmi_get_input_csc == NULL)
		return 0;

	return hdmip->hdmi_func.hdmi_get_input_csc();
}

static s32 disp_hdmi_get_input_color_range(struct disp_device* hdmi)
{
	struct disp_device_private_data *hdmip = disp_hdmi_get_priv(hdmi);
	if((NULL == hdmi) || (NULL == hdmip)) {
		DE_WRN("hdmi set func null  hdl!\n");
		return DIS_FAIL;
	}

	return DISP_COLOR_RANGE_16_235;
}

static s32 disp_hdmi_suspend(struct disp_device* hdmi)
{
	struct disp_device_private_data *hdmip = disp_hdmi_get_priv(hdmi);
	DE_WRN("\n");
	if((NULL == hdmi) || (NULL == hdmip)) {
		DE_WRN("hdmi set func null  hdl!\n");
		return DIS_FAIL;
	}

	if(hdmip->hdmi_func.hdmi_suspend != NULL) {
		hdmip->hdmi_func.hdmi_suspend();
	}

	return 0;
}

static s32 disp_hdmi_resume(struct disp_device* hdmi)
{
	struct disp_device_private_data *hdmip = disp_hdmi_get_priv(hdmi);
	DE_WRN("\n");
	if((NULL == hdmi) || (NULL == hdmip)) {
		DE_WRN("hdmi set func null  hdl!\n");
		return DIS_FAIL;
	}

	if(hdmip->hdmi_func.hdmi_resume != NULL) {
		hdmip->hdmi_func.hdmi_resume();
	}

	return 0;
}

s32 disp_init_hdmi(disp_bsp_init_para * para)
{
	s32 ret;
	s32 value;
	//get sysconfig hdmi_used
	ret = disp_sys_script_get_item("hdmi_para", "hdmi_used", &value, 1);
	if(ret == 1)
		hdmi_used = value;

	if(hdmi_used) {
		u32 num_screens;
		u32 disp;
		struct disp_device* hdmi;
		struct disp_device_private_data* hdmip;

		DE_INF("disp_init_hdmi\n");
#if defined(__LINUX_PLAT__)
		spin_lock_init(&hdmi_data_lock);
#endif

		num_screens = bsp_disp_feat_get_num_screens();
		hdmis = (struct disp_device *)disp_sys_malloc(sizeof(struct disp_device) * num_screens);
		if(NULL == hdmis) {
			DE_WRN("malloc memory fail!\n");
			return DIS_FAIL;
		}

		hdmi_private = (struct disp_device_private_data *)disp_sys_malloc(sizeof(struct disp_device_private_data) * num_screens);
		if(NULL == hdmi_private) {
			DE_WRN("malloc memory fail!\n");
			return DIS_FAIL;
		}

		for(disp=0; disp<num_screens; disp++) {
			hdmi = &hdmis[disp];
			hdmip = &hdmi_private[disp];

			if(!bsp_disp_feat_is_supported_output_types(disp, DISP_OUTPUT_TYPE_HDMI)) {
			    DE_WRN("screen %d do not support HDMI TYPE!\n", disp);
			    continue;
			}

			hdmi->disp = disp;
			sprintf(hdmi->name, "hdmi%d", disp);
			hdmi->type = DISP_OUTPUT_TYPE_HDMI;
			hdmip->mode = DISP_TV_MOD_720P_50HZ;

			hdmi->set_manager = disp_device_set_manager;
			hdmi->unset_manager = disp_device_unset_manager;
			hdmi->get_resolution = disp_device_get_resolution;
			hdmi->get_timings = disp_device_get_timings;

			hdmi->init = disp_hdmi_init;
			hdmi->exit = disp_hdmi_exit;

			hdmi->set_func = disp_hdmi_set_func;
			hdmi->enable = disp_hdmi_enable;
			hdmi->disable = disp_hdmi_disable;
			hdmi->is_enabled = disp_hdmi_is_enabled;
			hdmi->set_mode = disp_hdmi_set_mode;
			hdmi->get_mode = disp_hdmi_get_mode;
			hdmi->check_support_mode = disp_hdmi_check_support_mode;
			hdmi->get_input_csc = disp_hdmi_get_input_csc;
			hdmi->get_input_color_range = disp_hdmi_get_input_color_range;
			hdmi->suspend = disp_hdmi_suspend;
			hdmi->resume = disp_hdmi_resume;
			hdmi->detect = disp_hdmi_detect;

			hdmi->init(hdmi);
			if(bsp_disp_feat_is_supported_output_types(disp, DISP_OUTPUT_TYPE_HDMI))
				disp_device_register(hdmi);
		}
	}
	return 0;
}
