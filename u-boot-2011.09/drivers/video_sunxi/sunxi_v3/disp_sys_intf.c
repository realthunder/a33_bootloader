#include "de/bsp_display.h"
#include "disp_sys_intf.h"

/* cache flush flags */
#define  CACHE_FLUSH_I_CACHE_REGION       0
#define  CACHE_FLUSH_D_CACHE_REGION       1
#define  CACHE_FLUSH_CACHE_REGION         2
#define  CACHE_CLEAN_D_CACHE_REGION       3
#define  CACHE_CLEAN_FLUSH_D_CACHE_REGION 4
#define  CACHE_CLEAN_FLUSH_CACHE_REGION   5

/*
*******************************************************************************
*                     OSAL_CacheRangeFlush
*
* Description:
*    Cache flush
*
* Parameters:
*    address    :  start address to be flush
*    length     :  size
*    flags      :  flush flags
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void disp_sys_cache_flush(void*address, u32 length, u32 flags)
{
	if(address == NULL || length == 0) {
		return;
	}

	switch(flags) {
	case CACHE_FLUSH_I_CACHE_REGION:
	break;

	case CACHE_FLUSH_D_CACHE_REGION:
	break;

	case CACHE_FLUSH_CACHE_REGION:
	break;

	case CACHE_CLEAN_D_CACHE_REGION:
	break;

	case CACHE_CLEAN_FLUSH_D_CACHE_REGION:
	break;

	case CACHE_CLEAN_FLUSH_CACHE_REGION:
	break;

	default:
	break;
	}
	return;
}

/*
*******************************************************************************
*                     disp_sys_register_irq
*
* Description:
*    irq register
*
* Parameters:
*    irqno    	    ��input.  irq no
*    flags    	    ��input.
*    Handler  	    ��input.  isr handler
*    pArg 	        ��input.  para
*    DataSize 	    ��input.  len of para
*    prio	        ��input.    priority

*
* Return value:
*
*
* note:
*    typedef s32 (*ISRCallback)( void *pArg)��
*
*******************************************************************************
*/
int disp_sys_register_irq(u32 IrqNo, u32 Flags, void* Handler,void *pArg,u32 DataSize,u32 Prio)
{
	__inf("%s, irqNo=%d, Handler=0x%x, pArg=0x%x\n", __func__, IrqNo, (int)Handler, (int)pArg);
	irq_install_handler(IrqNo, (interrupt_handler_t *)Handler,  pArg);

	return 0;
}

/*
*******************************************************************************
*                     disp_sys_unregister_irq
*
* Description:
*    irq unregister
*
* Parameters:
*    irqno    	��input.  irq no
*    handler  	��input.  isr handler
*    Argment 	��input.    para
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void disp_sys_unregister_irq(u32 IrqNo, void* Handler, void *pArg)
{
	irq_free_handler(IrqNo);
}

/*
*******************************************************************************
*                     disp_sys_enable_irq
*
* Description:
*    enable irq
*
* Parameters:
*    irqno ��input.  irq no
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void disp_sys_enable_irq(u32 IrqNo)
{
	irq_enable(IrqNo);
}

/*
*******************************************************************************
*                     disp_sys_disable_irq
*
* Description:
*    disable irq
*
* Parameters:
*     irqno ��input.  irq no
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void disp_sys_disable_irq(u32 IrqNo)
{
	irq_disable(IrqNo);
}

void disp_sys_irqlock(void* lock, unsigned long *cpu_sr)
{
	return ;
}

void disp_sys_irqunlock(void* lock, unsigned long *cpu_sr)
{
	return;
}

void disp_sys_lock(void* lock)
{
	return;
}

void disp_sys_unlock(void* lock)
{
	return;
}

void * disp_sys_malloc(u32 Size)
{
	void * addr;

	addr = malloc(Size);
	return addr;
}

void disp_sys_free(void *Addr)
{
	free(Addr);
}

/* returns: 0:invalid, 1: int; 2:str, 3: gpio */
int disp_sys_script_get_item(char *main_name, char *sub_name, int value[], int count)
{
	int ret, retcode = 0;
	user_gpio_set_t  gpio_info;
	disp_gpio_set_t  *gpio_list;
	script_parser_value_type_t type;

	ret = script_parser_fetch_ex(main_name, sub_name, value, &type, count);
	if(ret < 0) {
		retcode = 0;//invalid
		//__wrn("fetch script data %s.%s fail\n", main_name, sub_name);
	} else {
		if(type == SCIRPT_PARSER_VALUE_TYPE_SINGLE_WORD) {
			retcode = 1;//int
			//__inf("%s.%s = %d\n", main_name, sub_name, *value);
		} else if(type == SCIRPT_PARSER_VALUE_TYPE_GPIO_WORD) {
			retcode = 3;//gpio
			memcpy(&gpio_info, value, sizeof(user_gpio_set_t));
			gpio_list = (disp_gpio_set_t  *)value;
			gpio_list->port = gpio_info.port;
			gpio_list->port_num = gpio_info.port_num;
			gpio_list->drv_level = gpio_info.drv_level;
			gpio_list->pull = gpio_info.pull;
			gpio_list->data = gpio_info.data;
			//__inf("%s.%s gpio_port=%d,gpio_port_num:%d, data:%d\n",main_name, sub_name, gpio_list->port, gpio_list->port_num, gpio_list->data);
		} else if(type == SCIRPT_PARSER_VALUE_TYPE_STRING) {
			retcode = 2;//str
			//__inf("%s.%s = %s\n", main_name, sub_name, (char*)value);
		}
	}
	
	return retcode;
}

int disp_sys_get_ic_ver(void)
{
    return 0;
}

int disp_sys_gpio_request(disp_gpio_set_t *gpio_list, u32 group_count_max)
{
	user_gpio_set_t gpio_info;
	gpio_info.port = gpio_list->port;
	gpio_info.port_num = gpio_list->port_num;
	gpio_info.mul_sel = gpio_list->mul_sel;
	gpio_info.drv_level = gpio_list->drv_level;
	gpio_info.data = gpio_list->data;

	__inf("OSAL_GPIO_Request, port:%d, port_num:%d, mul_sel:%d, pull:%d, drv_level:%d, data:%d\n", gpio_list->port, gpio_list->port_num, gpio_list->mul_sel, gpio_list->pull, gpio_list->drv_level, gpio_list->data);
	 //gpio_list->port, gpio_list->port_num, gpio_list->mul_sel, gpio_list->pull, gpio_list->drv_level, gpio_list->data);
	if(gpio_list->port == 0xffff) {
		__u32 on_off;
		on_off = gpio_list->data;
		//axp_set_dc1sw(on_off);
		axp_set_supply_status(0, PMU_SUPPLY_DC1SW, 0, on_off);

		return 0xffff;
	}

	return gpio_request(&gpio_info, group_count_max);
}

int disp_sys_gpio_release(int p_handler, s32 if_release_to_default_status)
{
	if(p_handler != 0xffff)
	{
		gpio_release(p_handler, if_release_to_default_status);
	}

	return 0;
}

/* direction: 0:input, 1:output */
int disp_sys_gpio_set_direction(u32 p_handler, u32 direction, const char *gpio_name)
{
	return gpio_set_one_pin_io_status(p_handler, direction, gpio_name);
}

int disp_sys_gpio_get_value(u32 p_handler, const char *gpio_name)
{
	return gpio_read_one_pin_value(p_handler, gpio_name);
}

int disp_sys_gpio_set_value(u32 p_handler, u32 value_to_gpio, const char *gpio_name)
{
	return gpio_write_one_pin_value(p_handler, value_to_gpio, gpio_name);
}

int disp_sys_power_enable(char *name)
{
	int ret = 0;

	ret = axp_set_supply_status_byregulator(name, 1);
	printf("%s, power %s, ret=%d\n", __func__, name, ret);
	return 0;

}

int disp_sys_power_disable(char *name)
{
	int ret = 0;

	ret = axp_set_supply_status_byregulator(name, 0);
	printf("%s, power %s, ret=%d\n", __func__, name, ret);
	return 0;
}

int disp_sys_pwm_request(u32 pwm_id)
{
	return pwm_id + 0x100;
}

int disp_sys_pwm_free(int p_handler)
{
	return 0;
}

int disp_sys_pwm_enable(int p_handler)
{
	int ret = 0;
	int pwm_id = p_handler - 0x100;

	ret = sunxi_pwm_enable(pwm_id);
	return ret;
}

int disp_sys_pwm_disable(int p_handler)
{
	int ret = 0;
	int pwm_id = p_handler - 0x100;

	sunxi_pwm_disable(pwm_id);
	return ret;
}

int disp_sys_pwm_config(int p_handler, int duty_ns, int period_ns)
{
	int ret = 0;
	int pwm_id = p_handler - 0x100;

	ret = sunxi_pwm_config(pwm_id, duty_ns, period_ns);
	return ret;
}

int disp_sys_pwm_set_polarity(int p_handler, int polarity)
{
	int ret = 0;
	int pwm_id = p_handler - 0x100;

	ret = sunxi_pwm_set_polarity(pwm_id, polarity);
	return ret;
}

/* clock */
int disp_sys_clk_set_rate(const char *id, unsigned long rate)
{
	int ret = 0;

	if(!strcmp(id, "pll_video0")) {
		/* fix p(8), div(0): rate = 24 * n / (div+1) / p */
		unsigned int factor_n = rate / 3000000;
		unsigned int reg_val = 0x00000003;

		factor_n &= 0xff;
		reg_val |= (factor_n << 8);
		writel(reg_val, 0x01c20010);
	} else if(!strcmp(id, "pll_de")) {
		/* fix div2(1), div1(0): rate = 24 * n / (div2+1) / (div1+1) */
		unsigned int factor_n = rate / 12000000;
		unsigned int reg_val = 0x00040000;

		factor_n &= 0xff;
		reg_val |= (factor_n << 8);

		writel(reg_val, 0x01c20048);
	}

	return ret;
}

unsigned long disp_sys_clk_get_rate(const char *id)
{
	unsigned long rate = 0;

	if(!strcmp(id, "pll_video0")) {
		/* fix p(8), div(0): rate = 24 * n / (div+1) / p */
		unsigned int factor_n;
		unsigned int reg_val = readl(0x01c20010);

		factor_n = (reg_val >> 8) & 0xff;
		rate = 3000000 * factor_n;
	} else if(!strcmp(id, "pll_de")) {
		/* fix div2(1), div1(0): rate = 24 * n / (div2+1) / (div1+1) */
		unsigned int factor_n;
		unsigned int reg_val = readl(0x01c20048);

		factor_n = (reg_val >> 8) & 0xff;
		rate = 12000000 * factor_n;
	}
	return rate;
}

int disp_sys_clk_set_parent(const char *id, const char *parent)
{
	int ret = 0;

	return ret;
}

int disp_sys_clk_enable(const char *id)
{
	int ret = 0;
	unsigned int reg_val;

	if(!strcmp(id, "pll_video0")) {
		reg_val = readl(0x01c20010);

		reg_val |= 0x80000000;
		writel(reg_val, 0x01c20010);
		/* wait for pll stable */
		__usdelay(100);
	} else if(!strcmp(id, "pll_de")) {
		reg_val = readl(0x01c20048);

		reg_val |= 0x80000000;
		writel(reg_val, 0x01c20048);
		/* wait for pll stable */
		__usdelay(100);
	} else if(!strcmp(id, "lcd0")) {
		/* pll_video0 */
		reg_val = readl(0x01c20010);

		reg_val |= 0x80000000;
		writel(reg_val, 0x01c20010);
		/* wait for pll stable */
		__usdelay(100);

		/* reset */
		reg_val = readl(0x01c202c4);
		reg_val |= 0x10;
		writel(reg_val, 0x01c202c4);
		/* bus gating */
		reg_val = readl(0x01c20064);
		reg_val |= 0x10;
		writel(reg_val, 0x01c20064);
		/* module gating */
		reg_val = readl(0x01c20118);
		reg_val |= 0x80000000;
		writel(reg_val, 0x01c20118);
	} else if(!strcmp(id, "lvds")) {
		writel(1, 0x01c202c8);
	} else if(!strcmp(id, "de")) {
		/* pll_de */
		reg_val = readl(0x01c20048);

		reg_val |= 0x80000000;
		writel(reg_val, 0x01c20048);
		/* wait for pll stable */
		__usdelay(100);

		/* reset */
		reg_val = readl(0x01c202c4);
		reg_val |= 0x1000;
		writel(reg_val, 0x01c202c4);
		/* bus gating */
		reg_val = readl(0x01c20064);
		reg_val |= 0x1000;
		writel(reg_val, 0x01c20064);
	}

	return ret;
}

int disp_sys_clk_disable(const char *id)
{

	int ret = 0;
	unsigned int reg_val;

	if(!strcmp(id, "pll_video0")) {
		reg_val = readl(0x01c20010);

		reg_val &= ~0x80000000;
		writel(reg_val, 0x01c20010);
	} else if(!strcmp(id, "pll_de")) {
		reg_val = readl(0x01c20048);

		reg_val &= ~0x80000000;
		writel(reg_val, 0x01c20048);
	} else if(!strcmp(id, "lcd0")) {
		/* module gating */
		reg_val = readl(0x01c20118);
		reg_val &= ~0x80000000;
		writel(reg_val, 0x01c20118);
		/* bus gating */
		reg_val = readl(0x01c20064);
		reg_val &= ~0x10;
		writel(reg_val, 0x01c20064);
		/* reset */
		reg_val = readl(0x01c202c4);
		reg_val &= ~0x10;
		writel(reg_val, 0x01c202c4);
		
	} else if(!strcmp(id, "lvds")) {
		writel(0, 0x01c202c8);
	} else if(!strcmp(id, "de")) {
		/* bus gating */
		reg_val = readl(0x01c20064);
		reg_val &= ~0x1000;
		writel(reg_val, 0x01c20064);
		/* reset */
		reg_val = readl(0x01c202c4);
		reg_val &= ~0x1000;
		writel(reg_val, 0x01c202c4);
	}

	return ret;
}

