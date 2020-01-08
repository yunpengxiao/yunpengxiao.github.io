# 深入浅出 C++ 11 右值引用

> 2018/7/15 -> 2019/11/30
>
> 彻底搞清楚：右值引用/移动语义/拷贝省略/通用引用/完美转发

[heading-numbering]

## [no-toc] [no-number] 目录

[TOC]

## 写在前面

> 如果你还不知道 C++ 11 引入的右值引用是什么，可以读读这篇文章，看看有什么 **启发**；如果你已经对右值引用了如指掌，也可以读读这篇文章，看看有什么 **补充**。欢迎交流~ 😉

尽管 **C++ 17 标准** 在去年底已经正式发布了，但由于 C++ 语言变得 **越来越复杂**，让许多人对很多新特性 **望而却步**。

对于 2011 年发布的 C++ 11 标准，很多人虽然对 **右值引用/通用引用/移动语义/完美转发/拷贝省略** 之类的概念都 **有所耳闻**，却没有详细了解其 **设计初衷/实现原理**，甚至对一些细节 **有所误解**（包括我 🙃）。

我刚开始学习 C++ 的时候，也常常 **混淆** 这几个 **概念**。但随着深入了解、与人探讨，才逐步理清楚这几个概念的来龙去脉。先分享几个我曾经的犯过的错。

### 误解：返回前，移动临时值

> [ES.56: Write `std::move()` only when you need to explicitly move an object to another scope](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Res-move)

``` cpp
std::string base_url  = tag->GetBaseUrl();
if (!base_url.empty())
  UpdateQueryUrl(std::move(base_url) + "&q=" + word_);
else
  UpdateQueryUrl(default_base_url_ + "&q=" + word_);
// ...
// maybe use |base_url| later...
```

上述代码的问题在于：对临时值使用 `std::move`，会导致后续代码不能使用 `base_url`；如果使用，会出现 **未定义行为** _(undefined behavior)_。（参考：[`std::basic_string(basic_string&&)`](https://en.cppreference.com/w/cpp/string/basic_string/basic_string)）

> 如何检查：
> 
> - 在运行时，在移动构造函数中，将被移动的值设置为无效状态，并在使用前检查
> - 在编译时，使用 Clang 标记对移动语义进行静态检查（参考：[Consumed Annotation Checking | Attributes in Clang](https://clang.llvm.org/docs/AttributeReference.html#consumed-annotation-checking)）

### 误解：被移动的值不能再使用

> [C.64: A move operation should move and leave its source in a valid state](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-move-semantic)

``` cpp
auto p = std::make_unique<int>(1);
auto q = std::move(p);
assert(p == nullptr);  // OK: use after move

p.reset(new int{2});   // or p = std::make_unique<int>(2);
assert(*p == 1);       // OK: realive now
```

很多人会认为：被移动后的值会进入一个 **非法状态** _(invalid state)_，从而不能再使用。例如，C++ 标准要求对象 **遵守移动语义**（参考 [sec|移动语义]）—— 被移动后，进入一个 **合法但未指定状态** _(“valid but unspecified” state)_ —— 即调用该对象的方法不会出现异常。处于这个状态的对象：

- （基本要求）能正确析构（不会重复释放已经被移动了的资源，例如 `unique_ptr::~unique_ptr()` 检查指针是否需要删除）
- （一般要求）重新赋值后，和新的对象没有差别（C++ 标准库基于这个假设）
- （更高要求）恢复为默认值（例如 `unique_ptr` 恢复为 `nullptr`）

关于移动语义的实现原理，参考 [sec|移动语义] 代码。

还有一个很明显的 **反例**：基本类型的变量（例如 `int i = 42;`）被移动后，一般保持为原有值（`i == 42`）。

### 误解：返回时，移动临时值

> [F.48: Don’t `return std::move(local)`](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rf-return-move-local)

``` cpp
std::unique_ptr<int> foo() {
  auto ret = std::make_unique<int>(64);
  //...
  return std::move(ret);  // -> return ret;
}
```

上述代码的问题在于：没必要使用 `std::move` 移动返回的临时值。

和传入的引用参数不一样，C++ 会把 **临时值** 当成 **右值** 返回，通过右值引用进行 [sec|移动语义] 移动构造（语言标准）；如果编译器允许 [sec|拷贝省略] 拷贝省略，还可以省略这一步的构造，直接把 `ret` 存放到返回值的内存里（编译器优化）。

> Never apply `std::move` or `std::forward` to local objects if they would otherwise be eligible for the return value optimization. —— Meyer Scott, _Effective Modern C++_

误用 `std::move()` 会 **阻止** 编译器的拷贝省略 **优化**。不过聪明的 Clang 会提示 `-Wpessimizing-move` 警告。

### 误解：不移动右值引用参数

> [F.18: For “will-move-from” parameters, pass by `X&&` and `std::move` the parameter](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rf-consume)

``` cpp
std::unique_ptr<int> bar(std::unique_ptr<int>&& val) {
  //...
  return val;    // not compile
                 // -> return std::move/forward(val);
}
```

上述代码的问题在于：没有对返回值使用 `std::move`，提示 `unique_ptr(const unique_ptr&) = delete` 不能编译。

> [If-it-has-a-name Rule](http://thbecker.net/articles/rvalue_references/section_05.html#no_name):
> 
> - Named rvalue references are lvalues.
> - Unnamed rvalue references are rvalues.

因为不论 **左值引用** 还是 **右值引用** 的变量（或参数），进入了作用域（函数体），经过初始化后，都会 **“退化” 为左值** —— 因为在作用域内，该变量可以被取地址、被赋值 —— **命名的右值引用变量** _(named rvalue reference variable)_ 是 **左值**，但该变量的 **类型** 却是 **右值引用**（[参考 StackOverflow 回答](https://stackoverflow.com/questions/28483250/rvalue-reference-is-treated-as-an-lvalue/28483352#28483352)）。

所以，返回右值引用变量时，需要使用 `std::move`/`std::forward` 显式的 **强制移动** 或 **完美转发**，将变量的类型 “还原” 为右值引用。

在 [sec|完美转发] 完美转发部分，详细描述了 `std::forward` 如何保持 **引用变量** 原本的 **左右值类型**。

### 误解：对象需要定义移动构造函数，才能实现移动语义

> [C.20: If you can avoid defining default operations, do](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-zero)
> 
> [C.21: If you define or `=delete` any default operation, define or `=delete` them all](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-five)

实际上，多数情况下：

- 如果 **没有定义** 构造/析构函数，编译器会 **自动生成** 移动构造/赋值函数（[rule of zero](https://en.cppreference.com/w/cpp/language/rule_of_three#Rule_of_zero)）
- 如果 **需要定义** 构造/析构函数，**尽量使用** `= default` 生成 移动构造/赋值函数（[rule of five](https://en.cppreference.com/w/cpp/language/rule_of_three#Rule_of_five)）

注意：如果定义了 构造/析构函数，但又忘记定义了 移动构造/赋值函数，会导致对象 **不可移动**。

## 基础知识

> 如果已经了解了 初始化/构造/左右值/引用 基本概念，可以直接跳过这个部分。

之所以会出现上边的误解，往往是因为 C++ 语言的复杂性 和 使用者对基础知识的掌握程度 不匹配。所以，这里有必要和大家缕一缕 变量和值的关系 和 各种引用的关系。

### 变量的初始化 vs 值的构造

C++ 要求每个 **变量** _(variable)_ 在编译时确定其 [**类型** _(type)_](https://en.cppreference.com/w/cpp/language/type)：基本类型/引用类型/指针类型/类的成员指针类型/数组类型/函数类型/枚举类型/类-联合类型、及上述类型对应的 `const/volatile` 类型。

变量的 [**初始化** _(initialization)_](https://en.cppreference.com/w/cpp/language/initialization) 是指 在变量创建时，为其提供 **初始值** _(initial value)_，即 把初始值绑定到变量上。对于不同类型的变量，有着不同的初始化方法。

而 [**引用类型** 变量的初始化](https://en.cppreference.com/w/cpp/language/reference_initialization)（包括：一般变量/类的成员变量/函数参数/函数返回值）和其他值类型变量的初始化 不太一样：

- 创建时，**必须初始化**（和指针不同，**不允许空引用**；但也存在 **悬垂引用**）
- 相当于是 其引用的值 的一个 **别名** _(alias)_（例如，对引用变量的 **赋值运算** _(assignment operation)_ 会赋值到 其引用的值 上）
- 一旦绑定了初始值，就 **不能重新绑定** 到其他值上了（和指针不同，赋值运算不能修改引用的指向；而对于 Java/JavaScript 等语言，对引用变量赋值，可以 **重新绑定** 引用的指向）

所以，并不是所有变量的初始化，都会导致值的 **构造** _(construction)_ —— 引用的初始化只会创建 **已有值的别名**。

### 左值引用 vs 右值引用 vs 常引用

C++ 11 强化了 [**值类别** _(value category)_](https://en.cppreference.com/w/cpp/language/value_category)：**左值** _(lvalue, left value)_ 指的是可以通过变量名/指针/引用使用的值，**右值** _(rvalue, right value)_ 指的是表达式返回的临时值。

区分左右值的 “不完全正确” 的方法：

- 左值可以 **取地址**，而右值不行
- 可以给左值 **赋值**，但右值不行

C++ 11 进一步引入了 [左右值 **引用** _(reference)_](https://en.cppreference.com/w/cpp/language/reference) 的概念：**左值引用** _(l-ref, lvalue reference)_ 是指通过 `&` 符号引用左值；**右值引用** _(r-ref, rvalue reference)_ 是指通过 `&&` 符号引用右值。

注意：一旦 **右值引用类型的变量** 被初始化后，就会在作用域内 **“退化” 为左值引用**（参考：[sec|误解：不移动右值引用参数]）。

``` cpp
void f(Data&  data);  // 1, data is l-ref
void f(Data&& data);  // 2, data is r-ref

Data   data;
Data&  data1 = data;
Data&& data2 = data;    // not compile: invalid binding
Data&& data2 = Data{};  // OK

f(data);    // 1, data is lvalue
f(Data{});  // 2, data is rvalue
f(data1);   // 1, data1 is l-ref
f(data2);   // 1, data2 degenerates to l-ref
```

另外，C++ 还支持了 **常引用** _(c-ref, const reference)_，**同时接受** 左值/右值 进行初始化：

``` cpp
void g(const Data& data);  // data is c-ref

g(data);    // ok, data is lvalue
g(Data{});  // ok, data is rvalue
```

C++ 11 之前已有的常引用 和 C++ 11 新引入的右值引用 都能接受右值的绑定，有什么区别呢？

- 对于通过 右值 初始化 右值引用/常引用，都可以将 右值 [**生命周期扩展** _(lifetime extension)_](https://en.cppreference.com/w/cpp/language/reference_initialization#Lifetime_of_a_temporary) 到 绑定该右值的 引用的生命周期
- 对于 绑定了右值的 右值引用，**可以修改** 引用的右值（参考：[sec|误解：不移动右值引用参数]）；而对于 绑定了右值的 常引用，不能修改引用的右值
- 对于同时接受 右值引用/常引用 参数的函数，编译器 **优先重载** 右值引用 的重载

``` cpp
const Data& data1 = Data{};   // OK: extend lifetime
data1.modify();               // not compile: const

Data&& data2 = Data{};        // OK: extend lifetime
data2.modify();               // OK: non-const

void f(const Data& data);  // 1, data is c-ref
void f(Data&& data);       // 2, data is r-ref

f(Data{});  // 2, prefer 2 over 1 for rvalue
```

## 移动语义

在 C++ 11 强化左右值概念后，我们可以针对右值进行优化。于是，C++ 11 中就提出了 **移动语义** _(move semantic)_（不考虑不包含资源的基本类型，例如 `int/double`，因为他们移动的代价和拷贝的相同）：

- 由于右值对象一般是临时对象，在移动时，对象包含的资源 **不需要先复制再删除**，只需要直接 **从旧对象移动到新对象**
- 要求被移动的对象，**能正确析构**（参考 [sec|误解：被移动的值不能再使用]）

在 C++ 提出移动语义之前，编译器往往可以针对需要移动的对象进行 [sec|拷贝省略] 拷贝省略的优化。然而，在 C++ 17 之前，拷贝省略只是编译器可选的优化项，不保证实现与否。

### 避免先复制再释放资源

针对一些包含了资源的对象，我们可以通过移动对象的资源进行优化。例如，我们常用的 STL 类模板都有：

- 以常引用作为参数的 **拷贝构造函数** _(copy constructor)_
- 以右值引用作为参数的 **移动构造函数** _(move constructor)_

``` cpp
template<typename T> class vector {
 public:
  vector(const vector& rhs); // copy data
  vector(vector&& rhs);      // move data
  ~vector();                 // dtor
 private:
  T* data_ = nullptr;
  size_t size_ = 0;
};

vector::vector(const vector& rhs) {
  auto &lhs = *this;
  lhs.size_ = rhs.size_;
  lhs.data_ = new T[rhs.size_];
  memcpy(lhs.data_, rhs.data, rhs.size_);  // copy data
}

vector::vector(vector&& rhs) {
  auto &lhs = *this;
  lhs.size_ = rhs.size_;
  lhs.data_ = rhs.data_;  // move data
  rhs.size_ = 0;
  rhs.data_ = nullptr;    // set data of rhs to null
}

vector::~vector() {
  if (data_)              // release only if owned
    delete[] data_;
}
```

上述代码中，构造函数 `vector::vector()` 根据参数的左右值类型判断：

- 参数为左值时，拷贝构造，使用 `memcpy` 拷贝原对象的内存
- 参数为右值时，移动构造，把指向原对象内存的指针 `data_`、内存大小 `size_` 复制到新对象，并把原对象这两个成员置 `0`

而析构函数 `vector::~vector()` 检查 data_ 是否有效，决定是否需要释放资源。

除了判断参数是否为左右值，我们还可以判断对象本身是否为左右值。可以给成员函数加上 **引用限定符** _(reference qualifier)_，针对对象本身的左右值类型进行优化。

``` cpp
class Foo {
 public:
  Data& data() & { return data_; }        // lvalue, ref
  Data data() && { return move(data_); }  // rvalue, move
};

auto ret1 = foo.data();    // foo is lvalue, ref and copy
auto ret2 = Foo{}.data();  // foo is rvalue, move directly
```

### 转移不可复制的资源

在之前写的 [资源管理小记](Resource-Management.md#资源和对象的映射关系) 提到：如果资源是不可复制的，那么装载资源的对象也应该是不可复制的。

如果资源对象不可复制，那么它就只能通过移动，创建新对象。例如，智能指针 `std::unique_ptr` 只允许移动构造，不允许拷贝构造。

``` cpp
template<typename T> class unique_ptr {
 public:
  unique_ptr(const unique_ptr& rhs) = delete;
  unique_ptr(unique_ptr&& rhs);  // move only
 private:
  T* data_ = nullptr;
};

unique_ptr::unique_ptr(unique_ptr&& rhs) {
  auto &lhs = *this;
  lhs.data_ = rhs.data_;
  rhs.data_ = nullptr;
}
```

上述代码中，`unique_ptr` 的移动构造过程和 `vector` 类似：

- 把指向原对象内存的指针 `data_` 复制到新对象
- 把原对象的指针 `data_` 置为空

### 反例：不遵守移动语义

移动语义只是语言上的一个 **概念**，具体是否移动对象的资源、如何移动对象的资源，都需要通过编写代码 **实现**。而移动语义常常被 **误认为**，编译器 **自动生成** 移动对象本身的代码（[sec|拷贝省略] 拷贝省略）。

为了证明这一点，我们可以实现不遵守移动语义的 `bad_vector::bad_vector(bad_vector&& rhs)`，执行拷贝语义：

``` cpp
bad_vector::bad_vector(bad_vector&& rhs) {
  auto &lhs = *this;
  lhs.size_ = rhs.size_;
  lhs.data_ = new T[rhs.size_];
  memcpy(lhs.data_, rhs.data, rhs.size_);  // copy data
}
```

那么，一个 `bad_vector` 对象在被 `move` 移动后仍然可用：

``` cpp
bad_vector<int> v1 { 0, 1, 2, 3 };
auto v2 = std::move(v1);

v1[0] = v2[3];           // ok, but a bit odd
assert(v1[0] != v1[0]);
assert(v1[0] == v1[3]);
```

虽然代码可以那么写，但是在语义上有问题，还违背了移动语义的初衷。

### 拷贝省略

尽管 C++ 引入了移动语义，移动的过程 仍然优化的空间 —— 与其调用一次没有太多意义的移动构造函数，不如让编译器 直接跳过这个过程 —— 于是就有了 [拷贝省略 _(copy elision)_](https://en.cppreference.com/w/cpp/language/copy_elision)。

很多人会把移动语义和拷贝省略 **混淆**：

- 移动语义是 **语言标准** 提出的概念，通过编写遵守移动语义的移动构造函数、右值限定成员函数，**逻辑上** 实现优化 **对象内资源** 的转移流程
- 拷贝省略是非标准的 **编译器优化**（C++ 17 前），跳过移动/拷贝构造函数，让编译器直接使用 **移动后的对象内存** 构造 **被移动的对象内存**（例如 [sec|误解：返回时，移动临时值] 中，直接在 函数返回值对象 的内存上，构造 函数局部对象 `ret` —— 在 **不同作用域** 里，共享 **同一块内存**）

C++ 17 进一步细化了 **值类别**，并要求编译器对 **纯右值** _(prvalue, pure rvalue)_ 进行拷贝省略优化。（[参考](https://jonasdevlieghere.com/guaranteed-copy-elision/)）

``` cpp
Data f() {
  Data val;
  // ...
  throw val;
  // ...
  return val;

  // NRVO from lvalue to ret (not guaranteed)
  // if NRVO is disabled, move ctor is called
}
 
void g(Date arg);

Data v = f();     // copy elision from prvalue (C++ 17)
g(f());           // copy elision from prvalue (C++ 17)
```

构造左值、构造参数时，传入的纯右值可以确保被优化 —— Return Value Optimization _(RVO)_；而返回的 **将亡值** _(xvalue, eXpiring value)_ 不保证被优化 —— Named Return Value Optimization _(NRVO)_。

## 通用引用

> 如果对模板编程不感兴趣，可以直接跳过这个部分。

### 概念

> Item 24: Distinguish universal references from rvalue references. —— Meyer Scott, _Effective Modern C++_

[Meyer Scott 指出](https://isocpp.org/blog/2012/11/universal-references-in-c11-scott-meyers)：有时候符号 `&&` 并不一定代表右值引用，它也可能是左值引用 —— 如果一个引用符号需要通过 左右值类型推导（模板参数类型 或 `auto` 推导），那么这个符号可能是左值引用或右值引用 —— 这叫做 **通用引用** _(universal reference)_。

``` cpp
// rvalue ref: no type deduction
void f1(Widget&& param1);
Widget&& var1 = Widget();
template<typename T> void f2(vector<T>&& param2);

// universal ref: type deduction
auto&& var2 = var1;
template<typename T> void f3(T&& param);
```

上述代码中，前三个 `&&` 符号不涉及引用符号的左右值类型推导，是右值引用；而后两个 `&&` 符号会根据初始值推导左右值类型：

- 对于 `var2`：因为 `var1` 已经 “退化” 为左值引用，所以 `var2` 也是左值引用（参考：[sec|误解：不移动右值引用参数]）
- 对于 `T&&`，如果 `param` 传入右值引用，`T&&` 是右值引用 `&&`；如果传入左值引用，`T&&` 也是左值引用 `&`

### 变长引用参数模板

C++ 11 引入了变长模板的概念，允许向模板参数里传入不同类型的不定长引用。由于每个类型可能是左值引用或右值引用，针对所有可能的左右值引用组合，**特化所有模板** 是 **不现实的**。

**假设没有** 通用引用的概念（即所有的 `&&` 均表示右值引用），模板 `std::make_unique` 至少需要两个重载：

- 对于传入的左值引用 `const Args& ...args`，只要展开 `args...` 就可以转发这一组左值引用
- 对于传入的右值引用 `Args&& ...args`，需要通过 `std::move` 转发出去，即 `std::move<Args>(args)...`

> 为什么要用 `std::move` 转发 —— 参考：[sec|误解：不移动右值引用参数]。

``` cpp
template<typename T, typename... Args>
unique_ptr<T> make_unique(const Args& ...args) {
  return unique_ptr<T> {
    new T { args... }
  };
}

template<typename T, typename... Args>
unique_ptr<T> make_unique(Args&& ...args) {
  return unique_ptr<T> {
    new T { std::move<Args>(args)... }
  };
}
```

上述代码存在一个问题：如果传入的 `...args` **既有** 左值引用 **又有** 右值引用，那么这两个模板都 **无法匹配**。

所以，**基于通用引用**，模板 `std::make_unique` 只需要一个重载：

``` cpp
template<typename T, typename... Args>
unique_ptr<T> make_unique(Args&& ...args) {
  return unique_ptr<T> {
    new T { std::forward<Args>(args)... }
  };
}
```

然而，我们还需要通过 `std::forward` 实现 **针对不同左右值类型的转发** —— 完美转发。

### 完美转发

什么是 **完美转发** _(perfect forwarding)_：

- 如果参数是 **左值引用**，直接以 **左值引用** 的形式（不使用 `std::move`），转发给下一个函数
- 如果参数是 **右值引用**，要先 “还原” 为 **右值引用** 的形式（使用 `std::move`），再转发给下一个函数

因此，[`std::forward`](https://en.cppreference.com/w/cpp/utility/forward) 通过模板重载，定义两个 **不涉及** 左右值类型 **推导** 的重载：

- 对 **左值引用参数** 可以根据需要返回 **左值引用类型** 或 **右值引用类型**
- 对 **右值引用参数** 仅返回 **右值引用类型**（断言：**不能** 返回 **左值引用类型**）

``` cpp
template <typename T>
T&& forward(std::remove_reference_t<T>& val) {
  // forward lvalue as either lvalue or rvalue
  return (static_cast<T&&>(val));
}

template <typename T>
T&& forward(std::remove_reference_t<T>&& val) {
  // forward rvalue as rvalue
  static_assert(!std::is_lvalue_reference_v<T>,
                "Cannot forward rvalue as lvalue.");
  return (static_cast<T&&>(val));
}
```

这是如何工作的呢？

- 不论引用类型的变量，经过初始化后，**变量** 本身都是 左值，但其 **类型** 仍然保留为 左值引用类型 或 右值引用类型（参考：[sec|误解：不移动右值引用参数]）
- `std::forward` 的形参 `val` 会根据 **实参的类型** 选择重载
- `std::forward` 的返回值 `static_cast<T&&>(val)` 经过 [**引用折叠** _(reference collapsing)_](https://en.cppreference.com/w/cpp/language/reference#Reference_collapsing) “还原” **实参的左右值类型**

## 写在最后 [no-number]

虽然这些东西你不知道，也不会伤害你；但如果你知道了，就可以合理利用，从而提升 C++ 的使用效率，避免不必要的问题。

感谢 [@flythief](https://github.com/thiefuniverse) 提出的修改意见~

如果有什么问题，**欢迎交流**。😄

Delivered under MIT License &copy; 2018, BOT Man
