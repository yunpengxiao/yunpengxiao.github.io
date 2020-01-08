# C++ 反汇编笔记

> 2019/8/11
> 
> 博观而约取，厚积而薄发。—— 苏轼

[heading-numbering]

## 目录 [no-toc] [no-number]

[TOC]

## 操作系统

### 计算机结构 _(Computer Architecture)_

- CPU + 内存 + I/O 设备
- 寄存器 -> CPU 缓存 -> 主内存 -> 内存缓存 -> 磁盘

### 进程 _(Process)_/线程 _(Thread)_

- 进程（正在执行的程序的一个实例）= 代码 + 运行状态（内存/线程/资源...）
- 线程（系统最小可调度的执行单元）= 栈内存 + 寄存器 + 线程本地存储 _(Thread Local Storage, TLS)_
- 进程状态变换：
  - dispatch: ready -> running
  - interrupt: running -> ready
  - I/O or event wait: running -> waiting
  - I/O or event complete: waiting -> ready

### 地址绑定 _(Address Binding)_

- 编译时：绝对代码
- 加载时：可重定位的代码
- 运行时：动态加载的代码

### 字节序 _(Byte Order/Endianness)_

- 大端：低到高
- 小端：高到低

### 进程地址空间 _(Process Address Space)_

- TEXT 代码
- READONLY_DATA 常量数据
- DATA 静态数据（已初始化）
- BSS 静态数据（未初始化）
- HEAP 堆（低到高）
- MEM_MAPPED 内存映射（低到高）
- STACK 栈（固定大小，高到低）

### 虚拟内存 _(Virtual Memory)_

- 32-bit 进程虚拟内存：
  - 空间：进程 2G 内存（`0x0 - 0x7FFFFFFF`）系统 2G 内存（`0x80000000-0xFFFFFFFF`）
  - 分页：4k-byte 分页
  - 分段：进程地址空间（部分段可能多进程共享）
- 类型：
  - Image：可执行文件（TEXT/READONLY_DATA）
  - Sharable/Shared：
    - Mapped File：file-backup 文件映射（MEM_MAPPED）
    - Sharable Memory：memory-backup 文件映射（MEM_MAPPED）
  - Private：
    - Heap：堆内存（HEAP）
    - Stack：栈内存（STACK）
    - Static：静态数据（DATA/BSS）
    - Process/Thread Environment Table（无法直接访问）
  - Page Table：内核维护的 当前进程页表（无法直接访问）
- 大小：
  - Private Size：进程申请的私有内存（Working Set + In Page File）
  - Working Set：进程使用中的物理内存（Private Size + Sharable/Shared Size）
  - Committed Size：进程提交的总内存（Private Size + Sharable/Shared Size）
  - Virtual Size：进程占用的总内存（Committed Size + Reserved Size）
- VirtualAlloc 流程：
  - Reserve：增加 Virtual Size
  - Commit：增加 Committed Size / Private Size
  - Read/Write/Exec：增加 Working Set
  - Decommit：减少 Committed Size / Private Size / Working Set
  - Release：减少 Virtual Size

### 缺页中断 _(Page Fault)_

- 如果进程访问的虚拟内存不在进程的物理内存中，产生缺页中断
- 硬中断 _(hard fault)_：从磁盘中加载
  - 页面文件 _(page file)_：存放被置换的虚拟内存
  - 内存映射文件 _(memory-mapped file)_：按需映射到虚拟内存中（包括可执行文件）
- 软中断 _(soft fault)_：在物理内存中共享（支持 Copy-on-Write）
  - Standby List：已被释放、没被修改，可以直接丢弃后再利用
  - Modified List：已被释放、已被修改，需要写回磁盘后再利用
- 固定页 _(pinned page)_：避免从物理内存中，把内存页置换出去

### 内存屏障 _(Memory Barrier)_

- 原因：指令重排 + 多核 CPU + CPU 流水线 + CPU 缓存
- 实现：为了实现 内存顺序，编译器/CPU/MMIO 设置内存访问的同步点
- 效果：防止指令重排（内存乱序访问）+ 保证数据可见性

## 函数调用

### 调用栈 _(Call Stack)_

- 参数（小于等于 4-byte）传递：优先使用寄存器，再使用栈内存
- 参数（大于 4-byte）传递：在栈上拷贝对象（空间为 `sizeof(obj)`，可能调用拷贝构造函数）
- 返回值（小于等于 4-byte）传递：通过 `eax` 返回
- 返回值（大于 4-byte 对象）传递：
  - 第一个参数传入为对象预留的栈上内存地址，作为 this 指针调用构造函数
  - 通过 `eax` 返回对象 this 指针（和传入的地址一致）

### 栈帧 _(Stack Frame)_

- `eip` 当前指令
  - 执行下一条指令 自动修改
  - `jxx [ADDR]` 手动修改
  - `call [ADDR]` 手动修改（相当于 `push eip; mov eip, [ADDR]`）
  - `ret` 手动修改（相当于 `pop eip`）
- `ebp` 当前栈底
  - 函数入口 `push ebp; mov ebp, esp` 手动修改
  - 函数出口 `pop ebp` 手动还原
- `esp` 当前栈顶
  - `push/pop REG` 自动修改
  - `sub/add esp n` 手动修改（申请/释放 参数或局部变量，复写传播优化）
  - 函数出口 `ret n`（用于 被调用者退栈）
    - 先弹出 `[n/4]` 个 4-byte 大小的参数（相当于 `add esp n`）
    - 再弹出 4-byte 大小的 `RetAddr`（相当于 `pop eip`）
- 内存布局（高到低）
  - 传入参数（访问方式：`[ebp+n], n >= 8`）
  - `RetAddr` 上一个函数的 返回地址，即下一个指令的 `eip`（访问方式：`[ebp+4]`）
  - `ChildEBP` 上一个函数的 基地址 `ebp`（访问方式：`[ebp]`）
  - 局部变量（访问方式：`[ebp-n], n >= 4`）
- 基于 _Frame Pointer Omission (FPO)_ 的内存布局（高到低）
  - 传入参数（访问方式：`[esp+x+n], n >= 4`）
  - `RetAddr` 上一个函数的 返回地址，即下一个指令的 `eip`（访问方式：`[esp+x]`）
  - 局部变量（访问方式：`[ebp+x-n], x > n >= 4`）
  - （`esp` 寻址：根据栈顶找到栈底 `esp+x` 后寻址，当前局部变量占用 x-byte 空间，并省略 `ebp` 压栈）

### 调用约定 _(Calling Convention)_

- 参数压栈方向：右到左
- 不稳定 _(volatile)_ 寄存器：`eax`/`ecx`/`edx` 需要调用者保存；其余由被调用者保存
- 调用者清理 _(caller clean-up)_
  - `__cdecl`（默认方式，支持变长参数）
- 被调用者退栈 _(callee clean-up)_
  - `__stdcall`
  - `__fastcall` 前两个参数放入 `ecx`/`edx`，其余参数压栈
- 根据有没有变长参数决定
  - `thiscall` 把 this 指针放入 `ecx`，参数压栈

## C++ 语言

### 编译 _(Compile)_

- 编译前处理
  - 预处理 _(preprocess)_
- 编译时转换
  - 静态成员 _(static member)_ -> 全局（静态）对象
  - 默认参数 _(default argument)_/成员变量默认值 _(default value)_ -> 硬编码调用参数
  - 内联函数 _(inline function)_ -> 不生成 `call`/`ret`
  - 引用 _(reference)_ -> 指针 _(pointer)_
  - Lambda 表达式 -> 生成 Functor 内部类
  - `new`/`delete` -> 先申请内存再调用构造函数/先调用析构函数再释放内存
  - 导出符号 _(export symbol)_：命名空间 && 类/函数/变量
  - 名字改写/修饰 _(name mangling/decoration)_：将符号进行统一编码
- 编译时检查
  - `const`
  - `constexpr`
  - `static_assert`
  - 左值 _(lvalue)_ / 右值 _(rvalue)_
  - 成员访问控制 _(member access control)_（`private`/`protected`/`public`）
  - 函数签名 _(function signature)_（函数名 + 参数类型列表）

### 控制流 _(Control Flow)_

- `if`
  - `jxx END`
- `if-else`
  - `jxx ELSE-IF/ELSE`
  - `jmp END`
- `switch` 直接跳转
  - `jxx CASE`
  - `break` -> `jmp END`
- `switch` 线性跳表
  - `jxx DEFAULT/END`
  - `jmp dword ptr [eax*4+CASE_TABLE]`
  - `break` -> `jmp END`
- `switch` 非线性索引表
  - `jxx DEFAULT/END`
  - `xor eax, eax` + `mov al, byte ptr (INDEX_TABLE)[reg]`
  - `jmp dword ptr [eax*4+CASE_TABLE]`
  - `break` -> `jmp END`
- `do`
  - `jxx BEG`
- `while`
  - `jxx END`
  - `jmp BEG`
- `for`
  - INIT -> `jmp CMP`
  - STEP
  - CMP -> `jxx END`
  - BODY -> `jmp STEP`
  - END

### 异常 _(Exception)_

- [Google 风格](https://google.github.io/styleguide/cppguide.html#Exceptions) **已禁用**
- Windows 通过 [Structured Exception Handling (SEH)](https://docs.microsoft.com/en-us/cpp/cpp/structured-exception-handling-c-cpp) 实现

### 作用域 _(Scope)_

- 常量：
  - 包括 基本类型的 常量/字符串文字量
  - 放在 READONLY_DATA 段
- 基本类型 全局/静态 变量：
  - 包括 基本类型的 全局变量/类的静态成员/函数局部的静态变量
  - 放在 DATA 段
- 全局对象：
  - 包括 全局变量/类的静态成员
  - 放在 BSS 段
  - `main()` 进入前构造，`main()` 退出后析构
- 静态对象：
  - 包括 函数的静态变量
  - 放在 BSS 段
  - 函数首次进入前构造，`main()` 退出后析构（如果构造过）
- 局部对象：
  - 包括 函数的局部变量/函数的参数/函数的返回值
  - 放在 STACK 段
  - 分配栈空间后构造，释放栈空间前析构
- 堆对象：
  - 通过 `new` 创建 /`delete` 销毁
  - 放在 HEAP 段
  - 分配堆空间后构造，释放堆空间前析构

### 堆内存管理 _(Heap Memory Management)_

- 由 系统 和 C 运行时 _(C Runtime, CRT)_ 共同管理
- 堆段 _(segment/bin)_：
  - 固定大小，由操作系统管理，给程序使用；满了继续申请，空了还给系统
  - Windows 上：申请 `VirtualAlloc`/`HeapCreate`，释放 `VirtualFree`/`HeapDestroy`
- 堆块 _(block/chunk)_：
  - 在堆段上分配，包括 本块大小、前一堆块大小、使用/空闲等 + 数据部分（C++ 代码可见）
  - Windows 上：申请 `HeapAlloc`，释放 `HeapFree`
  - C++ 层面：申请 `malloc`/`new`，释放 `free`/`delete`
- 低碎片堆 _(Low Fragmentation Heap, LFH)_

### 内存模型 _(Memory Model)_

- 最小可寻址单位是 Byte，但读写数据的最小单位是 内存位置
- 内存位置 _(memory location)_：
  - 一个标量对象（算术类型、指针类型、枚举类型、`std::nullptr_t`）
  - 非零长 位域 (bit field) 最大连续序列
- 数据竞争 _(data race)_：多个线程同时访问一个内存位置，且至少一个是写操作
  - 现象：线程从某个内存位置读取值，可能读到 初值、同一线程所写入的值、另一线程所写入的值
- 内存顺序 _(memory order)_：
  - Relaxed：无顺序约束，仅原子性
  - Consume Load：
    - 当前线程 **依赖于被读取值的** 读或写不能被重排到 **此前**（弱 向后保护）
    - 其他线程 **依赖于被读取值的** Release 对当前线程 Consume 可见
  - Acquire Load：
    - 当前线程 读或写不能被重排到 **此前**（向后保护）
    - 其他线程 Release 对当前线程 Acquire 可见
  - Release Store：
    - 当前线程 读或写不能被重排到 **此后**（向前保护）
    - 当前线程 Release 对其他线程 Acquire 可见
    - 当前线程 Release 对其他线程 **依赖于被读取值的** Consume 可见
  - Acquire-Release Read-Modify-Write：
    - 当前线程 读或写不能被重排到 **此前后**（局部保护）
    - Acquire for Read + Release for Write
  - Sequentially-Consistent：
    - **所有线程** 以同一顺序观测到所有修改
    - Acquire Load / Release Store / Acquire-Release Read-Modify-Write

## C++ 面向对象

### 对象内存布局 _(Object Memory Layout)_

- 基类布局：`[虚表指针 (opt)]-[数据成员-...]`
- 派生类布局：`[基类布局-...]-[数据成员-...]`
- 对于 继承 _(inheritance)_ 和 组合 _(composition)_ 的内存布局一致
- 对于空类，占用 1-byte 空间
- 连续内存，x86 程序按照 4-byte 对齐

### 虚函数表 _(Virtual Function Table)_

- 基类布局：`[类型信息指针 (opt)]-[虚函数指针-...]`
- 派生类布局：`[类型信息指针 (opt)]-[基类虚函数指针-...]-[派生类虚函数指针-... (opt)]`
- 如果禁用 [RTII](https://en.cppreference.com/w/cpp/types)，则没有类型信息指针
- 对于纯虚函数，虚函数指针指向 [`_purecall`](../2017/Cpp-Windows-Crash.md)

### 虚函数调用 _(Virtual Function Call)_

- `lea ecx, [OBJ+n]` 存入对象 `OBJ` 在偏移 `n` 后的基类 this 指针
- `mov eax, [ecx]` 取出基类 this 指针对应的虚表指针
- `call dword ptr [eax+n]` 调用虚表的偏移 `n` 对应的虚函数

### 构造函数 _(Constructor)_/析构函数 _(Destructor)_

- 本质：把对象内存起始地址作为 this 指针，通过 `thiscall` 方式调用
- 调用顺序（两者严格相反）：
  - 构造：`基类构造函数->...->成员构造函数->...->当前类构造函数`（按定义顺序从前到后）
  - 析构：`当前类析构函数->...->成员析构函数->...->基类析构函数`（按定义顺序从后到前）
- 避免在构造/析构时调用虚函数：
  - 对于直接调用，编译器已知虚函数指针，不需要查虚表
  - 对于间接调用（调用的非虚函数调用了虚函数），[如果调用了纯虚函数，就会出现崩溃](../2017/Cpp-Windows-Crash.md)
- 每个构造/析构函数入口，写入当前类的虚表指针：
  - 属于 [C++ 的保护措施](https://isocpp.org/wiki/faq/strange-inheritance#calling-virtuals-from-ctors)，构造/析构过程中分别写入，而不是一次性写入
  - 否则如果使用了派生类的虚表指针，就会（间接）调用派生类的虚函数，但派生类的数据成员未构造或已析构，导致崩溃）
- 基类的析构函数必须声明为虚函数：否则可能直接用派生类 this 指针调用基类析构函数

### 多重继承 _(Multiple Inheritance)_

- 虚表：
  - 派生类的虚表个数 = 有虚表的基类个数
  - 派生类自己的虚函数指针，只追加到 第一个基类 的虚表后
- 派生类指针 调用 基类虚函数
  - 需要把 this 指针偏移到基类的地址上（向后 `+n` 找到基类地址）
  - 派生类重载的虚函数里，this 指针也指向对应基类的偏移位置（向前 `-n` 还原派生类地址）
  - 对于第一个基类，`n == 0` 地址相同
  - 对于非第一个基类，先检查输入的地址是否为空（第一个基类不需要检查）

### 虚继承 _(Virtual Inheritance)_

- 菱形继承 _(Diamond Inheritance)_ 对象内存布局：
  - 虚基类：`[虚表指针 (opt)]-[数据成员-...]`（不受影响，因为不知道被继承）
  - 虚继承的基类：`[虚表指针]-[虚偏移指针]-[数据成员-...]-[虚基类布局-...]`
  - 虚继承的派生类：`[虚继承基类布局（除去 虚基类布局）-...]-[数据成员-...]-[虚基类布局-...]`
- 虚表：
  - 对于每个派生类，虚基类的虚表（包括虚析构函数）独立存放
  - 派生类自己的虚函数指针，只追加到 第一个非虚基类 的虚表后
- 虚偏移数据（4-byte * 2）：
  - 在当前类的布局中 `[当前类地址] - [虚偏移指针]` 偏移（一般为 `-4`）
  - 在当前类的布局中 `[虚基类地址] - [虚偏移指针]` 偏移
- 派生类 利用 虚偏移指针，找到 虚基类地址：
  - `mov ecx, dword ptr [虚偏移指针]`，从 虚偏移指针 找到 虚偏移数据
  - `mov edx, dword ptr [ecx+4]`，从 虚偏移数据 读取 虚基类偏移量
  - `lea eax, [虚偏移指针+edx]`，利用 虚基类偏移量 求出 虚基类地址
- 构造顺序：
  - `虚基类构造函数->...->基类构造函数->...->成员构造函数->...`
  - 保证只在最终的派生类里构造一次：添加一个参数，表示 虚基类 是否在 派生类构造时 已经被构造了
- 析构顺序：
  - `成员析构函数->...->基类析构函数->...` + 独立的 `虚基类析构函数->...`
  - 保证只在最终的派生类里析构一次：最后独立调用 虚基类析构函数

## 上层知识

### 基础概念

- 设计模式/RAII/智能指针/弱引用/回调/线程模型
- 日志/字符串操作/时间/i18n/操作系统/文件/网络/调试辅助

### C++ 标准库

- 理解 STL 中的概念，和 STL 的基本实现原理（可以借助 [NatVis](https://docs.microsoft.com/en-us/visualstudio/debugger/create-custom-views-of-native-objects) 可视化查看对象）
- 从 `std::string` 内存中定位 `c_str_/size_/capacity_` 数据
- 从 智能指针 内存中定位 `ptr_/ref_count_` 等数据

### [对象生命周期模型](../2018/Resource-Management.md)

- 互斥所有权 `std::unique_ptr`
- 共享所有权 `std::shared_ptr`
- 弱引用关系 `std::weak_ptr`
- 外部调用管理 `Class::OnDestroy() { delete this; }`
- 线程安全：创建、访问、销毁

## 参考 [no-number]

- 《C++ 反汇编与逆向分析技术揭秘》钱林松 赵海旭
- 《软件调试》张银奎
- [图说 C++ 对象模型：对象内存布局详解 - melonstreet](https://www.cnblogs.com/QG-whz/p/4909359.html)
- [使用 Windbg 分析 C++ 的虚函数表原理 - bingoli](https://bingoli.github.io/2019/03/27/windbg-multi-inherit/)
- [使用 Windbg 分析 C++ 的多重继承原理 - bingoli](https://bingoli.github.io/2019/03/21/virtual-table-by-windbg/)
- [Dance In Heap（一）：浅析堆的申请释放及相应保护机制 - hellowuzekai](https://www.freebuf.com/articles/system/151372.html)
- [浅析 Windows 下堆的结构 - hellowuzekai](https://www.freebuf.com/articles/system/156174.html)
- [FPO - Larry Osterman's WebLog](https://blogs.msdn.microsoft.com/larryosterman/2007/03/12/fpo/)

如果有什么问题，**欢迎交流**。😄

Delivered under MIT License &copy; 2019, BOT Man
