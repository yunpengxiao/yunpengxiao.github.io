# C++ 20 协程

> 2019/7/11
> 
> C++ 20 协程的 背景/应用/原理/体验

[slide-mode]

---

### 背景

- [从 时空维度 看 I/O 模型](IO-Models.md)
  - 阻塞/非阻塞：空间维度，代码编写是否连贯
  - 同步/异步：时间维度，等待时是否执行其他代码
- 多进程/多线程 -> I/O 复用（轮询）-> 回调 -> **协程**
- 同步 -> 异步，阻塞 -> 非阻塞 -> 阻塞
- 有栈协程（自己切换调用栈，例如 [libco](https://github.com/Tencent/libco)/[libgo](https://github.com/yyzybb537/libgo)）
  - Unix `getcontext/setcontext/swapcontext`
  - Windows `CreateFiber/SwitchToFiber`（纤程）
- 无栈协程（需要语言支持，例如 C#/JavaScript...）

---

### 应用 - 同步下载

``` cpp
std::string DownloadSync() {
  auto res = Fetch("https://xxx");
  if (!res.empty())
    return res;
  return Fetch("https://yyy");
}

std::cout << DownloadSync();
```

---

### 应用 - 异步下载

``` cpp
Future<std::string> DownloadAsync() {
  auto res = co_await Fetch("https://xxx");
  if (!res.empty())
    co_return res;
  co_return co_await Fetch("https://yyy");
}

std::cout << DownloadAsync().get();
```

---

### 应用 - 流式读取

``` cpp
Generator<char> ReadFile() {
  for (char ch : Read("file.txt")) {
    if (can_stop)
      co_return;
    co_yield toupper(ch);
  }
}

for (char upper_ch : ReadFile())
  std::cout << upper_ch;
```

---

### 概念

- C++ 协程（[N4775](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/n4775.pdf)）：
  - 包含以下 `co_` 关键字的函数 `Ret(Args...)`
  - **要求** `Ret` 符合 `Coroutine` 概念
- `co_return`：
  - 退出 `Coroutine` 协程函数
  - 返回 `Future (Coroutine)` 概念的结果
- `co_yield`：返回 `Generator (Coroutine)` 概念的结果
- `co_await`：等待 `Awaitable` 概念的返回结果

---

### 原理 - `Coroutine` 概念

``` cpp
concept Coroutine {
  struct promise_type {
    Coroutine get_return_object();
    Awaitable initial_suspend();
    Awaitable final_suspend();
    void unhandled_exception();  // opt
  };
};
```

- `std::coroutine_traits<Ret, Args...>::promise_type`

---

### 原理 - `Coroutine` 展开（无异常处理）

``` cpp
Ret __FN__(Args...) {
  Ret::promise_type __promise;
  __return__ = __promise.get_return_object();
  co_await __promise.initial_suspend();
  __fn__
final_suspend:
  co_await __promise.final_suspend();
}
```

---

### 原理 - `Coroutine` 展开（有异常处理）

``` cpp
Ret __FN__(Args...) {
  Ret::promise_type __promise;
  __return__ = __promise.get_return_object();
  co_await __promise.initial_suspend();
  try { __fn__ } catch (...) {
    __promise.unhandled_exception();
  }
final_suspend:
  co_await __promise.final_suspend();
}
```

---

### 原理 - `Future (Coroutine)` 概念

``` cpp
concept Future : Coroutine {
  struct promise_type {
    void return_value(const T& value);
    // or
    void return_void();
  };
};
```

---

### 原理 - `Generator (Coroutine)` 概念

``` cpp
concept Generator : Coroutine {
  struct promise_type {
    Awaitable yield_value(const T& value);
    void return_void();
  };
};
```

---

### 原理 - `co_return/co_yield` 展开

``` cpp
// co_return;
__promise.return_void();
goto final_suspend;

// co_return val;
__promise.return_value(val);
goto final_suspend;

// co_yield val;
co_await __promise.yield_value(val);
```

---

### 原理 - `std::coroutine_handle<>`

``` cpp
struct coroutine_handle<> {
  void resume();      // resume coroutine
  void destroy();     // destroy coroutine
  bool done() const;  // suspended at final
};

template <typename Promise>
struct coroutine_handle : coroutine_handle<>;
```

---

### 原理 - `Awaitable` 概念

``` cpp
concept Awaitable {
  bool await_ready();
  void await_suspend(std::coroutine_handle<>);
  auto await_resume();
};

bool await_ready(Awaitable&);
void await_suspend(Awaitable&,
                   std::coroutine_handle<>);
auto await_resume(Awaitable&);
```

---

### 原理 - `co_await` 展开

``` cpp
// /* ret = */ co_await awaiter;
if (!awaiter.await_ready()) {
  awaiter.await_suspend(__handle);
  __switch_context__
}
/* ret = */ awaiter.await_resume();
```

- `__switch_context__` **当前线程** 切换到其他协程
- `await_suspend()` 可以把 `__handle` 传递给 **其他线程**
- 调用 `__handle.resume();` 激活协程（注意 **数据竞争**）

---

### 原理 - `Awaitable` 概念（改进版）

``` cpp
concept Awaitable {
  bool await_ready();
  bool await_suspend(std::coroutine_handle<>);
  auto await_resume();
};

bool await_ready(Awaitable&);
bool await_suspend(Awaitable&,
                   std::coroutine_handle<>);
auto await_resume(Awaitable&);
```

---

### 原理 - `co_await` 展开（改进版）

``` cpp
// /* ret = */ co_await awaiter;
if (!awaiter.await_ready() &&
    awaiter.await_suspend(__handle)) {
  __switch_context__
}
/* ret = */ awaiter.await_resume();
```

- `await_suspend()` 检查状态决定是否挂起
- 参考：[Gor Nishanov, C++ Coroutines – a negative overhead abstraction](https://www.slideshare.net/SergeyPlatonov/gor-nishanov-c-coroutines-a-negative-overhead-abstraction)

---

### 体验

- [示例代码](Cpp-Coroutine/coroutine.cpp)
- 优势：
  - **异步 + 阻塞** = 执行高效 + 调用简单
  - 使用 `try-catch` **统一处理异常**
- 不足：
  - 人工编写 + 编译器生成 + STL 支持 = **关系混乱**
  - 执行 `co_await` 后，之前的 **上下文可能失效**
  - [C++2a Coroutines and Dangling References - Arthur O’Dwyer](https://quuxplusone.github.io/blog/2019/07/10/ways-to-get-dangling-references-with-coroutines/)

---

### Q & A

<br/>
<br/>
<br/>
<br/>

[align-right]

# 谢谢 🙂

---
