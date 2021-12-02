# MAL 组件架构适配

目前 MAL 组件适配了 ARM 架构的结构。该文档以 ARM 为例，讲解其他架构如何适配 MAL 组件

## 架构移植调用的 API

对于不同的架构，如：arm、riscv、mips；需要实现不同的 ops。

```c
rt_err_t rt_mpu_ops_register(struct rt_mpu_ops *ops);
```

| 参数 | 描述                          |
| ---- | ----------------------------- |
| ops  | 每个架构对 mal 的不同实现方法 |

目前 mal ops 需要实现以下接口：

```c
struct rt_mpu_ops
{
   rt_err_t (*init) (struct rt_mal_region *regions); /* 初始化函数 */
   void (*switch_table) (rt_thread_t thread, rt_uint8_t mpu_protect_area_num, struct mpu_protect_regions* mpu_protect_areas);
   /* 表切换函数 */
   rt_err_t (*get_info) (rt_thread_t thread, rt_uint32_t type, void *arg); /* 获取配置信息 */
};
```

## 1.1 创建架构文件夹

在 mal/arch 文件目录下创建所需要适配的架构，例如 `arm`、`riscv`。

```shell
MAL
├───port 							// 不同架构下的 MPU 对接 MAL 的移植文件
│   └───arm                         // arm 架构
│   	└───arm_mal.c 
│   	└───arm_mal.h
│   └───riscv                       // riscv 架构
│   	└───riscv_mal.c 
│   	└───riscv_mal.h
```

## 1.2 修改 SConscript 文件

修改 mal 目录下的 SConscript 文件，根据不同的架构添加架构适配文件：

```python
if GetDepend('ARCH_ARM'):
    src += Glob('arch/arm/*.c')
    path += [cwd + '/arch/arm']
elif GetDepend('ARCH_riscv'):
    src += Glob('arch/riscv/*.c')
    path += [cwd + '/arch/riscv']
```

## 1.3 MAL OPS 接口适配

对于不同的架构，MAL 抽象出了一套通用的接口，用户只需要简单的实现以下接口功能，即可完成适配

### 1.3.1 初始化接口

使用该接口，完成对不同架构 MPU 的初始化，在这里主要完成对系统硬件的 MPU 配置：Flash、RAM、外设 等访问权限，这些配置用户无法通过 MAL 组件层的接口进行修改。是全局有效的配置：

```c
rt_err_t (*init) (struct rt_mal_region *regions);
```

| 参数    | 描述               |
| ------- | ------------------ |
| regions | mal 系统硬件配置表 |

### 1.3.2 表切换接口

用户通过 MAL 接口完成对表的操作，在该接口中，通过读取 MAL 配置表，设置 MPU 寄存器的值。该接口主要功能：对用户配置的 region 表进行切换；读取保护区域的 region 配置，对于不同的线程设置不同的访问权限；如果开启了线程栈保护，对线程栈的底部 32 个字节进行读保护：

```c
void (*switch_table) (rt_thread_t thread, rt_uint8_t mpu_protect_area_num, struct mpu_protect_regions* mpu_protect_areas);
```

| 参数    | 描述     |
| ------- | -------- |
| access  | 访问权限 |
| execute | 可取指   |

### 1.3.3 获取 MAL 信息

用过可通过该接口，获取当前 MAL 的一些配置信息：

```c
 rt_err_t (*get_info) (rt_thread_t thread, rt_uint32_t type, void *arg);
```

| 参数   | 描述                                                         |
| ------ | ------------------------------------------------------------ |
| thread | 线程句柄。线程句柄由用户提供出来，并指向对应的线程控制块内存地址 |
| type   | 信息类型                                                     |
| arg    | 存放信息地址                                                 |
| 返回值 |                                                              |
| RT_EOK | 查询成功                                                     |
| OTHERS | 查询失败                                                     |

type 的取值定义如下表所示：

| 参数                    | 描述                                  |
| ----------------------- | ------------------------------------- |
| GET_MPU_REGIONS_NUMBER  | 获取当前线程 MAL 已配置的 region 数目 |
| GET_MPU_REGIONS_CONFGIG | 获取当前线程 MAL 所有 region 配置     |