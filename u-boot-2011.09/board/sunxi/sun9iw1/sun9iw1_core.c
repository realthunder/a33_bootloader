/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Young <guoyingyang@allwinnertech.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <malloc.h>
#include <asm/io.h>
#include <fastboot.h>
#include "sun9iw1_core.h"
#include <sys_config.h>
extern void __usdelay(u32 time);
extern void jump_to_resume(void);
extern int get_cluster_id(void);
extern int get_cpu_id(void);



/* you can get current run_cpu idnum
	cluster0 : 0,1,2,3
	cluster1 : 4,5,6,7
	*/
int get_core_id(void)
{
	return ((get_cluster_id() * 4) + get_cpu_id());
}
unsigned static int sunxi_chip_rev(void)
{
	/* sun9iw1p1 chip revsion init */
	if ((readl(R_PRCM_BASE + 0x190) >> 0x3) & 0x1) {
		return   SUN9I_REV_B;
	} else {
		return  SUN9I_REV_A;
	}
}

static int sun9i_ca15_power_switch_set(unsigned int cluster, unsigned int cpu, u32 enable)
{
	if (sunxi_chip_rev() >= SUN9I_REV_B) {
		if (enable) {
			if (0x00 == readl(R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
				printf("%s: power switch enable already\n", __func__);
				return 0;
			}
			/* de-active cpu power clamp */
			writel(0xFE, R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
			udelay(20);

			writel(0xF8, R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
			udelay(10);

			writel(0xE0, R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
			udelay(10);

			writel(0x80, R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
			udelay(10);

			writel(0x00, R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
			udelay(20);
			while(0x00 != readl(R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
				;
			}
		} else {
			if (0xFF == readl(R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
				printf("%s: power switch disable already\n", __func__);
				return 0;
			}
			writel(0xFF, R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
			udelay(30);
			while(0xFF != readl(R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
				;
			}
		}
	} else {
		if (enable) {
			if (0xFF == readl(R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
				printf("%s: power switch enable already\n", __func__);
				return 0;
			}
			writel(0x01, R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
			udelay(20);

			writel(0x07, R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
			udelay(10);

			writel(0x1F, R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
			udelay(10);

			writel(0x7F, R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
			udelay(10);

			writel(0xFF, R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
			udelay(20);
			while(0xFF != readl(R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
				;
			}
		} else {
			if (0x00 == readl(R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
				printf("%s: power switch disable already\n", __func__);
				return 0;
			}
			writel(0x00, R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
			udelay(30);
			while(0x00 != readl(R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
				;
			}
		}
	}
	return 0;
}


static int sun9i_ca7_power_switch_set(unsigned int cluster, unsigned int cpu, u32 enable)
{
	if (enable) {
		if (0x00 == readl(R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
			printf("%s: power switch enable already\n", __func__);
			return 0;
		}
		/* de-active cpu power clamp */
		writel(0xFE, R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
		udelay(20);

		writel(0xF8, R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
		udelay(10);

		writel(0xE0, R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
		udelay(10);

		writel(0x80, R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
		udelay(10);

		writel(0x00, R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
		udelay(20);
		while(0x00 != readl(R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
			;
		}
	} else {
		if (0xFF == readl(R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
			printf("%s: power switch disable already\n", __func__);
			return 0;
		}
		writel(0xFF, R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
		udelay(30);
		while(0xFF != readl(R_PRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
			;
		}
	}
	return 0;
}

static int sun9i_cpu_power_switch_set(unsigned int cluster, unsigned int cpu, u32 enable)
{
	int ret;
	if (cluster == A15_CLUSTER) {
		ret = sun9i_ca15_power_switch_set(cluster, cpu, enable);
	} else {
		ret = sun9i_ca7_power_switch_set(cluster, cpu, enable);
	}
	return ret;
}

int sun9i_cpu_power_set(unsigned int cluster, unsigned int cpu, u32 enable)
{
	unsigned int value;

	if (enable) {
		/*
		 * power-up cpu core process
		 */
		printf("sun9i power-up cluster-%d cpu-%d\n", cluster, cpu);

		/* if boot cpu, should enable boot cpu hotplug first.*/
		//if (SUN9I_IS_BOOT_CPU(cluster, cpu)) {
		//	sun9i_boot_cpu_hotplug_enable(1);
		//}

		/* assert cpu core reset */
		value  = readl(CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		value &= (~(1<<cpu));
		writel(value, CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		udelay(10);

		/* assert cpu power-on reset */
		value  = readl(R_PRCM_BASE + SUNXI_CLUSTER_PWRON_RESET(cluster));
		value &= (~(1<<cpu));
		writel(value, R_PRCM_BASE + SUNXI_CLUSTER_PWRON_RESET(cluster));
		udelay(10);

		/* L1RSTDISABLE hold low */
		if (cluster == A7_CLUSTER) {
			/* L1RSTDISABLE control bit just use for A7_CLUSTER,
			 * the A15_CLUSTER default reset by hardware when power-up,
			 * software can't control it.
			 */
			value = readl(CPUCFG_BASE + SUNXI_CLUSTER_CTRL0(cluster));
			value &= ~(1<<cpu);
			writel(value, CPUCFG_BASE + SUNXI_CLUSTER_CTRL0(cluster));
		}

		/* release power switch */
		sun9i_cpu_power_switch_set(cluster, cpu, 1);

		/* clear power-off gating */
		value = readl(R_PRCM_BASE + SUNXI_CLUSTER_PWROFF_GATING(cluster));
		value &= (~(0x1<<cpu));
		writel(value, R_PRCM_BASE + SUNXI_CLUSTER_PWROFF_GATING(cluster));
		udelay(20);

		/* de-assert cpu power-on reset */
		value  = readl(R_PRCM_BASE + SUNXI_CLUSTER_PWRON_RESET(cluster));
		value |= ((1<<cpu));
		writel(value, R_PRCM_BASE + SUNXI_CLUSTER_PWRON_RESET(cluster));
		udelay(10);

		/* de-assert core reset */
		value  = readl(CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		value |= (1<<cpu);
		writel(value, CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		udelay(10);

		printf("sun9i power-up cluster-%d cpu-%d already\n", cluster, cpu);
	} else {
		/*
		 * power-down cpu core process
		 */
		printf("sun9i power-down cluster-%d cpu-%d\n", cluster, cpu);

		/* enable cpu power-off gating */
		value = readl(R_PRCM_BASE + SUNXI_CLUSTER_PWROFF_GATING(cluster));
		value |= (1 << cpu);
		writel(value, R_PRCM_BASE + SUNXI_CLUSTER_PWROFF_GATING(cluster));
		udelay(20);

		/* active the power output switch */
		sun9i_cpu_power_switch_set(cluster, cpu, 0);

		/* if boot cpu, should disable boot cpu hotplug.*/
		//if (SUN9I_IS_BOOT_CPU(cluster, cpu)) {
		//	sun9i_boot_cpu_hotplug_enable(0);
		//}
	//	printf("sun9i power-down cpu%d ok.\n", cpu);
	}
	return 0;
}


int sun9i_cluster_power_set(unsigned int cluster, u32 enable)
{
	unsigned int value;
	int          i;

	if (enable) {
		printf("sun9i power-up cluster-%d\n", cluster);

		/* active ACINACTM */
		value = readl(CPUCFG_BASE + SUNXI_CLUSTER_CTRL1(cluster));
		value |= (1<<0);
		writel(value, CPUCFG_BASE + SUNXI_CLUSTER_CTRL1(cluster));

		/* assert cluster cores resets */
		value = readl(CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		value &= (~(0xF<<0));   /* Core Reset    */
		writel(value, CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		udelay(10);

		/* assert cluster cores power-on reset */
		value = readl(R_PRCM_BASE + SUNXI_CLUSTER_PWRON_RESET(cluster));
		value &= (~(0xF));
		writel(value, R_PRCM_BASE + SUNXI_CLUSTER_PWRON_RESET(cluster));
		udelay(10);

		/* assert cluster resets */
		value = readl(CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		value &= (~(0x1<<24));  /* SOC DBG Reset */
		value &= (~(0xF<<16));  /* Debug Reset   */
		value &= (~(0x1<<12));  /* HReset        */
		value &= (~(0x1<<8));   /* L2 Cache Reset*/
		if (cluster == A7_CLUSTER) {
			value &= (~(0xF<<20));  /* ETM Reset     */
		} else {
			value &= (~(0xF<<4));   /* Neon Reset   */
		}
		writel(value, CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		udelay(10);

		/* Set L2RSTDISABLE LOW */
		if (cluster == A7_CLUSTER) {
			value = readl(CPUCFG_BASE + SUNXI_CLUSTER_CTRL0(cluster));
			value &= (~(0x1<<4));
			writel(value, CPUCFG_BASE + SUNXI_CLUSTER_CTRL0(cluster));
		} else {
			value = readl(CPUCFG_BASE + SUNXI_CLUSTER_CTRL0(cluster));
			value &= (~(0x1<<0));
			writel(value, CPUCFG_BASE + SUNXI_CLUSTER_CTRL0(cluster));
		}

		udelay(1000);

		/* clear cluster power-off gating */
		value = readl(R_PRCM_BASE + SUNXI_CLUSTER_PWROFF_GATING(cluster));
		value &= (~(0x1<<4));
		writel(value, R_PRCM_BASE + SUNXI_CLUSTER_PWROFF_GATING(cluster));
		udelay(20);

		/* de-active ACINACTM */
		value = readl(CPUCFG_BASE + SUNXI_CLUSTER_CTRL1(cluster));
		value &= (~(1<<0));
		writel(value, CPUCFG_BASE + SUNXI_CLUSTER_CTRL1(cluster));

		/* de-assert cores reset */
		value = readl(CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		value |= (0x1<<24);  /* SOC DBG Reset */
		value |= (0xF<<16);  /* Debug Reset   */
		value |= (0x1<<12);  /* HReset        */
		value |= (0x1<<8);   /* L2 Cache Reset*/
		if (cluster == A7_CLUSTER) {
			value |= (0xF<<20);  /* ETM Reset     */
		} else {
			value |= (0xF<<4);   /* Neon Reset   */
		}
		writel(value, CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
                udelay(20);

		/* de-assert cores power-on reset */
		value = readl(R_PRCM_BASE + SUNXI_CLUSTER_PWRON_RESET(cluster));
		value |= (0xF);
		writel(value, R_PRCM_BASE + SUNXI_CLUSTER_PWRON_RESET(cluster));
		udelay(60);

		/* de-assert cores reset */
		value = readl(CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		value |= (0xF<<0);   /* Core Reset    */
		writel(value, CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
                udelay(20);

		printf("sun9i power-up cluster-%d ok\n", cluster);

	} else {

		printf("sun9i power-down cluster-%d\n", cluster);

		/* active ACINACTM */
		value = readl(CPUCFG_BASE + SUNXI_CLUSTER_CTRL1(cluster));
		value |= (1<<0);
		writel(value, CPUCFG_BASE + SUNXI_CLUSTER_CTRL1(cluster));

		/* assert cluster cores resets */
		value = readl(CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		value &= (~(0xF<<0));   /* Core Reset    */
		writel(value, CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		udelay(10);

		/* assert cluster cores power-on reset */
		value = readl(R_PRCM_BASE + SUNXI_CLUSTER_PWRON_RESET(cluster));
		value &= (~(0xF));
		writel(value, R_PRCM_BASE + SUNXI_CLUSTER_PWRON_RESET(cluster));
		udelay(10);

		/* assert cluster resets */
		value = readl(CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		value &= (~(0x1<<24));  /* SOC DBG Reset */
		value &= (~(0xF<<16));  /* Debug Reset   */
		value &= (~(0x1<<12));  /* HReset        */
		value &= (~(0x1<<8));   /* L2 Cache Reset*/
		if (cluster == A7_CLUSTER) {
			value &= (~(0xF<<20));  /* ETM Reset     */
		} else {
			value &= (~(0xF<<4));   /* Neon Reset   */
		}
		writel(value, CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		udelay(10);

		/* enable cluster and cores power-off gating */
		value = readl(R_PRCM_BASE + SUNXI_CLUSTER_PWROFF_GATING(cluster));
		value |= (1<<4);
		value |= (0xF<<0);
		writel(value, R_PRCM_BASE + SUNXI_CLUSTER_PWROFF_GATING(cluster));
		udelay(20);

		/* disable cluster cores power switch */
		for (i = 0; i < 4; i++) {
			sun9i_cpu_power_switch_set(cluster, i, 0);
		}		
		printf("sun9i power-down cluster-%d ok\n", cluster);
	}

	return 0;
}

#define SUNXI_SID_PBASE            0x01c0e000
unsigned int sunxi_get_soc_bin(void)
{
	unsigned int sunxi_soc_chipid[4];
	u32 type = 0;
	/* sun9iw1p1 soc chip id init */
	sunxi_soc_chipid[0] = readl(SUNXI_SID_PBASE + 0x200);
	sunxi_soc_chipid[1] = readl(SUNXI_SID_PBASE + 0x200 + 0x4);
	sunxi_soc_chipid[2] = readl(SUNXI_SID_PBASE + 0x200 + 0x8);
	sunxi_soc_chipid[3] = readl(SUNXI_SID_PBASE + 0x200 + 0xc);

	type = (sunxi_soc_chipid[1] >> 14) & 0x3f;
	printf("chip id type is %d\n",type);
	switch (type)
	{
		case 0b000000: //default
			return 0;
		case 0b000001: //normal
			return 1;
		case 0b000011: //
			return 0;
		case 0b000111:
			return 2;
		default:
			return 0;
	}
}

uint sunxi_scan_VF_table(uint table_num,uint boot_clock)
{
	char vf_table[16];
	char freq_lel[16];
	char volt_lel[16];
	uint volt = 0;
	uint max_freq = 0;
	uint min_freq = 0;
	uint freq= 0;
	uint set_clock = 0;
	uint lel_num = 0;
	int i = 0;
	if(boot_clock == 0)
	{
		printf("boot_clock is Invalid \n");
		return -1;
	}

	memset(vf_table , 0x00, 16);
	memset(freq_lel, 0x00,16);
	memset(volt_lel , 0x00,16);
	sprintf(vf_table ,"%s%d","vf_table",table_num);
	printf("======= %s =====\n",vf_table);

	if(script_parser_fetch(vf_table,"B_max_freq",(int*)&max_freq,sizeof(int)/4))
	{
		printf("can not find B_max_freq from script \n");
		return -1;
	}
	printf("max_freq is %d \n",max_freq);
	if(script_parser_fetch(vf_table,"B_min_freq",(int*)&min_freq,sizeof(int)/4))
	{
		printf("can not find B_min_freq from script \n");
		return -1;
	}
	printf("min_freq is %d \n",min_freq);

	set_clock = boot_clock * 1000000 ;
	if(set_clock > max_freq)
		set_clock = max_freq;
	if(set_clock < min_freq)
		set_clock = min_freq;
	printf("clock is %d MHZ\n",set_clock/1000000);

	if(script_parser_fetch(vf_table,"B_LV_count",(int*)&lel_num,sizeof(int)/4))
	{
		printf("can not find B_LV_count from script \n");
		return -1;
	}
	printf("lel_num is %d\n",lel_num);
	for(i = lel_num; i >= 0;i--)
	{
		sprintf(freq_lel,"%s%d%s","B_LV",i,"_freq");
		if(script_parser_fetch(vf_table,freq_lel,(int*)&freq,sizeof(int)/4)) 
		{
			printf("can not find B_LV_count from script \n");
			return -1;
		}
		printf("==== %s = %d ====\n",freq_lel,freq);

		if(set_clock <= freq)
		{
			sprintf(volt_lel,"%s%d%s","B_LV",i,"_volt");
			if(script_parser_fetch(vf_table,volt_lel,(int*)&volt,sizeof(int)/4))
			{
				printf("can not find volt_lel from script \n");
				return -1;
			}
			printf("find volt_lel = %d \n",volt);
			return volt;
		}
	}
	return -1;
}
