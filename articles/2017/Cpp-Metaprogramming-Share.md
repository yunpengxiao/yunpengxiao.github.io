# 浅谈 C++ 元编程

> BOT Man, 2017/10/13
>
> 分享 C++ 元编程中的 基本原理及应用

[slide-mode]

---

## 主要内容

- [sec|基本概念]
- [sec|历史背景]
- [sec|语言支持]
- [sec|演算规则]
- [sec|应用和难点]

---

## 基本概念

👉 **元编程** _(metaprogramming)_ 通过操作 **程序实体** _(program entity)_，在 **编译时** _(compile time)_ 计算出 **运行时** _(runtime)_ 需要的常数、类型、代码的方法。

- 一般编程：编写 **程序** _(program)_，编译器 **编译** _(compile)_ 到目标代码，
  **运行时** 执行
- 元编程：**模板** _(template)_ 机制，编译器 **推导** _(deduce)_，
  **编译时** 生成程序

**两级编程** _(two-level programming)_ /
**生成式编程** _(generative programming)_ /
**模板元编程** (template metaprogramming)

---

## 基本概念

👉 C++ 语言 = C 语言的超集 + **抽象机制** _(abstraction mechanisms)_ + 标准库

👉 抽象机制 = **面向对象编程** _(object-oriented programming)_ +
**模板编程** _(template programming)_

- **类** _(class)_ -> 已有 **类型** _(type)_ 构造出新的类型
- **模板** _(template)_ -> 表示 **通用概念** _(general concept)_

---

## 基本概念

👉 模板编程 = **泛型编程** _(generic programming)_ + **元编程** _(meta-programming)_

- **通用概念** 的抽象，**类型** 或 **算法** _(algorithm)_，不关心编译器
- 模板推导时的 **选择** _(selection)_ 和 **迭代** _(iteration)_

---

## 历史背景

- 1988，David R. Musser, Alexander A. Stepanov **模板**
- 1988+，Alexander A. Stepanov, Bjarne Stroustrup **C++ 标准模板库**
  _(C++ Standard Template Library, C++ STL)_
- 1994，Erwin Unruh **编译错误信息** 计算素数
- 1995，Todd Veldhuizen 首次提出 **模板元编程**，数值计算
- 2001，Andrei Alexandrescu 通用的模板元编程，**Loki**
- 2007，Andrei Alexandrescu 发明 **D 语言**

---

## 语言支持

模板分类：

- **类型构造器** _(type constructor)_
  - **类模板** _(class template)_：**类型** 的抽象
  - **函数模板** _(function template)_：**算法** 的抽象
- **语法糖** _(syntactic sugar)_
  - **别名模板** _(alias template)_（C++ 11）：**类型别名** _(type alias)_ 的简记
  - **变量模板** _(variable template)_（C++ 14）：**常量** _(constant)_ 的简记

---

## 语言支持

模板参数：

- 值参数，类型参数，模板参数
- **变长模板** _(variadic template)_（C++ 11）-> `std::tuple`

推导规则：

- **特化** _(specialization)_ ~ 函数的 **重载** _(overload)_
- **实例化** _(instantiation)_ ~ 函数的 **绑定** _(binding)_

---

## 演算规则

- **编译时测试** _(compile-time test)_ ~ **选择语句** _(selection statement)_
  - 测试 常量表达式
  - 测试 类型
    - 是否为特定的类型 -> 特化
    - 是否满足某些条件 -> `type_traits` + SFINAE => 常量表达式
- **编译时迭代** _(compile-time iteration)_ ~ **循环语句** _(loop statement)_

---

## 演算规则 —— 测试表达式

``` cpp
template <unsigned Val> struct _isZero {
    constexpr static bool value = false;
};

template <> struct _isZero <0> {
    constexpr static bool value = true;
};

template <unsigned Val>
constexpr bool isZero = _isZero<Val>::value;

// Client

static_assert (!isZero<1>, "compile error");
static_assert (isZero<0>, "compile error");
```

- **测试类型 是否为特定的类型** 类似
- 模板参数换为 `typename` / `class`

---

## 演算规则 —— 测试类型 满足条件

``` cpp
template <typename T>
constexpr bool isNum = std::is_arithmetic<T>::value;

template <typename T>
constexpr bool isStr = std::is_same<T, const char *>::value;

template <typename T>
constexpr bool isBad = !isNum<T> && !isStr<T>;

template <typename T>
std::enable_if_t<isNum<T>, std::string> ToString (T num) {
    return std::to_string (num);
}

template <typename T>
std::enable_if_t<isStr<T>, std::string> ToString (T str) {
    return std::string (str);
}

template <typename T>
std::enable_if_t<isBad<T>, std::string> ToString (T bad) {
    static_assert (!isBad<T>, "bad type :-(");
}







// Client

auto a = ToString (1);  // std::to_string (num);
auto b = ToString (1.0);  // std::to_string (num);
auto c = ToString ("0x0");  // std::string (str);
auto d = ToString (std::string {});  // bad type :-(
```

---

## 演算规则 —— 测试类型 反例

``` cpp
template <typename T>
std::string ToString (T val) {
    if (isNum<T>) return std::to_string (val);
    else if (isStr<T>) return std::string (val);
    else static_assert (!isBad<T>, "bad type :-(");
}
```

---

## 演算规则 —— 定长模板的迭代

``` cpp
template <unsigned N>
constexpr unsigned _Factor () { return N * _Factor<N - 1> (); }

template <>
constexpr unsigned _Factor<0> () { return 1; }

template <unsigned N>
constexpr unsigned Factor = _Factor<N> ();


// Client

static_assert (Factor<0> == 1, "compile error");
static_assert (Factor<1> == 1, "compile error");
static_assert (Factor<4> == 24, "compile error");
```

---

## 演算规则 —— 变长模板的迭代

``` cpp
template <typename T>
constexpr auto Sum () {
    return T (0);
}

template <typename T, typename... Ts>
constexpr auto Sum (T arg, Ts... args) {
    return arg + Sum<T> (args...);
}


// Client

static_assert (Sum (1) == 1, "compile error");
static_assert (Sum (1, 2, 3) == 6, "compile error");
```

---

## 元编程的威力 —— ORM

``` cpp
auto oldUsers = mapper.Query (User {})
    .Where (
        field (user.age) >= 64 &&
        field (user.name) != nullptr
    )
    .OrderBy (field (user.age))
    .ToVector ();
```

- 映射器 `mapper` 生成、执行 SQL 语句，填充 C++ 对象
- 返回值 `oldUsers` 类型推导为 `std::vector<User>`

---

## 元编程的威力 —— ORM

``` cpp
struct User {
    std::optional<std::string> name;
    int age;
    ORMAP ("UserTable", age, name);
};

SELECT * FROM UserTable
WHERE (age >= 64 AND name IS NOT NULL)
ORDER BY age
```

> See [ORM-Lite](https://github.com/BOT-Man-JL/ORM-Lite) by _BOT-Man-JL_

---

## 应用和难点

#### 应用

- **数值计算** _(numeric computation)_
- **类型推导** _(type deduction)_ -> 推到 DSL 类型
- **代码生成** _(code generation)_ -> 生成非 C++ 代码

#### 难点

- 代码膨胀 -> 产生过多实例
- 实例化错误 -> 静态定位
- 调试模板 -> 动态定位

---

## Q & A

<br/>
<br/>

[align-center]

👉 原文：[浅谈 C++ 元编程](Cpp-Metaprogramming.md)

<br/>

[align-right]

# 谢谢 🙂

---
