# 深入 C++ 元组 (tuple) 实现

> BOT Man, 2017/12/1
>
> 分享 C++ 元组 (tuple) 的实现方式

[slide-mode]

---

## 主要内容

- [sec|什么是 `tuple`]
- [sec|存储实现]
- [sec|构造函数]
- [sec|`tuple_element` & `get`]
- [sec|更多内容]

---

## 什么是 `tuple`

``` cpp
using Person = tuple<std::string, char, int>;
Person john { "John"s, 'M', 21 };
Person jess { "Jess"s, 'F', 19 };
Person jack = make_tuple ("Jack"s, 'M', 20);

std::string john_name = get<0> (john);
int jess_age = get<int> (jess);
char gender_jack;
tie (ignore, gender_jack, ignore) = jack;
```

---

## 什么是 `tuple`

``` cpp
std::set<Person> group { john, jess, jack };

using Hobby = tuple<std::string, int>;
Hobby kongfu { "Kong Fu", 2 };  // implicit

// Select and Join from 2 tables :-)
//   tuple < name, gender, age, hobby, score >
//   tuple { "Johh"s, 'M', 21, "Kong Fu"s, 2 }
auto john_hobby = tuple_cat (john, kongfu);
```

## 什么是 `tuple`

> Reference: https://github.com/BOT-Man-JL/ORM-Lite

``` cpp
// ORM-Lite
auto usersOrderList = mapper.Query (userModel)
    .Join (userModel,
           field (userModel.user_id) ==
           field (orderModel.user_id)
    ).ToList ();

// SELECT * FROM UserModel
// JOIN OrderModel
// ON UserModel.user_id = OrderModel.user_id
```

---

## 存储实现

``` cpp
template <typename... Types>
class tuple : Types... {};
```

- `tuple<int> : int`

---

## 存储实现

``` cpp
template <typename T>
struct _tuple_leaf { T value_; };

template <typename... Types>
class tuple : _tuple_leaf<Types>... {};
```

- `tuple<int, int>`
- `tuple<int, double>` === `tuple<double, int>`

---

## 存储实现

``` cpp
template <size_t, typename T>
struct _tuple_leaf { T value_; };

template <typename S, typename... Ts>
struct _tuple;

template <size_t... Is, typename... Ts>
struct _tuple<index_sequence<Is...>, Ts...>
  : _tuple_leaf<Is, Ts>... {};
```

---

## 存储实现

``` cpp
template <typename... Ts>
class tuple : _tuple<
  make_index_sequence<sizeof...(Ts)>,
  Ts...> {};
```

- [libc++ (clang)](https://llvm.org/svn/llvm-project/libcxx/trunk/include/tuple)
- `((_tuple<Index, Type> &) t)._val`

---

## 存储实现

``` cpp
template<>
class tuple<>;

template<typename Head, typename ...Tails>
class tuple<Head, Tails ...> {
  Head _head;
  tuple<Tails ...> _tails;
};
```

- `t._tails._tails._tails._head`

---

## 存储实现

``` cpp
template<>
class tuple<>;

template<typename Head, typename ...Tails>
class tuple<Head, Tails ...>
  : tuple<Tails ...> { Head _val; };
```

- MSVC STL
- `((tuple<BaseType> &) t)._val`

## 存储实现

``` cpp
template<>
class tuple<>;

template<typename Head, typename ...Tails>
class tuple<Head, Tails ...>
  : tuple<Tails ...>, _leaf<Head> {};
```

- [libstdc++ (gcc)](https://gcc.gnu.org/svn/gcc/trunk/libstdc++-v3/include/std/tuple)
- `((_leaf<BaseHead> &) ((tuple<BaseType> &) t)._val`

---

## 构造函数

``` cpp
// default ctor
tuple ();

// value direct ctor
tuple (const Head &, const Tails &...);

// value convert ctor
template<typename T, typename ...Ts>
  tuple (T &&arg, Ts &&...args);

// tuple convert ctor
template<typename ...Rhs>
  tuple (const tuple<Rhs ...> &rhs);
template<typename ...Rhs>
  tuple (tuple<Rhs ...> &&rhs);

// copy/move ctor
tuple (const tuple &);
tuple (tuple &&);
```

---

### （右值引用）

``` cpp
template<typename T> class vector {
    vector(const vector &); // copy data
    vector(vector &&);      // move data
};

template<typename T> class unique_ptr {
    unique_ptr(const unique_ptr &) = delete;
    unique_ptr(unique_ptr &&);  // move only
};
```

---

### （通用引用）

``` cpp
// rvalue ref: no type deduction
void f1 (Widget &&param);
Widget &&var1 = Widget ();
template<typename T> void f2 (vector<T> &&);

// universal ref: type deduction
auto &&var2 = var1;       
template<typename T> void f3 (T &&param);
```

---

### （完美转发）

``` cpp
template<typename T, typename... Args>
unique_ptr<T> make_unique(Args &&...args) {
    return unique_ptr<T> {
        new T { std::forward<Args>(args)... }
    };
}
```

- `& & => &`
- `& && => &` / `&& & => &`
- `&& && => &&`

---

## 构造函数

``` cpp
template<typename T, typename ...Ts>
tuple (T &&arg, Ts &&...args) :
    Tail (std::forward<Ts> (args)...),
    _val (std::forward<T> (arg)) {}

template<typename ...Rhs>
tuple (tuple<Rhs ...> &&rhs) :
    Tail (std::move (rhs._tail ())),
    _val (std::move (rhs._head ())) {}
```

---

## 构造函数

``` cpp
// ambiguous:
//   tuple<int> { int }
//   tuple<int> { tuple<int> }

tuple<int> t (tuple<int> { 1 });
```

- 引入模板匹配错误 `SFINAE` + `is_convertible`
- 值转换方式声明为 `explicit`

---

## 构造函数

``` cpp
// only check T & Head here

template<typename T, typename ...Ts,
    typename = std::enable_if_t<
        std::is_convertible<T, Head>::value
    >
> explicit tuple (T &&arg, Ts &&...args) :
    Tail (std::forward<Ts> (args)...),
    _val (std::forward<T> (arg)) {}
```

---

## `tuple_element` & `get`

``` cpp
template<size_t I, typename Tuple>
struct tuple_element;

template<size_t I, typename ...Ts>
tuple_element<I, tuple<Ts ...>>::type &get (
    tuple<Ts ...> &);
// (const Tuple &) -> const Elem &
// (Tuple &&) -> Elem &&
// (const Tuple &&) -> const Elem &&
```

---

## `tuple_element` & `get`

``` cpp
template<size_t I, typename T, typename ...Ts>
struct tuple_element<I, tuple<T, Ts ...>>
    : tuple_element<I - 1, tuple<Ts ...>> {};

template<typename T, typename ...Ts>
struct tuple_element<0, tuple<T, Ts ...>> {
    using type = T;
    using _tuple = tuple<T, Ts ...>;
};
```

---

## `tuple_element` & `get`

``` cpp
template<size_t I, typename ...Ts>
tuple_element<I, tuple<Ts ...>>::type &get (
    tuple<Ts ...> &t)
{
    // resolve to base, and return head value
    return ((tuple_element<I, tuple<Ts ...>>
        ::_tuple &) t)._val;
}
```

---

## 更多内容

- `tuple_size`
- `operator==` / `operator<`
- `get` (by type)
- `swap`
- `make_tuple` / `tie` / `forward_as_tuple` / `tuple_cat`

#### 感悟

- 严谨的推导
- 语言工具的最小正交

---

## Q & A

👉 完整实现：[`tuple.h`](Inside-Cpp-Tuple/tuple.h)

👉 完整测试：[`test_tuple.cpp`](Inside-Cpp-Tuple/test_tuple.cpp)

<br />
<br />
<br />

[align-right]

# 谢谢 🙂

---
