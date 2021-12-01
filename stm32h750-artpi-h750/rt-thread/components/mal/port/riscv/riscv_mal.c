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

#define DBG_TAG    "mal.riscv"
#ifdef RT_MAL_USING_LOG
#define DBG_LVL    DBG_LOG
#else 
#define DBG_LVL    DBG_INFO
#endif
#include <rtdbg.h>

static rt_err_t rt_mpu_riscv_get_info(rt_thread_t thread, rt_uint32_t type, void *arg)
{
    return RT_EOK;
}

static void rt_mpu_riscv_switch_table(rt_thread_t thread, rt_uint8_t mpu_protect_area_num, 
                                      struct mpu_protect_regions* mpu_protect_areas)
{
    return;
}

static rt_err_t rt_mpu_riscv_init(struct rt_mal_region *tables)
{
    return RT_EOK;
}

static struct rt_mpu_ops riscv_mpu_ops =
{
    .init         = rt_mpu_riscv_init,
    .switch_table = rt_mpu_riscv_switch_table,
    .get_info     = rt_mpu_riscv_get_info
};

static int rt_mpu_riscv_register(void)
{
    rt_err_t result = RT_EOK;

    result = rt_mpu_ops_register(&riscv_mpu_ops);
    if (result != RT_EOK)
    {
        LOG_E("riscv mal ops register failed");
    }

    return result;
}
INIT_BOARD_EXPORT(rt_mpu_riscv_register);
