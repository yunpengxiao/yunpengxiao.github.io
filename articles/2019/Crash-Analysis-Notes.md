# 崩溃分析笔记

> 2019/8/4
> 
> We all earn our pay and reputations not by how we debug, but by how quickly and accurately we do it. —— Mark Russinovich

[heading-numbering]

通过崩溃现场排查问题，是一件很有意思的事情 —— 像是 “侦探推理” ——

- 你只能看到崩溃瞬间的状态（崩溃时转储为 Dump 文件），从有限的信息里收集线索
- 你可能看不到崩溃的位置的代码（例如第三方模块），需要利用“逆向”思维来大胆假设
- 你需要掌握更全面的知识（了解 操作系统/编译原理/C++，熟悉被分析的代码），从有限的线索里验证猜想

最近的 Windows/C++ 崩溃分析工作，让我再次学到了如何 “从底层的视角，看上层的事物”。

## 目录 [no-toc] [no-number]

[TOC]

## 崩溃类型

> 如何提高感知力和控制力，就像学游泳一样，要到水中练，去亲近水。——《格蠹汇编》张银奎

### 内存破坏崩溃

- 原因：
  - 野指针：释放后使用/重复释放，可能覆写正常对象内存
    - 异步回调时，在抛出任务时绑定的对象已被销毁
    - 同步调用时，调用栈上层函数 销毁 下层函数 还在使用的对象
    - 循环内（间接）删除节点，导致迭代器失效：`for(auto it = c.begin(); it != c.end(); ++it) c.erase(it);`
    - 存储裸指针 `std::vector<T*>` 而不是弱引用 `std::vector<base::WeakPtr<T>>`
  - 缓冲区溢出：写入错误的 起始指针 或 缓冲区长度
  - 指针强转：基类指针 `static_cast<>()` 向下转成错误的派生类，对象布局不对齐
  - 数据竞争：
    - 其他线程 销毁 当前线程 正在使用的对象（例如 `std::unique_ptr<>` 管理的对象被其他线程销毁，但当前线程正在通过裸指针使用；可以改用 `std::shared_ptr<>` 将对象 PIN 到使用的函数内）
    - 非线程安全对象被其他线程 创建/访问/销毁（例如 UI 数据只允许 UI 线程 **访问**，可能重入字体缓存）
- 排查：
  - Mini Dump 数据不全，只能用 Full Dump 分析（[sec|判断对象是否有效]）
  - 如果没有立即崩溃，由于数据损坏，可能导致 **其他崩溃**
- 辅助：
  - 添加日志/上报
  - 检查堆破坏（例如 Pageheap/AddressSanitizer）
  - 检查栈破坏（例如 `__security_cookie`/`__chkesp`/`0xCC`/StackVars）
  - 逻辑错误立即崩溃（例如 `assert`/`CHECK`）

### 空指针崩溃

- 原因：
  - 函数的前置条件改变，调用者对条件的假设（例如 `debug_assert`/`DCHECK`）失效
  - 内存破坏导致
- 排查：
  - 回溯调用栈，可以发现空指针首次出现的位置，并推断出现的路径
  - 空指针加上 offset 再访问数据成员，变为野指针崩溃（标识：指针值小于对象大小）

### 除零崩溃

- 原因：
  - 除 `/` 0
  - 取模 `%` 0
- 原理：`cdq; idiv eax, ?` -> `eax` 商 & `edx` 余数

### 栈溢出崩溃

- 原因：
  - 逻辑缺陷导致无穷递归调用（例如 发送 `WM_CLOSE` 导致无限弹出关闭确认对话框，阻塞代码）
  - 在栈上申请较大的局部变量，耗尽栈内存
- 排查：
  - 回溯调用栈，找到递归调用的函数区间
  - 通过 `ChildEBP` 分析各个栈帧内存使用情况

### 内存申请崩溃

- 原因：
  - 申请过大的内存
  - 堆碎片化
- 排查：
  - 校验外部输入合法性

### 系统调用崩溃

- 现象：
  - `QueryPerformanceFrequency`/`localtime(&-1)`/`wcsftime(..., nullptr)` 函数调用崩溃
  - `DLLMain`/`WinProc` 回调位置错误（被重定向到无效函数）
- 原因：
  - 系统内部缺陷
  - 外部代码注入

### 指令错误崩溃

- 类型：
  - `ILLEGAL_INSTRUCTION` 指令无效
  - `MISALIGNED_CODE` 指令不对齐
  - `PRIVILEGED_INSTRUCTION` 无权限执行
  - `SOFTWARE_NX_FAULT` 不在可执行段内
- 原因：
  - 外部注入导致指令损坏
  - 内存破坏导致跳转错误
- 特点：调用栈分散，不易于聚合统计
- 解决：
  - [禁止第三方注入](https://blog.chromium.org/2017/11/reducing-chrome-crashes-caused-by-third.html)
  - 检查可执行文件完整性

### 磁盘错误崩溃

- 现象：内存映射文件 `MEM_MAPPED` 读取时，磁盘 `hardware_disk` 错误 `IN_PAGE_ERROR`
- 原因：
  - 磁盘损坏/磁盘驱动错误
  - 文件存放于网络位置，但网络不稳定（例如 网吧无盘系统）

## Windbg 常用命令

> 工欲善其事，必先利其器。——《论语·卫灵公》

- `!analyze -v; .ecxr` 定位崩溃位置
- `version`/`|`/`~`/`.frame` 系统/进程/线程/栈帧
- `kPL` 查看调用栈和参数，并隐藏代码位置
- `kvL` 查看调用栈和FPO/调用约定，并隐藏代码位置
- `dv` 列举当前栈帧上 参数、局部变量 的值
- `dt TYPE [ADDR]` 查看 TYPE 类型对象（在 ADDR 上）的内存布局
- `dx ((TYPE *)ADDR)`
  - 使用 [NatVis](https://docs.microsoft.com/en-us/visualstudio/debugger/create-custom-views-of-native-objects) 查看 TYPE 可视化数据（内置 STL 支持）
  - 根据 对象内存布局 + 虚函数表指针，还原 ADDR 上 TYPE 实际的派生类
- `dps __VFN_table` 查看虚函数表指针对应的虚函数地址
- `u ADDR`/`ub ADDR`/`uf ADDR/SYM` 反汇编/往前反汇编/反汇编函数
- `x MOD!SYM*` 匹配函数符号（包括虚函数表符号）
- `s -d 0 L?80000000 VAL` 搜索内存中是否存在 VAL 值
- `!address ADDR` 查看地址属性（堆/栈/代码区，可读/可写/可执行，已提交/保留）
- `!heap -a [SEGMENT]` 查看（堆段 SEGMENT 上）内存分配详情

> 调试常用命令：
> 
> - `bp`/`bu`/`bm` 设置断点/设置未加载模块断点/设置特定符号断点
> - `ba [e|r|w] [1|2|4] ADDR` 设置访问特定地址的断点
> - `b? ... "CMD"` 中断后执行 CMD 命令（例如 `r eax=0;g` 复现崩溃）
> - `bl`/`bc` 列举断点/清除断点

## 分析技巧

> Dump 文件就像照片，每个文件都可能让我们想起一段故事，时间越长，越值得回味。——《格蠹汇编》张银奎

### 聚合统计维度

- 顶部调用栈（哈希值）：
  - 所有模块的前 K 个指令在模块中的偏移
  - 特定模块的前 K 个指令在模块中的偏移
- 加载模块/外部进程/安全软件：查找共同的注入模块/外部环境
- 操作系统/硬件情况

### 判断代码是否有效-1

- `uf eip` 查看崩溃代码的指令
- `lm a eip` 查看崩溃代码对应模块的地址范围，记录 `START1`
- `?eip-START1` 计算崩溃代码在模块中的偏移量，记录 `OFFSET`
- 本地运行相同版本的模块 `MODULE`
- `lm m MODULE` 查看正常代码对应模块的地址范围，记录 `START2`
- `uf START2+OFFSET` 查看正常代码的指令，和崩溃代码比较

### 判断代码是否有效-2

- `uf eip` 查看崩溃函数的指令
- 检查是否有明显逻辑错误（例如，`[esi/epb+-10000h]`/不常见的长指令/调用不合理的函数）

### 判断对象是否有效

- 如果对象内存 **没被覆写**：
  - 如果 `__VFN_table` 有效，表示头部完整
  - 如果 [`base::WeakPtrFactory::ptr_ == this`](https://cs.chromium.org/chromium/src/base/memory/weak_ptr.h?q=base::WeakPtrFactory)，表示尾部完整
  - 通过 `ref_count_/size_` 等变量推测数据成员是否有效
  - 判断有没有执行过析构函数的迹象（例如 `ptr_ = nullptr;`）
- 如果对象内存 **已被覆写**：
  - 通过 `db/da/du` 查看是否被写成 字符串
  - 通过 `dps` 查看是否被写成 其他对象
- 检查其他平级的成员对象：
  - 指针值是否临近（指针成员变量，要求在同一构造函数里创建，不一定准确）
  - 连续内存是否完整（非指针成员变量）

### 查找内存中的对象

- `x` 列出类的（所有）虚函数表指针
- `s -d` 搜索内存中的虚函数表指针（多重继承取最小指针）
- `dt` 还原对象内存布局
- 通过再次搜索（成员指针）或 减去偏移量（成员对象），找到对象的持有者

### 排查异步回调任务

- 切换 [`base::TaskAnnotator::RunTask()`](https://cs.chromium.org/chromium/src/base/task/common/task_annotator.cc?q=base::TaskAnnotator::RunTask) 栈帧，查看异步调试信息（借助 magic number 判断数据完整性）
- 通过 [`base::Location base::PendingTask::posted_from;`](https://cs.chromium.org/chromium/src/base/location.h?q=base::Location) 定位任务抛出的位置（静态部分：文件名/函数名/行号 + 动态部分：PC 寄存器）
- 通过 [`base::debug::TaskTrace base::PendingTask::task_backtrace;`](https://cs.chromium.org/chromium/src/base/debug/task_trace.h?q=base::debug::TaskTrace) 查看任务链（当前任务的所有前继任务）

### 考虑崩溃时机

- `main()` 前初始化（全局对象）
- 启动时初始化（消息循环栈对象）
- 消息循环/定时器阶段（静态对象 + `LazySingleton`）
- 退出时反初始化（消息循环栈对象 + `AtExitManager`）
- `main()` 后反初始化（静态对象 + 全局对象）

### 排查外部因素

- 发布线上运营任务，触达之前的不常用路径
- 第三方注入经常导致 指令错误崩溃
- 修复某些崩溃，引发后续执行代码的崩溃

## 参考 [no-number]

- [C++ 反汇编笔记](Cpp-Disassembly-Notes.md)（必备知识点）
- [Windbg 调试命令详解 - 张佩](http://yiiyee.cn/blog/2013/08/23/windbg/)
- [利用 C++ 类对象的虚拟函数表指针在内存中搜索对象 - 张佩](https://blog.csdn.net/blog_index/article/details/7016696)
- [从堆里寻找丢失的数据 - 张银奎](http://advdbg.org/blogs/advdbg_system/articles/3413.aspx)

> Debugging is twice as hard as writing the code in the first place. —— Kernighan’s Law

如果有什么问题，**欢迎交流**。😄

Delivered under MIT License &copy; 2019, BOT Man
