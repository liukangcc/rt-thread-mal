/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-03-17     supperthomas first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#ifdef RT_USING_MAL
#include <mal.h>
#endif

/* defined the LED0 pin: PI8 */
#define LED0_PIN    GET_PIN(I, 8)
#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       1024
#define THREAD_TIMESLICE        5

#ifdef RT_USING_WIFI
    extern void wlan_autoconnect_init(void);
#endif

#define THREAD_MEMORY_SIZE 1024
uint8_t thread_stack[THREAD_MEMORY_SIZE] __attribute__((aligned(THREAD_MEMORY_SIZE)));
uint8_t thread1_stack[THREAD_MEMORY_SIZE] __attribute__((aligned(THREAD_MEMORY_SIZE)));

uint8_t protect_memory[THREAD_MEMORY_SIZE] __attribute__((aligned(THREAD_MEMORY_SIZE)));

uint8_t aml_test_memory[THREAD_MEMORY_SIZE] __attribute__((aligned(THREAD_MEMORY_SIZE)));

struct rt_thread tid= {0};
struct rt_thread tid1 = {0};

#define RT_MPU_PERIPHERALS_START_ADDRESS             0x40000000UL
#define RT_MPU_PERIPHERALS_END_ADDRESS               0x5FFFFFFFUL

/* ROM */
#define RT_MPU_FLASH_START_ADDR      0x90000000UL
#define RT_MPU_FLASH_SIZE            (16384UL * 1024)

/* RAM */
#define RT_MPU_SRAM_START_ADDR       0x24000000
#define RT_MPU_SRAM_SIZE             (512UL * 1024)

//struct rt_mal_region regions[4] = {0};
static void mpu_init(void)
{
//    regions[RT_MPU_FLASH_REGION].region = RT_MPU_FLASH_REGION;
//    regions[RT_MPU_FLASH_REGION].addr = RT_MPU_FLASH_START_ADDR, 
//    regions[RT_MPU_FLASH_REGION].size = RT_MPU_FLASH_SIZE;
//    regions[RT_MPU_FLASH_REGION].attribute = rt_mpu_region_attribute(RT_MPU_REGION_RO, 
//                                                                     RT_MPU_REGION_EXECUTE_ENABLE, 
//                                                                     RT_MPU_REGION_SHAREABLE_ENABLE,       
//                                                                     RT_MPU_REGION_CACHEABLE_ENABLE,
//                                                                     RT_MPU_REGION_BUFFERABLE_ENABLE,
//                                                                     RT_MPU_REGION_TEX_DISABLE,
//                                                                     0);

//    regions[RT_MPU_INTERNAL_SRAM_REGION].region = RT_MPU_INTERNAL_SRAM_REGION;
//    regions[RT_MPU_INTERNAL_SRAM_REGION].addr = RT_MPU_SRAM_START_ADDR, 
//    regions[RT_MPU_INTERNAL_SRAM_REGION].size = RT_MPU_SRAM_SIZE;
//    regions[RT_MPU_INTERNAL_SRAM_REGION].attribute = rt_mpu_region_attribute(RT_MPU_REGION_PRIVILEGED_RW, 
//                                                                     RT_MPU_REGION_EXECUTE_ENABLE, 
//                                                                     RT_MPU_REGION_SHAREABLE_ENABLE,       
//                                                                     RT_MPU_REGION_CACHEABLE_ENABLE,
//                                                                     RT_MPU_REGION_BUFFERABLE_ENABLE,
//                                                                     RT_MPU_REGION_TEX_DISABLE,
//                                                                     0);

//    regions[RT_MPU_EXTERNAL_SRAM_REGION].region = RT_MPU_EXTERNAL_SRAM_REGION;
//    regions[RT_MPU_EXTERNAL_SRAM_REGION].addr = 0, 
//    regions[RT_MPU_EXTERNAL_SRAM_REGION].size = 0;
//    regions[RT_MPU_EXTERNAL_SRAM_REGION].attribute = 0;  
//    
//    regions[RT_MPU_PRIPHERALS_REGION].region = RT_MPU_PRIPHERALS_REGION;
//    regions[RT_MPU_PRIPHERALS_REGION].addr = RT_MPU_PERIPHERALS_START_ADDRESS, 
//    regions[RT_MPU_PRIPHERALS_REGION].size = RT_MPU_PERIPHERALS_END_ADDRESS - RT_MPU_PERIPHERALS_START_ADDRESS;
//    regions[RT_MPU_PRIPHERALS_REGION].attribute = rt_mpu_region_attribute(RT_MPU_REGION_PRIVILEGED_RW, 
//                                                                     RT_MPU_REGION_EXECUTE_ENABLE, 
//                                                                     RT_MPU_REGION_SHAREABLE_ENABLE,       
//                                                                     RT_MPU_REGION_CACHEABLE_ENABLE,
//                                                                     RT_MPU_REGION_BUFFERABLE_ENABLE,
//                                                                     RT_MPU_REGION_TEX_DISABLE,
//                                                                     0);

//    rt_mpu_init(regions);
}

void MemManage_Handler(void)
{
//    rt_uint32_t fault_address, fault_type;;

//    fault_address = SCB->MMFAR; /* memory manage faults address */
//    rt_kprintf("mem manage fault:\n");
//    rt_kprintf("SCB_CFSR_MFSR:0x%02X ", SCB->CFSR);

//    if (SCB->CFSR & (1<<0))
//    {
//        /* [0]:IACCVIOL */
//        rt_kprintf("IACCVIOL ");
//    }

//    if (SCB->CFSR & (1<<1))
//    {
//        /* [1]:DACCVIOL */
//        rt_kprintf("DACCVIOL ");
//    }

//    if (SCB->CFSR & (1<<3))
//    {
//        /* [3]:MUNSTKERR */
//        rt_kprintf("MUNSTKERR ");
//    }

//    if (SCB->CFSR & (1<<4))
//    {
//        /* [4]:MSTKERR */
//        rt_kprintf("MSTKERR ");
//    }

//    if (SCB->CFSR & (1<<7))
//    {
//        /* [7]:MMARVALID */
//        rt_kprintf("SCB->MMAR:%08X\n", SCB->MMFAR);
//    }

//    fault_type = 0;            /* memory manage faults status */

//    rt_mpu_exception_handler(rt_thread_self(), (void *)fault_address, fault_type);

    while (1);
}

#define THREAD_MEMORY_SIZE 1024
uint8_t protect_memory[THREAD_MEMORY_SIZE] __attribute__((aligned(THREAD_MEMORY_SIZE)));

static void thread1_entry(void *param)
{

   while (1)
   {
        protect_memory[0] = 1;
        rt_thread_mdelay(1000);
   }
}

//static void mpu_thread_handle(void *addr, rt_uint32_t attribute)
//{
//	rt_kprintf("error memory addr: %p\n", addr);
//}

int main(void)
{
    /* set LED0 pin mode to output */
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
//    mpu_init();
//    rt_thread_init(&tid, "mpu", thread1_entry, RT_NULL, thread_stack, THREAD_MEMORY_SIZE, THREAD_PRIORITY, 20);
//    {
//        rt_mpu_attach(&tid, protect_memory, THREAD_MEMORY_SIZE, RT_MPU_REGION_NO_ACCESS);
//        rt_mpu_exception_sethook(&tid, mpu_thread_handle);
//        rt_thread_startup(&tid);
//    }

    while (1)
    {
        rt_pin_write(LED0_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED0_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }
}

#include "stm32h7xx.h"
static int vtor_config(void)
{
    /* Vector Table Relocation in Internal QSPI_FLASH */
    SCB->VTOR = QSPI_BASE;
    return 0;
}
INIT_BOARD_EXPORT(vtor_config);
