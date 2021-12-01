# MPU 抽象层

## 1、介绍

MAL （MPU Abstract Layer），即 mpu 抽象层。是 RT-Thread 自主研发的，支持安全的内存访问。

用户代码可以任意的访问非法的内存区域（其他任务的堆栈、野指针等），造成堆栈溢出、内存被修改、硬件错误等问题。所以需要由 MPU 来拘束任务、外设、内存的读写权限，提高系统的稳定性、安全性、成熟性、可靠性。MAL 组件就在这种需求下诞生了。

### 1.1 功能

- 线程堆栈保护，防止堆栈溢出
- 内存隔离，某一任务用到的内存区域不想被其他任务破坏
- MoM 支持为每个线程设定不同区域的内存访问权限，mpu 表快速切换 

### 1.2 内存权限

内存支持以下 6 种访问权限：

- no access
- privileged access only
- unprivileged access read-only
- full access
- privileged access read-only
- read-only access

### 1.3 框架图

![image-20211123161228115](doc/figures/frame.png)

### 1.4 数据流图

![image-20211123161228115](doc/figures/stream.png)

### 1.5 目录结构

MAL 组件目录结构如下所示：

```shell
MAL
├───docs 
│   └───figures                     // 文档使用图片
├───inc                             // 头文件目录
├───src                             // 源文件目录
├───port                            // 不同架构下的 MPU 对接 MAL 的移植文件
│   LICENSE                         // 组件许可证
│   README.md                       // 软件包使用说明
│   Kconfig                         // 提供配置选项
└───SConscript                      // 工程构建脚本
```

## MAL API

MAL API 如下所示，[点击此处查看 API 参数详解](doc/mal_api.md)。

```c
/* 提供给应用层用户调用的 API */
rt_err_t rt_mpu_attach(rt_thread_t thread, void* addr, size_t size, rt_uint32_t attribute);
rt_err_t rt_mpu_attach_table(rt_thread_t thread, struct mpu_regions *regions);

rt_err_t rt_mpu_delete(rt_thread_t thread, rt_uint8_t region);
rt_err_t rt_mpu_refresh(rt_thread_t thread, void *addr, size_t size, rt_uint32_t attribute, rt_uint8_t region);

rt_err_t rt_mpu_enable_protect_area(rt_thread_t thread, void *addr, size_t size, rt_uint32_t attribute);
rt_err_t rt_mpu_disable_protect_area(rt_thread_t thread, rt_uint8_t region);

void rt_mpu_exception_sethook(rt_thread_t thread, void (*hook)(void* addr, rt_uint32_t attribute));

rt_err_t rt_mpu_insert(rt_thread_t thread, void *addr, size_t size, rt_uint32_t attribute, rt_uint8_t region);
rt_err_t rt_mpu_get_info(rt_thread_t thread, rt_uint32_t type, void *arg);

void rt_mpu_table_switch(rt_thread_t thread);

/* 用于架构移植调用的 API */
rt_err_t rt_mpu_ops_register(struct rt_mpu_ops *ops);

/* 用于 BSP 移植调用的 API */
rt_err_t rt_mpu_init(struct rt_mal_region *tables);
void rt_mpu_exception_handler(rt_thread_t thread, void* addr, rt_uint32_t attribute);
```

## 移植 MAL

MAL 移植分为两个层面的移植：架构层移植，和 BSP 层移植。

架构层移植：架构移植文件位于 MAL 组件的 port 文件夹下，以 xxx_mal.c 命名，如 arm 架构移植文件命名为 arm_mal.c，适用于所有 arm 架构。

- [不同 架构 移植 MAL 组件教程](doc/移植教程.md) 

BSP 层移植：BSP 移植文件位于具体的 bsp 中，主要工作是初始化 MPU，完成 MPU 异常处理。

- [不同 BSP 移植 MAL 组件教程](doc/移植教程.md) 

## 使用 MAL

### 开启 MAL 组件

在工程目录下，打开 `env` 工具，使能 MPU 抽象层：

![enable_mal](../../../../../mpu/stm32h750-artpi-h750/rt-thread/components/mal/doc/figures/enable_mal.png)

### 示例：设置线程保护区域

设置一块内存区域，只有当前线程具有访问权限，其他线程禁止访问。

   ```c
   #define THREAD_MEMORY_SIZE 1024
   uint8_t protect_memory[THREAD_MEMORY_SIZE] __attribute__((aligned(THREAD_MEMORY_SIZE)));
   
   static void thread1_entry(void *param)
   {
      while (1)
      {
          protect_memory[0] = 1; /* mpu1 thread will trigger memmory fault */
          rt_thread_mdelay(1000);
      }
   }
   
   static void mpu1_thread_handle(void *addr, rt_uint32_t attribute)
   {
   	rt_kprintf("error memory addr: %p\n", addr);
   }
   
   int main(void)
   {
       /* set LED0 pin mode to output */
       rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
       mpu_init();
       rt_thread_init(&tid, "mpu", thread1_entry, RT_NULL, thread_stack, THREAD_MEMORY_SIZE, THREAD_PRIORITY, 20);
       {
           rt_mpu_enable_protect_area(&tid, protect_memory, THREAD_MEMORY_SIZE, RT_MPU_REGION_PRIVILEGED_RW); /* 设置保护区域 */
           rt_thread_startup(&tid);
       }
   
       rt_thread_init(&tid1, "mpu1", thread1_entry, RT_NULL, thread1_stack, THREAD_MEMORY_SIZE, THREAD_PRIORITY, 20);
       {
           rt_mpu_exception_sethook(&tid1, mpu1_thread_handle);
           rt_thread_startup(&tid1);
       }
   
       while (1)
       {
           rt_pin_write(LED0_PIN, PIN_HIGH);
           rt_thread_mdelay(500);
           rt_pin_write(LED0_PIN, PIN_LOW);
           rt_thread_mdelay(500);
       }
   }
   ```

当线程 `mpu1` 访问内存区域 `protect_memory` 时，就会触发内存异常中断服务。如果该线程注册了 mpu 异常回调函数，mal 组件层就会调用该函数。

![image-20211130110741512](../../../../../mpu/stm32h750-artpi-h750/rt-thread/components/mal/doc/figures/handle.png)

### 示例：设置线程受限区域

只针对当前线程，禁止对某块区域进行读写操作：

```c
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

static void mpu_thread_handle(void *addr, rt_uint32_t attribute)
{
	rt_kprintf("error memory addr: %p\n", addr);
}

int main(void)
{
    /* set LED0 pin mode to output */
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
    mpu_init();
    rt_thread_init(&tid, "mpu", thread1_entry, RT_NULL, thread_stack, THREAD_MEMORY_SIZE, THREAD_PRIORITY, 20);
    {
        rt_mpu_attach(&tid, protect_memory, THREAD_MEMORY_SIZE, RT_MPU_REGION_NO_ACCESS);
        rt_mpu_exception_sethook(&tid, mpu_thread_handle);
        rt_thread_startup(&tid);
    }

    while (1)
    {
        rt_pin_write(LED0_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED0_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }
}
```

当线程 `mpu` 访问内存区域 `protect_memory` 时，就会触发 mpu 异常。





