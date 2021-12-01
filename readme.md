## 描述

MAL (mpu abstract layer) 是 rt-thread 官方基于 Memory Protect Unit (MPU) 做的一款用于内存保护的抽象层组件。MAL 组件代码位于 `stm32h750-artpi-h750/rt-thread/components/mal`

## 作用

内存保护单元（MPU），提供了内存区域保护功能。通过 MPU，可以使程序更具健壮性，提高嵌入式系统的稳定性，使系统更加安全。通过使用 MPU 组件能使 RT-Thread 操作系统在进军军工、医疗、汽车等行业更具竞争力。MAL 组件具有以下特性：

- 线程堆栈溢出检测
- 内存隔离，某一线程使用的内存区域，其他线程禁止访问
- 为每个线程设定不同的区域的内存的访问权限

## 反馈

如果你对 MAL 组件有任何提议，欢迎向本仓库提出 PR 和 issue：

1. 代码 BUG
2. 接口定义是否清晰，抽象
3. 功能是否实现

## 群号

如果你对 MAL 组件感兴趣，欢迎加入群，和大家一起完善 MAL 组件：



## 文档

MAL 组件文档：

- [mal 简介与使用](stm32h750-artpi-h750/rt-thread/components/mal/readme.md)
- [mal api 介绍](stm32h750-artpi-h750/rt-thread/components/mal/doc/mal_api.md)
- [mal 移植教程](stm32h750-artpi-h750/rt-thread/components/mal/doc/mal_port.md)

