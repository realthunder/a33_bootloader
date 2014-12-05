#ifndef __HDMI_HW_INTERFACE_H_
#define __HDMI_HW_INTERFACE_H_

typedef void (*hdmi_udelay) (__u32 us);

int api_set_func(hdmi_udelay udelay);
void sunxi_video_config(u32 vic);
void sunxi_audio_config(s32 sample_rate);
int sunxi_get_hpd(void);
void sunxi_set_reg_base(unsigned int address);

#endif /* API_H_ */

