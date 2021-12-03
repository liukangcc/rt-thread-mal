#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#ifdef RT_USING_MAL
#include <mal.h>
#endif

#define THREAD_PRIORITY    25
#define THREAD_MEMORY_SIZE 1024
uint8_t thread_stack[THREAD_MEMORY_SIZE] __attribute__((aligned(THREAD_MEMORY_SIZE)));
uint8_t protect_memory[32] __attribute__((aligned(32)));

struct rt_thread tid = {0};

static void thread1_entry(void *param)
{
    while (1)
    {
        protect_memory[0] = 1;
        rt_thread_mdelay(1000);
    }
}

static rt_err_t mpu1_thread_handle(void *addr, rt_uint32_t attribute)
{
    rt_kprintf("error memory addr: %p\n", addr);

    rt_thread_detach(rt_thread_self());
    rt_schedule();

    return RT_EOK;
}

int main(void)
{
    rt_thread_init(&tid, "mpu", thread1_entry, RT_NULL, thread_stack, THREAD_MEMORY_SIZE, THREAD_PRIORITY, 20);
    {
        rt_mpu_attach(&tid, protect_memory, 32, RT_MPU_REGION_PRIVILEGED_RO);
        rt_mpu_exception_sethook(&tid, mpu1_thread_handle);
        rt_thread_startup(&tid);
    }

    while (1)
    {
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
