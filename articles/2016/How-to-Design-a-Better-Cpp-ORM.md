# 如何设计一个更好的 C++ ORM

> 2016/11/26
>
> “用C++的方式读写数据库，简直太棒了！”

上一篇相关文章：[如何设计一个简单的 C++ ORM](How-to-Design-a-Naive-Cpp-ORM.md)
（[旧版代码](https://github.com/BOT-Man-JL/ORM-Lite/tree/v1.0)）
😉

关于这个设计的代码和样例 😊：
https://github.com/BOT-Man-JL/ORM-Lite/tree/v1.1

> **WARNING：早期文章不足之处较多，推荐阅读**：
> 
> - [浅谈 C++ 元编程](../2017/Cpp-Metaprogramming.md)
> - [简单的 C++ 结构体字段 反射](../2018/Cpp-Struct-Field-Reflection.md)

## [no-toc] TOC

[TOC]

## 0. 上一版本的问题

上一个版本较为简单，仅仅支持了最基本的
[CRUD](https://en.wikipedia.org/wiki/Create,_read,_update_and_delete)
，存在以下不足：

1. 不支持 `Nullable` 数据类型；
2. **自动判断C++对象成员的字段名**所用的方法耦合度高；
3. **表达式系统**不够完善；
4. 不支持**多表**操作和选择**部分字段**的操作；
5. 由于没有使用 `ORMAP` 导致的**报错可读性差**；

利用了课余时间，在最新版本中已经改进了以上内容；😄

如果你只是对**模板**部分感兴趣，可以直接看

- `4. 推导查询结果`；
- `5. 更人性化的编译时报错`；

## 1. `Nullable` 字段

### 1.1 为什么要支持 `Nullable` 字段

- 尽管C++原生数据类型并没有 `Nullable` 的支持，但是SQL里有 `null`；
- 当两个表合并时，会产生**可能为空的字段**；
  （例如，和一个空表 `LEFT JOIN` 后，每一行都会带有 `null` 字段）

### 1.2 基本语义和实现

所以，ORMLite中实现了一个类似 C# 的 `Nullable<T>`：

- 默认 构造/赋值：对象为空；
- 值 构造/赋值：对象为值；
- 复制/移动：目标和源同值；
- GetValueOrDefault：返回 值 或 默认非空值；
- 比较：两个对象相等，当且仅当
  - 两个值都为 `空`；
  - 两个值都 `非空` 且 具有相同的 `值`；

具体参考：
https://stackoverflow.com/questions/2537942/nullable-values-in-c/28811646#28811646

## 2. 提取对象成员字段名

这里的提取对象成员**字段名**，指的是：
如果想表示 `UserModel` 的 `user_id` 字段，可以通过
`UserModel user` 的 `user.user_id` 推导出来；
（之前的文章讲的不是很明白😂）

``` cpp
UserModel user;
auto field = FieldExtractor { user };

// Get Field of 'user'
auto field_user_id = field (user.user_id);

// Get string of Field Name
auto fieldName_user_id = field_user_id.fieldName;
```

另外，在**跨表**查询时，除了字段名，我们还需要保存**表名**；

### 2.1 之前的实现

在上一篇文章里，我曾经使用这种方法实现**自动判断C++对象的成员字段名**：

> 由于没有想到很好的办法，所以目前使用了**指针**进行**运行时**判断：
>
> ``` cpp
> queryHelper.__Accept (FnVisitor (),
>                       [&property, &isFound, &index] (auto &val)
> {
>     if (!isFound && property == &val)
>         isFound = true;
>     else if (!isFound)
>         index++;
> });
> fieldName = FieldNames[index];
> ```
>
> 相当于使用 `Visitor` 遍历这个对象，找到对应成员的**序号**；

总结起来有两点：

- 保存一个**被注入对象的引用** `queryHelper`；
- 每次**遍历**这个引用，判断各个字段**指针**是否相同；

首先，这么做将导致 `queryHelper` 和 `Queryable` 对象严重耦合：

- 每一个 `Queryable` 对象里都需要保存对应的 `queryHelper`；
- 当一个 `Queryable` 对象可以判断多个不同的表的字段名时，
  需要将所有 `queryHelper` 保存为一个 `tuple`；
  （因为不同的 `queryHelper` 是不同的数据类型，不能直接用 `list`）

另外，这将会导致（巨大的）运行时开销：

每次查询的时间复杂度 `O(m, n) = queryHelper个数 * queryHelper内字段数`；
当需要判断字段名的次数很大的时候，这将是很复杂的事情。。。（虽然计算速度很快）

### 2.2 用 Hash Table 实现 —— `FieldExtractor`

由于每个**被注入对象**的字段的**地址**在**判断前**已经确定，
所以我们可以构造一个 `FieldExtractor`，并把这些**地址**装入一个
`std::unordered_map<const void *, Field>`中，
并不需要保存该对象的引用；

``` cpp
template <typename Arg>
FieldExtractor (const Arg &arg) { Extract (arg); }

template <typename Arg, typename... Args>
FieldExtractor (const Arg &arg, const Args & ... args)
    : FieldExtractor (args...) { Extract (arg); }

template <typename C>
void Extract (const C &helper)
{
    // Why Place these local vars here:
    // Walk around gcc/clang 'undefined reference' HELL...
    const auto &fieldNames = C::__FieldNames ();
    constexpr auto tableName = C::__TableName;

    size_t index = 0;
    helper.__Accept (
        [this, &index, &fieldNames, &tableName] (auto &val)
    {
        //_map.emplace (...);
    });
}
```

然后提供一个 `Field<T> operator () (const T &field)` 接口；
用 `field` 的**地址**查表构造 `Field<T>`；
并将**字段名**和**所属的表**的信息存于 `Field<T>`；

``` cpp
template <typename T>
inline Field<T> operator () (const T &field)
{
    try
    {
        // Find the pointer at _map
        return _map.at ((const void *) &field);
    }
    catch (...)
    {
        throw std::runtime_error ("No Such Field...");
    }
}
```

最后通过重载
`NullableField<T> operator () (const Nullable<T> &field)`
给 `Nullable` 类型字段生成对应的 `NullableField<T>`；

## 3. 表达式系统

利用 `FieldExtractor` 我们就可以很方便的提取出数据库表里的**字段**了；
提取出的**字段**，可以通过**C++原生**的表达式运算，生成对应的**SQL表达式**；

### 3.1 基本类型设计

字段和聚合函数：

- `Selectable` 为 `Field` 和 `Aggregate` 的基类；
- `Field` 为 普通数据字段，也是 `NullableField` 的基类；
- `NullableField` 为 可为空数据字段；
- `Aggregate` 为 聚合函数；

表达式：

- `Expr` 为 条件语句；
- `SetExpr` 为 赋值语句，仅用于 `ORMapper.Update`；

### 3.2 从字段生成表达式

这里，很容易可以想到，我们只需要**重载**关系运算符就可以了：

``` cpp
template <typename T>
inline Expr operator == (const Selectable<T> &op, T value)
{ return Expr (op, "=", std::move (value)); }
...
```

- 由于 `Field`，`NullableField`， `Aggregate` 都是 `Selectable`，
  我们只需要重载一次就可以应用到它们上边；
- 字段和聚合函数 设计为模板 `Selectable<T>`，可以实现编译时的强类型检查；

另外，我们可以**特殊化**部分模板来实现针对**特殊字段**的运算；
例如：

- `NullableField` 可以和 `nullptr` 比较产生 `IS NULL` 运算；
- 字符串类型的 `Field` 可以使用 `LIKE` 运算符，做正则式匹配；

``` cpp
template <typename T>
inline Expr operator == (const NullableField<T> &op, nullptr_t)
{ return Expr { op, " is null" }; }

inline Expr operator & (const Field<std::string> &field,
                        std::string val)
{ return Expr (field, " like ", std::move (val)); }

...
```

## 4. 推导查询结果

第一个版本中，我们并没有实现**多表**和 `SELECT` 的操作；
但是为了实现完整的ORM功能，还是继续把它完善了；

### 4.1 我们要做什么

#### 使用 `queryHelper` 产生查询结果

上一版本中，`ORMapper.Query` 生成的每个 `ORQuery`（现在改为`Queryable`）
都依赖于一个固定的 `C queryHelper`——
在 `ToVector` 和 `ToList` 时，都根据这个 `queryHelper` 生成结果；

``` cpp
template <typename C>
ORQuery<C> Query (const C &queryHelper)
{
    return ORQuery<C> (queryHelper, this);
}
```

#### 修改 `queryHelper` 类型，实现不同的返回值

当然，如果使用了 `SELECT <columns ...>` 或 `JOIN` 之后，
在 `ToVector` 和 `ToList` 时，返回的结果就不是原来 `Query` 时的类型了；

简单想来，可以用下面的方式表示这个结果：

- `SELECT` 产生的每一**行**就可以使用 `std::tuple<F1, F2, ...>` 表示；
- `JOIN` 产生的每一**行**都是两个类型对象的**并**；
  例如 `C1 JOIN C2`，将当前的 `C1` 转变为 `std::tuple<C1, C2>`；
- 对于三个表合并的时候，应该变为 `std::tuple<C1, C2, C3>`，
  而不是简单的 `std::tuple<std::tuple<C1, C2>, C3>`
  （这么做有点反人类 😆）；
- `UNION` 等复合选择 产生的结果和原来相同，并在**编译时**进行类型检查；

不过，如果把 `C1` 和 `C2` 的所有数据成员提取出来，排在一**行**，
变为 `std::tuple<C1F1, C1F2, ..., C2F1, C2F2, ...>`，
就可以实现 `SELECT` / `JOIN` / `UNION` 的统一结果；

#### 低头不见抬头见的 `Nullable`

虽然 `C1` 和 `C2` 都可能不含有 `Nullable` 字段，但是它们合并之后的表里，
可能会由于 `C1` 有这一项，而 `C2` 没有导致 `C2` 出现空缺；
`C2` 中字段原本的数据类型，在这种情况下就不能很好的反映真实的结果；

所以，我们给 `std::tuple<C1F1, ..., C2F1, ...>` 加一层 `Nullable`，
变为 `std::tuple<Nullable<C1F1>, ..., Nullable<C2F1>, ...>`，
就可以很好的解决了 `NULL` 的问题；

#### 更好的改进

另外，如果原本 `C*F*` 的数据类型就是 `Nullable`，
就没有必要加上一层 `Nullable`了
（`Nullable<Nullable<T>>` 没什么实际意义）；

这样，得到的最后结果是 `std::tuple<Nullable<T>, ...>`
（其中 `T` 为C++的基本数据类型） —— 结果更统一，便于解析😎；

最后，对于没有使用过 `Select` / `Join` 的 `Queryable` 在调用
`ToVector` 和 `ToList` 时，返回的仍是原始的数据类型；

### 4.2 如何实现

#### 推导 `Select ()` 返回的 `tuple`

`Select ()` 接受的是 `Field` 或者 `Aggregate`，
所以新的 `Queryable` 的查询结果类型
可以通过传入 `Selectable<T>, ...` 用以下的方式推导出：

``` cpp
template <typename T>
inline auto SelectToTuple (const Selectable<T> &)
{
    return std::make_tuple (Nullable<T> {});
}

template <typename T, typename... Args>
inline auto SelectToTuple (const Selectable<T> &arg,
                           const Args & ... args)
{
    return std::tuple_cat (SelectToTuple (arg),
                           SelectToTuple (args...));
}
```

- `const Selectable<T> &` 获取 `Selectable` 的类型 `T`
- `std::make_tuple` 生成只有一个 `Nullable<T>` 的 `tuple`；
- `std::tuple_cat` 将每个 `SelectToTuple` 产生的 `tuple` 拼接起来；

#### 推导 `Join ()` 返回的 `tuple`

`Join ()` 接受的是 **需要合并的表对应的Class**，
新的 `Queryable` 的查询结果类型 可以通过传入
原来的 `queryHelper` 和 **需要合并的表对应的Class** 的一个对象
用以下的方式推导出：

``` cpp
template <typename C>
inline auto JoinToTuple (const C &arg)
{
    using TupleType = decltype (arg.__Tuple ());
    constexpr size_t size = std::tuple_size<TupleType>::value;
    return TupleHelper<TupleType, size>::ToNullable (
        arg.__Tuple ());
}

template <typename... Args>
inline auto JoinToTuple (const std::tuple<Args...>& t)
{
    // TupleHelper::ToNullable is not necessary
    return t;
}

template <typename Arg, typename... Args>
inline auto JoinToTuple (const Arg &arg,
                         const Args & ... args)
{
    return std::tuple_cat (JoinToTuple (arg),
                           JoinToTuple (args...));
}
```

- 实际转换模板的是 两个 `JoinToTuple`，
  一个处理带有 `ORMAP` 的对象，另一个处理 原先已经是 `tuple` 的对象；
- 所有的处理结果，使用 `std::tuple_cat` 拼接；
- 后者直接返回这个 `tuple` 对象（这里已经保证了所有元素是 `Nullable`）；
- 前者调用 `ORMAP` 注入的 `.__Tuple ()` 返回一个带有所有成员的 `tuple`，
  然后使用 `TupleHelper::ToNullable` 给 `tuple` 加一层 `Nullable`；

#### 给 `tuple` 加一层 `Nullable`

这里我们需要引入一个帮助模板函数 `FieldToNullable`：

``` cpp
template <typename T>
inline auto FieldToNullable (const T &val)
{ return Nullable<T> (val); }

template <typename T>
inline auto FieldToNullable (const Nullable<T> &val)
{ return val; }
```

- 对于普通的类型 `T`，构造并返回一个 `Nullable<T>` 对象；
- 通过**重载**，对于 `Nullable` 类型，直接返回这个对象；

这相当于每个通过一次 `FieldToNullable` 的结果，被保证是 `Nullable<T>`，
其中 `T` 为C++的基本数据类型；

然后，利用 `TupleHelper::ToNullable` 遍历一个 `tuple` 的所有类型，
使用 `FieldToNullable` 处理，
将不是 `Nullable` 的类型元素转变为 `Nullable`；

``` cpp
template <typename TupleType, size_t N>
struct TupleHelper
{
    static inline auto ToNullable (const TupleType &tuple)
    {
        return std::tuple_cat (
            TupleHelper<TupleType, N - 1>::ToNullable (tuple),
            std::make_tuple (
                FieldToNullable (std::get<N - 1> (tuple)))
        );
    }
}

template <typename TupleType>
struct TupleHelper <TupleType, 1>
{
    static inline auto ToNullable (const TupleType &tuple)
    {
        return std::make_tuple (
            FieldToNullable (std::get<0> (tuple)));
    }
}
```

- 类似上边，使用 `std::make_tuple` 生成 `tuple`，
  `std::tuple_cat` 拼接 `tuple`；
- `struct TupleHelper <TupleType, N>` 相当于是 `N` 个参数时的**重载**，
  `struct TupleHelper <TupleType, 1>` 针对于 `1` 个参数特殊化；

#### 保证 复合操作 的类型安全

我们只需要设计接口时，仅接受相同返回类型的 `Queryable` 就可以了：

``` cpp
Queryable Union (const Queryable &queryable) const;
...
```

### 4.3 更好的实现

【补充于 2016/12/10】

#### `Nullable` 仅需要保存类型

对于之前的 `FieldToNullable`，
在进行 `Nullable` 转换时并不需要**实际**的产生这个**对象**，
只需要**推导**我们需要的**类型**就可以了，这可以减少**运行时开销**；

``` cpp
// Type To Nullable
// Get Nullable Type Wrappers for Non-nullable Types
template <typename T> struct TypeToNullable
{
    using type = Nullable<T>;
};
template <typename T> struct TypeToNullable <Nullable<T>>
{
    using type = Nullable<T>;
};
template <typename T>
using TypeToNullable_t = typename TypeToNullable <
    std::remove_cv_t<std::remove_reference_t<T>> >::type;
```

- 这里把 `FieldToNullable` 改名为 `TypeToNullable`
- 另外使用一个 `using` 作为 `typename ... ::type` 的简记；

#### 使用 `std::index_sequence` + `std::tuple_element_t` 推导模板

之前的 `TupleHelper::ToNullable` 的作用在于：
给 `tuple` 加一层 `Nullable`，并构造相应的对象；

既然是要遍历 `tuple`，我们就可以使用 C++ 14 的 `std::index_sequence`
来遍历，并用 `std::tuple_element_t` 逐一取出相应类型进行加 `Nullable`；

``` cpp
// Tuple To Nullable Impl
// Apply 'TypeToNullable' to each element of Tuple
template <typename TupleType, std::size_t... I>
static inline auto TupleToNullable_Impl (
    const TupleType &, std::index_sequence<I...>)
{
    // Unpacking Tricks :-)
    // Expand each of 'I'
    // with 'TypeToNullable_t<tuple_element_t<...>>' as a sequence
    return std::tuple <
        TypeToNullable_t<std::tuple_element_t<I, TupleType>>...
    > {};
}

// Tuple To Nullable
// Produce the 'index_sequence' for 'tuple'
template <typename TupleType>
static inline auto TupleToNullable (const TupleType &tuple)
{
    constexpr auto size = std::tuple_size<TupleType>::value;
    return TupleToNullable_Impl (
        tuple, std::make_index_sequence<size> {});
}
```

- `std::tuple_size` 获取 `Tuple` 的长度；
- `std::make_index_sequence` **产生**相应长度的**序列**；
- 变长模板参数 `std::size_t... I` **接受**对应的
  `std::index_sequence<I...>` **序列**；
- `I ...` 在表达式中**展开**这个**序列**，并**套入**
  `TypeToNullable_t<std::tuple_element_t<...>>` **格式**；
- **展开的结果**放入一个 `std::tuple` 中，并构造/返回一个对象；

具体参考：
https://en.cppreference.com/w/cpp/language/parameter_pack

（另外分享一下：[如何使用 `index_sequence` 实现 Tuple Visitor](https://stackoverflow.com/questions/26902633/how-to-iterate-over-a-tuple-in-c-11/26902803#26902803)）

#### 直接拆解 `std::tuple` 取出元素

与其使用 `tuple_element_t` 来逐一取出元素**类型信息**（不需要保留数据），
不如**直接**在模板里边把它拆开吧！😎

``` cpp
// Tuple To Nullable
// Apply 'TypeToNullable' to each element of Tuple
template <typename... Args>
static inline auto TupleToNullable (
    const std::tuple<Args...> &)
{
    // Unpacking Tricks :-)
    // Expand each of 'Args'
    // with 'TypeToNullable_t<...>' as a sequence
    return std::tuple<
        TypeToNullable_t<Args>...
    > {};
}
```

- 这里省去了 `TupleToNullable_Impl`；
- 变长模板参数 `typename... Args` **接受**对应的
  `std::tuple<Args...>` **序列**；
- `Args ...` 在表达式中**展开**这个**序列**，并**套入**
  `TypeToNullable_t<...>` **格式**；
- **展开的结果**放入一个 `std::tuple` 中，并构造/返回一个对象；

#### 改写 “推导 `Join ()` 返回的 `tuple`”

``` cpp
// QueryResult To Nullable
template <typename C>
static inline auto QueryResultToTuple (const C &arg)
{
    // Injected friend
    return TupleToNullable (arg.__Tuple ());
}
template <typename... Args>
static inline auto QueryResultToTuple (
    const std::tuple<Args...>& t)
{
    // TupleToNullable is not necessary: Nullable already...
    return t;
}

// Construct Tuple from QueryResult List
template <typename... Args>
static inline auto JoinToTuple (const Args & ... args)
{
    return decltype (std::tuple_cat (
        QueryResultToTuple (args)...
    )) {};
}
```

- 最后使用 `decltype (std::tuple_cat (...)) {}` 而不是直接返回，
  可以避免**运行时**的对象拷贝；
- 这里相对于上边的实现，进一步减少了**编译时**和**运行时**开销；

## 5. 更人性化的编译时报错

【补充于 2016/12/4】

### 5.1 灾难性的报错

由于**ORM Lite**整体上使用的是**模板**，所以这里会导致一个问题：

如果用户传入模板的参数**不匹配**（一般是忘记加入 `ORMAP`）时，
就会因为**找不到对应成员**或**缺少相关操作的重载**而编译错误；

如果设计模板时**不考虑这个问题**，
就会使得编译**报错的可读性很差**，用户就很难从中察觉问题所在；

例如，

``` cpp
struct SessionModel { /* forgot to add 'ORMAP' */ };
...
mapper.Query (SessionModel {}).ToList ();
```

在 MSVC 2015 上报错：

```
error C2039: '__TableName': is not a member of 'SessionModel'
error C2065: '__TableName': undeclared identifier
error C2440: 'initializing': cannot convert from 'initializer list' to 'BOT_ORM::Queryable<SessionModel>'
error C2039: '__Accept': is not a member of 'SessionModel'
error C2039: '__this': is not a member of 'SessionModel'
```

### 5.2 使用 SFINAE 检查注入

为了检测是否加入 `ORMAP`，我们可以使用
[SFINAE](https://en.cppreference.com/w/cpp/language/sfinae)
编译时**检查**是否存在我们注入的某些**成员**；

在我看来，**SFINAE** 简单说来就是**模板的最优匹配**
—— 模板每次都会从所有候选中找到**最特殊化**的一个进行匹配；

``` cpp
template<typename T> class HasInjected
{
    template<typename U> struct SFINAE {};
    template<typename U> static constexpr std::true_type Test (
        SFINAE<decltype (U::__TableName)>*)
    {
        return std::true_type {};
    }
    template<typename U> static constexpr std::false_type Test (...)
    {
        return std::false_type {};
    }
public:
    static constexpr bool value =
        decltype (Test<std::remove_reference_t<
                  std::remove_cv_t<T>>> (nullptr))::value;
};
```

- `struct SFINAE` 用于接受**任何类型**，作为辅助；
- 两个 `Test` 函数接受不同类型的**模板参数**，来**返回**不同类型的值；
  - 前者接受参数 `SFINAE<decltype (U::__TableName)>*`；
    当 `Test<U>` 的 `U` 有成员 `__TableName` 时，`SFINAE` 就可以匹配；
  - 后者接受参数 `...`，表示**默认**匹配，即不满足上面条件；
  - `nullptr` 当作**参数**传入上边的两个函数（因为这里通过参数判断）；
- `Test<std::remove_reference_t<std::remove_cv_t<T>>>`
  将 `T` 原本的类型传给 `<typename U>` 进行前面的匹配；
- 用 `decltype` 获取匹配的函数**返回值类型**；
  并通过 `::value` 获取 `true_type` 和 `false_type` 的内容；
- 这一过程在**编译时**完成，最后得到的 `value` 是 `constexpr`；

具体参考：
https://stackoverflow.com/questions/87372/check-if-a-class-has-a-member-function-of-a-given-signature

### 5.3 使用 `static_assert` 静态断言

有了强大的 `SFINAE`，我们就可以通过**静态断言**，
在**编译时**检查是否加入 `ORMAP` 了；

``` cpp
#define NO_ORMAP "Please Inject the Class with 'ORMAP' first"
...
template <typename C>
Queryable<C> Query (C queryHelper)
{
    // Add static assert before Produce Query Object
    static_assert (HasInjected<C>::value, NO_ORMAP);
    ...
}
```

但是，这仍然不够好：
编译器遇到 `static_assert` **断言失败**之后，仍然会继续编译：

```
error C2338: Please Inject the Class with 'ORMAP' first
error C2039: '__TableName': is not a member of 'SessionModel'
error C2065: '__TableName': undeclared identifier
error C2440: 'initializing': cannot convert from 'initializer list' to 'BOT_ORM::Queryable<SessionModel>'
error C2039: '__Accept': is not a member of 'SessionModel'
error C2039: '__this': is not a member of 'SessionModel'
```

当报错很多的时候，**多出来的一行**
`Please Inject the Class with 'ORMAP' first` 往往很难被察觉； 😶

### 5.4 使用 `std::enable_if` 选择模板

可以写两个不同的模板解决**继续编译的问题**：

- 一个是正确的，里边**使用**了已经注入的**依赖**；
- 一个是错误的，里边只有**一行** `static_assert` 用于**报错**；

``` cpp
template <typename C>
std::enable_if_t<!HasInjected<C>::value, Queryable<C>>
    Query (C queryHelper)
{
    static_assert (HasInjected<C>::value, NO_ORMAP);
}
template <typename C>
std::enable_if_t<HasInjected<C>::value, Queryable<C>>
    Query (C queryHelper)
{
    ...
}
```

- `std::enable_if` 测试 `HasInjected<C>`，并**最优匹配**；
  - 将满足条件的 `Query` 转到前一个模板，并**报错**；
  - 不满足条件的 `Query` 转到后一个模板，并实现逻辑；
  - 两个模板的**匹配条件**恰好**互斥**；
- 前一个模板里的 `static_assert (HasInjected<C>::value, NO_ORMAP);`
  不能写成 `static_assert (false, NO_ORMAP);`；
  由于 **Expression SFINAE**，在满足 `HasInjected<C>` 的情况下，
  编译器会仍然编译前一个模板的内容；（这个在 VS 2015 支持的不好）

这样一来，上边冗长的报错就化简为**一行**😎：

```
error C2338: Please Inject the Class with 'ORMAP' first
```

具体参考：
- https://en.cppreference.com/w/cpp/types/enable_if
- https://devblogs.microsoft.com/cppblog/expression-sfinae-improvements-in-vs-2015-update-3/
- https://devblogs.microsoft.com/cppblog/partial-support-for-expression-sfinae-in-vs-2015-update-1/

### 5.5 其他 `std::enable_if` 的用法

对于有的函数需要使用 `auto` **推导返回值** 或 **没有返回值**（如构造函数），
就不能将 `enable_if` 写在返回值；

所以，我们可以引入一个**带默认值的辅助参数**，进行**最优匹配**；

``` cpp
template <typename C>
auto fn (...,
         std::enable_if_t<!BOT_ORM_Impl::HasInjected<C>::value> *
         = nullptr)
{
    static_assert (HasInjected<C>::value, NO_ORMAP);
    // return void
}
template <typename C>
auto fn (...,
         std::enable_if_t<BOT_ORM_Impl::HasInjected<C>::value> *
         = nullptr)
{
    ...
    // return ...
}
```

当然，这个**带默认值的辅助参数**思想也可以用到**模板参数**上；

具体参考（这里总结了上述三种方法）：
https://stackoverflow.com/questions/17829874/understanding-sfinae/17830071#17830071

### 5.6 使用 `std::void_t` 改进 `HasInjected`

【补充于 2016/12/10】

听取了 [@qicosmos](https://github.com/qicosmos) 的建议，
学习了一下 `void_t`，然后实现干净多了；😄

``` cpp
template <typename T> class HasInjected
{
    template <typename, typename = void_t<>>
    struct Test : std::false_type {};
    template <typename U>
    struct Test <U, std::void_t<decltype (U::__TableName)>>
        : std::true_type {};
public:
    static constexpr bool value = Test<T>::value;
};
```

具体参考：
https://en.cppreference.com/w/cpp/types/void_t

## 0xFF. 写在最后

由于我的知识有限，实现上可能有不足，欢迎指正；

> Modularity is something every software designer does in their sleep.
> —— Scott Shenker

如果对以上内容及ORM Lite有什么问题，
欢迎 **指点** **讨论** 😉：
https://github.com/BOT-Man-JL/ORM-Lite/issues

Delivered under MIT License &copy; 2016, BOT Man