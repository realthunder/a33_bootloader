#ifndef __MMC_DEF__
#define __MMC_DEF__

//#define SUNXI_MMCDBG

#ifdef SUNXI_MMCDBG
#define MMCINFO(fmt...)	tick_printf("[mmc]: "fmt)//err and info
#define MMCDBG(fmt...)	tick_printf("[mmc]: "fmt)//dbg
#define MMCMSG(fmt...)	tick_printf(fmt)//data or register and so on
#else
#define MMCINFO(fmt...)	tick_printf("[mmc]: "fmt)
#define MMCDBG(fmt...)
#define MMCMSG(fmt...)
#endif


#define DRIVER_VER  "2014-07-15 17:16:00"

//#define TUNING_LEN		(1)//The address which store the tuninng pattern
//#define TUNING_ADD		(38192-TUNING_LEN)//The address which store the tuninng pattern
#define TUNING_LEN		(10)//The length of the tuninng pattern
#define TUNING_ADD		(38192-2-TUNING_LEN)//The address which store the tuninng pattern
#define REPEAT_TIMES	(30)
#define SAMPLE_MODE 	(2)
#define AUTO_SAMPLE_MODE (2)

#endif
