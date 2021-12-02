#include <rtthread.h>
#include <board.h>

#ifdef RT_USING_MAL
#include <mal.h>

#define RT_MPU_PERIPHERALS_START_ADDRESS                0x40000000UL
#define RT_MPU_PERIPHERALS_END_ADDRESS                  0x5FFFFFFFUL

/* ROM */
#define RT_MPU_FLASH_START_ADDR                         0x90000000UL
#define RT_MPU_FLASH_SIZE                               (16384UL * 1024)

/* RAM */
#define RT_MPU_SRAM_START_ADDR                          0x24000000
#define RT_MPU_SRAM_SIZE                                (512UL * 1024)

struct rt_mal_region regions[4] = {0};
static int rt_hw_mpu_init(void)
{
    regions[RT_MPU_FLASH_REGION].region = RT_MPU_FLASH_REGION;
    regions[RT_MPU_FLASH_REGION].addr = RT_MPU_FLASH_START_ADDR, 
    regions[RT_MPU_FLASH_REGION].size = RT_MPU_FLASH_SIZE;
    regions[RT_MPU_FLASH_REGION].attribute = rt_mpu_region_attribute(RT_MPU_REGION_RO, 
                                                                     RT_MPU_REGION_EXECUTE_ENABLE, 
                                                                     RT_MPU_REGION_SHAREABLE_ENABLE,       
                                                                     RT_MPU_REGION_CACHEABLE_ENABLE,
                                                                     RT_MPU_REGION_BUFFERABLE_ENABLE,
                                                                     RT_MPU_REGION_TEX_DISABLE,
                                                                     0);

    regions[RT_MPU_INTERNAL_SRAM_REGION].region = RT_MPU_INTERNAL_SRAM_REGION;
    regions[RT_MPU_INTERNAL_SRAM_REGION].addr = RT_MPU_SRAM_START_ADDR, 
    regions[RT_MPU_INTERNAL_SRAM_REGION].size = RT_MPU_SRAM_SIZE;
    regions[RT_MPU_INTERNAL_SRAM_REGION].attribute = rt_mpu_region_attribute(RT_MPU_REGION_PRIVILEGED_RW, 
                                                                     RT_MPU_REGION_EXECUTE_ENABLE, 
                                                                     RT_MPU_REGION_SHAREABLE_ENABLE,       
                                                                     RT_MPU_REGION_CACHEABLE_ENABLE,
                                                                     RT_MPU_REGION_BUFFERABLE_ENABLE,
                                                                     RT_MPU_REGION_TEX_DISABLE,
                                                                     0);

    regions[RT_MPU_EXTERNAL_SRAM_REGION].region = RT_MPU_EXTERNAL_SRAM_REGION;
    regions[RT_MPU_EXTERNAL_SRAM_REGION].addr = 0, 
    regions[RT_MPU_EXTERNAL_SRAM_REGION].size = 0;
    regions[RT_MPU_EXTERNAL_SRAM_REGION].attribute = 0;  
    
    regions[RT_MPU_PRIPHERALS_REGION].region = RT_MPU_PRIPHERALS_REGION;
    regions[RT_MPU_PRIPHERALS_REGION].addr = RT_MPU_PERIPHERALS_START_ADDRESS, 
    regions[RT_MPU_PRIPHERALS_REGION].size = RT_MPU_PERIPHERALS_END_ADDRESS - RT_MPU_PERIPHERALS_START_ADDRESS;
    regions[RT_MPU_PRIPHERALS_REGION].attribute = rt_mpu_region_attribute(RT_MPU_REGION_PRIVILEGED_RW, 
                                                                     RT_MPU_REGION_EXECUTE_ENABLE, 
                                                                     RT_MPU_REGION_SHAREABLE_ENABLE,       
                                                                     RT_MPU_REGION_CACHEABLE_ENABLE,
                                                                     RT_MPU_REGION_BUFFERABLE_ENABLE,
                                                                     RT_MPU_REGION_TEX_DISABLE,
                                                                     0);

    rt_mpu_init(regions);
    
    return RT_EOK;
}
INIT_APP_EXPORT(rt_hw_mpu_init);

void MemManage_Handler(void)
{
   rt_uint32_t fault_address, fault_type;;

   fault_address = SCB->MMFAR; /* memory manage faults address */
   rt_kprintf("mem manage fault:\n");
   rt_kprintf("SCB_CFSR_MFSR:0x%02X ", SCB->CFSR);

   if (SCB->CFSR & (1<<0))
   {
       /* [0]:IACCVIOL */
       rt_kprintf("IACCVIOL ");
   }

   if (SCB->CFSR & (1<<1))
   {
       /* [1]:DACCVIOL */
       rt_kprintf("DACCVIOL ");
   }

   if (SCB->CFSR & (1<<3))
   {
       /* [3]:MUNSTKERR */
       rt_kprintf("MUNSTKERR ");
   }

   if (SCB->CFSR & (1<<4))
   {
       /* [4]:MSTKERR */
       rt_kprintf("MSTKERR ");
   }

   if (SCB->CFSR & (1<<7))
   {
       /* [7]:MMARVALID */
       rt_kprintf("SCB->MMAR:%08X\n", SCB->MMFAR);
   }

   fault_type = SCB->CFSR;            /* memory manage faults status */
    
   rt_mpu_exception_handler(rt_thread_self(), (void *)fault_address, fault_type);

    while (1);
}

#endif
