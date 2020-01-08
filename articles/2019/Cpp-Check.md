# 漫谈 C++ 的各种检查

> 2019/9/20
> 
> What you don't use you don't pay for. (zero-overhead principle) —— Bjarne Stroustrup

在学习了 [Chromium/base 库](https://github.com/chromium/chromium/tree/master/base)（[笔记](Chromium-Base-Notes.md)）后，我体会到了一般人和 **优秀工程师** 的差距 —— 拥有较高的个人素质固然重要，但更重要的是能 **降低开发门槛**，让其他人更快的融入团队，一起协作（尤其像 Chromium **开源项目** 由社区维护，开发者水平参差不齐）。

> 没吃过猪肉，但见过猪跑。🙃

项目中，降低开发门槛的方法有很多：除了 制定 **代码规范**、划分 **功能模块**、完善 **单元测试** _(unit test)_、推行 **代码审查** _(code review)_、整理 **相关文档** 之外，针对强类型的编译语言 C++，Chromium/base 库加入了大量的 **检查** _(check)_。

为什么代码中需要各种检查？在 C++ 中调用一个函数、使用一个类、实例化一个模板时，对传入的参数、使用的时机，往往会有很多 **限制** _(constraint/restriction)_（例如，数值参数不能传入负数、对象的访问不是线程安全的、函数调用不能重入）；而处理限制的方法有很多：

1. 口口相传：在 **代码审查** 时，有经验的开发者 向 新手开发者 传授经验（很容易失传）
2. 文档说明：在 **相关文档** 中，提示使用者 功能模块的各种隐含限制（很容易被忽略）
3. 检查限制：在合理划分 **功能模块** 的前提下，对模块的隐含限制 进行检查，并加入针对检查的 **单元测试**（最安全的保障，单元测试即文档）

本文主要分享 Chromium/base 库中使用的一些限制检查：

[TOC]

## 编译时检查

编译时静态检查，主要依靠 C++ 语言提供的 **语法支持**/**静态断言** 和 **编译器扩展** 实现 —— 在检查失败的情况下，编译失败。

### 测试设施

如何确保代码中添加的检查有效呢？最高效的方法是：为 “检查” 添加单元测试。但对于 编译时检查 遇到了一个 **难点** —— 如果检查失败，那么编译就无法通过。

为此，Chromium 支持 [编译失败测试 _(no-compile test)_](https://dev.chromium.org/developers/testing/no-compile-tests)：

- 单元测试文件中，每个用例通过 `#ifdef` 切割
- 每个用例中，标明 编译失败后期望的 报错细节
- 通过 `#define` 运行各个用例
- 在编译失败后，检查 报错细节 是否和预期一致

对应的单元测试文件后缀为 `*_unittest.nc`，通过 [`nocompile.gni`](https://github.com/chromium/chromium/blob/master/build/nocompile.gni) 加入单元测试工程。

### 可拷贝性检查

C++ 语言本身有很多编译时检查（例如 类的成员访问控制 _(member access control)_、`const` 关键字 在编译成汇编语言后，不能反编译还原），但 C++ 对象默认是可拷贝的，从而带来了许多问题（参考 [资源管理小记](../2018/Resource-Management.md#资源和对象的映射关系)）。

尤其是 **多态** _(polymorphic)_ 类的默认拷贝行为，一般都不符合预期：

- [C.67: A polymorphic class should suppress copying](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-copy-virtual)
- [C.130: For making deep copies of polymorphic classes prefer a virtual clone function instead of copy construction/assignment](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rh-copy)

为此，Chromium 提供了两个 [常用的宏](https://github.com/chromium/chromium/blob/master/base/macros.h)：

- `DISALLOW_COPY_AND_ASSIGN` 用于禁用类的 拷贝构造函数 和 拷贝赋值函数
- `DISALLOW_IMPLICIT_CONSTRUCTORS` 用于禁用类的 默认构造函数 和 拷贝行为

由于 Chromium 大量使用了 C++ 的多态特性，这些宏随处可见。

### 参数类型检查

Chromium 还基于 [现代 C++ 元编程](../2017/Cpp-Metaprogramming.md) 技术，通过 `static_assert` 进行静态断言。

在之前写的 [深入 C++ 回调](Inside-Cpp-Callback.md) 中，分析了 Chromium 的 [`base::Callback<>` + `base::Bind()`](https://github.com/chromium/chromium/blob/master/docs/callback.md) 回调机制，提到了相关的静态断言检查。

`base::Bind` 为了 [处理失效的（弱引用）上下文](Inside-Cpp-Callback.md#如何处理失效的（弱引用）上下文)，针对弱引用指针 [`base::WeakPtr`](https://github.com/chromium/chromium/blob/master/base/memory/weak_ptr.h) 扩展了 `base::IsWeakReceiver` 检查，判断弱引用的上下文是否有效；并通过 静态断言检查传入参数，**强制要求使用者遵循** 弱引用检查的规范：

- `base::Bind` 不允许直接将 **`this` 指针** 绑定到 类的成员函数 上，因为 `this` 裸指针可能失效 变成野指针
- `base::Bind` 不允许绑定 **lambda 表达式**，因为 `base::Bind` 无法检查 lambda 表达式捕获的 弱引用 的 有效性
- `base::Bind` 只允许将 `base::WeakPtr` 指针绑定到 **没有返回值的**（返回 `void`）类的成员函数 上，因为 当弱引用失效时不调用回调，也没有返回值

`base::Callback` 区分 [回调只能执行一次还是可以多次](Inside-Cpp-Callback.md#回调只能执行一次还是可以多次)，通过 引用限定符 _(reference qualifier)_ `&&` / `const &`，区分 在对象处于 非 const 右值 / 其他 状态时的 `Run` 成员函数，只允许一次回调 `base::OnceCallback` 在 非 const 右值 状态下调用 `Run` 函数，保证严谨的 [资源管理语义](../2018/Resource-Management.md#资源和对象的映射关系)：

``` cpp
base::OnceClosure cb; std::move(cb).Run();        // OK
base::OnceClosure cb; cb.Run();                   // not compile
const base::OnceClosure cb; cb.Run();             // not compile
const base::OnceClosure cb; std::move(cb).Run();  // not compile
```

另外，静态断言检查还广泛应用在 Chromium/base 的 **容器**、**智能指针** 模板的实现中，用于生成 [可读性更好的](../2017/Cpp-Metaprogramming.md#实例化错误) **实例化错误信息**。

### 线程标记检查

最新的 Chromium 使用了 Clang 编译，通过扩展 **线程标记** _(thread annotation)_，[静态分析线程安全问题](https://clang.llvm.org/docs/ThreadSafetyAnalysis.html)。（参考：[Thread Safety Annotations for Clang - DeLesley Hutchins](https://llvm.org/devmtg/2011-11/Hutchins_ThreadSafety.pdf)）

Chromium/base 的单元测试文件 [`thread_annotations_unittest.nc`](https://github.com/chromium/chromium/blob/master/base/thread_annotations_unittest.nc) 描述了一些 锁的错误使用场景（假设数据 data 被锁 lock 保护，定义标记为 `Type data GUARDED_BY(lock);`）：

- 访问 data 之前，忘记获取 lock
- 获取 lock 之后，忘记释放 lock

这些错误能在编译时被 Clang 检查到，从而编译失败。

## 运行时检查

运行时动态检查，主要基于 Chromium/base 库提供的 **断言 `DCHECK`/`CHECK`** 实现 —— 如果断言失败，运行着的程序会立即终止。

其中，`DCHECK` 只对 调试版 _(debug)_ 有效，而 `CHECK` 也可用于 发布版 _(release)_ —— 从而避免在发布版进行 无用的检查。

### 测试设施

检查的方法很直观 —— 构造一个检查失败的场景，期望断言失败。

Chromium/base 基础设施中的 `EXPECT_DCHECK_DEATH` 提供了这个功能，对应的单元测试文件后缀为 `*_unittest.cc`。

### 数值溢出检查

C++ 的数值类型，都是固定大小的 [标量类型](https://en.cppreference.com/w/cpp/language/type) —— 如果存储数值超出范围，会导致 **溢出** _(overflow)_。

例如，尝试通过 **使用无符号数 避免出现负数**，往往是一个典型的徒劳之举。（比如 `unsigned(0) - unsigned(1) == UINT_MAX`，参考 [ES.106: Don’t try to avoid negative values by using `unsigned`](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Res-nonnegative)）

为此，Chromium 的 [base/numerics](https://github.com/chromium/chromium/tree/master/base/numerics) 提供了一个 无依赖 _(dependency-free)_、仅头文件 _(header-only)_ 的模板库，处理数值溢出问题：

- `base::StrictNumeric`/`base::strict_cast<>()` **编译时 阻止溢出** —— 如果 类型转换 有溢出的可能性，通过静态断言报错
- `base::CheckedNumeric`/`base::checked_cast<>()` **运行时 检查溢出** —— 如果 数值运算/类型转换 出现溢出，立即终止程序
- `base::ClampedNumeric`/`base::saturated_cast<>()` **运行时 截断运算** —— 如果 数值运算/类型转换 出现溢出，对计算结果 **截断** _(non-sticky saturating)_ 处理

### 线程相关检查

最新的 Chromium/base 线程模型引入了线程池，并支持了 **序列** _(sequence)_ 的概念 —— 相对于线程池中的普通任务 乱序调度，同一序列的任务 能保证被 顺序调度 —— 因此，[推荐使用 逻辑序列 而不是 物理线程](https://github.com/chromium/chromium/blob/master/docs/threading_and_tasks.md#prefer-sequences-to-physical-threads)：

- 同一物理线程 只能同时运行 一个逻辑序列，使得 序列模型 等效于 单线程模型
- 同一物理线程 可以用于运行 多个逻辑序列，提高 物理线程 的利用率

线程/序列 相关的检查主要依赖于 **线程/序列 本地存储**：

- 每个线程有独立的 [`base::ThreadLocalStorage`](https://github.com/chromium/chromium/blob/master/base/threading/thread_local_storage.h) 线程本地存储 _(thread local storage, TLS)_
- 每个序列有独立的 [`base::SequenceLocalStorageSlot`](https://github.com/chromium/chromium/blob/master/base/threading/sequence_local_storage_slot.h) 序列本地存储 _(sequence local storage, SLS)_
- 当 逻辑序列 被放到 物理线程 上执行时，把当前序列的 SLS 关联到 执行线程的 TLS 上

#### 线程安全检查

很多时候，某个对象只会在 **同一线程/序列** 中 **创建/访问/销毁**：

- 正常情况下，**无竞争** _(contention-free)_ 模型没必要保证 **线程安全** _(thread-safety)_，因为 线程同步操作/原子操作 会带来 **不必要的开销**
- 异常情况下，一旦被 多线程同时使用，访问冲突导致 **数据竞争** _(data race)_，可能出现 未定义行为

为此，Chromium 借助 [`base::ThreadChecker`](https://github.com/chromium/chromium/blob/master/base/threading/thread_checker.h)/[`base::SequenceChecker`](https://github.com/chromium/chromium/blob/master/base/sequence_checker.h) **检查对象是否只在 同一线程/序列 中使用**：

- `[THREAD|SEQUENCE]_CHECKER(checker)` 创建并关联 线程/序列 `checker`
- `DCHECK_CALLED_ON_VALID_[THREAD|SEQUENCE](checker)` 检查或关联 `checker` 和 当前执行环境的 线程/序列
- `DETACH_FROM_[THREAD|SEQUENCE](checker)` 解除 `checker` 和 线程/序列 的关联
- 另外，发布版的检查实现为 [空对象](https://en.wikipedia.org/wiki/Null_object_pattern)，即总能通过检查

实现的 **核心思想** 非常简单：

- 线程/序列 创建时，通过 TLS/SLS 记录 当前线程/序列的 ID（例如 线程 ID、序列 ID）
- `checker` 构造时，记录 当前线程/序列的 ID
- `checker` 检查时，读取 当前线程/序列的 ID，和 `checker` 记录的 ID 比较
- `checker` 析构时，先执行检查（可以提前 解除关联）
- 另外，`checker` 读写 数据成员时，需要进行互斥的 线程同步操作（锁）

在 [sec|通知迭代检查] 提到，`base::ObserverList` 借助 `iteration_sequence_checker_` 在迭代时检查 对象操作 是否线程/序列安全。

#### 线程限制检查

程序中常常会有一些 **特殊用途的线程**（例如 客户端 UI 主线程），而这些线程往往有着 **特殊的限制**（例如，UI 线程要求保持 **响应性** _(responsive)_，[实时响应用户输入](https://github.com/chromium/chromium/blob/master/docs/threading_and_tasks.md#keeping-the-browser-responsive)）。

为此，Chromium 借助 [`base::ThreadRestrictions`](https://github.com/chromium/chromium/blob/master/base/threading/thread_restrictions.h) **检查 可能涉及线程限制的函数 在当前执行的线程上 是否允许**：

- 阻塞 _(blocking)_ 操作
  - 主要包括文件 I/O 操作（有可能被系统缓存，从而不阻塞）
  - 可能导致线程 交出 CPU 执行机会，进入 wait 状态
- 同步原语 _(sync primitive)_
  - 执行 线程同步操作
  - 可能导致程序 死锁 _(deadlock)_/卡顿 _(jank)_
- CPU 密集工作 _(CPU intensive work)_
  - 超过 100ms CPU 时间的操作
  - 可能导致程序 卡顿 _(jank)_
- 单例 _(singleton)_ 操作
  - 对于 非泄露型 [`base::Singleton`](https://github.com/chromium/chromium/blob/master/base/memory/singleton.h)，会在 [`base::AtExitManager`](https://github.com/chromium/chromium/blob/master/base/at_exit.h) 注册 “退出时销毁单例对象”
  - 如果主线程先退出，在 `base::AtExitManager` 中销毁单例，导致仍在运行的 non-joinable 线程再访问单例时，出现野指针崩溃

实现的 **核心思想** 也很简单：

- 通过 TLS 记录 当前线程的限制情况（每种限制用一个 TLS `bool` 存储）
- 对于 可能涉及限制的函数，调用前先检查 当前线程 是否允许某个限制

在最新的 Chromium/base 中，线程限制检查 被进一步封装为 [`base::ScopedBlockingCall`](https://github.com/chromium/chromium/blob/master/base/threading/scoped_blocking_call.h)，并应用于大量文件 I/O 相关函数中。

#### 死锁检查

Chromium 通过 [`base::internal::CheckedLock`](https://github.com/chromium/chromium/blob/master/base/task/common/checked_lock.h) 检查 死锁 _(deadlock)_。

实现的 **核心思想** 非常简单 —— **检查等待链是否成环**：

- 维护一个 全局的 <从每个 lock 到其 predecessor lock> 映射表（创建时添加，销毁时移除）
- 维护一个 当前线程的 <已获取 lock> 列表（TLS 存储；获取时记录，释放时移除）
- 创建时，断言 predecessor 已创建（如果 predecessor 不存在，可能顺序错误）
- 获取时，断言 predecessor 是当前线程最近获取的 lock（若不是，可能顺序错误）

### 观察者模式检查

在之前写的 [令人抓狂的 观察者模式](Insane-Observer-Pattern.md) 中，介绍了如何通过 Chromium/base 提供的 [`base::ObserverList`](https://github.com/chromium/chromium/blob/master/base/observer_list.h)，检查 观察者模式 的一些潜在问题。

#### 生命周期检查

由于观察者和被观察者的生命周期往往是解耦的，所以总会出现一些阴差阳错的问题：

- [观察者先销毁](Insane-Observer-Pattern.md#问题-观察者先销毁)
  - 问题：若 `base::ObserverList` 通知时不检查 观察者是否有效，可能导致 野指针崩溃
  - 解决：观察者继承于 [`base::CheckedObserver`](https://github.com/chromium/chromium/blob/master/base/observer_list_types.h)，在通知前 `base::ObserverList` 检查观察者弱引用 `base::WeakPtr` 的有效性
- [被观察者先销毁](Insane-Observer-Pattern.md#问题-被观察者先销毁)
  - 问题：若 `base::ObserverList` 销毁时不检查 观察者列表是否为空，可能导致 被观察者销毁后，观察者不能再移除（野指针崩溃）
  - 解决：模板参数 `check_empty` 若为 `true`，在析构时断言 “观察者已被全部移除”

#### 通知迭代检查

观察者可能在 `base::ObserverList` 通知时，再访问同一个 `base::ObserverList` 对象：

- 添加观察者
  - 问题：是否需要在 本次迭代中，继续通知 新加入的观察者
  - 解决：被观察者参数 [`base::ObserverListPolicy`](https://github.com/chromium/chromium/blob/master/base/observer_list.h) 决定迭代过程中，是否通知 新加入的观察者
- 移除观察者
  - 问题：循环内（间接）删除节点，导致迭代器失效（崩溃）`for(auto it = c.begin(); it != c.end(); ++it) c.erase(it);`
  - 解决：观察者节点 `MarkForRemoval()` 标记为 “待移除”，然后等迭代结束后移除
- 通知迭代重入
  - 问题：许多情况下，若不考虑 重入情况，可能会导致 [死循环问题](Insane-Observer-Pattern.md#问题-死循环)
  - 解决：模板参数 `allow_reentrancy` 若为 `false`，在迭代时断言 “正在通知迭代时 不允许重入”
- 销毁被观察者
  - 问题：需要立即停止 迭代过程，让所有迭代器 全部失效
  - 解决：通过特殊的 [`base::internal::WeakLinkNode`](https://github.com/chromium/chromium/blob/master/base/observer_list_internal.h) + 双向链表 [`base::LinkedList`](https://github.com/chromium/chromium/blob/master/base/containers/linked_list.h) 存储 `base::ObserverList` 所有的迭代器；在 `base::ObserverList` 析构时，将迭代器 标记为无效（自动停止迭代），并 移除、销毁
- 线程安全问题
  - 问题：由于 `base::ObserverList` 不是线程安全的，在通知迭代时，需要保证其他操作在 同一线程/序列
  - 解决：被观察者成员 `iteration_sequence_checker_` 在迭代开始时关联序列，在结束时解除关联，在迭代过程中检查 移除观察者/通知重入/销毁被观察者 操作是否序列安全（参考 [sec|线程安全检查]）

和 [`base::Singleton`](https://github.com/chromium/chromium/blob/master/base/memory/singleton.h) 一样，Chromium/base 的设计模式实现 堪称 C++ 里的典范 —— 无论是功能上，还是性能上，均为 “**人无我有，人有我优**”。

## 写在最后 [no-toc]

> 站在巨人的肩膀上。—— 艾萨克·牛顿

Chromium/base 库一直在 **迭代、优化**，**学习、借鉴** 许多其他优秀的开源项目。例如，[sec|线程标记检查] 使用的标记就来源于 [`abseil`](https://github.com/abseil/abseil-cpp)。

由于 Chromium/base 改动频繁，本文某些细节 **可能会过期**。如果有什么新发现，**欢迎补充**~ 😉

> 另外，**检查虽好，但不要滥用**。Rostislav Khlebnikov 在 CppCon2019 的演讲 [Avoid Misuse of Contracts](https://github.com/CppCon/CppCon2019/blob/master/Presentations/avoid_misuse_of_contracts/avoid_misuse_of_contracts__rostislav_khlebnikov__cppcon_2019.pdf) 中提出如何合理的使用 [**契约** _(contract)_](https://zh.cppreference.com/w/cpp/language/attributes/contract)（已从 C++ 20 中移除；但与本文的检查同理）：
> 
> - 仅检查不能违反的限制（违反时出现 未定义行为 _(undefined behavior)_，而不是抛出/返回异常）
> - 检查不能影响核心逻辑，不能产生 副作用 _(side effect)_
> - 检查不能完全替代 测试/文档/输入合法性校验，需要适当权衡

如果有什么问题，**欢迎交流**。😄

Delivered under MIT License &copy; 2019, BOT Man
