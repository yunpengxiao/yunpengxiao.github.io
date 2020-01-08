# C++ 11 右值引用总结

> BOT Man, 2018/8/10
>
> 你想知道的：右值引用/移动语义/拷贝省略/通用引用/完美转发

[slide-mode]

---

### 你想知道的：

- [sec|右值引用]
- [sec|移动语义]
- [sec|拷贝省略]
- [sec|通用引用]
- [sec|完美转发]

---

### 错误：返回前，移动临时值

``` cpp
void fn() {
  std::string base_url  = tag->GetBaseUrl();
  if (!base_url.empty())
    UpdateQueryUrl(std::move(base_url) + "&q=" + word_);
  else
    UpdateQueryUrl(default_base_url_ + "&q=" + word_);
  // WARNING: can NOT use `base_url`...
}
```

---

### 修改：返回前，移动临时值

``` cpp
std::string GetQueryUrl(const Tag* tag) {
  ASSERT(tag);
  std::string base_url  = tag->GetBaseUrl();
  if (!base_url.empty())
    return std::move(base_url) + "&q=" + word_;
  else
    return default_base_url_ + "&q=" + word_;
}
UpdateQueryUrl(GetQueryUrlString(tag));
```

---

### 误解：返回时，不移动右值引用参数

``` cpp
using Ptr = std::unique_ptr<int>;
Ptr fn(Ptr&& val) {
  //...
  return val;  // not compile
               // -> return std::move(val);
}
```

- 错误：`unique_ptr(const unique_ptr&) = delete`
- 传入的 `Ptr&&` 在函数体内变成 `Ptr&`

---

### 误解：返回时，移动临时值

``` cpp
using Ptr = std::unique_ptr<int>;
Ptr fn() {
  Ptr ret = std::make_unique<int>(64);
  //...
  return std::move(ret);  // -> return ret;
}
```

- 没必要使用 `std::move` 移动返回的临时值
- 返回的 `ret` 是一个右值，`unique_ptr(unique_ptr&&)`

---

### 右值引用

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

---

### 右值引用

- 左值：取地址、赋值 / 右值：临时对象
- 引用：左值引用 `&`/ 右值引用 `&&`
  - `&&` 初始化后，退化成 `&`（可 取地址、赋值）
  - [sec|误解：返回时，不移动右值引用参数]
- 另外，常引用：

``` cpp
void f(const Data& data);  // data is c-ref

f(data);    // ok, data is lvalue
f(Data{});  // ok, data is rvalue
```

---

### 移动语义

``` cpp
T _new = Getter(); // Getter { return _old; }
Setter(_old);      // Setter(T _new) { ... }
```

转移 临时对象 内的资源（指针、句柄）：

- 拷贝语义：先复制到 _new，再删除 _old
- 移动语义：直接从 _old 移动到 _new
- 拷贝省略：编译器优化（非标准）

---

### 用途：避免先复制再释放资源 1

``` cpp
template<typename T> class vector {
 public:
  vector(const vector& rhs); // copy data
  vector(vector&& rhs);      // move data
 private:
  T* data_;
  size_t size_;
};
```

- 两个构造函数

---

### 用途：避免先复制再释放资源 1

``` cpp
vector::vector(const vector& rhs) {
  this->size_ = rhs.size_;
  this->data_ = new T[rhs.size_];
  memcpy(this->data_, rhs.data, rhs.size_);
}
```

参数为左值时，拷贝构造：

- 新对象使用 `memcpy` 拷贝原对象的内存
- 原对象不变

---

### 用途：避免先复制再释放资源 1

``` cpp
vector::vector(vector&& rhs) {
  this->size_ = rhs.size_;
  this->data_ = rhs.data_;  // move data
  rhs.size_ = 0;
  rhs.data_ = nullptr;      // set to null
}
```

参数为右值时，移动构造：

- 把原对象的 `data_`/`size_` 复制到新对象
- 把原对象的 `data_`/`size_` 置 `0`

---

### 用途：避免先复制再释放资源 2

``` cpp
class Foo {
 public:
  T& data() & { return d_; }        // ref
  T data() && { return move(d_); }  // move
};

T ret1 = foo.data();    // lvalue, ref & copy
T ret2 = Foo{}.data();  // rvalue, move
```

---

### 用途：转移不可复制的资源

``` cpp
template<typename T> class unique_ptr {
 public:
  unique_ptr(const unique_ptr& rhs) = delete;
  unique_ptr(unique_ptr&& rhs);  // move only
 private:
  T* data_;
};
```

- 仅有移动构造函数

---

### 用途：转移不可复制的资源

``` cpp
unique_ptr::unique_ptr(unique_ptr&& rhs) {
  this->data_ = rhs.data_;
  rhs.data_ = nullptr;
}
```

- 移动旧对象资源
- 置空旧对象指针

---

### 反例：不遵守移动语义

``` cpp
bad_vector::bad_vector(bad_vector&& rhs) {
  this->size_ = rhs.size_;
  this->data_ = new T[rhs.size_];
  memcpy(this->data_, rhs.data, rhs.size_);
}
```

- `bad_vector` 对象在被 `move` 移动后仍然可用

---

### 拷贝省略

| 移动语义 | 拷贝省略 |
|---------|---------|
| 语言标准 | 编译器优化 |
| 编写 移动构造函数、右值限定成员函数 | 编译器 跳过移动/拷贝构造函数 |
| 代码逻辑上 转移 对象内 资源 | 编译器 直接移动 整个对象 内存 |

---

### 拷贝省略

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
 
void g(Data arg);

Data v = f();     // copy elision from prvalue (C++ 17)
g(f());           // copy elision from prvalue (C++ 17)
```

构造左值、构造参数时，传入的纯右值可以确保被优化；而返回值不保证被优化。

---

### 通用引用

<br />

[通用引用](Cpp-Rvalue-Reference.md#通用引用)

---

### 完美转发

<br />

[完美转发](Cpp-Rvalue-Reference.md#完美转发)

---

### Q & A



<br />
<br />
<br />

[align-right]

# 谢谢 🙂

---
