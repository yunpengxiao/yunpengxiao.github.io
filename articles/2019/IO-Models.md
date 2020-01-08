# 从时空维度看 I/O 模型

> 2019/5/6
> 
> 从时间/空间的维度，总结 **同步异步**、**阻塞非阻塞** I/O 模型的异同

在学习了 [V8 JavaScript 引擎](https://v8.dev/) 后，我发现：JavaScript 代码不知道外部 C++ 代码的存在，而 C++ 代码可以灵活的控制多个 JavaScript 代码环境。

正如地球是圆的一样：看问题的维度（而不是角度）不同，对世界的认识也不一样。

本文用几个简单的例子阐述 **同步异步**、**阻塞非阻塞** 的 **不同世界观**。

## 同步、阻塞

例如，调用 UNIX 系统的 `send()` 通过 **普通的** `fd` 发送数据：

``` cpp
ssize_t ret = send(fd, buffer, len, flags);
```

- 当前线程的函数调用 **阻塞到 I/O 完成时**
- 如果需要 **提高 I/O 吞吐率**，可以采用 **多进程/多线程** 的形式（多个线程同时发起 I/O，分别阻塞处理）

## 同步、非阻塞

例如，调用 UNIX 系统的 `send()` 通过 **非阻塞的** `fd` 发送数据：

``` cpp
evutil_make_socket_nonblocking(fd);

size_t len_to_send = len;
while (!len_to_send) {
  ssize_t ret = send(fd, buffer, len, flags);
  // case1: ready to send
  if (ret >= 0) {
    len_to_send -= ret;
    continue;
  }
  // case2: not ready
  if (EVUTIL_SOCKET_ERROR() == EAGAIN) {
    continue;
  }
  // case3: error
  throw err;
}
```

- 可以使用 [libevent](http://libevent.org/) 提供的 [`evutil_make_socket_nonblocking()`](https://github.com/libevent/libevent/blob/master/include/event2/util.h) 将 fd 设置为非阻塞
- 函数调用 **立即返回**：
  - 如果调用时可以发送数据，则立即发送数据
  - 如果调用时暂时无法发送数据，[`EVUTIL_SOCKET_ERROR()`](https://github.com/libevent/libevent/blob/master/include/event2/util.h) 返回 `EAGAIN` / `EWOULDBLOCK`
- 如果需要 **提高 I/O 吞吐率**，可以采用 **I/O 复用** 的形式（一个线程同时发起多个 I/O，同时检查各个 I/O 的完成情况；推荐阅读：[epoll 的本质是什么](https://my.oschina.net/editorial-story/blog/3052308)）

## 异步、非阻塞

例如，Node.js 通过 [`fs.readFile()`](https://nodejs.org/api/fs.html#fs_fs_readfile_path_options_callback) 读取文件：

``` javascript
fs.readFile(filename, (err, data) => {
  if (err) {
    // handle |err|
  }
  // use |data|
});
```

- 需要系统/语言支持，一般提供基于 [回调](https://en.wikipedia.org/wiki/Callback_%28computer_programming%29) 的接口：
  - 函数 `fs.readFile()` **发起 I/O 请求**，然后 **立即返回**
  - 在 “发起 I/O 请求” 到 “I/O 完成” 之间，当前线程会 **执行其他代码**
  - **I/O 完成时**，当前线程 **回调** `(err, data) => { ... }` 函数，并传入数据 `data`（如果成功）或错误 `err`（如果失败）
- 如果系统/语言不支持，可以基于 **I/O 复用** 在 **用户态** 实现类似的封装（例如 [libevent](http://libevent.org/) 封装了 `epoll` 操作，提供了一组基于回调的接口）
- 回调的线程/调用栈 可能不一样：
  - Unix 的 [`aio_read()`](https://linux.die.net/man/3/aio_read) 和 Windows 的 [`ReadFileEx()`](https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-readfileex) 由 **系统回调**，线程/调用栈不确定
  - Node.js 的 `fs.readFile()` 由 JavaScript 环境在 **JavaScript 线程回调**
  - **用户态** I/O 复用的封装，由 **阻塞的线程回调**（例如 libevent 调用 `event_base_dispatch()` 函数）

## 异步、阻塞

例如，Node.js 用 [`util.promisify`](https://nodejs.org/api/util.html#util_util_promisify_original) 封装 `fs.readFile()` 接口：

``` javascript
const readFileAsync = util.promisify(fs.readFile);

try {
  const data = await readFileAsync(filename);
  // use |data|
} catch (err) {
  // handle |err|
}
```

- 需要系统/语言支持 [协程](https://en.wikipedia.org/wiki/Coroutine)（例如 [ECMAScript6](https://en.wikipedia.org/wiki/ECMAScript)），一般采用基于协程 `async/await` 原语的接口：
  - 函数 `readFileAsync` **发起 I/O 请求**，然后 **阻塞到 I/O 完成时**
  - 在 “发起 I/O 请求” 到 “I/O 完成” 之间，当前线程会 **切换执行其他代码**
  - **I/O 完成时**，当前线程 **切换回去**，并返回数据 `data`（如果成功）或抛出异常 `err`（如果失败）
- 如果系统/语言不支持，则无法实现（例如 **UNIX 系统/C 语言 不支持** 协程，参考：[Asynchronous I/O | Wikipedia](https://en.wikipedia.org/wiki/Asynchronous_I/O#Forms)）

## 世界观对比

**阻塞/非阻塞** 像是 **空间** 维度的对比 —— “发起 I/O 请求” 代码是否 **紧跟着** “I/O 完成处理逻辑”：

- **阻塞模型** 把函数调用 **阻塞到 I/O 完成时**，并通过函数的返回值返回 **I/O 结果**，保证往下执行的代码能 **直接使用结果**
- **非阻塞模型** “发起 I/O 请求” 就 **立即返回**，往下执行的代码 **得不到 I/O 结果**：
  - 对于 **同步模型**，需要调用者 **不断检查** I/O 是否完成
  - 对于 **异步模型**，在 I/O 结束时，**通过回调传递** I/O 的结果

**同步/异步** 像是 **时间** 维度的对比 —— 从 “发起 I/O 请求” 到 “I/O 完成” 之间，同一线程会不会 **执行其他代码**：

- **同步模型** 同一线程里的代码执行顺序是 **连续的**：
  - 对于 **阻塞模型**，调用者会 **一直等到** 当前 I/O 完成后，再往下执行
  - 对于 **非阻塞模型**，调用者会 **不断检查** 当前 I/O 的执行情况，直到完成后才往下执行
- **异步模型** 同一线程里的代码执行顺序是 **跳跃的**：
  - 对于 **非阻塞模型**，通过 **回调传递** 调用者发起的 I/O 的结果，**回调顺序不确定**
  - 对于 **阻塞模型**，通过协程的 **挂起/恢复** 机制，阻塞当前代码的执行，同时切换到其他代码执行，**切换顺序不确定**

## 写在最后

随着编程语言的发展，I/O 模型的演进 **从同步到异步，从阻塞到非阻塞再回到阻塞**：

- **异步模型** 能避免 **单线程** 等待 I/O 的 **时间浪费**，**吞吐率更高**
- **阻塞模型** 能让代码更 **连贯**，写起来逻辑更 **清晰**

最复杂的模型是 **异步、非阻塞**，需要编程人员有更高的素质：

- 空间上代码不连贯：一般需要调用者把 “发起 I/O 请求” 代码的一些数据，作为 **回调上下文** _(context)_ 传给 “I/O 完成处理逻辑”（如果需要进行 **连续多次** I/O 操作，一层层的回调函数嵌套，会导致 [回调地狱 _(callback hell)_](http://callbackhell.com/) 问题）
- 时间上执行不连贯：不确定 “发起 I/O 请求” 到 “I/O 完成” 之间，当前线程执行了什么代码（由于代码执行顺序不明确，需要注意 **上下文失效** 的情况；参考：[深入 C++ 回调](Inside-Cpp-Callback.md#回调是同步还是异步的)）

从 “同步、阻塞” 看似又回到了 “异步、阻塞”，两者的区别在于：当前线程的函数调用阻塞后，前者由操作系统通过 **线程切换** 调度，后者则通过 **协程切换** 调度。后者的优势在于：

1. 协程切换比线程切换 **开销小**（无需系统调用，自己管理调用栈内存，没有数量限制）
2. 协程切换比线程切换 **有更多机会被执行**（**协程** 不管怎么切换，执行的代码都是 **当前线程**；**线程** 一旦切换，执行的代码可能就 **不是当前进程** 里的了）
3. 协程的单线程模型 **更可控**（不用考虑多线程的 **数据竞争** 问题，没有锁的开销）

> 最后聊个臆想：为什么 **阻塞 I/O 和非阻塞 I/O 容易理解**，而 **异步 I/O 比同步 I/O 难以理解** 呢？因为 “低维度的生物永远无法理解高维度的空间” 🙃
> 
> - **阻塞 I/O 的代码** 类似于 **一维空间** 里的生物，**非阻塞 I/O 的代码** 类似于生活在 **二维空间** 里：相对于顺序执行，**空间上增加一个维度**，就可以实现非顺序的执行
> - 假设 **同步 I/O 的代码** 运行在我们的 **三维空间** 里，那么 **异步 I/O 的代码** 需要时间维度的支持，至少在 **四维空间** 里才能实现：
>   - 三维空间感知到的 **时间总是线性的**，正如秒针滴答滴答有规律的跳着
>   - 而四维空间里，可以挂起一个的世界的运行，切换到另一个平行世界里运行；被挂起的三维空间里，时间仿佛静止在了某一刻，而四维空间里的时间却还在流动（调用四维空间里的 `GetTimeNow()` 就会发现时间并没有静止，只不过两个空间的时间不是线性关系）

感谢 [@flythief](https://github.com/thiefuniverse) / [@WalkerJG](https://github.com/WalkerJG) 的 review~

如果有什么问题，**欢迎交流**。😄

Delivered under MIT License &copy; 2019, BOT Man
