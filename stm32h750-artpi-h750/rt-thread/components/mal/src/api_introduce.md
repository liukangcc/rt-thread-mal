# MAL 组件 API 介绍文档

## MAL 组件初始化

该 api 由 bsp 用户自己进行配置

```
int mal_init(void)
{
	/* 默认 region 配置 */
	/* FLASH - region 0 */
	0x08000000 - 0x08200000
	/* 内部 ram - region 1  */
	0x20000000 - 0x20020000
	/* 外部 ram - region 2  */
	0x24000000 - 0x30000000
	/* 外设 - region 3 */
	0x40000000 - 0x5FFFFFFF
}
INIT_BOARD_EXPORT(rt_mpu_init);
```

## MAL OPS 注册

不同的架构注册不同的接口

```c
static struct rt_mpu_ops* mpu_ops = RT_NULL;
rt_err_t rt_mpu_ops_register(struct rt_mpu_ops *ops)
{
	mpu_ops = ops;
    
    return RT_EOK;
}

在 port/arm/arm_mal.c 文件中注册 ops
static const struct rt_mpu_ops arm_mpu_ops =
{
    .switch_table = _switch_table,
};

static void arm_mpu_ops_register(void)
{
    rt_err_t result;

    result = rt_mpu_ops_register(&arm_mpu_ops);

    RT_ASSERT(result == RT_EOK);
}
INIT_BOARD_EXPORT(arm_mpu_ops_register);
```

## MAL 表添加单条配置

单个 region 配置，在当前线程 regions 表，按顺序添加一条配置：

![](D:\repo\gitlab\mpu\figures\attach.png)

代码实现：

```c
/**
 * @brief   This function will initialize a thread. It's used to initialize a
 *          static thread object.
 *
 * @param   thread is the thread object.
 *
 * @param   name is the name of thread, which shall be unique.
 *
 * @param   addr is the protected area.
 *
 * @param   size is the protected area length.
 *
 * @param   attribute is the read and write permission of the thread to the protected area.
 *
 * @return  Return the operation status. If the return value is RT_EOK, the function is successfully executed.
 *          If the return value is any other values, it means this operation failed.
 */
rt_err_t rt_mpu_attach(rt_thread_t thread, void *addr, size_t size, rt_uint32_t attribute)
{
    if (thread->mpu_table.number == MPU_REGIONS_NUMBER)
    {
        LOG_E("no region can be config");
        return;
    }
}
```

## MAL 表添加多条配置

多个 region 配置，在当前线程 regions 表，按顺序添加多条配置：

![](D:\repo\gitlab\mpu\figures\attachs.png)

代码实现如下：

```
struct mpu_regions
{
	void * address;
	rt_uint32_t size;
	rt_uin32_t parameters;
};
/**
 * @brief   This function will initialize a thread. It's used to initialize a
 *          static thread object.
 *
 * @param   thread is the thread object.
 *
 * @param   name is the name of thread, which shall be unique.
 *
 * @param   regions is the protected regions config.
 *
 * @return  Return the operation status. If the return value is RT_EOK, the function is successfully executed.
 *          If the return value is any other values, it means this operation failed.
 */
rt_err_t rt_mpu_attach_table(rt_thread_t thread, struch mpu_regions *regions)
{
	return RT_EOK;
}
```

## MAL 表删除

用户删除某条配置。根据 region 编号删除对应的 region 配置：

![](D:\repo\gitlab\mpu\figures\delete.png)

代码实现如下：

```c
rt_err_t rt_mpu_delete(rt_thread_t thread, rt_uint8_t region)
{
	/* 遍历 MPU tables ，找到地址为 address 的配置项并删除配置项， 该配置后边的其他项向前移动 */
    return RT_EOK;
}
```

## MAL 表更新

用户更新某条配置；例如：由可读可写配置为只读。根据 region 编号修改对应的 region 配置：

![](D:\repo\gitlab\mpu\figures\refresh.png)

代码实现如下：

```c
rt_err_t rt_mpu_refresh(rt_thread_t thread, void *addr, size_t size, rt_uint32_t attribute, rt_uint8_t region)
{
    /* 更新当前线程 mpu 表 */
    /* 无效化其他未配置区域 */
    /* 从全局配置表中获取配置，从最高优先级 region 开始配置保护区域 */
	return RT_EOK;
    
}
```

## MAL 表插入

在指定位置插入配置：

![](D:\repo\gitlab\mpu\figures\insert.png)

代码实现如下：

```c
/* pos 为插入的 region 位置 */
rt_err_t rt_mpu_insert(rt_thread_t thread, void *addr, size_t size, rt_uint32_t attribute, rt_uint8_t region)
{
	return RT_EOK;
}
```

## MAL 表切换

在线程切换时，快速切换 MAL 配置

```c
switch_to_thread
	IMPORT  rt_mpu_table_switch
	IMPORT  rt_current_thread

	.
    .
    .

    PUSH    {r0-r3, r12, lr}
    LDR     r1, =rt_current_thread
    LDR     r0, [r1]
    BL      rt_mpu_table_switch     ; switch mpu table 
    POP     {r0-r3, r12, lr}

void rt_mpu_table_switch(struct rt_thread* thread)
{
	mpu_ops->switch_table(thread);
}
```

## 获取 MAL 配置信息

```c
#define GET_MPU_REGIONS_NUMBER 
#define GET_MPU_REGIONS_CONFGIG

rt_err_t rt_mpu_get_info(rt_uint32_t type， void *arg)
{
    rt_thread_t thread;
    thread = rt_thread_self();

    switch (type)
    {
        case GET_MPU_REGIONS_NUMBER: /* 获取 regions 数量*/
            args =  thread->mpu_table.number;
        case GET_MPU_REGIONS_CONFIG: /* 获取 mpu 表配置 */
            args = 
    }
}
```

## 设置保护区域

当前系统中最多可设置两个被保护区域：

![](D:\repo\gitlab\mpu\figures\protect.png)

代码试下如下：

```c
static struct mpu_region_protect rt_mpu_protect[2];

rt_rtt_t rt_mpu_enable_protect_area(rt_thread_t thread, void *addr, size_t size, rt_uint32_t attribute)
{
	return RT_EOK;
}
```

## 关闭保护区域

取消线程保护区域：

![](D:\repo\gitlab\mpu\figures\delete_protect.png)

代码实现如下：

```c
rt_err_t rt_mpu_disabpe_protect_area(rt_thread_t thread, void *addr)
{
	return RT_EOK;
}
```

## 注册回调

触发 MPU 异常时回调函数

```c
void rt_mpu_exception_hook(rt_err_t *func())
{
    thread->mpu_exception_cb = func();
}
```

BSP  开发者，触发 MPU 异常回调

```c
void rt_mpu_exception_handler()
{
	thread->mpu_exception_cb();
}
```

## 使用例程

配置 mpu 线程堆栈保护：

```c
#define THREAD_MEMORY_SIZE 1024
uint8_t thread_stack[THREAD_MEMORY_SIZE] __attribute__((aligned(THREAD_MEMORY_SIZE)));

int main(void)
{
    struct rt_thread tid;
    rt_uint32_t attribute = 0;
    rt_thread_init(&tid, "mpu", thread1_entry, RT_NULL, thread_stack, THREAD_MEMORY_SIZE, THREAD_PRIORITY, 20);
    {
        attribute = rt_mpu_region_attribute(RT_MPU_REGION_PRIVILEGED_RO, 	  /*  特权级只读	*/ 		                                                                         RT_MPU_REGION_EXECUTE_DISABLE,    /*  不可取指	*/
                                            RT_MPU_REGION_SHAREABLE_ENABLE,   /*  可共享	*/
                                            RT_MPU_REGION_CACHEABLE_ENABLE,   /*  可缓存	*/
                                            RT_MPU_REGION_BUFFERABLE_ENABLE,  /*  可缓冲	*/
                                            0,                                /*  类型拓展	*/
                                            0                                 /*  字 region	*/
                                           );
        rt_thread_mpu_attach(&tid, thread_stack + THREAD_MEMORY_SIZE - 32, 32, attribute);
        rt_thread_startup(&tid);
    }
}
```

