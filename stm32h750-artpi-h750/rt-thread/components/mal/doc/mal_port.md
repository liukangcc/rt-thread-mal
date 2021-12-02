# MAL 移植教程

## 架构移植

本部分为架构移植教程，根据此教程可以完成在不同架构上移植 MAL 组件，需根据架构实现 MAL 层提供的 ops 并调用 rt_mpu_ops_register() 进行注册。

如下是由 MAL 层提供的 rt_mpu_ops ，移植时需要实例化 rt_mpu_ops 并实现 ops 操作。

```c
struct rt_mpu_ops
{
   rt_err_t (*init) (struct rt_mal_region *regions);
   void (*switch_table) (rt_thread_t thread,
                        rt_uint8_t mpu_protect_area_num,
                        struct mpu_protect_regions* mpu_protect_areas);
   rt_err_t (*get_info) (rt_thread_t thread, rt_uint32_t type, void *arg);
};
```

本部分以 arm 架构为例进行架构移植。

### MPU 配置表切换

在 rt-thread 线程切换函数中，添加 mpu 配置表更新函数：

```c
switch_to_thread
	IMPORT  rt_mpu_table_switch
	IMPORT  rt_current_thread

    LDR     r1, =rt_interrupt_to_thread
    LDR     r1, [r1]
	LDR     r1, [r1]                ; load thread stack pointer

    IF      {FPU} != "SoftVFP"
    LDMFD   r1!, {r3}               ; pop flag
    ENDIF

    LDMFD   r1!, {r4 - r11}         ; pop r4 - r11 register

    IF      {FPU} != "SoftVFP"
    CMP     r3,  #0                 ; if(flag_r3 != 0)
    VLDMFDNE  r1!, {d8 - d15}       ; pop FPU register s16~s31
    ENDIF

    MSR     psp, r1                 ; update stack pointer

    IF      {FPU} != "SoftVFP"
    ORR     lr, lr, #0x10           ; lr |=  (1 << 4), clean FPCA.
    CMP     r3,  #0                 ; if(flag_r3 != 0)
    BICNE   lr, lr, #0x10           ; lr &= ~(1 << 4), set FPCA.
    ENDIF

    PUSH    {r0-r3, r12, lr}
    LDR     r1, =rt_current_thread
    LDR     r0, [r1]
    BL      rt_mpu_table_switch     ; switch mpu table
    POP     {r0-r3, r12, lr}
```

### ops 实现

arm 架构上 ops 实现示例如下（参考文件 [arm_mal.c](../port/arm/arm_mal.c)） ：

```c
static rt_err_t _mpu_init(struct rt_mal_region *tables)
{
    ...
}

static void _mpu_switch_table(rt_thread_t thread,
                                    rt_uint8_t mpu_protect_area_num,
                                    struct mpu_protect_regions* mpu_protect_areas)
{
    ...
}

static rt_err_t _mpu_get_info(rt_thread_t thread, rt_uint32_t type, void *arg)
{
    ...
}

static struct rt_mpu_ops _mpu_ops =
{
    .init         = _mpu_init,
    .switch_table = _mpu_switch_table,
    .get_info     = _mpu_get_info
};
```

### ops 注册

arm 架构上 ops 注册示例如下（参考文件 [arm_mal.c](../port/arm/arm_mal.c) ）：

```c
static int rt_mpu_arm_register(void)
{
    rt_err_t result = RT_EOK;

    /* 调用 rt_mpu_ops_register 注册 ops */
    result = rt_mpu_ops_register(&_mpu_ops);
    if (result != RT_EOK)
    {
        LOG_E("arm mal ops register failed");
    }

    return result;
}
INIT_BOARD_EXPORT(rt_mpu_arm_register);  /* 自动初始化 */
```

## BSP 移植

本部分教程引导用户在自己的开发板上移植 MAL 组件；MAL 组件的移植非常简单，只需要实现以下两个接口即可：

- MPU 初始化函数（需调用 rt_mpu_init() ）
- MPU 异常处理函数（需在中断处理函数中调用 rt_mpu_exception_handler() ）

本部分以 ART-PI BSP 为例进行移植。

### MPU 初始化函数

根据具体硬件情况，完成板级 MPU 初始化配置，并调用 rt_mpu_init()。

```c
/* ROM */
#define RT_MPU_FLASH_START_ADDR      0x90000000UL
#define RT_MPU_FLASH_SIZE            (16384UL * 1024)

/* RAM */
#define RT_MPU_SRAM_START_ADDR       0x24000000
#define RT_MPU_SRAM_SIZE             (512UL * 1024)

/* PERIPHERALS */
#define RT_MPU_PERIPHERALS_START_ADDRESS             0x40000000UL
#define RT_MPU_PERIPHERALS_SIZE                      0x1FFFFFFFUL

static void mpu_init(void)
{
    static struct rt_mal_region regions[4] = {0};

    /* Flash region configuration */
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
	/* internal sram region configuration */
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
	/* external sram region configuration */
    regions[RT_MPU_EXTERNAL_SRAM_REGION].region = RT_MPU_EXTERNAL_SRAM_REGION;
    regions[RT_MPU_EXTERNAL_SRAM_REGION].addr = 0,
    regions[RT_MPU_EXTERNAL_SRAM_REGION].size = 0;
    regions[RT_MPU_EXTERNAL_SRAM_REGION].attribute = 0;

    /* propherals region configuration */
    regions[RT_MPU_PRIPHERALS_REGION].region = RT_MPU_PRIPHERALS_REGION;
    regions[RT_MPU_PRIPHERALS_REGION].addr = RT_MPU_PERIPHERALS_START_ADDRESS,
    regions[RT_MPU_PRIPHERALS_REGION].size = RT_MPU_PERIPHERALS_SIZE;
    regions[RT_MPU_PRIPHERALS_REGION].attribute = rt_mpu_region_attribute(RT_MPU_REGION_PRIVILEGED_RW,
                                                                     RT_MPU_REGION_EXECUTE_ENABLE,
                                                                     RT_MPU_REGION_SHAREABLE_ENABLE,
                                                                     RT_MPU_REGION_CACHEABLE_ENABLE,
                                                                     RT_MPU_REGION_BUFFERABLE_ENABLE,
                                                                     RT_MPU_REGION_TEX_DISABLE,
                                                                     0);

    rt_mpu_init(regions);
}
```

### MPU 异常处理函数

对于 ART-Pi BSP，当发生内存访问错误时，会触发 `MemManage_Handler` 中断，用户需要记录当前发生中断的内存地址，线程，以及中断类型。调用 rt_mpu_exception_handler() 可以触发用户设置的 hook 钩子函数。

 ```c
 void MemManage_Handler(void)
 {
     rt_uint32_t fault_address, fault_type;

     fault_address = SCB->MMFAR; /* memory manage faults address */
     rt_kprintf("mem manage fault:\n");
     rt_kprintf("SCB_CFSR_MFSR:0x%02X", SCB->CFSR);

     if (SCB->CFSR & (1<<0))
     {
         /* [0]:IACCVIOL */
         rt_kprintf("IACCVIOL");
     }

     if (SCB->CFSR & (1<<1))
     {
         /* [1]:DACCVIOL */
         rt_kprintf("DACCVIOL");
     }

     if (SCB->CFSR & (1<<3))
     {
         /* [3]:MUNSTKERR */
         rt_kprintf("MUNSTKERR");
     }

     if (SCB->CFSR & (1<<4))
     {
         /* [4]:MSTKERR */
         rt_kprintf("MSTKERR");
     }

     if (SCB->CFSR & (1<<7))
     {
         /* [7]:MMARVALID */
         rt_kprintf("SCB->MMAR:%08X\n", SCB->MMFAR);
     }

     fault_type = SCB->CFSR;            /* memory manage faults status */

     rt_mpu_exception_handler(rt_thread_self(), (void *)fault_address, fault_type);
 }
 ```

