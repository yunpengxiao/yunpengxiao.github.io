# 再谈“回调 vs 接口”设计

> 2019/12/21
> 
> 如何提高感知力和控制力，就像学游泳一样，要到水中练，去亲近水。——《格蠹汇编》张银奎

这两年写过很多类似的文章，先整理这一系列的思路：

- [如何浅显的解释 回调函数](../2017/Callback-Explained.md)
  - 从 [**函数式编程** _(functional programming)_](https://en.wikipedia.org/wiki/Functional_programming)、[**一等公民** _(first-class function)_](https://en.wikipedia.org/wiki/First-class_function)、[**高阶函数** _(higher-order function)_](https://en.wikipedia.org/wiki/Higher-order_function) 的角度，介绍回调函数 **是什么**
  - 从 [**依赖注入** _(dependency injection, DI)_](https://en.wikipedia.org/wiki/Dependency_injection)、[**控制反转** _(Inversion of Control, IoC)_](https://en.wikipedia.org/wiki/Inversion_of_control) 的角度，解释 **为什么** 要用回调函数
  - 最后举了 JavaScript 和 C 语言的例子，简单介绍了同步/异步回调 **怎么做**
- [回调 vs 接口](../2017/Callback-vs-Interface.md)
  - 举了 C++ 的例子，用 继承/组合接口、绑定函数/构造 lambda 的方式，实现 [**观察者模式** _(observer pattern)_](https://en.wikipedia.org/wiki/Observer_pattern)
  - 讨论了使用接口的 **局限性**、使用回调的 **灵活性**
  - 最后举了 面向过程 C 语言（没有对象和闭包）、面向对象脚本语言 JavaScript（动态类型 + 垃圾回收）、面向对象编译语言 C++（静态类型 + 内存管理）的例子，进一步分析 **如何实现** 回调机制
- [对 编程范式 的简单思考](../2019/Thinking-Programming-Paradigms.md)
  - 从 [**编程范式** _(programming paradigm)_](https://en.wikipedia.org/wiki/Programming_paradigm) 的角度，分析了 “回调 vs 接口” **问题的本质**
  - 提到了可以用 函数式的 [**闭包** _(closure)_](https://en.wikipedia.org/wiki/Closure_%28computer_programming%29) 替换 面向对象的 [**设计模式** _(design patterns)_](https://en.wikipedia.org/wiki/Design_Patterns)
  - 然而，C++/Java 等面向对象语言，本质上借助了 **类** _(class)_ 和 **对象 _(object)_** 实现 `std::function<>` 和 `lambda`
- [深入 C++ 回调](Inside-Cpp-Callback.md)
  - 从 **同步** _(sync)_、**异步** _(async)_、回调次数 等角度，讨论了 C++ 回调中的 **所有权** _(ownership)_ 和 **生命周期管理** _(lifetime management)_ 问题
  - 指出了 C++ 原生的 [`std::bind`](https://en.cppreference.com/w/cpp/utility/functional/bind)/[`lambda`](https://en.cppreference.com/w/cpp/language/lambda) + [`std::function`](https://en.cppreference.com/w/cpp/utility/functional/function) 的 **不足**，和 Chromium 的 [`base::Bind`](https://github.com/chromium/chromium/blob/master/base/bind.h) + [`base::Callback`](https://github.com/chromium/chromium/blob/master/base/callback.h) 的 **优势**

本文用两个例子，从 **代码设计** _(code design)_ 的角度谈谈一些想法。

## 回复 TL;DR [no-toc]

最近有位热心的朋友留言：

- [理解 观察者、中介者 模式](../2017/Observer-Mediator-Explained.md) 最后提到的 **如何用回调替换接口** 的方案不够详细（建议看看 [回调 vs 接口](../2017/Callback-vs-Interface.md)，如果还有问题欢迎指出~ 😉）
- 希望多写写关于 **回调和接口** 的文章

![reader-comment](Callback-vs-Interface-Explained/reader-comment.jpg)

因为微信不让私信回复了😐，所以写一篇文章来讨论🙃。

另外，由衷感谢每位读者的支持和讨论~ 😉

## 论道 TL;DR [no-toc]

最近看到一篇关于 iOS 编译优化的文章，里边的一个方案引起了我的注意：

> - 因为 **模板** 的 **编译时多态** 编译速度过慢，而且会导致代码膨胀
> - 所以引入 **虚基类**，用 **运行时多态** 的方法 “优化”
> - 引入一个 `hyper_function<>` 全量 **drop-in** 替换 `std::function<>`
> - `hyper_function<>`（[代码链接](https://gist.github.com/lingol/ed9feab92da9e341487855084411df4e) / [备份链接](Callback-vs-Interface-Explained/hyper_function.hpp)）参考自 Arthur O’Dwyer 的 [_37 percent of HyperRogue’s compilation time is due to `std::function`_](https://quuxplusone.github.io/blog/2019/01/06/hyper-function/)

在好奇心的驱使下，看了一下这个方案（以异步下载 `DownloadAsync` 为例，通过回调 `on_done` 返回结果）：

- 原始方案 **基于泛型** `Result` 的 `std::function<>` 接口：

``` cpp
template <typename Result>
void DownloadAsync(std::function<void(Result*)> on_done);
```

- 全部改为 **基于基类** `ResultBase` 的 `hyper_function<>` 接口：

``` cpp
class ResultBase {
 public:
  virtual Json ToJson() const = 0;
};
void DownloadAsync(hyper_function<void(ResultBase*)> on_done);
```

- 基于上述两个接口，调用者都可以 **传递 lambda 表达式** 处理结果：

``` cpp
class DerivedResult : public ResultBase {
 public:
  Json ToJson() const override;  // virtual, but unused
  std::string ToString() const;  // non-virtual
};

DownloadAsync([](DerivedResult* d) {
  Print(d->ToString());  // non-virtual call
});
```

这颠覆了我的认知：

- 调用者传递的 `hyper_function<void(Derived*)>` 竟然可以 **隐式转换** 成 `hyper_function<void(Base*)>` 类型的 **可调用** _(callable)_ 对象
- 而一般这是 **不允许的** —— 只有 `std::function<void(Base*)>` 可以转换成 `std::function<void(Derived*)>`，反之不行

因为，[`std::function<>` 构造时](https://en.cppreference.com/w/cpp/utility/functional/function/function) 会用 [`std::is_invocable<>`](https://en.cppreference.com/w/cpp/types/is_invocable) 检查 **函数签名** _(function signature)_ 的兼容性：

- 原始的可调用对象 `void(Base*)` 可以接受 `Derived*` 参数（`Derived*` **向上转换** _(up cast)_ 为 `Base*`），转化后的 `void(Derived*)` 当然也可以接受 `Derived*` 参数 —— **变窄** _(narrowed)_ 可行
- 原始的可调用对象 `void(Derived*)` 不一定能接受 `Base*` 参数（`Base*` **向下转换** _(down cast)_ 为 `Derived*`），故不能转换成 `void(Base*)` —— **变宽** _(widen)_ 不行

在好奇心的驱使下，看了 [`hyper_function<>` 的实现原理](https://gist.github.com/lingol/ed9feab92da9e341487855084411df4e)（[备份链接](Callback-vs-Interface-Explained/hyper_function.hpp)）：

- 先用 [`std::is_convertible<>`](https://en.cppreference.com/w/cpp/types/is_convertible) 检查参数是否可以 **“反向转换”** 😵
- 再用 [`reinterpret_cast<>()`](https://en.cppreference.com/w/cpp/language/reinterpret_cast) 对可调用对象的函数签名 **类型强转** 😑

这有什么危害 —— 绕过类型检查，可能导致 **函数调用崩溃**，给后人挖坑：

- 之前曾遇到过类似的崩溃 —— `Base* d1 = new Derived1; Derived2* d2 = static_cast<Derived2*>(d1);` 导致 **对象布局不对齐**（参考：[崩溃分析笔记](Crash-Analysis-Notes.md)）
- 基于这个假设，用 `hyper_function<>` 做了试验，于是出现了 **错误的内存访问** https://godbolt.org/z/M4uBjQ

> 网友评论：滥用向下转换，和用 C 语言的 `void*` 有什么差别？都是在 拿枪打自己的脚 _(shoot yourself in the foot)_。🙄

另外，朴素的 `hyper_function<>`（包括 [Arthur 的版本](https://github.com/zenorogue/hyperrogue/blob/master/hyper_function.h)）虽然提升了编译速度，但增加了 创建时堆分配、使用/销毁时虚函数 的开销。而 `std::function<>` 利用编译时技巧，**内联** _(inline)_ 存储可调用对象，减少运行时开销。（感谢 [卜恪](https://www.zhihu.com/people/Madokakaroto) 大佬的指正；但这 **不是本文的重点**）

## 探讨 [no-toc]

其实，在刚学 C++ 的时候，也曾犯过类似的设计错误 —— **试图用面向对象的方法，解决泛型编程的问题，从而引入了无用的虚基类**：

- 创建 `Derived` 对象
- 转换为 基类指针 `Base*` 存储对象
- 使用对象前，先还原为 派生类指针 `Derived*`
- 使用对象时，却不使用 `Base` 提供的纯虚方法

接下来用 “异步下载” 的例子，从代码设计的角度，阐述这个问题的 **由来** 和 **解法**：

[TOC]

### 朴素设计 —— 数据和计算耦合

下载完成后 **打印结果** 可以实现为：

``` cpp
void DownloadAsyncAndPrint() {
  // ... download async and construct |result| ...
  Print(result);
}
```

下载完成后 **写数据库** 可以实现为：

``` cpp
void DownloadAsyncAndWriteToDB() {
  // ... download async and construct |result| ...
  WriteToDB(result);
}
```

通过 [**抽取函数** _(extract function)_](https://refactoring.com/catalog/extractFunction.html) 重构公共逻辑（异步下载的核心逻辑）：

``` cpp
std::future<Result> DownloadAsyncImpl();

void DownloadAsyncAndPrint() {
  Result result = co_await DownloadAsyncImpl();
  Print(result);
}

void DownloadAsyncAndWriteToDB() {
  Result result = co_await DownloadAsyncImpl();
  WriteToDB(result);
}
```

**存在的问题**：无法封装 “异步下载” 模块 ——

- 一方面 **不可能针对 所有需求** 提供上述接口（有人需要打印结果，有人需要写数据库，还有人需要...）
- 另一方面 **需要提供 不涉及实现细节** 的接口（比如 `DownloadAsyncImpl` 基于 [C++ 20 的协程](Cpp-Coroutine-Share.md)，可以改用多线程实现，但调用者并不关心）

—— 本质上，面向过程的结构化设计，导致数据 `result` 生产和消费的逻辑 **耦合** _(coupling)_ 在了一起，不易于扩展。

### 控制反转 —— 解耦发送者和接收者

为了解决这个问题，需要引入 **控制反转** _(IoC)_。从 纯面向对象 的视角看：

- 一个数据：`result`
- 两个角色：发送者（`DownloadAsyncImpl`）和 接收者（`Print`/`WriteToDB`）
- 一个目的：**解耦** _(decouple)_ 发送者和接收者

一般把公共逻辑抽象为 **框架** _(framework)_，再用以下两种方法实现（参考：[控制反转 —— 计算可扩展性](../2017/Thinking-Scalability.md#控制反转-计算可扩展性)）。

1) 使用 [**模板方法模式** _(template method pattern)_](../2017/Design-Patterns-Notes-3.md#Template-Method)，通过 **继承** _(inheritance)_，在发送者（虚基类）上 **重载** 接收者（`protected` 纯虚方法）逻辑：

``` cpp
// interface
class Downloader {
 public:
  virtual ~Downloader() = default;
  void DownloadAsync() {
    Result result = co_await DownloadAsyncImpl();
    Handle(result);
  }
 protected:
  virtual void Handle(const Result& result) const = 0;
};

// client code
class PrintDownloader : public Downloader {
 protected:
  void Handle(const Result& result) const override {
    Print(result);
  }
};
auto print_downloader = std::make_unique<PrintDownloader>();
print_downloader->DownloadAsync();
```

2) 使用 [**策略模式** _(strategy pattern)_](../2017/Design-Patterns-Notes-3.md#Strategy)，通过 **组合** _(composition)_，向发送者（类/函数）**传递** 接收者（派生类）逻辑：

``` cpp
// interface
class Handler {
 public:
  virtual ~Handler() = default;
  virtual void Handle(const Result& result) const = 0;
};

void DownloadAsync(std::unique_ptr<Handler> handler) {
  Result result = co_await DownloadAsyncImpl();
  handler->Handle(result);
}

// client code
class WriteToDBHandler : public Handler {
 public:
  void Handle(const Result& result) const override {
    WriteToDB(result);
  }
};
DownloadAsync(std::make_unique<WriteToDBHandler>());
```

**存在的问题**：引入了 **基于类的** _(class-based)_ 面向对象 ——

- 模板方法 基于继承，接收者 **派生于** 发送者，在运行时 **不能动态更换** 接收者；故有 “组合优于继承” _(favor object composition over class inheritance)_
- 策略模式 基于组合，但要为 **每种类型定义** 一个接收者的 **接口（虚基类）**，仍要和 “类” 捆绑在一起（参考：[回调 vs 接口](../2017/Callback-vs-Interface.md)）

—— 本质上，面向对象的 **封装** _(encapsulation)_ 把 数据 和 对数据的操作（方法）捆绑在类里，引入了复杂的 **类层次结构** _(class hierarchy)_，最后沦为 **面向类编程** _(class-oriented programming)_。（参考：[对编程范式的简单思考](../2019/Thinking-Programming-Paradigms.md)）

[![theory-vs-reality](Callback-vs-Interface-Explained/theory-vs-reality.png)](https://www.reddit.com/r/ProgrammerHumor/comments/418x95/theory_vs_reality/)

王垠的 [解密“设计模式”](https://www.yinwang.org/blog-cn/2013/03/07/design-patterns)（[备份](Thinking-Programming-Paradigms/解密设计模式.html)）批判了（面向对象）设计模式的 **“历史局限性”**：

> （设计模式）变成了一种教条，带来了公司里程序的严重复杂化以及效率低下 ... 什么都得放进 `class` 里 ... 代码弯了几道弯，让人难以理解。

孟岩的 [function/bind的救赎（上）](https://blog.csdn.net/myan/article/details/5928531) 也提到 “面向类编程” 脱离了 **“对象的本质”**：

> Simula 和 Smalltalk 最重大的不同，就是 Simula **用方法调用的方式** 向对象发送消息，而 Smalltalk 构造了更灵活和更纯粹的消息发送机制 ... C++ **静态消息机制** 还引起了更深严重的问题 —— 扭曲了人们对面向对象的理解 ... “面向对象编程” 变成了 “面向类编程”，“面向类编程” 变成了 **“构造类继承树”**。

Joe Armstrong（Erlang 主要发明者）[也批评过](http://codersatwork.com/) “面向类编程” 的 **“污染性”**：

> The problem with object-oriented languages is they’ve got all this implicit environment that they carry around with them. You wanted a **banana** but what you got was a **gorilla holding the banana** and the **entire jungle**.

[![gorilla-holding-banana](Callback-vs-Interface-Explained/gorilla-holding-banana.jpg)](https://www.johndcook.com/blog/2011/07/19/you-wanted-banana/)

### 回调闭包 —— 函数签名“替换”类层次结构

其实，可以使用 **回调闭包** _(callback closure)_ 实现等效的 **依赖注入** _(DI)_ 功能：

``` cpp
// interface
using OnDoneCallable = std::function<void(const Result& result)>;
void DownloadAsync(OnDoneCallable callback) {
  Result result = co_await DownloadAsyncImpl();
  callback(result);
}

// client code
DownloadAsync(std::bind(&Print));
```

上述代码去掉了 `class`，把 `handler` 对象改为 `callback` 闭包，把 虚函数调用 改为 回调闭包的调用，不再需要接口和继承 —— 脱离了 “类” 的束缚，是不是 **清晰多了**？

> 实现上：`std::function<>` 通过 **类型擦除** _(type erasure)_ 将闭包适配到同一类型上。（参考：[回调 vs 接口](../2017/Callback-vs-Interface.md)）

正如 Steve Yegge 的 [Execution in the Kingdom of Nouns](http://steve-yegge.blogspot.com/2006/03/execution-in-kingdom-of-nouns.html) 提到的（面向对象 vs 函数式）：

> Object Oriented Programming puts the **Nouns first and foremost**. It's not as if OOP has suddenly made **verbs less important** in the way we actually think. It's a strangely **skewed perspective**.

—— 面向对象的错误在于：把名词（对象）作为一等公民，而动词（函数）只能附属于（捆绑在）对象上。

### 泛型编程 —— 抽象概念“替换”类层次结构

实际上，也可以使用 **泛型编程** _(generic programming)_ 进一步化简：

``` cpp
// interface
template <typename OnDoneCallable>
void DownloadAsync(OnDoneCallable callback) {
  Result result = co_await DownloadAsyncImpl();
  callback(result);
}

// client code
DownloadAsync(std::bind(&Print));
```

上述代码中：

- 回调闭包 属于一个抽象的 [**概念** _(concept)_](https://en.cppreference.com/w/cpp/experimental/constraints)，即一个能处理 `Result` 的 **可调用对象**
- 函数模板 `DownloadAsync<>` 只关心 `callback` 能处理 `result`，而不需要关心它的实际类型是什么

> 实现上：泛型编程通过 **模板** _(template)_ 的 **编译时多态**，实现 **静态派发** _(static dispatch)_。（参考：[简单的 C++ 结构体字段反射](../2018/Cpp-Struct-Field-Reflection.md)）

正如 Alexander Stepanov（STL 主要设计者）[在采访中说的](http://www.stlport.org/resources/StepanovUSA.html)（面向对象 vs 泛型编程）：

> - I find OOP **technically** unsound. It attempts to decompose the world in terms of interfaces that vary on a single type.
> - I find OOP **philosophically** unsound. It claims that everything is an object.
> - I find OOP **methodologically** wrong. It starts with classes.

—— 试想一下，如果错误设计了 STL（容器的迭代器基于 `Iterator` 接口、对象的比较基于 `Comparable` 接口），会是一番怎样的场景？

### 错误设计 —— `Derived-Base-Derived`

所以，[sec|论道 TL;DR] 提到的设计，问题出在哪？

- 虚基类 **破坏泛化**
  - 回调实际处理的 `DerivedResult` 不得不继承于 `ResultBase` 接口
  - 派生类 `DerivedResult` 不得不实现 “可能用不到的” `ToJson()` 纯虚方法
- 接口的 **语义错误**
  - 从函数签名看 `hyper_function<void(ResultBase*)>` 能处理 `ResultBase` 对象
  - 而实际传入的回调闭包，却只能处理 `DerivedResult` 对象
- 杂糅的 **设计缺陷** —— 试图用 **面向对象的方法**，解决 **泛型编程的问题**

**基于泛型** 的 “直接做法” 是 —— 用 `typename OnDoneCallable` **直接泛化** 可调用对象：

``` cpp
template <typename OnDoneCallable>
void DownloadAsync(OnDoneCallable on_done);
```

**基于泛型和回调** 的 “间接做法” 是 —— 先用 `std::function<>` **擦除** 可调用对象的 **类型**，再用 `typename Result` **泛化** 回调函数的 **签名**（优化前的原始方案）：

``` cpp
template <typename Result>
void DownloadAsync(std::function<void(Result*)> on_done);
```

**不用泛型** 的 “正确做法” 是 —— 针对不同类型 **分别定义并实现**（基于回调的）接口：

``` cpp
void DownloadAsync(std::function<void(RawResult*)> on_done);
void DownloadAsync(std::function<void(HtmlResult*)> on_done);
void DownloadAsync(std::function<void(JsonResult*)> on_done);
```

**不用泛型和回调** 的 “正确做法” 是 —— 引入面向对象机制，使用 **策略模式** 定义不同处理接口（也可以用 **模板方法模式**）：

``` cpp
class RawResultHandler {
 public:
  virtual ~RawResultHandler() = default;
  virtual void Handle(const RawResult& result) const = 0;
};
class HtmlResultHandler {
 public:
  virtual ~HtmlResultHandler() = default;
  virtual void Handle(const HtmlResult& result) const = 0;
};
class JsonResultHandler {
 public:
  virtual ~JsonResultHandler() = default;
  virtual void Handle(const JsonResult& result) const = 0;
};

void DownloadAsync(std::unique_ptr<RawResultHandler> handler);
void DownloadAsync(std::unique_ptr<HtmlResultHandler> handler);
void DownloadAsync(std::unique_ptr<JsonResultHandler> handler);
```

假设，再进一步 **去掉控制反转**，我们还可以...（篇幅有限，读者自由发挥）

## 写在最后 [no-toc]

先 “空谈” 几点感悟，后续再写文章细聊：

- 学会思辨，而不盲从
- 保持好奇心，不止于了解 how，更需要探究 why（存在即合理）
- 多看多学多练，才能体会不同方案的利弊
- 炼钢者，莫用土法
- 此消彼长：编译时间 vs 运行时间，短期 “ROI” vs 长期可维护性
- 通过交流、学习，写出默认正确的代码 _(Write the **right** code **by default**)_

如果有什么问题，**欢迎交流**。😄

Delivered under MIT License &copy; 2019, BOT Man
