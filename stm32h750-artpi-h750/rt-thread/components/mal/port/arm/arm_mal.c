/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-25     liukang      first version
 */

#include <arm_mal.h>
#include <board.h>

#define DBG_TAG    "mal.arm"
#ifdef RT_MAL_USING_LOG
#define DBG_LVL    DBG_LOG
#else 
#define DBG_LVL    DBG_INFO
#endif
#include <rtdbg.h>

static rt_uint32_t rt_mpu_align_min(rt_uint32_t len)
{
    rt_uint32_t region_size, value = 4;

    /* 32 is the smallest region size, 31 is the largest valid value */
    for (region_size = RT_MPU_ALIGN_SMALLEST_SIZE; value < 31UL; region_size <<= 1UL)
    {
        if( len <= region_size)
        {
            break;
        }
        else
        {
            value++;
        }
    }

    return value;
}

/* enable mpu */
static void rt_mpu_arm_enable(void)
{
    ARM_MPU_Enable(4);
}

/* disable mpu */
static void rt_mpu_arm_disable()
{
    ARM_MPU_Disable(); 
}

static rt_uint32_t rt_mpu_arm_rasr_value(int attribute, int size)
{
    rt_uint32_t execute, access, type_extern, shareable, cacheable, bufferable, sub_region;

    execute     = (attribute >> REGION_EXECUTE_Pos) & 0x1UL;
    access      = (attribute >> REGION_PERMISSION_Pos) & 0x7UL;
    type_extern = (attribute >> REGION_TEX_Pos) & 0x7UL; 
    shareable   = (attribute >> REGION_SHAREABLE_Pos) & 0x1UL;
    cacheable   = (attribute >> REGION_CACHEABLE_Pos) & 0x1UL;
    bufferable  = (attribute >> REGION_BUFFERABLE_Pos) & 0x1UL; 
    sub_region  = (attribute >> REGION_SRD_Pos) & 0xFFUL; 

    return ARM_MPU_RASR(execute, access, type_extern, shareable, cacheable, bufferable, sub_region, size);
};

static void _arm_get_regisger_value(uint32_t rnr, int* rbar, int* rasr)
{
    MPU->RNR = rnr;
    *rbar = MPU->RBAR;
    *rasr = MPU->RASR;
}

static void rt_mpu_arm_get_region_config(void *arg)
{
    int index = 0, rbar, rasr;
    struct rt_mal_region *tables = (struct rt_mal_region *)arg;

    for (index = 0; index < RT_MPU_REGIONS_NUMBER; index++)
    {
        _arm_get_regisger_value(index, &rbar, &rasr);
        tables[index].region    = rbar & 0xFUL;
        tables[index].addr      = rbar & 0xFFFFFF80UL;
        if (((rasr >> MPU_RASR_SIZE_Pos) & 0x1FUL) > 0)
        {
            tables[index].size      = 1 << (((rasr >> MPU_RASR_SIZE_Pos) & 0x1FUL) + 1);
        }
        else
        {
            tables[index].size      = 0;
        }

        tables[index].attribute = rt_mpu_region_attribute((rasr >> MPU_RASR_AP_Pos) & 0x03UL,
                                                          (rasr >> MPU_RASR_XN_Pos) & 0x01UL,
                                                          (rasr >> MPU_RASR_S_Pos)  & 0x01UL, 
                                                          (rasr >> MPU_RASR_C_Pos)  & 0x01UL,
                                                          (rasr >> MPU_RASR_B_Pos)  & 0x01UL,  
                                                          (rasr >> MPU_RASR_TEX_Pos)  & 0x03UL,
                                                          (rasr >> MPU_RASR_SRD_Pos)  & 0xFFUL);
    }
}

static rt_err_t rt_mpu_arm_get_info(rt_thread_t thread, rt_uint32_t type, void *arg)
{
    rt_err_t result = RT_EOK;
    if (thread == RT_NULL || arg == RT_NULL)
    {
        return -RT_ERROR;
    }
    
    switch (type)
    {
        case GET_MPU_REGIONS_NUMBER:
            *(int *)arg = thread->setting.index; 
        break;
        
        case GET_MPU_REGIONS_CONFGIG:
            rt_mpu_arm_get_region_config(arg);
        default:
            LOG_W("not support type: %d", type);
            result = -RT_ERROR;
        break;
    }

    return result;
}

static void rt_mpu_arm_general_region_table_switch(rt_thread_t thread)
{
    int align_size, align_addr, rabr, rasr, index = 0;

    /* Invalidate other region. */
    for (index = 0; index < RT_MPU_NUM_CONFIGURABLE_REGION; index++)
    {
        ARM_MPU_ClrRegion(RT_MPU_FIRST_CONFIGURABLE_REGION + index);
    }

    /* current thread stack is proteced? */
    if ((thread->setting.tables[0].addr == 0) && (thread->stack_size != 0))
    {
        LOG_D("thread: %s, stack_addr: %p, size: %d", thread->name, thread->stack_addr, RT_MPU_THREAD_PROTECT_SIZE);
        thread->setting.tables[0].region = RT_MPU_THREAD_STACK_REGION;
        thread->setting.tables[0].addr = (rt_uint32_t)thread->stack_addr;
        thread->setting.tables[0].size = RT_MPU_THREAD_PROTECT_SIZE;
        thread->setting.tables[0].attribute = rt_mpu_region_attribute(RT_MPU_REGION_PRIVILEGED_RO,
                                                                    RT_MPU_REGION_EXECUTE_ENABLE,
                                                                    RT_MPU_REGION_SHAREABLE_ENABLE,
                                                                    RT_MPU_REGION_CACHEABLE_ENABLE,
                                                                    RT_MPU_REGION_BUFFERABLE_ENABLE,
                                                                    RT_MPU_REGION_TEX_DISABLE,
                                                                    0UL);
    }

    for (index = 0; index < thread->setting.index + 1; index++)
    {
        align_size = rt_mpu_align_min(thread->setting.tables[index].size);                                                 /* 对齐后的字节数 */
        align_addr = (rt_uint32_t)RT_ALIGN_DOWN((rt_uint32_t)thread->setting.tables[index].addr,  1 << (align_size + 1));  /* 对齐后的地址 */
        rabr = ARM_MPU_RBAR((rt_uint32_t)thread->setting.tables[index].region, align_addr);
        rasr = rt_mpu_arm_rasr_value(thread->setting.tables[index].attribute, align_size); /* MPU Region Attribute and Size Register Value */
        
        ARM_MPU_SetRegion(rabr, rasr);
    }
}

static void rt_mpu_arm_protect_region_table_switch(rt_thread_t thread, rt_uint8_t mpu_protect_area_num, 
                                                   struct mpu_protect_regions* mpu_protect_areas)
{
    int align_size, align_addr, rabr, rasr, index = 0;

    for (index = 0; index < RT_MPU_THREAD_PROTECT_MEM_NUM_REGION; index++)
    {
        ARM_MPU_ClrRegion(RT_MPU_THREAD_PROTECT_MEM_REGION_0 + index);
    }

    for (index = 0; index < mpu_protect_area_num; index++)
    {
        align_size = rt_mpu_align_min(mpu_protect_areas[index].tables.size);
        align_addr = (rt_uint32_t)RT_ALIGN_DOWN((rt_uint32_t)mpu_protect_areas[index].tables.addr, 1 << (align_size + 1));

        /* thread can access this region */
        if (mpu_protect_areas[index].thread == thread)
        {
            rabr = ARM_MPU_RBAR((rt_uint32_t)mpu_protect_areas[index].tables.region, align_addr);
            rasr = rt_mpu_arm_rasr_value(mpu_protect_areas[index].tables.attribute, align_size); /* MPU Region Attribute and Size Register Value */
        }
        /* thread can't access this region */
        else
        {
            int attribute = (mpu_protect_areas[index].tables.attribute & (~REGION_PERMISSION_Msk)) | ((RT_MPU_REGION_NO_ACCESS << REGION_PERMISSION_Pos) & REGION_PERMISSION_Msk);
            rabr = ARM_MPU_RBAR((rt_uint32_t)mpu_protect_areas[index].tables.region, align_addr);
            rasr = rt_mpu_arm_rasr_value(attribute, align_size);
        }

        ARM_MPU_SetRegion(rabr, rasr);
    }
}
static void rt_mpu_arm_switch_table(rt_thread_t thread, rt_uint8_t mpu_protect_area_num, 
                                    struct mpu_protect_regions* mpu_protect_areas)
{
    RT_ASSERT(thread != RT_NULL);

    rt_mpu_arm_disable();

    rt_mpu_arm_general_region_table_switch(thread);
    rt_mpu_arm_protect_region_table_switch(thread, mpu_protect_area_num, mpu_protect_areas);

    rt_mpu_arm_enable();
}

static rt_err_t rt_mpu_arm_init(struct rt_mal_region *tables)
{
    int align_size, align_addr, rbar, rasr, index = 0;
    int regions_number = 0;
    
    rt_mpu_arm_disable();
    regions_number = MPU->TYPE >> 8 & 0xFF;
    if (regions_number == 0)
    {
        LOG_E("this cpu not support mpu");
        return -RT_ERROR;
    }
    
    if (regions_number != RT_MPU_REGIONS_NUMBER)
    {
        LOG_W("this cpu support regions number is: %d", regions_number);
    }

    /* The only permitted number of regions are 8 or 16. */
    RT_ASSERT((RT_MPU_REGIONS_NUMBER == 8 ) || RT_MPU_REGIONS_NUMBER == 16);
    if (tables == RT_NULL)
    {
        LOG_W("please init the system region first.");
        return -RT_ERROR;
    }

    for (index = 0; index < RT_MPU_THREAD_STACK_REGION; index++)
    {
        if (tables[index].size > 0)
        {
            align_size = rt_mpu_align_min(tables[index].size);
            align_addr = (rt_uint32_t)RT_ALIGN_DOWN((rt_uint32_t)tables[index].addr, 1 << (align_size + 1));
            
            rbar = ARM_MPU_RBAR((rt_uint32_t)tables[index].region, align_addr);
            rasr = rt_mpu_arm_rasr_value(tables[index].attribute, align_size);

            ARM_MPU_SetRegion(rbar, rasr);
        }            
    }

    rt_mpu_arm_enable();
    
    LOG_I("mpu init success.");
    
    return RT_EOK;
}

static struct rt_mpu_ops arm_mpu_ops =
{
    .init         = rt_mpu_arm_init,
    .switch_table = rt_mpu_arm_switch_table,
    .get_info     = rt_mpu_arm_get_info
};

static int rt_mpu_arm_register(void)
{
    rt_err_t result = RT_EOK;

    result = rt_mpu_ops_register(&arm_mpu_ops);
    if (result != RT_EOK)
    {
        LOG_E(" arm mal ops register failed");
    }

    return result;
}
INIT_BOARD_EXPORT(rt_mpu_arm_register);
