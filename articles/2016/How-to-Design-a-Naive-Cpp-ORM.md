# 如何设计一个简单的 C++ ORM

> 2016/11/15
>
> “没有好的接口，用C++读写数据库和写图形界面一样痛苦”

阅读这篇文章前，你最好知道什么是
[_Object Relation Mapping (ORM)_](https://en.wikipedia.org/wiki/Object-relational_mapping)

关于这个设计的代码和样例 😊：
https://github.com/BOT-Man-JL/ORM-Lite/tree/v1.0

阅读这篇文章后，欢迎阅读下一篇 [如何设计一个更好的 C++ ORM](How-to-Design-a-Better-Cpp-ORM.md)
😉

> **WARNING：早期文章不足之处较多，推荐阅读**：
> 
> - [浅谈 C++ 元编程](../2017/Cpp-Metaprogramming.md)
> - [简单的 C++ 结构体字段 反射](../2018/Cpp-Struct-Field-Reflection.md)

## [no-toc] TOC

[TOC]

## 为什么C++要ORM

> As good object-oriented developers got tired of this repetitive work,
> their typical tendency towards enlightened laziness started
> to manifest itself in the creation of tools to help automate
> the process.
>
> When working with relational databases,
> the culmination of such efforts were object/relational mapping tools. 

- 一般的C++数据库接口，都需要**手动**生成SQL语句；
- 手动生成的查询字符串，常常会因为**模型改动**而失效；
- 查询语句/结果和C++**原生数据**之间的转换，每次都要手动解析；

#### 我为什么要写ORM

C++大作业需要实现一个在线的对战游戏，其中的游戏信息需要保存到数据库里；

而我最初始的里没有使用 ORM 导致生成 SQL 语句的代码占了好大一个部分；
并且这一大堆代码里的小错误往往很难被发现；

每次修改游戏里怪物的模型都需要同步修改这些代码；
然而在修改的过程中经常因为疏漏而出现小错误；

所以，我打算让代码生成这段代码； 😇

## 市场上的C++ ORM

大致可以分成这几类：

- 使用 **预编译器** 生成**模型**和操作:
  - [ODB](http://www.codesynthesis.com/products/odb)
  - [LiteSQL](https://sourceforge.net/projects/litesql)
- 使用 **宏** 生成**模型**和操作：
  - [sqlpp11](https://github.com/rbock/sqlpp11)
- 需要在定义**模型**时，通过**手动**插入 **代码** 进行注入：
  - [Hiberlite ORM](https://github.com/paulftw/hiberlite)
  - [Open Object Store](https://github.com/zussel/oos)
  - [Wt::Dbo](https://www.webtoolkit.eu/wt/doc/tutorial/dbo/tutorial.html)
  - [QxORM](https://www.qxorm.com)(Qt风格的庞大。。)

以上的方案都使用了
[Proxy Pattern](https://en.wikipedia.org/wiki/Proxy_pattern)
和
[Adapter Pattern](https://en.wikipedia.org/wiki/Adapter_pattern)
实现 ORM 功能，并提供一个用于和数据库交换数据的 **容器**；

所以我打算封装一个直接操作 **原始模型** 的 ORM； 😎

## 一个简单的设计 —— ORM Lite

### 0. 这个ORM要做什么

- 将对C++**对象操作**转化成SQL**查询语句**
  ([LINQ to SQL](https://en.wikipedia.org/wiki/Language_Integrated_Query#LINQ_to_SQL_.28formerly_called_DLINQ.29))；
- 提供**C++ Style**接口，更方便的使用；

我的设计上大致分为6个方面：

1. 封装SQL链接器
2. 遍历对象内需要持久化的成员
3. 序列化和反序列化
4. 获取类名和各个字段的字符串
5. 获取字段类型
6. 将对C++对象的操作转化为SQL语句

### 1. 封装SQL链接器

为了让ORM支持各种数据库，
我们应该把对**数据库的操作**抽象为一个**统一**的 `Execute`：

``` cpp
class SQLConnector
{
public:
    SQLConnector (const std::string &connectionString);
    void Execute (...);
};
```

- 一个有点意思的接口设计 ——
  [`std::db`](https://github.com/cruisercoder/cppstddb)
- 因为SQLite比较简单，目前只实现了SQLite的版本；
- MySql版本应该会在
  [这里](https://github.com/thiefuniverse/ORM-Lite) 维护。。。😅

### 2. 遍历对象内需要持久化的成员

#### 2.1 使用 Visitor Pattern + Variadic Template 遍历

一开始，我想到的是使用
[Visitor Pattern](https://en.wikipedia.org/wiki/Visitor_pattern)
组合
[Variadic Template](https://en.wikipedia.org/wiki/Variadic_template)
进行成员的遍历；

首先，在**模型**处加入 `__Accept` 操作；
通过 `VISITOR` 接受不同的 `Visitor` 来实现特定功能；
并用 `__VA_ARGS__` 传入需要持久化的**成员列表**：

``` cpp
#define ORMAP(_MY_CLASS_, ...)                            \
template <typename VISITOR>                               \
void __Accept (VISITOR &visitor)                          \
{                                                         \
    visitor.Visit (__VA_ARGS__);                          \
}                                                         \
template <typename VISITOR>                               \
void __Accept (VISITOR &visitor) const                    \
{                                                         \
    visitor.Visit (__VA_ARGS__);                          \
}                                                         \
```

然后，针对不同功能，实现不同的 `Visitor`；
再通过统一的 `Visit` 接口，接受**模型**的**变长**数据成员参数；
例如 `ReaderVisitor`：

``` cpp
class ReaderVisitor
{
public:
    // Data to Exchange
    std::vector<std::string> serializedValues;

    template <typename... Args>
    inline void Visit (Args & ... args)
    {
        _Visit (args...);
    }

protected:
    template <typename T, typename... Args>
    inline void _Visit (T &property, Args & ... args)
    {
        _Visit (property);
        _Visit (args...);
    }
    
    template <typename T>
    inline void _Visit (T &property) override
    {
        serializedValues.emplace_back (std::to_string (property));
    }

    template <>
    inline void _Visit <std::string> (std::string &property) override
    {
        serializedValues.emplace_back ("'" + property + "'");
    }
};
```

- `Visit` 将操作转发给带有**变长模板**的 `_Visit`；
- 有**变长模板**的 `_Visit` 将各个操作转发给**处理单个数据**的 `_Visit`；
- **处理单个数据**的 `_Visit` 将**模型**的数据
  和 `Visitor` 一个 `public` 数据成员（`serializedValues`）交换；

不过，这么设计有一定的缺点：

- 我们需要预先定义所有的 `Visitor`，灵活性不够强；
- 我们需要把**和需要持久化的成员交换的数据**保存到 `Visitor` 内部，
  增大了代码的耦合；

#### 2.2 带有 泛型函数参数 的 `Visitor`

（使用了C++14的特性）

所以，我们可以让 `Visit` 接受一个**泛型函数参数**，用这个函数进行实际的操作；

在**模型**处加入的 `__Accept` 操作改为：

``` cpp
template <typename VISITOR, typename FN>                  \
void __Accept (const VISITOR &visitor, FN fn)             \
{                                                         \
    visitor.Visit (fn, __VA_ARGS__);                      \
}                                                         \
template <typename VISITOR, typename FN>                  \
void __Accept (const VISITOR &visitor, FN fn) const       \
{                                                         \
    visitor.Visit (fn, __VA_ARGS__);                      \
}                                                         \
```

- `fn` 为**泛型函数参数**；
- 每次调用 `__Accept` 的时候，把 `fn` 传给 `visitor` 的 `Visit` 函数；

然后，我们可以定义一个统一的 `Visitor`，遍历传入的参数，并调用 `fn` ——
相当于将 `Visitor` 抽象为一个 `for each` 操作：

``` cpp
class FnVisitor
{
public:
    template <typename Fn, typename... Args>
    inline void Visit (Fn fn, Args & ... args) const
    {
        _Visit (fn, args...);
    }

protected:
    template <typename Fn, typename T, typename... Args>
    inline void _Visit (Fn fn, T &property, Args & ... args) const
    {
        _Visit (fn, property);
        _Visit (fn, args...);
    }

    template <typename Fn, typename T>
    inline void _Visit (Fn fn, T &property) const
    {
        fn (property);
    }
};
```

最后，实际的数据交换操作通过传入特定的 `fn` 实现：

``` cpp
queryHelper.__Accept (FnVisitor (),
                      [&argv, &index] (auto &val)
{
    DeserializeValue (val, argv[index++]);
});
```

- 对比上边，这个方法实际上是在**处理单个数据**的 `_Visit` 将**模型**的数据
  传给回调函数 `fn`；
- `fn` 使用
  [Generic Lambda](https://en.wikipedia.org/wiki/C%2B%2B14#Generic_lambdas)
  接受不同类型的数据成员，然后再转发给其他函数（`DeserializeValue`）；
- 通过**capture**和**需要持久化的成员**交换的数据；

#### 2.3 另一种设计——用 `tuple` + Refrence 遍历

（使用了C++14的特性）

虽然最后版本没有使用这个设计，不过作为一个不错的思路，我还是记下来了；😁

首先，在**模型**处通过加入生成 `tuple` 的函数：

``` cpp
#define ORMAP(_MY_CLASS_, ...)                            \
inline decltype (auto) __ValTuple ()                      \
{                                                         \
    return std::forward_as_tuple (__VA_ARGS__);           \
}                                                         \
inline decltype (auto) __ValTuple () const                \
{                                                         \
    return std::forward_as_tuple (__VA_ARGS__);           \
}                                                         \
```

- `forward_as_tuple` 将 `__VA_ARGS__` 传入的参数转化为引用的 `tuple`；
- `decltype (auto)` 自动推导返回值类型；

然后，定义一个 `TupleVisitor`：

``` cpp
// Using a _SizeT to specify the Index :-), Cool
template < size_t > struct _SizeT {};

template < typename TupleType, typename ActionType >
inline void TupleVisitor (TupleType &tuple, ActionType action)
{
    TupleVisitor_Impl (tuple, action,
                       _SizeT<std::tuple_size<TupleType>::value> ());
}

template < typename TupleType, typename ActionType >
inline void TupleVisitor_Impl (TupleType &tuple, ActionType action,
                               _SizeT<0>)
{}

template < typename TupleType, typename ActionType, size_t N >
inline void TupleVisitor_Impl (TupleType &tuple, ActionType action,
                               _SizeT<N>)
{
    TupleVisitor_Impl (tuple, action, _SizeT<N - 1> ());
    action (std::get<N - 1> (tuple));
}
```

- 其中使用了 `_SizeT` 巧妙的进行 `tuple` 下标的判断；
- 具体参考
  https://stackoverflow.com/questions/18155533/how-to-iterate-through-stdtuple

最后，类似上边，实际的数据交换操作通过 `TupleVisitor` 完成：

``` cpp
auto tuple = queryHelper.__ValTuple ();
TupleVisitor (tuple, [&argv, &index] (auto &val)
{
    DeserializeValue (val, argv[index++]);
});
```

#### 2.4 问题

- 使用 `Variadic Template` 和 `tuple` 遍历数据，
  其函数调用的确定，都是**编译时**就生成的，这会带来一定的代码空间开销；
- 后两个方法可能在 实例化**Generic Lambda** 的时候，
  针对 不同类型的**模型**的 不同数据成员类型 实例化出不同的副本，
  代码大小更大；🤔

### 3. 序列化和反序列化

通过 [序列化](https://en.wikipedia.org/wiki/Serialization)，
将 C++ 数据类型转化为字符串，用于查询；
通过 反序列化，
将查询得到的字符串，转回 C++ 的数据类型；

#### 3.1 重载函数 `_Visit`

针对**每种支持的数据类型**重载一个 `_Visit` 函数，
然后对其进行相应的**序列化和反序列化**；

以序列化为例：

``` cpp
void _Visit (long &property) override
{
    serializedValues.emplace_back (std::to_string (property));
}

void _Visit (double &property) override
{
    serializedValues.emplace_back (std::to_string (property));
}

void _Visit (std::string &property) override
{
    serializedValues.emplace_back ("'" + property + "'");
}
```

#### 3.2 使用 `std::iostream`

然而，**针对每种支持的数据类型重载**，这种事情在标准库里已经有人做好了；
所以，我们可以改用了 `std::iostream` 进行**序列化和反序列化**；

以反序列化为例：

``` cpp
template <typename T>
inline std::ostream &SerializeValue (std::ostream &os,
                                     const T &value)
{
    return os << value;
}

template <>
inline std::ostream &SerializeValue <std::string> (
    std::ostream &os, const std::string &value)
{
    return os << "'" << value << "'";
}
```

### 4. 获取类名和各个字段的字符串

我们可以使用**宏**中的 `#` 获取传入参数的文字量；
然后将这个字符串作为 `private` 成员存入这个类中：

``` cpp
#define ORMAP(_MY_CLASS_, ...)                            \
constexpr static const char *__ClassName = #_MY_CLASS_;   \
constexpr static const char *__FieldNames = #__VA_ARGS__; \
```

其中
- `#_MY_CLASS_` 为**类名**；
- `#__VA_ARGS__` 为传入可变参数的字符串；
- `__FieldNames` 可以通过简单的字符串处理获得各个**字段**的字符串

### 5. 获取字段类型

在**新建**数据库的 `Table` 的时候，
我们不仅需要**类名**和各个**字段**的字符串，
还需要获得各个**字段的数据类型**；

#### 5.1 使用 `typeid` 运行时判断

在 `Visitor` **遍历成员**时，将每个成员的 `typeid` 保存起来：

``` cpp
template <typename T>
void _Visit (T &property) override
{
    typeIds.emplace_back (typeid (T));
}
```

**运行时**根据 `typeid` 判断类型并匹配字符串：

``` cpp
if (typeId == typeid (int))
    typeFmt = "int";
else if (typeId == typeid (double))
    typeFmt = "decimal";
else if (typeId == typeid (std::string))
    typeFmt = "varchar";
else
    return false;
```

#### 5.2 使用 `<type_traits>` 编译时判断

由于对象的类型在**编译时**已经可以确定，
所以我们可以直接使用 `<type_traits>` 进行**编译时**判断：

``` cpp
constexpr const char *typeStr =
    (std::is_integral<T>::value &&
     !std::is_same<std::remove_cv_t<T>, char>::value &&
     !std::is_same<std::remove_cv_t<T>, wchar_t>::value &&
     !std::is_same<std::remove_cv_t<T>, char16_t>::value &&
     !std::is_same<std::remove_cv_t<T>, char32_t>::value &&
     !std::is_same<std::remove_cv_t<T>, unsigned char>::value)
    ? "integer"
    : (std::is_floating_point<T>::value)
    ? "real"
    : (std::is_same<std::remove_cv_t<T>, std::string>::value)
    ? "text"
    : nullptr;

static_assert (
    typeStr != nullptr,
    "Only Support Integral, Floating Point and std::string :-)");
```

- `constexpr` **编译时**完成推导，减少**运行时**开销；
- `static_assert` **编译时**类型检查；

### 6. 将对C++对象的操作转化为SQL语句

这里，我们应该提供
[Fluent Interface](https://en.wikipedia.org/wiki/Fluent_interface)：

``` cpp
auto query = mapper.Query (helper)
    .Where (
        Field (helper.name) == "July" &&
        (Field (helper.id) <= 90 && Field (helper.id) >= 60)
    )
    .OrderByDescending (helper.id)
    .Take (3)
    .Skip (10)
    .ToVector ();
```

#### 6.1 映射到对应的表中

对于一般的操作，通过模板类型参数，获取 `__ClassName`：

``` cpp
template <typename C>
void Insert (const C &entity)
{
    auto tableName = C::__ClassName;
    // ...
}
```

**带有条件**的查询通过 `Query<MyClass>`，将自动映射到 `MyClass` 表中；
并返回自己的**引用**，实现**Fluent Interface**：

``` cpp
template <typename C>
ORQuery<C> Query (const C &queryHelper)
{
    return ORQuery<C> (queryHelper, this);
}

ORQuery &Where (const Expr &expr)
{
    // Parse expr
    return *this;
}
```

#### 6.2 自动将C++表达式转为SQL表达式

首先，引入一个 `Expr` 类，用于保存条件表达式；
将 `Expr` 对象传入 `ORQuery.Where`，以实现条件查询：

``` cpp
struct Expr
{
    std::vector<std::pair<const void *, std::string>> expr;

    template <typename T>
    Expr (const T &property,
          const std::string &relOp,
          const T &value)
        : expr { std::make_pair (&property, relOp) }
    {
        // Serialize value into expr.front ().second
    }

    inline Expr operator && (const Expr &right)
    {
        // Return Composite Expression
    }

    // ...
}
```

- `expr` 保存了表达式序列，包括**该成员的指针**和**关系运算符 值**的字符串；
- 重载 `&& ||` 实现表达式的**复合条件**；

不过这样的接口还不够友好；
因为如果我们要生成一个 `Expr` 则需要手动传入 `const std::string &relOp`：

``` cpp
mapper.Query (helper)
    .Where (
        Expr (helper.name, "=", std::string ("July")) &&
        (Expr (helper.id, "<=", 90) && Expr (helper.id, ">=", 60))
    )
    // ...
```

所以，我们在这里引入一个 `Expr_Field` 实现**自动构造表达式**：

``` cpp
template <typename T>
struct Expr_Field
{
    const T& _property;

    Expr_Field (const T &property)
        : _property (property)
    {}

    inline Expr operator == (T value)
    { return Expr { _property, "=", std::move (value) }; }

    // != > < >= <=
}

template <typename T>
Expr_Field<T> Field (T &property)
{
    return Expr_Field<T> { property };
}
```

- `Field` 函数用更短的语句，返回一个 `Expr_Field` 对象；
- 重载 `== != > < >= <=` 生成带有对应关系的 `Expr` 对象；

#### 6.3 自动判断C++对象的成员字段名

由于没有想到很好的办法，所以目前使用了**指针**进行**运行时**判断：

``` cpp
queryHelper.__Accept (FnVisitor (),
                      [&property, &isFound, &index] (auto &val)
{
    if (!isFound && property == &val)
        isFound = true;
    else if (!isFound)
        index++;
});
fieldName = FieldNames[index];
```

相当于使用 `Visitor` 遍历这个对象，找到对应成员的**序号**；

#### 6.4 问题

之后的版本可能考虑：

- 支持**更多的SQL操作**（如跨表）；
- 改用**语法树**实现；（欢迎 Pull Request 😉）

## 写在最后

这篇文章是我的第一篇技术类博客，写的比较浅，见谅；

> 你有一个苹果，我有一个苹果，我们彼此交换，每人还是一个苹果；
> 你有一种思想，我有一种思想，我们彼此交换，每人可拥有两种思想。

如果对以上内容及ORM Lite有什么问题，
欢迎 **指点** **讨论** 😉：
https://github.com/BOT-Man-JL/ORM-Lite/issues

Delivered under MIT License &copy; 2016, BOT Man