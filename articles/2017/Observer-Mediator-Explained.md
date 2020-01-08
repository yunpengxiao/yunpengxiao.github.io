# 理解观察者、中介者模式

> 2017/10/6
>
> Any fool can write code that a computer can understand.
> Good programmers write code that humans can understand.
> —— Martin Fowler

[heading-numbering]

## [no-toc] [no-number] TOC

[TOC]

## 为什么要写这篇文章

- **观察者** _(observer)_ 模式和 **中介者** _(mediator)_ 模式
  - [调用流程非常相似](Design-Patterns-Notes-3.md#Decouple-Sender-Receiver)
  - 网上相关资料、代码对两者区别的解释不够清楚
  - 两个设计模式在 **图形界面** _(GUI)_ 编程中，被广泛使用
- 学习的过程是：不知道 -> 知道 -> 能向别人解释清楚

## 基本概念

首先需要知道 [回调函数的基本概念](Callback-Explained.md) 。。

### 观察者 _(observer)_ 模式

- 通过 **订阅-发布** _(subscribe-publish)_ 模型，消除组件之间双向依赖
- 消息的 **发布者** _(subject)_ 不需要知道 **观察者** _(observer)_ 的存在
- 两者只需要约定消息的格式（如何订阅、如何发布），就可以通信
- [笔记链接](Design-Patterns-Notes-3.md#Observer)

### 中介者 _(mediator)_ 模式

- 通过设置 **消息中心** _(message center)_，避免组件之间直接依赖
- 所有的 **协同者** _(colleague)_ 只能通过 **中介者** _(mediator)_ 进行通信，
  而相互之间不知道彼此的存在
- 当各个组件的消息出现循环时，消息中心可以消除组件之间的依赖混乱
- [笔记链接](Design-Patterns-Notes-3.md#Mediator)

### 两者的联系

- 中介者模式 一般通过 观察者模式 实现
  - 协同者 作为 发布者，中介者 作为 观察者
  - 协同者 发布消息 -> 中介者 收到并处理消息 -> 中介者 直接发送消息给 协同者
  - 协同者 不依赖于 中介者
- 当组件之间依赖关系简单时，可以直接使用 观察者模式
- 当组件之间依赖关系复杂是，需要借助 中介者模式 梳理关系

## 需求（HTML）

我们需要实现一个简单的 输入框-选择框 同步的功能：

- 从 输入框 输入：将输入的 文本 同步到 选择框（如果存在对应的 选择项）
- 从 选择框 选择：将选择的 选择项 同步到 输入框

<div style='box-shadow: 0px 2px 5px 0px rgba(0,0,0,0.16), 0px 2px 10px 0px rgba(0,0,0,0.12);padding:8px;margin:20px;text-align:center'>
  <h4>在线演示 - Live Demo</h4>
  <input id="textbox" style="width:80%"
         oninput="document.getElementById('listbox').value =
         document.getElementById('textbox').value"
         value="one" />
  <br />
  <select id="listbox" style="width:80%"
          onchange="document.getElementById('textbox').value =
          document.getElementById('listbox').value">
    <option value="one">one</option>
    <option value="two">two</option>
    <option value="three">three</option>
  </select>
  <p><small>Powered by BOT-Mark, 2017 😘</small></p>
</div>

#### [no-number] 在线演示 HTML 代码

``` html
<input id="textbox"
       oninput="document.getElementById('listbox').value =
       document.getElementById('textbox').value" />
<select id="listbox"
        onchange="document.getElementById('textbox').value =
        document.getElementById('listbox').value">
</select>
```

## 实现（C++）

- 代码使用 C++ 编写，可以通过 [脚本](Observer-Mediator-Explained/run.sh) 运行
- 代码基于一个 ~~假想的~~ 😂 **组件库** _(Widget Library)_ 进行开发
- 代码流程包含两个部分：
  - _Client_ 初始化流程：初始化组件，并设置依赖
  - _Invoker_ 模拟用户行为：模拟用户对组件的操作，并查看效果（自动化测试）
- 如果所有的 _Invoker_ 行为符合预期，通过测试：
  1. 设置 输入框 的文本，检查 选择框 的选择项是否同步
  2. 设置 选择框 的选择项，检查 输入框 的文本是否同步

### ~~假想的~~ 组件库

- 我们使用的组件库包含两个基本组件：输入框 和 选择框
- [代码链接](Observer-Mediator-Explained/widget.h)

> 利用 `using MyItem = std::string;` 定义默认的文本类型为一般的字符类型，并填入模板参数 `Item`。

#### 输入框 `TextBox`

- 设置输入框文本 `SetText`
- 获取输入框文本 `GetText`
- 接收用户行为 `OnInput`
  - 当用户在输入框输入文本时，组件库调用这个虚函数
  - 组件库的使用者重载这个函数，定义组件行为

``` cpp
template<typename Item>
class TextBox {
    Item _item;

public:
    TextBox (const Item &item);

    void SetText (const Item &item);
    const Item &GetText () const;

    // Interface for Invoker
    virtual void OnInput () = 0;
};
```

#### 选择框 `ListBox`

- 设置选择项 `SetSelection`
- 获取选择项 `GetSelection`
- 接收用户行为 `OnChange`
  - 当用户在选择框选择项目时，组件库调用这个虚函数
  - 组件库的使用者重载这个函数，定义组件行为

``` cpp
template<typename Item>
class ListBox {
    std::vector<Item> _items;
    unsigned _index;

public:
    ListBox (const std::vector<Item> &items,
             unsigned index = 0);

    void SetSelection (const Item &item);
    const Item &GetSelection () const;

    // Interface for Invoker
    virtual void OnChange () = 0;
};
```

### 最简单的实现

- 通过自定义组件的方法，重载原始的用户行为，从而实现界面逻辑
- [代码链接](Observer-Mediator-Explained/solution-plain.cpp)

#### 自定义输入框

- 继承于 `TextBox<MyItem>`，额外保存一个 选择框 的引用
- 当用户输入 `OnInput` 时，调用 选择框 的设置函数 `SetSelection`，设置为 输入框 的内容 `GetText`

``` cpp
class MyTextBox : public TextBox<MyItem> {
    std::weak_ptr<MyListBox> _listbox;

public:
    MyTextBox (const MyItem &item);

    void SetListBox (std::weak_ptr<MyListBox> &&p) { _listbox = p; }
    void OnInput () override {
        if (auto p = _listbox.lock ())
            p->SetSelection (this->GetText ());
    }
};
```

#### 自定义选择框

- 继承于 `ListBox<MyItem>`，额外保存一个 输入框 的引用
- 当用户选择 `OnChange` 时，调用 输入框 的设置函数 `SetText`，设置为 选择框 的选项 `GetSelection`

``` cpp
class MyListBox : public ListBox<MyItem> {
    std::weak_ptr<MyTextBox> _textbox;

public:
    MyListBox (const std::vector<MyItem> &items,
               unsigned index = 0);

    void SetTextBox (std::weak_ptr<MyTextBox> &&p) { _textbox = p; }
    void OnChange () override {
        if (auto p = _textbox.lock ())
            p->SetText (this->GetSelection ());
    }
};
```

#### 初始化流程

- 分别构造一个 输入框 `textbox` 和 选择框 `listbox`
- 相互设置为依赖对象

``` cpp
auto textbox = std::make_shared<MyTextBox> (items[0]);
auto listbox = std::make_shared<MyListBox> (items, 0);

textbox->SetListBox (listbox);
listbox->SetTextBox (textbox);
```

#### 相关讨论

- 类似于 [sec|在线演示 HTML 代码] 在线演示的代码，`MyTextBox` 和 `MyListBox`
  - 构成强耦合 —— 两者相互依赖，协同调用（一个类的成员函数内，调用另一个类的成员函数）
  - 不易于复用 —— 硬编码界面逻辑，难以重复利用
- 当界面变得复杂时，不易于维护，例如
  - 新增组件：需要新组件和原有的两个组件分别耦合，界面逻辑变得复杂而且零散
  - 修改行为：如果需要修改个组件的行为，可能涉及到多处代码的改动（没遇到过，目前至少假设。。。）

### 改进 —— 基于 观察者模式 的实现

- 应用观察者模式，将用户行为委托到观察者的回调函数上，消除组件之间双向依赖
- [代码链接](Observer-Mediator-Explained/solution-observer.cpp)

> 在 [sec|~~假想的~~ 组件库] 原有组件库 的基础上，我们封装了一个可观察的组件库 _(Observable Widget Library)_，用于实现观察者模式。

#### 可观察的组件库

- 继承于 `TextBox<Item>`/`ListBox<Item>`，额外保存一个 `Observer` 的引用
- 将原始的用户行为，重定向到 观察者 上：
  - 当用户输入 `OnInput` 时，调用 观察者 的回调函数 `TextUpdated`，设置为 输入框 的内容 `GetText`
  - 当用户选择 `OnChange` 时，调用 观察者 的回调函数 `SelectionChanged`，设置为 选择框 的选项 `GetSelection`
- [代码链接](Observer-Mediator-Explained/observable-widget.h)

``` cpp
template<typename Item>
class ObservableTextBox : public TextBox<Item> {
public:
    ObservableTextBox (const Item &item);

    class Observer {
    public:
        // Interface for Observer
        virtual void TextUpdated (const Item &) = 0;
    };
    void SetObserver (std::weak_ptr<Observer> &&p) { _observer = p; }

    // Interface for Invoker
    void OnInput () override {
        if (auto p = _observer.lock ())
            p->TextUpdated (this->GetText ());
    }

private:
    std::weak_ptr<Observer> _observer;
};

template<typename Item>
class ObservableListBox : public ListBox<Item> {
public:
    ObservableListBox (const std::vector<Item> &items,
                       unsigned index = 0);

    class Observer {
    public:
        // Interface for Observer
        virtual void SelectionChanged (const Item &) = 0;
    };
    void SetObserver (std::weak_ptr<Observer> &&p) { _observer = p; }

    // Interface for Invoker
    void OnChange () override {
        if (auto p = _observer.lock ())
            p->SelectionChanged (this->GetSelection ());
    }

private:
    std::weak_ptr<Observer> _observer;
};
```

#### 相互观察的输入框/选择框

- 定义观察关系：
  - `MyTextBox` 继承于 `ObservableTextBox<MyItem>` 和 `ObservableListBox<MyItem>::Observer`，即 我们的输入框 作为 选择框的观察者
  - `MyListBox` 继承于 `ObservableListBox<MyItem>` 和 `ObservableTextBox<MyItem>::Observer`，即 我们的选择框 作为 输入框的观察者
- 定义观察行为：
  - 输入框 观察到 选择框变化 `SelectionChanged` 时，更新文本 `SetText`
  - 选择框 观察到 输入框输入 `TextUpdated` 时，更新选项 `SetSelection`

``` cpp
class MyTextBox :
    public ObservableTextBox<MyItem>,
    public ObservableListBox<MyItem>::Observer
{
public:
    MyTextBox (const MyItem &item);

    void SelectionChanged (const MyItem &item) override {
        this->SetText (item);
    }
};

class MyListBox :
    public ObservableListBox<MyItem>,
    public ObservableTextBox<MyItem>::Observer
{
public:
    MyListBox (const std::vector<MyItem> &items,
               unsigned index = 0);

    void TextUpdated (const MyItem &item) override {
        this->SetSelection (item);
    }
};
```

#### 初始化流程

- 分别构造一个 输入框 `textbox` 和 选择框 `listbox`
- 相互设置为观察的对象（发布者）

``` cpp
auto textbox = std::make_shared<MyTextBox> (items[0]);
auto listbox = std::make_shared<MyListBox> (items, 0);

textbox->SetObserver (listbox);
listbox->SetObserver (textbox);
```

#### 相关讨论

- 使用观察者模式
  - 接收到用户行为的组件（发布者） 将 组件的用户行为 作为消息，发布到 订阅了这个消息的组件（观察者）上
  - 从而实现了 **界面逻辑的处理** 从 **接收到用户行为的组件** 转移到 **对这个用户行为感兴趣的组件**
  - 而不是由 接收到用户行为的组件 直接处理消息，从而解除了双向的相互依赖（因为接收到消息的一方需要依赖于处理消息的一方）
  - **在这个例子中**，输入框内容 发生变化时，它本身不知道如何处理（因为它不是 选择框，不能更新选择项），而是通知对这个变化改兴趣的 选择框 去处理当前的用户行为（更新选择项）
- 但是，当界面变得复杂时，组件对用户行为的处理逻辑仍然非常零散

### 再改进 —— 基于 中介者模式 的实现

- 应用中介者模式，将用户行为委托到中介者上，避免组件之间直接依赖
- [代码链接](Observer-Mediator-Explained/solution-mediator.cpp)

> 中介者模式基于观察者模式实现，所以这里仍使用之前定义的 [sec|可观察的组件库] 可观察的组件库。

#### 定义消息中心（中介者）

- 定义观察关系：
  - 输入框 `ObservableTextBox<MyItem>` 和 选择框 `ObservableListBox<MyItem>` 不再相互观察，而是作为独立的组件存在
  - 中介者 `Mediator` 继承于 `MyTextBox::Observer` 和 `MyListBox::Observer`，即 作为 输入框、选择框的观察者
- 定义观察行为：
  - 中介者 观察到 输入框输入 `TextUpdated` 时，更新 选择框选项 `SetSelection`
  - 中介者 观察到 选择框变化 `SelectionChanged` 时，更新 输入框文本 `SetText`

``` cpp
using MyTextBox = ObservableTextBox<MyItem>;
using MyListBox = ObservableListBox<MyItem>;

class Mediator :
    public MyTextBox::Observer,
    public MyListBox::Observer
{
    std::shared_ptr<MyTextBox> _textbox;
    std::shared_ptr<MyListBox> _listbox;

public:
    Mediator (std::shared_ptr<MyTextBox> &textbox,
              std::shared_ptr<MyListBox> &listbox);

    void TextUpdated (const MyItem &item) override {
        _listbox->SetSelection (item);
    }

    void SelectionChanged (const MyItem &item) override {
        _textbox->SetText (item);
    }
};
```

#### 初始化流程

- 分别构造 输入框 `textbox`、选择框 `listbox` 和 中介者 `mediator`
- 将 中介者 作为另外两个对象（发布者）的 观察者

``` cpp
auto textbox = std::make_shared<MyTextBox> (items[0]);
auto listbox = std::make_shared<MyListBox> (items, 0);
auto mediator = std::make_shared<Mediator> (textbox, listbox);

textbox->SetObserver (mediator);
listbox->SetObserver (mediator);
```

#### 相关讨论

- 使用中介者模式
  - 化简了观察关系：所有组件只能和 中介者 通信，组件之间没有消息传递
  - 化简了观察行为：原本零散的消息传递关系，集中于 中介者 内部实现
- 相对于零散的观察者
  - 把 **组件之间消息的耦合** 转化为 **中介者的内聚**
  - 从而实现了 高内聚、低耦合
- 使用分散的观察者模式，可能出现调用顺序不确定的问题 —— 处理逻辑依赖于通知顺序，通知顺序依赖于注册顺序，而注册顺序可能隐式的被修改（参考：[令人抓狂的观察者模式 之 顺序耦合](../2019/Insane-Observer-Pattern.md#问题-顺序耦合)）

### 另一种基于 中介者模式 的实现

- 中介者 作为消息中心，保存了对所有组件的引用（依赖于所有的组件），从而对所有的组件进行协调
- 所以，我们可以使用另一种 观察者模式 的实现 —— 基于 **拉取模型** _(pull model)_
- [代码链接](Observer-Mediator-Explained/solution-mediator-pull.cpp)

#### 基于拉取的可观察组件库

- 和 [sec|可观察的组件库] **推送模型** _(push model)_ 不同，`Observer` 的接口没有参数
  - 发布者 仅仅告知有消息到达，而不告知消息的内容
  - 观察者 不能直接从接收到的消息获取内容（进一步通过拉取的方式获取消息）
- 这就类似于
  - 推送模型：手机（发布者）收到消息时，消息提示音响起，用户（观察者）能在锁屏界面上看到消息的内容
  - 拉取模型：朋友圈（发布者）更新时，只会显示一个小红点，用户（观察者）需要点进去才能看到更新
- [代码链接](Observer-Mediator-Explained/observable-widget-pull.h)

``` cpp
class ObservableTextBox::Observer {
public:
    // Interface for Observer
    virtual void TextUpdated () = 0;  // omitting param 'const Item &'
};

void ObservableTextBox::OnInput () {
    if (auto p = _observer.lock ())
        p->TextUpdated ();     // omitting argument 'this->GetText ()'
}
```

#### 重新定义消息中心（中介者）

- 当 观察者 收到消息时，中介者 通过被观察对象（发布者）获取需要的内容，而不是直接从推送的消息中获取
- 由于 中介者 保存了对所有组件的引用（依赖于所有的组件），可以方便的直接获取需要的内容
- 而对于 没有中介者 的设计，保存交叉引用会导致代码变得混乱，进而退化为 [sec|最简单的实现] 最开始讨论的形式

``` cpp
void Mediator::TextUpdated () {
    _listbox->SetSelection (_textbox->GetText ());
}

void Mediator::SelectionChanged () {
    _textbox->SetText (_listbox->GetSelection ());
}
```

## [no-number] 写在最后

> 延伸阅读：[回调 vs 接口](Callback-vs-Interface.md) —— 如何进一步优化观察者模式的实现
>
> 在实践中，观察者可以直接通过 **可调用对象** _(callable object)_ 实现，而不需要引入额外的 **类层次结构** _(class hierarchy)_：
>
> - 回调函数 **接口** _(interface)_ 可以改造为 回调函数 **对象** _(object)_
>   - `void ObservableTextBox::Observer::TextUpdated(const Item &);` ->
>   - `using ObservableTextBox::TextUpdatedObserver = std::function<void(const Item &)>;`
> - **重载** _(override)_ 回调函数接口 可以改造为 **注入** _(inject)_ 回调函数对象
>   - `void ObservableTextBox::SetObserver (std::weak_ptr<Observer> &&observer);` + `void Mediator::TextUpdated(const MyItem &item) override;` ->
>   - `void ObservableTextBox::SetObserver(TextUpdatedObserver observer);`

本文仅是我对设计模式的一些理解。如果有什么问题，望**不吝赐教**。😄

感谢 [@flythief](https://github.com/thiefuniverse) 提出的修改意见~

> 相关文章：设计模式笔记
>
> - [Creational Patterns](Design-Patterns-Notes-1.md)
> - [Structural Patterns](Design-Patterns-Notes-2.md)
> - [Behavioral Patterns](Design-Patterns-Notes-3.md)

Delivered under MIT License &copy; 2017, BOT Man
