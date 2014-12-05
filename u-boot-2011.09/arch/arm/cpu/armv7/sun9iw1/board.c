/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
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
#include <asm/io.h>
#include <pmu.h>
#include <asm/arch/timer.h>
#include <asm/arch/key.h>
#include <asm/arch/clock.h>
#include <asm/arch/cpu.h>
#include <asm/arch/sys_proto.h>
#include <boot_type.h>
#include <sys_partition.h>
#include <sys_config.h>

/* The sunxi internal brom will try to loader external bootloader
 * from mmc0, nannd flash, mmc2.
 * We check where we boot from by checking the config
 * of the gpio pin.
 */
DECLARE_GLOBAL_DATA_PTR;

u32 get_base(void)
{

	u32 val;

	__asm__ __volatile__("mov %0, pc \n":"=r"(val)::"memory");
	val &= 0xF0000000;
	val >>= 28;
	return val;
}

/* do some early init */
void s_init(void)
{
	watchdog_disable();
}

void reset_cpu(ulong addr)
{
	watchdog_enable();
#ifndef CONFIG_A50_FPGA
loop_to_die:
	goto loop_to_die;
#endif
}

void v7_outer_cache_enable(void)
{
	return ;
}

void v7_outer_cache_inval_all(void)
{
	return ;
}

void v7_outer_cache_flush_range(u32 start, u32 stop)
{
	return ;
}

void enable_caches(void)
{
    icache_enable();
    dcache_enable();
}

void disable_caches(void)
{
    icache_disable();
	dcache_disable();
}

int display_inner(void)
{
	tick_printf("version: %s\n", uboot_spare_head.boot_head.version);

	return 0;
}

int script_init(void)
{
    uint offset, length;
	char *addr;

	offset = uboot_spare_head.boot_head.uboot_length;
	length = uboot_spare_head.boot_head.length - uboot_spare_head.boot_head.uboot_length;
	addr   = (char *)CONFIG_SYS_TEXT_BASE + offset;

    debug("script offset=%x, length = %x\n", offset, length);

	if(length)
	{
		memcpy((void *)SYS_CONFIG_MEMBASE, addr, length);
		script_parser_init((char *)SYS_CONFIG_MEMBASE);
	}
	else
	{
		script_parser_init(NULL);
	}

	return 0;
}

struct bias_set
{
	int  vol;
	int  index;
};

int power_config_gpio_bias(void)
{
	char gpio_bias[32], gpio_name[32];
	char *gpio_name_const="pa_bias";
	char port_index;
	char *axp=NULL, *supply=NULL, *vol=NULL;
	uint main_hd;
	uint bias_vol_set;
	int  index, ret, i;
	uint port_bias_addr;
	uint vol_index, config_type;
	int  pmu_vol;
	struct bias_set bias_vol_config[8] =
		{ {1800, 0}, {2500, 6}, {2800, 9}, {3000, 0xa}, {3300, 0xd}, {0, 0} };

	main_hd = script_parser_fetch_subkey_start("gpio_bias");

	index = 0;
	while(1)
	{
		memset(gpio_bias, 0, 32);
		memset(gpio_name, 0, 32);
		ret = script_parser_fetch_subkey_next(main_hd, gpio_name, (int *)gpio_bias, &index);
		if(!ret)
		{
			lower(gpio_name);
			lower(gpio_bias);

			port_index = gpio_name[1];
			gpio_name[1] = 'a';
			if(strcmp(gpio_name_const, gpio_name))
			{
				printf("invalid gpio bias name %s\n", gpio_name);

				continue;
			}
			gpio_name[1] = port_index;
			i=0;
			axp = gpio_bias;
			while( (gpio_bias[i]!=':') && (gpio_bias[i]!='\0') )
			{
				i++;
			}
			gpio_bias[i++]='\0';

			if(!strcmp(axp, "constant"))
			{
				config_type = 1;
			}
			else if(!strcmp(axp, "floating"))
			{
				printf("ignore %s bias config\n", gpio_name);

				continue;
			}
			else
			{
				config_type = 0;
			}

			if(config_type == 0)
			{
				supply = gpio_bias + i;
				while( (gpio_bias[i]!=':') && (gpio_bias[i]!='\0') )
				{
					i++;
				}
				gpio_bias[i++]='\0';
			}

			printf("supply=%s\n", supply);
			vol = gpio_bias + i;
			while( (gpio_bias[i]!=':') && (gpio_bias[i]!='\0') )
			{
				i++;
			}

			bias_vol_set = simple_strtoul(vol, NULL, 10);
			for(i=0;i<5;i++)
			{
				if(bias_vol_config[i].vol == bias_vol_set)
				{
					break;
				}
			}
			if(i==5)
			{
				printf("invalid gpio bias set vol %d, at name %s\n", bias_vol_set, gpio_name);

				break;
			}
			vol_index = bias_vol_config[i].index;

			if((port_index >= 'a') && (port_index <= 'h'))
			{
				//获取寄存器地址
				port_bias_addr = SUNXI_PIO_BASE + 0x300 + 0x4 * (port_index - 'a');
			}
			else if(port_index == 'j')
			{
				//获取寄存器地址
				port_bias_addr = SUNXI_PIO_BASE + 0x300 + 0x4 * (port_index - 'a');
			}
			else if((port_index == 'l') || (port_index == 'm'))
			{
				//获取寄存器地址
				port_bias_addr = SUNXI_R_PIO_BASE + 0x300 + 0x4 * (port_index - 'l');
			}
			else
			{
				printf("invalid gpio port at name %s\n", gpio_name);

				continue;
			}
			printf("axp=%s, supply=%s, vol=%d\n", axp, supply, bias_vol_set);
			if(config_type == 1)
			{
				writel(vol_index, port_bias_addr);
			}
			else
			{
				pmu_vol = axp_probe_supply_status_byname(axp, supply);
				if(pmu_vol < 0)
				{
					printf("sunxi board read %s %s failed\n", axp, supply);

					continue;
				}

				if(pmu_vol > bias_vol_set)	//pmu实际电压超过需要设置的电压
				{
					//电压降低到需要电压
					axp_set_supply_status_byname(axp, supply, bias_vol_set, 1);
					//设置寄存器
					writel(vol_index, port_bias_addr);
				}
				else if(pmu_vol < bias_vol_set)	//pmu实际电压低于需要设置的电压
				{
					//设置寄存器
					writel(vol_index, port_bias_addr);
					//把pmu电压调整到需要的电压
					axp_set_supply_status_byname(axp, supply, bias_vol_set, 1);
				}
				else
				{
					//如果实际电压等于需要设置电压，直接设置即可
					writel(vol_index, port_bias_addr);
				}
			}
			printf("reg addr=0x%x, value=0x%x, pmu_vol=%d\n", port_bias_addr, readl(port_bias_addr), bias_vol_set);
		}
		else
		{
			printf("config gpio bias voltage finish\n");

			break;
		}
	}

	return 0;
}

static void __disable_unused_mode(void)
{
	writel(1, 0x01C08014);
	writel(0, 0x01C08010);
	writel(1, 0x01C08014);
}
#ifdef CONFIG_BOOT_A15
int check_a15_flag(void);
void get_boot_cpu_flag(void);
#endif
extern int get_cluster_id(void);
int power_source_init(void)
{
	int pll1;
	int dcdc3_vol;
	int dcdc3_vol_d;

	if(script_parser_fetch("power_sply", "dcdc3_vol", &dcdc3_vol, 1))
	{
		dcdc3_vol_d = 1200;
	}
	else
	{
		dcdc3_vol_d = dcdc3_vol%10000;
	}
	if(axp_probe() > 0)
	{
#ifdef CONFIG_BOOT_A15
        get_boot_cpu_flag();
        check_a15_flag();
#endif
		if(!axp_probe_power_supply_condition())
		{
			printf("try to set dcdc3 to %d mV\n", dcdc3_vol_d);
			if(!axp_set_supply_status(0, PMU_SUPPLY_DCDC3, dcdc3_vol_d, -1))
			{
				tick_printf("PMU: dcdc3 %d\n", dcdc3_vol_d);
                if(get_cluster_id() == 0)
				    sunxi_clock_set_corepll(uboot_spare_head.boot_data.run_clock, 0);
			}
			else
			{
				printf("axp_set_dcdc3 fail\n");
			}
		}
		else
		{
			printf("axp_probe_power_supply_condition error\n");
		}
	}
	else
	{
		printf("axp_probe error\n");
	}

	pll1 = sunxi_clock_get_corepll();
    if(get_cluster_id() == 0)
	    tick_printf("PMU: pll1 %d Mhz\n", pll1);
    else
        tick_printf("PMU : pll2 %d Mhz \n",pll1);
    axp_set_charge_vol_limit();
    axp_set_all_limit();
    axp_set_hardware_poweron_vol();

	__disable_unused_mode();

	axp_set_power_supply_output();
	axp_slave_set_power_supply_output();

	power_config_gpio_bias();
    power_limit_init();
    return 0;
}

#ifdef CONFIG_BOOT_A15
extern void save_runtime_context(u32* addr);
extern int get_core_id(void);
extern int get_cpu_id(void);
extern int get_cluster_id(void);
extern int sun9i_cpu_power_set(unsigned int cluster, unsigned int cpu, u32 enable);
extern void jump_to_resume(void);
extern int sun9i_cluster_power_set(unsigned int cluster, u32 enable);
extern int sunxi_clock_set_C1corepll(int frequency, int core_vol);
extern uint sunxi_scan_VF_table(uint table_num,uint boot_clock);
extern unsigned int sunxi_get_soc_bin(void);
extern void force_change_download_flag(void);

#define R_PRCM_BASE	  			0x08001400
#define BOOT_A15_FLAG           0xA5
#define BOOT_A7_FLAG            0xA7
#define R_CPU_SOFT_ENTRY_REG	(R_PRCM_BASE + 0x164)
/*get rtc reg(0b11) value*/
void get_boot_cpu_flag(void)
{
    unsigned int boot_cpu_flag = 0;
    volatile unsigned int *rtc_addr = (volatile unsigned int *)(0x08001400 + 0x1f0);
    uint boot_a15[8] = {0x100,0x101,0x102,0x103,0x00,0x01,0x02,0x03};
    uint boot_a7[8] = {0x00,0x01,0x02,0x03,0x100,0x101,0x102,0x103};
    int cpu0 = 0; 
	if(uboot_spare_head.boot_data.work_mode == WORK_MODE_BOOT)
    {
        if(script_parser_fetch("cpu_logical_map", "cpu0", &cpu0, 1))
	    {
		    return ;
	    }
        *rtc_addr = (3<<16); //get boot cpu flag from rtc_reg
        boot_cpu_flag = *rtc_addr & 0xff;
        printf("boot cpu flag is %x\n",boot_cpu_flag);
        if(boot_cpu_flag == BOOT_A15_FLAG)
        {
            printf("change to boot_cpu : A15 \n");
            if(cpu0 == 0x00)//change cpu_logic_map from sys_config
            {
                printf("remodify cpu_logical_map in sys_config\n");
                script_parser_patch("cpu_logical_map","cpu0",(void *)&boot_a15[0],1);
                script_parser_patch("cpu_logical_map","cpu1",(void *)&boot_a15[1],1);
                script_parser_patch("cpu_logical_map","cpu2",(void *)&boot_a15[2],1);
                script_parser_patch("cpu_logical_map","cpu3",(void *)&boot_a15[3],1);
                script_parser_patch("cpu_logical_map","cpu4",(void *)&boot_a15[4],1);
                script_parser_patch("cpu_logical_map","cpu5",(void *)&boot_a15[5],1);
                script_parser_patch("cpu_logical_map","cpu6",(void *)&boot_a15[6],1);
                script_parser_patch("cpu_logical_map","cpu7",(void *)&boot_a15[7],1);
                gd->force_download_uboot = 1;
            }
            return ;
        }
        else if(boot_cpu_flag == BOOT_A7_FLAG)
        {
            printf("change to boot_cpu : A7 \n");
            if(cpu0 == 0x100)
            {
                printf("remodify cpu_logical_map in sys_config \n");
                script_parser_patch("cpu_logical_map","cpu0",(void *)&boot_a7[0],1);
                script_parser_patch("cpu_logical_map","cpu1",(void *)&boot_a7[1],1);
                script_parser_patch("cpu_logical_map","cpu2",(void *)&boot_a7[2],1);
                script_parser_patch("cpu_logical_map","cpu3",(void *)&boot_a7[3],1);
                script_parser_patch("cpu_logical_map","cpu4",(void *)&boot_a7[4],1);
                script_parser_patch("cpu_logical_map","cpu5",(void *)&boot_a7[5],1);
                script_parser_patch("cpu_logical_map","cpu6",(void *)&boot_a7[6],1);
                script_parser_patch("cpu_logical_map","cpu7",(void *)&boot_a7[7],1);
                gd->force_download_uboot = 1;
            }
            return ;
        }
        else
        {
            printf("boot_cpu_flag is invalid \n");
            gd->force_download_uboot = 0;
            return ;
        }
    }
    else
    {
        gd->force_download_uboot = 0;
        return ;
    }
}
/*clear rtc reg */
void clear_boot_cpu_flag(void)
{
    volatile unsigned int *rtc_addr = (volatile unsigned int *)(0x08001400 + 0x1f0);
    uint temp = 0;
	if(uboot_spare_head.boot_data.work_mode == WORK_MODE_BOOT)
    {
        do
        {
            temp = (3<<16)|(0<<8);
            *rtc_addr = temp;
            temp |= (1<<31);
            *rtc_addr = temp;
            temp &= ~(1<<31);
            *rtc_addr = temp;
        }
        while((*rtc_addr & 0xff)!= 0);
    }
}


int check_a15_flag(void)
{
	if(uboot_spare_head.boot_data.work_mode == WORK_MODE_BOOT)
	{
		int ret  = 0;
		int cpu0 = 0;
		int pre_cpuid = get_cpu_id();
		int pre_cluster = get_cluster_id();
		int pre_coreid = get_core_id();
	    if(script_parser_fetch("cpu_logical_map", "cpu0", &cpu0, 1))
		{
			tick_printf("a7 boot \n");
			return 0;
	    }
		else
		{
            int boot_kernel_cpuid = 0;
            int boot_kernel_cluster = 0;
            int boot_kernel_coreid = 0;
            boot_kernel_cpuid = cpu0 & ~(0xfffffff0);
			boot_kernel_cluster = (cpu0 & ~(0xfffff0ff))>>8;
            boot_kernel_coreid = (boot_kernel_cluster<<2) + boot_kernel_cpuid;
            //timer_init();
			tick_printf(" boot kernel : cpu %d \n",boot_kernel_coreid);
			save_runtime_context((u32 *)0x08100000);
			ret = get_core_id();
			if( ret != boot_kernel_coreid )
			{
				u32 addr = 0;
				/* map brom address to 0x0 */
				__usdelay(1000);
				/*resumed cpu can go to this addr if you set func addr to this reg*/
				addr = (u32)jump_to_resume;
				writel(addr, R_CPU_SOFT_ENTRY_REG);
				/* map brom address to 0x0 */
				writel(0x16AA0000, 0x008000E0);
				/*if boot kernel core is the same cluster,do not set cluster*/
				if(pre_cluster != boot_kernel_cluster)
					sun9i_cluster_power_set(boot_kernel_cluster,1);
				sun9i_cpu_power_set(boot_kernel_cluster, boot_kernel_cpuid, 1);
				asm("wfi": : : "memory","cc");
			}
			else if(pre_coreid != boot_kernel_coreid)
			{
				if(pre_cluster != boot_kernel_cluster)
					sun9i_cluster_power_set(pre_cluster,0);
               sun9i_cpu_power_set(pre_cluster, pre_cpuid, 0);
			}
#if 1
            if(pre_cluster != boot_kernel_cluster)
            {
                if(boot_kernel_cluster)
				{
					uint core_vol = 0;
					uint table_num = 0;
					/*distinguish bin*/
					table_num = sunxi_get_soc_bin();
					/*scan cpu VF table*/
					core_vol = sunxi_scan_VF_table(table_num,uboot_spare_head.boot_data.run_clock);
					/*set core frequency and vol*/
                    sunxi_clock_set_C1corepll(uboot_spare_head.boot_data.run_clock,core_vol);
				}
            }
#endif
           // timer_exit();
		   //while(*(volatile uint *)0 != 0x12);
		}
	}
	return 0;
}
#endif


