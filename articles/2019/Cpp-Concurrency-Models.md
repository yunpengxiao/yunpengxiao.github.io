# 深入 C++ 并发模型

> 2019/12/7
> 
> TODO

## 多线程

- 为什么要并发
  - 并发 vs 并行
- Patterns（[从时空维度看 I/O 模型](IO-Models.md)）
  - 多线程 + 同步阻塞 I/O
  - I/O 复用 + 同步非阻塞 I/O
  - 回调 + 异步非阻塞 I/O
  - 协程 + 异步阻塞 I/O
- 问题
  - 资源管理
  - 线程安全
  - UI 主线程
- Thread pool
  - Join vs Detach
  - Interruptable

### Lock based

- Problems
  - Hiding
  - Leaking
- Practice
  - Granularity
    - stand-alone or transaction
    - Protect data, not code
    - Lock striping/splitting (ConcurrentHashMap)
    - Object Pool: global lock -> sequential
  - Deadlock/Livelock
- Waiting
  - Idle: system (select/poll/epoll) or condtion variable
  - For Resource: mutex
  - Anti-pattern: sleep/yield
    - [“后文预览：Sleep 反模式”《多线程服务器的常用编程模型》](https://github.com/downloads/chenshuo/documents/multithreaded_server.pdf)
- Approach
  - `lock(...)` - all-or-nothing simultaneously
  - `recursive_mutex` - reentrance for current thread
  - `timed_mutex` - timeout
  - `shared_lock` - reader/writter lock
  - `condition_variable` - repeating event
  - `future`/`promise` - once event

### Lock free

- Memory model
- Busy waiting
- Transaction?

## Contention free

- `PostTask[AndReply[WithResult]]`
  - bind functor with data = closure
- use data flow to replace shared data
- use thread local object with checker
  - `base::ThreadChecker/SequenceChecker`

## Active Object / Actor

## Clojure STM

- 使用不可变的值：Separate mutable identity from immutable state by value，参考 [Clojure](https://clojure.org/about/state#_working_models_and_identity)

## Communicating Sequential Processes

## 多线程和所有权

- 如果使用互斥所有权，使用时 `std::unique_ptr::get()` 后，原始对象被释放，从而导致野指针
  - [R.37: Do not pass a pointer or reference obtained from an aliased smart pointer](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rr-smartptrget)
- 如果使用共享所有权，通过 PIN `std::shared_ptr` 拷贝副本后，原始对象被释放，所有权转移到副本上，导致析构时机/位置不确定
  - 线程亲和对象，必须在特定线程上完成析构
  - 析构复杂对象，可能意外的阻塞其他线程
  - 如果使用弱引用指针 `std::weak_ptr`，使用时需要调用 `lock()` 方法，也会出现所有权转移的问题
  - 可以放到 `BlockingQueue<shared_ptr<void>>`，在专门的线程统一析构（参考：[shared_ptr 技术与陷阱 |《当析构函数遇到多线程 —— C++ 中线程安全的对象回调》陈硕](https://github.com/downloads/chenshuo/documents/dtor_meets_mt.pdf)）

## 多线程和 `shared_ptr`

- 构造过程 一般不涉及多线程（除了单例）
- 通过 **原子性引用计数** _(atomic ref-count)_ 中间层，保证析构过程 线程安全
- 指针本身 非线程安全，需要使用原子操作 [`std::atomic<std::shared_ptr<T>>`](https://en.cppreference.com/w/cpp/memory/shared_ptr/atomic2)
  - [copy-on-write](https://blog.csdn.net/Solstice/article/details/3351751)/double-buffer
- 使用指向的对象 非线程安全，需要 加锁保护

TODO: shared_ptr

- 指针本身是值语义
- 直接存储原始对象指针，不需要虚析构函数（不会从基类指针删除）
- 意外引用/循环引用 -> 无法释放
- 放在 header 里，会导致 `incomplete type` 问题
- 如果继承了 `std::enable_shared_from_this`，必须要使用至少一个 `std::shared_ptr` 来装载原始对象；否则调用 `shared_from_this()` 方法时，会抛出 `std::bad_weak_ptr` 异常

TODO:

- 多线程回调
- 线程安全的观察者模式
- 多线程资源管理

如果有什么问题，**欢迎交流**。😄

Delivered under MIT License &copy; 2019, BOT Man
