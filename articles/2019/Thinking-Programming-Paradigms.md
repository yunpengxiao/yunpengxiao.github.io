# 对编程范式的简单思考

> 2019/1/16
> 
> 编程最重要的事情，其实是让写出来的符号，能够简单地对实际或者想象出来的“世界”进行建模。一个程序员最重要的能力，是直觉地看见符号和现实物体之间的对应关系。 —— 王垠

## TOC [no-toc]

[TOC]

## 为什么要写这篇文章 TL;DR

### #1 [no-toc]

一位朋友曾经曾经和我说：“千万别和不知道回调函数的人，解释什么是回调函数”。（见 [如何浅显的解释回调函数](../2017/Callback-Explained.md)）我本以为 **回调函数** _(callback function)_ 是一个非常简单的概念，但和许多刚入门编程的人解释这个概念的时候，他们都觉得很 **费解**。

直到现在，我才发现，原来要理解回调函数，就需要先接受 [函数是一等公民 _(first-class function)_](https://en.wikipedia.org/wiki/First-class_function) 的事实（函数和数据一样都可以被存储、传递），然后理解 [高阶函数 _(higher-order function)_](https://en.wikipedia.org/wiki/Higher-order_function) 的概念（函数可以作为参数传递到另一个函数里）。

这对于没有接触过 **函数式** 的人来说，简直是 **世界观的颠覆**，因为：

- **面向过程** 里，数据是数据、操作是操作
- **面向对象** 里，数据和操作放到对象里，属于对象的一部分

### #2 [no-toc]

为了批判 **面向对象** 里 “操作必须放到对象里” 的回调思想，写了一篇文章 [回调 vs 接口](../2017/Callback-vs-Interface.md)（后来读到 陈硕 也有一篇类似的文章 [《以 `boost::function` 和 `boost::bind` 取代虚函数》](https://github.com/downloads/chenshuo/documents/CppPractice.pdf)），但境界还不够，一直没有发现这个问题的 **本质** —— **函数式 vs 面向对象**。

同样是实现 [依赖注入 _(dependency injection)_](https://martinfowler.com/articles/injection.html) 的功能：

- **面向对象** 需要借助接口和继承（紧耦合）
- **函数式** 高阶函数特性就能原生支持（松耦合）

### #3 [no-toc]

之前我也 ~~跟风~~ 写过一篇 [高阶函数：消除循环和临时变量](../2018/Higher-Order-Functions.md)，讲的是如何使用 `all`/`any`/`map`/`filter`/`fold` 之类的高阶函数。现在想才明白了问题的 **本质** —— 使用 **函数式** 的方法，实现 **面向对象** 的 [内部迭代 _(internal iteration)_](https://en.wikipedia.org/wiki/Iterator#Internal_Iterators)（类似 [访问者模式 _(visitor pattern)_](../2017/Design-Patterns-Notes-3.md#Visitor)，属于 [迭代器模式 _(iterator pattern)_](../2017/Design-Patterns-Notes-3.md#Iterator)），从而消除循环和迭代器临时变量。

### #4 [no-toc]

最近终于读懂了几篇 [王垠的博客](https://www.yinwang.org/)，大概能理解了文章的思想（虽然比较偏激，但论述非常严谨）：

- [解密“设计模式”](https://www.yinwang.org/blog-cn/2013/03/07/design-patterns)：批判（面向对象）设计模式（[备份](Thinking-Programming-Paradigms/解密设计模式.html)）
- [Purely functional languages and monads](https://yinwang0.wordpress.com/2013/11/16/pure-fp-and-monads/)：批判“纯”函数式（[备份](Thinking-Programming-Paradigms/Purely-functional-languages-and-monads_Surely-I-Am-Joking.html)）
- [编程的宗派](https://www.yinwang.org/blog-cn/2015/04/03/paradigms)：批判“纯”面向对象、“纯”函数式 和 说“各有各的好处”的“好好先生”（[备份](Thinking-Programming-Paradigms/编程的宗派.html)）

在《解密“设计模式”》提到，**面向对象** 的 “设计模式” 是为了解决 “一切皆对象” 思想导致的问题。即使是 Erich Gamma（设计模式作者之一）粉丝的我，也深有感触：

- 由于 函数不是一等公民、不支持高阶函数，我们需要 [创建型模式 _(creational patterns)_](../2017/Design-Patterns-Notes-1.md)（两种工厂、原型、创建者）
- 由于 不支持闭包、数据必须放到对象里，我们需要 [结构型模式 _(structural patterns)_](../2017/Design-Patterns-Notes-2.md)（适配器、桥接、组合、装饰器、代理）
- 由于 操作必须放到对象里，我们需要 [行为型模式 _(behavioral patterns)_](../2017/Design-Patterns-Notes-3.md)（命令、责任链、观察者、中介者、状态、策略、模板方法、迭代器、访问者、解释器）

![Happy Coding by Erich Gamma](../2017/Thinking-Coding/Happy-Coding.jpg)

[align-center]

> Happy Coding _(by Erich Gamma)_

本文总结一下自己的一些体会，并用一个例子加以阐述。

## 三种范式

我只接触过的三种 [编程范式 _(programming paradigm)_](https://en.wikipedia.org/wiki/Programming_paradigm)：

- [面向过程 _(procedural programming)_](https://en.wikipedia.org/wiki/Procedural_programming)
- [面向对象 _(object-oriented programming)_](https://en.wikipedia.org/wiki/Object-oriented_programming)
- [函数式 _(functional programming)_](https://en.wikipedia.org/wiki/Functional_programming)

| 范式 | 数据 | 计算 | 计算数据 |
|-|---|---|---|
| 面向过程 | 数据结构 | 控制流（顺序/选择/循环）| 用控制流描述算法，修改数据 |
| 面向对象 | 对象的字段 | 对象的方法+面向过程 | 调用对象的方法，派发消息 |
| 函数式 | 数据结构/闭包 | 链式调用/模式匹配/递归调用 | 用数据/函数调用函数 |

### 面向过程

> Algorithms + Data Structures = Programs（算法 + 数据结构 = 程序）—— Niklaus Wirth

数据和计算是两个不同的角色：

- 数据 **存储** 了程序运行的 **状态**
- 计算 通过修改数据，**变换** 运行的 **状态**

> 参考：[图灵机 _(Turing machine)_](https://en.wikipedia.org/wiki/Turing_machine)

### 面向对象

- [封装 _(encapsulation)_](https://en.wikipedia.org/wiki/Encapsulation_%28computer_programming%29)：将数据和计算放到一起，并引入访问控制，从而实现不变式 _(invariant)_
- [继承 _(inheritance)_](https://en.wikipedia.org/wiki/Inheritance_%28object-oriented_programming%29)：共享数据和计算，避免冗余
  - class-based —— 通过派生类支持多继承
  - prototype-based —— 通过原型链实现单继承
- [多态 _(polymorphism)_](https://en.wikipedia.org/wiki/Polymorphism_%28computer_science%29)：面向对象的 **核心**，派发同一个消息 或 调用同一个方法，实现不同的操作
  - subtyping —— 运行时的 **重写** _(override)_
  - ad-hoc/parametric —— 编译时/运行时的 **重载** _(overload)_

> 参考：[浅谈面向对象编程](../2018/Object-Oriented-Programming.md)

### 函数式

由于数据是 **有状态的** _(stateful)_，而计算是 **无状态的** _(stateless)_；所以需要将数据 **绑定** _(bind)_ 到函数上，得到“有状态”的函数，即 [闭包 _(closure)_](https://en.wikipedia.org/wiki/Closure_%28computer_programming%29)。通过构造、传递、调用 闭包，实现复杂的功能组合。

> 参考：[λ 演算 _(lambda calculus)_](https://en.wikipedia.org/wiki/Lambda_calculus)

## 例子：运行时动态选择计算操作

用 C++ 模拟一个简单的编辑器，点击界面上不同的按钮，执行不同的操作，并讨论各个范式如何在运行时 **动态选择** 计算操作（很简单，点击“新建”则新建文件，点击“打开”则打开文件，点击“保存”则保存文件）：

[align-center]

<p>
<button onclick="alert('ClickNew ')">新建</button>
<button onclick="alert('ClickOpen')">打开</button>
<button onclick="alert('ClickSave')">保存</button>
</p>

``` cpp
enum Action {
  ClickNew,
  ClickOpen,
  ClickSave,
};
```

在程序的系统内：

- 点击事件的 **发送者** 是各个按钮
- 点击事件的 **接收者** 是程序所操作的文件

### 面向过程

> [代码链接](Thinking-Programming-Paradigms/procedural.cc)

定义文件 **数据结构**（用一个 `struct` 封装起来）：

``` cpp
struct File {
  // data
};
```

定义文件 **操作**（传递数据进行操作，返回 `bool` 表示是否成功）：

``` cpp
bool NewFile(File** file);        // opreation
bool OpenFile(File** file);       // opreation
bool SaveFile(const File* file);  // opreation
```

（点击事件的 **接收者**）定义文件 **数据**（实例化数据结构，存储程序的运行状态）：

``` cpp
File* g_current_file;  // data storage
```

（点击事件的 **发送者**）定义 **如何操作数据**（跟据不同的 `action` 修改数据 `g_current_file`，选择不同的操作，变换程序的运行状态）：

``` cpp
if (action == ClickNew) {
  NewFile(&g_current_file);
} else if (action == ClickOpen) {
  OpenFile(&g_current_file);
} else if (action == ClickSave) {
  SaveFile(g_current_file);
}
```

上边代码的主要问题是：

- 事件的发送者 **直接依赖** 事件的接收者，即点击时直接用 `g_current_file` 调用 `*File` 函数
- 导致按钮 **不能复用** —— 如果按钮点击后执行其他操作，就需要继续添加 `if-else` 语句

为了让代码更灵活，我们可以使用 **面向对象** 的方法。

### 面向对象

> [代码链接](Thinking-Programming-Paradigms/object-oriented.cc)

定义文件 **类** _(class)_（将文件的 **数据** 和 **操作** 封装到一起，分别作为类的字段和方法；引入访问控制，隐藏数据，暴露操作；并通过 **异常** _(exception)_ 表示是否成功）：

``` cpp
class File {
 public:
  static std::unique_ptr<File> New();   // opreation
  static std::unique_ptr<File> Open();  // opreation
  void Save() const;                    // opreation

 private:
  // data
};
```

定义命令接口 `Command`（利用 [命令模式 _(command pattern)_](../2017/Design-Patterns-Notes-3.md#Command)，消除点击事件的 **接收者** 和 **发送者** 之间的依赖）：

``` cpp
class Command {
 public:
  virtual ~Command() = default;
  virtual void Run() = 0;  // indirection
};
```

定义新建/打开/保存文件对应的实际命令 `*Command`（实现 `Command` 接口，执行实际的 **操作**；存储 **上下文** _(context)_ **数据**，并在执行操作时使用）：

``` cpp
class NewCommand : public Command {
 public:
  NewCommand(std::unique_ptr<File>& file) : file_(file) {}
  void Run() override { file_.get() = File::New(); }

 private:
  std::reference_wrapper<std::unique_ptr<File>> file_;  // context
};

class OpenCommand : public Command { /* ... */ };
class SaveCommand : public Command { /* ... */ };
```

（点击事件的 **接收者**）定义文件 **对象** _(object)_（实例化类，存储对应的数据和操作）：

``` cpp
std::unique_ptr<File> file_;  // data storage
```

（在点击事件的 **接收者** 和 **发送者** 之间）定义 **中间层** _(indirection)_（将点击事件的接收者 `file_` 传入 `Command` 的上下文；并利用 [依赖注入 _(dependency injection)_](https://en.wikipedia.org/wiki/Dependency_injection) 的方法，为不同的 `action` 分配不同的 `Command`）：

``` cpp
std::unordered_map<Action, std::unique_ptr<Command>> handlers_;

handlers_.emplace(ClickNew,
    std::make_unique<NewCommand>(std::ref(file_)));
handlers_.emplace(ClickOpen,
    std::make_unique<OpenCommand>(std::ref(file_)));
handlers_.emplace(ClickSave,
    std::make_unique<SaveCommand>(std::cref(file_)));
```

（点击事件的 **发送者**）定义 **如何操作数据**（通过中间层派发消息，利用多态机制，动态选择具体执行的操作）：

``` cpp
handlers_.at(action)->Run();
```

在点击保存按钮时，消息的传递流程是：

```
          Run                         Save
(Sender) -----> SaveCommand(Command) ------> File(Receiver)
```

命令模式最核心的地方就是：事件的发送者 发送 `Run` 消息给 `Command`，对于不同类型的 `Command` 对象，会执行不同的操作，从而实现动态选择行为。这样，事件的发送者 **不依赖于** 事件的接收者 —— 按钮不关心自己被点击之后执行什么操作，照着命令去做就行。

为了 **解耦** _(decouple)_ 事件的发送者和接收者，面向对象就引入了好几种不同的 [行为型设计模式 _(behavioral patterns)_](../2017/Design-Patterns-Notes-3.md#Decouple-Sender-Receiver)。

这代码还可以化简吗？可以，用 **函数式** 的方法化简！

### 函数式

> [代码链接](Thinking-Programming-Paradigms/object-oriented.cc)

复用上一个例子的文件 **类** `class File` 和 **对象** `file_`（点击事件的 **接收者**）。

重新定义 `Command`（将命令定义为一个函数）：

``` cpp
using Command = std::function<void()>;
```

利用新的 `Command` 定义类似的 **中间层**（将 `file_` 作为上下文构造闭包，再分配给各个 `action`）：

``` cpp
std::unordered_map<Action, Command> handlers_ = {
    {ClickNew, [this] { file_ = File::New(); }},
    {ClickOpen, [this] { file_ = File::Open(); }},
    {ClickSave, [this] { if (file_) file_->Save(); }},
};
```

（点击事件的 **发送者**）重新定义 **如何操作数据**（通过中间层选择函数，并执行对应的函数）：

``` cpp
handlers_.at(action)();
```

和 **面向对象** 方法的区别在于：

- 面向对象 把数据和操作 **放到对象里**
- 函数式 把数据和计算 **放到闭包里**

对于 C++，上面的代码 **本质** 上是通过 **面向对象** 实现的：

- [`std::function`](https://en.cppreference.com/w/cpp/utility/functional/function) 是基于带有 `operator()(...)` 抽象类，在构造时利用泛型技巧，抹除传入的 [可调用 _(callable)_](https://en.cppreference.com/w/cpp/concept/Callable) 对象的类型，仅保留调用的签名（[原理](https://shaharmike.com/cpp/naive-std-function/) / [代码](../2017/Callback-vs-Interface/std_function.cpp)）
- [lambda 表达式](https://en.cppreference.com/w/cpp/language/lambda) 会被编译为带有 `operator()(...)` 的类，并构造时捕获当前的上下文（类似前面的 `NewCommand`）；可以传入 `std::function` 封装为更抽象的可调用对象

> 2019/2/22 补充：
> 
> 使用命令模式，相对于传递函数，更适用于需要 **撤销 (undo)/重做 (redo)** 的情况。（参考：[游戏设计模式](https://gameprogrammingpatterns.com/command.html)）

## 写在最后

一切并不都是“对象”，可以用 **函数** 表示 **纯粹的数学运算**：

- 将 JSON 数据 **转换** 成 XML
- 把两幅图像 **合成** 为一幅新的图像
- 用爬虫 **提取** HTML 页面链接

一切也并不都是“函数”，需要用 **对象** 记录 **随时间变换的状态**：

- 用户 **打开** 的一个个窗口
- 在文本框 **输入** 的一个个文字
- 从其他设备上 **同步** 过来的一条条记录

所以，正如王垠在《编程的宗派》所说的：

> - 有些东西本来就是有随时间变化的“状态”的，如果你偏要用“纯函数式”语言去描述它，当然你就进入了那些 monad 之类的死胡同。
> - 如果你进入另一个极端，一定要用对象来表达本来很纯的数学函数，那么你一样会把简单的问题搞复杂。

本文仅是我的一些个人理解。如果有什么问题，**欢迎交流**。😄

Delivered under MIT License &copy; 2019, BOT Man
