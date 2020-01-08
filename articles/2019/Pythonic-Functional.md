# Python 风格的函数式

> 2019/11/23
> 
> Readability counts. —— The Zen of Python, by Tim Peters (`import this`)

现代编程语言之间 常常相互借鉴（例如 几乎所有语言都支持了 lambda 表达式/匿名函数/闭包），所以许多人会说：

> 学什么编程语言都一样，会一种就行。

但我 **不赞同** 这个观点 —— 我认为：用不同的语言写代码，就应该 “入乡随俗”，多领会各种语言的 **设计的艺术**。

> 三人行，必有我师焉；择其善者而从之，其不善者而改之。——《论语‧述而》

Python 为了提高 **可读性** _(readability)_，提供了很多 **语法糖** _(syntactic sugar)_，开创了别具一格的 **Python 风格** _(Pythonic)_ 的 **函数式编程** _(functional programming)_。

> 本文提到的所有概念 均可参考文中的 **链接**，实例代码包括了一些常见的 **Tricks**。😉

[TOC]

## 什么是 Pythonic (TL;DR)

举个例子，实现一个简单的需求：

- 按行打印 **当前脚本** 内容
- 去掉每行 **右侧空字符**
- 过滤所有 **空行**（去掉右侧空字符后）
- 打印时加上 **行号**（不包括空行）

**学习 Python 前**，凭感觉会这么写：

``` python
file = open(__file__)
try:
    index = 1
    while True:
        line = file.readline()
        if not line:
            break

        strip_line = line.rstrip()
        if len(strip_line) != 0:
            print('{:2}: {}'.format(index, strip_line))
            index += 1
finally:
    file.close()
```

**学习 Python 后**，只需要 3 行代码：

``` python
with open(__file__) as file:
    for index, line in enumerate(filter(len, map(str.rstrip, file)), 1):
        print(f'{index}: {line}')
```

- 安全的打开/关闭文件
  - 前：将 `close()` 写在 [`finally` 语句](https://docs.python.org/3/reference/compound_stmts.html#the-try-statement) 内，避免异常时泄露
  - 后：使用 [`with` 语句](https://docs.python.org/3/reference/compound_stmts.html#the-with-statement)（类似 [C++ 的 **资源获取即初始化** _(Resource Acquisition Is Initialization, RAII)_](https://en.cppreference.com/w/cpp/language/raii) 思想）
- 读取文件的每一行
  - 前：使用 `while` 循环调用 `readline()` 函数，直到读到 `None` 时结束
  - 后：使用 `for` 循环遍历 [**迭代器** _(iterator)_](https://docs.python.org/3/howto/functional.html#iterators) 获取结果
- 去掉空字符、过滤空行
  - 前：使用临时变量存储每行 `rstrip()` 的结果，使用 `if` 判断 `len()` 是否为空
  - 后：使用高阶函数 [`map()`](https://docs.python.org/3/library/functions.html#map)/[`filter()`](https://docs.python.org/3/library/functions.html#filter) 消除循环和临时变量（[参考《高阶函数：消除循环和临时变量》](../2018/Higher-Order-Functions.md)）
- 记录行号
  - 前：使用自增的临时变量存储（上述代码如果少一个缩进，结果就会不一样）
  - 后：使用 [`enumerate()` 函数](https://docs.python.org/3/library/functions.html#enumerate) 从 `1` 开始生成下标，并通过 [**可迭代解包** _(iterable unpacking)_](https://docs.python.org/3/reference/simple_stmts.html#assignment-statements) 直接得到 `index` 和 `line` 变量
- 格式化输出
  - 前：使用常规的 [`format()` 函数](https://docs.python.org/3/library/stdtypes.html#str.format)
  - 后：使用特有的 [f-string _(formatted string literal)_](https://docs.python.org/3/reference/lexical_analysis.html#f-strings) 化简

所以，什么是 Pythonic —— 用代码描述 **做什么** _(what-to-do)_，而不是 **怎么做** _(how-to-do)_ —— 提升可读性。

## 迭代器

什么是 [**迭代器** _(iterator)_](https://docs.python.org/3/library/stdtypes.html#iterator-types) —— 用于 **遍历容器中元素** 的对象，需要支持 [`next()` 函数](https://docs.python.org/3/library/functions.html#next)（即 对象实现 [`__next__()` 方法](https://docs.python.org/3/library/stdtypes.html#iterator.__next__)）：

- 如果仍有元素，返回容器中的下一个元素
- 如果迭代结束，抛出 [`StopIteration` 异常](https://docs.python.org/3/library/exceptions.html#StopIteration)

> Python 中的迭代器，类似于 [C++ 的 **输入迭代器** _(input iterator)_](https://en.cppreference.com/w/cpp/named_req/InputIterator) —— 每次只能读取一个元素，不能跨越，也不能后退，更不能随机访问。

迭代器在 Python 中随处可见，最常用于 `for` 循环：

``` python
for i in [1, 2, 3]:
    print(i)  # use(i)
```

如果用 `while` 循环，会比较冗长：

``` python
it = iter([1, 2, 3])
while True:
    try:
        i = next(it)
        print(i)  # use(i)
    except StopIteration:
        break
```

Python 提出了 [**可迭代** _(iterable)_](https://docs.python.org/3/glossary.html#term-iterable) 的概念，支持从 容器或迭代器 通过 [`iter()` 函数](https://docs.python.org/3/library/functions.html#iter) 返回迭代器（即 容器或迭代器实现 [`__iter__()` 方法](https://docs.python.org/3/reference/datamodel.html#object.__iter__)；而迭代器返回 `self`，即 `iter(it) == it`）。

## 高阶函数

普通迭代器 只能遍历容器的 **已有元素**；但在多数情况下，需要在迭代过程中，修改 **原始元素** 并构造出 **新的元素**。

在命令式编程中，常用 `for` 循环遍历已有元素，并用 临时变量 存储修改后的结果；而函数式编程中，常用 [**高阶函数** _(higher-order function)_](https://en.wikipedia.org/wiki/Higher-order_function) 消除循环和临时变量（[具体方法参考《高阶函数：消除循环和临时变量》](../2018/Higher-Order-Functions.md)）：

``` python
map(str.upper, ['aaa', 'bbb'])
# ['AAA', 'BBB']

filter(lambda x: x % 2, range(10))
# [1, 3, 5, 7, 9]

reduce(lambda d, s: dict(d, **{s: s.upper()}), ['aaa', 'bbb'], {})
# {'aaa': 'AAA', 'bbb': 'BBB'}  (Trick: construct dict)
```

> 注：
> 
> - 上述代码仅用于 Python 2（原因见下文）
> - Python 3 [移除了 `reduce` 内置函数](https://docs.python.org/3.0/whatsnew/3.0.html#builtins)，替换为 [`functools.reduce()`](https://docs.python.org/3/library/functools.html#functools.reduce)

[align-center]

[img=max-width:80%]

[![emoji 版本的 map/filter/reduce 的解释](../2018/Higher-Order-Functions/emoji-map-filter-reduce.png)](http://www.globalnerdy.com/2016/06/23/map-filter-and-reduce-explained-using-emoji/)

## 生成器

在 Python 2 中，内置的高阶函数 [`map()`](https://docs.python.org/2/library/functions.html#map)/[`filter()`](https://docs.python.org/2/library/functions.html#filter) 以及 [`zip()`](https://docs.python.org/2/library/functions.html#zip)/[`range()`](https://docs.python.org/2/library/functions.html#range) 会直接返回 [`list` 类型](https://docs.python.org/3/library/stdtypes.html#lists) 的结果。对于使用者来说，局限性非常大。

一方面，**无用的计算** 会带来 **额外的开销**：

- 例如，设计一个读取数据库的函数（表中有 1,000,000 行数据），通过 [`return cursor.fetchall()`](https://dev.mysql.com/doc/connector-python/en/connector-python-api-mysqlcursor-fetchall.html) 一次性返回所有数据
- 如果使用者 并不需要 所有数据，从数据库中读取了 不需要的部分，造成浪费
- 如果使用者 需要修改 部分数据，那么使用者还需要遍历 整个列表，非常耗时

``` python
def get_data():
    # ...
    return cursor.fetchall()

data = get_data()
# <list of 1,000,000 rows> (MemoryError)
```

另一方面，**不支持** 表示 **无穷的** _(potential infinite)_ 数据结构：

- 例如，表示一个从 0 到 [`sys.maxint`](https://docs.python.org/2/library/sys.html#sys.maxint) 的范围
- Python 2 中 [`range(sys.maxint)`](https://docs.python.org/2/library/functions.html#range) 会返回一个 **很长的列表**，占用大量内存
- Python 3 [移除了 `sys.maxint`，并支持无限大数值](https://docs.python.org/3.0/whatsnew/3.0.html#integers)，而构造出一个 **无限长的范围（列表）**，将无法存储在内存

``` python
range(sys.maxint)
# MemoryError
```

在函数式编程中，常用 [**惰性求值** _(lazy evaluation)_](https://en.wikipedia.org/wiki/Lazy_evaluation) 的方法解决上述问题。

Python 使用 [**生成器** _(generator)_](https://docs.python.org/3/library/stdtypes.html#generator-types) 实现惰性求值 —— 带有 [`yield` 表达式](https://docs.python.org/3/reference/expressions.html#yield-expressions) 的函数，按需生成并返回结果 —— 对外提供和迭代器相同的 `__iter__()`/`__next__()` 接口，实现 **可迭代** 的概念（对使用者透明的 [**鸭子类型** _(duck typing)_](https://en.wikipedia.org/wiki/Duck_typing) —— 无需关心是迭代器还是生成器）：

- 对于读取数据库的函数，可以将 [`return`](https://docs.python.org/3/reference/simple_stmts.html#the-return-statement) 改为 [`yield`](https://docs.python.org/3/reference/simple_stmts.html#the-yield-statement)，通过 [`yield cursor.fetchone()`](https://dev.mysql.com/doc/connector-python/en/connector-python-api-mysqlcursor-fetchone.html) 逐个返回结果
- 而使用者可以通过 和迭代器相同的方式（例如 `for` 循环），按需使用或修改数据

``` python
def get_data():
    # ...
    yield cursor.fetchone()

for row in get_data():
    print(row)
```

- Python 2 额外支持了 [`itertools.imap()`](https://docs.python.org/2/library/itertools.html#itertools.imap)/[`itertools.ifilter()`](https://docs.python.org/2/library/itertools.html#itertools.ifilter)/[`itertools.izip()`](https://docs.python.org/2/library/itertools.html#itertools.izip)/[`xrange()`](https://docs.python.org/2/library/functions.html#xrange) 用于替换内置函数：返回迭代器（生成器），而不是列表
- Python 3 直接修改了 [`map()`](https://docs.python.org/3/library/functions.html#map)/[`filter()`](https://docs.python.org/3/library/functions.html#filter)/[`zip()`](https://docs.python.org/3/library/functions.html#zip)/[`range()`](https://docs.python.org/3/library/functions.html#func-range) 等内置函数：[返回迭代器（生成器），而不是列表](https://docs.python.org/3.0/whatsnew/3.0.html#views-and-iterators-instead-of-lists)（取代了 Python 2 的 `itertools.i*()`/`xrange()` 函数）

``` python
range(sys.maxsize)
# range(0, 9223372036854775807)

for x in range(sys.maxsize):
    print(x)
# 0 1 2 3 ...

zip(*[[1, 2], [3, 4], [5, 6]])
# <zip object at 0x000001F9BCD2AB88>

list(zip(*[[1, 2], [3, 4], [5, 6]]))
# [(1, 3, 5), (2, 4, 6)]  (Trick: matrix transpose)
```

- Python 还提供了 [`itertools.count()`](https://docs.python.org/3/library/itertools.html#itertools.count)/[`itertools.cycle()`](https://docs.python.org/3/library/itertools.html#itertools.cycle)/[`itertools.repeat()`](https://docs.python.org/3/library/itertools.html#itertools.repeat) **无穷迭代器** _(infinite iterator)_（生成器），用于表示无穷的数据结构

``` python
dict(zip(itertools.count(), ['a', 'b', 'c']))
# {0: 'a', 1: 'b', 2: 'c'}  (Trick: enumerate)

list(itertools.repeat('{}', 3))
# ['{}', '{}', '{}']  (Trick: sequence repetition)
#   ['{}'] * 3 == ['{}', '{}', '{}']
#    '{}'  * 3 ==      '{}{}{}'
```

## 推导式

Python 为了化简 `map()`/`filter()` 的高阶函数写法，提供了 [类似 Haskell](https://wiki.haskell.org/List_comprehension) 的 [**列表/字典/集合 推导式** _(list/dict/set comprehensions)_ 和 **生成器表达式** _(generator expressions)_](https://docs.python.org/3/howto/functional.html#generator-expressions-and-list-comprehensions) 语法：

``` python
(s.upper() for s in ['aaa', 'bbb'])
# <generator object <genexpr> at 0x0000029CA8E65938>

[s.upper() for s in ['aaa', 'bbb']]
# ['AAA', 'BBB']

[x for x in range(10) if x % 2]
# [1, 3, 5, 7, 9]

{s: s.upper() for s in ['aaa', 'bbb']}
# {'aaa': 'AAA', 'bbb': 'BBB'}

{s.upper() for s in ['aaa', 'bbb']}
# {'AAA', 'BBB'}
```

- 一方面，可以使用 **生成器表达式** 快速构造 `map()`/`filter()` 等效的迭代器（生成器）
- 另一方面，可以使用 **推导式** 直接构造出 列表/字典/集合 对象

上边简单的例子看不出 推导式相对于高阶函数 的 **优势**，所以下边举一个求 0-100 之间所有 [毕达哥拉斯三元组](https://en.wikipedia.org/wiki/Pythagorean_triple) 的例子。

用 **命令式编程** 的直观方法是使用 **三层 for 循环** 实现：

``` python
ret = []
for x in range(1, 100):
    for y in range(1, 100):
        for z in range(1, 100):
            if x < y and x ** 2 + y ** 2 == z ** 2:
                ret.append((x, y, z))
# [(3, 4, 5), (5, 12, 13), ... (65, 72, 97)]
```

可以用 **笛卡尔积** _(cartesian product)_ [`itertools.product()`](https://docs.python.org/3/library/itertools.html#itertools.product) 化简三层循环：

``` python
ret = []
for x, y, z in itertools.product(range(1, 100), repeat=3):
    if x < y and x ** 2 + y ** 2 == z ** 2:
        ret.append((x, y, z))
```

然而，基于笛卡尔积的遍历会 **存在冗余**，可以根据三元组定义，优化 遍历顺序、迭代下标、判断条件：

``` python
ret = []
for z in range(1, 100):
    for x in range(1, z + 1):
        for y in range(x, z + 1):
            if x ** 2 + y ** 2 == z ** 2:
                ret.append((x, y, z))
```

进一步用 **函数式编程** 的高阶函数 `map()`/`filter()` 消除循环和临时变量：

- 通过 [`itertools.chain.from_iterable()`](https://docs.python.org/3/library/itertools.html#itertools.chain) 链接迭代器列表，实现 [`flatmap()`](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/flatMap)，将 二维数组 展开为 一维数组
- 通过嵌套闭包，传递上一轮迭代的元素

``` python
def flatmap(*args):
    return itertools.chain.from_iterable(map(*args))
list(filter(
    lambda t: t[0] ** 2 + t[1] ** 2 == t[2] ** 2,
    flatmap(lambda z:
            flatmap(lambda x:
                    map(lambda y:
                        (x, y, z),
                        range(x, z + 1)),
                    range(1, z + 1)),
            range(1, 100))))
```

最后用 [**嵌套列表推导式** _(nested list comprehensions)_](https://docs.python.org/3/tutorial/datastructures.html#nested-list-comprehensions) 实现 **优雅** _(elegant)_ 的函数式代码：

- 没有 **心智负担** _(cognitive load)_，不需要思考用 `map()` 还是 `flatmap()`
- 在保证 **高效**（惰性求值）的情况下，**可读性** 最佳（缩进最整齐的方案）

``` python
[(x, y, z) for z in range(1, 100)
           for x in range(1, z + 1)
           for y in range(x, z + 1)
           if x ** 2 + y ** 2 == z ** 2]
```

> 延伸阅读：
> 
> - [Python List Comprehensions: Explained Visually](https://treyhunner.com/2015/12/python-list-comprehensions-now-in-color/) —— 可视化解释：如何把 高阶函数 转换为 列表推导式
> - [Overusing list comprehensions and generator expressions in Python](https://treyhunner.com/2019/03/abusing-and-overusing-list-comprehensions-in-python/) —— 如何正确使用 列表推导式
> - ["Modern" C++ Lamentations](http://aras-p.info/blog/2018/12/28/Modern-C-Lamentations/) —— 使用 C++ 20 range 实现上述代码的问题（编译慢、运行慢、心智负担）
> - [The Surprising Limitations of C++ Ranges Beyond Trivial Cases](https://www.fluentcpp.com/2019/09/13/the-surprising-limitations-of-c-ranges-beyond-trivial-use-cases/) —— 如果你觉得高阶函数例子中，为了 _make it right_ 而引入的 `flatmap()` 很复杂；读完这篇文章，你会发现 C++ 里 _make it compile_ 的方式更复杂

## 最后聊聊 Python 语言

首先，虽然 Python 的可读性不错，但对 **其他语言用户** 的可写性不好（仁者见仁，智者见智）：

- `len(LIST)` 而不是 `LIST.length()`（参考：[Why does Python use methods for some functionality (e.g. list.index()) but functions for other (e.g. len(list))? | Design and History FAQ](https://docs.python.org/3/faq/design.html#why-does-python-use-methods-for-some-functionality-e-g-list-index-but-functions-for-other-e-g-len-list)）
- `STR.join(LIST)` 而不是 `LIST.join(STR)`（但 `LIST.split(STR)` 却是有的，参考：[Why is join() a string method instead of a list or tuple method? | Design and History FAQ](https://docs.python.org/3/faq/design.html#why-is-join-a-string-method-instead-of-a-list-or-tuple-method)）
- `COND ? EXPR1 : EXPR2` 三元运算符 写为 `EXPR1 if COND else EXPR`（参考：[Is there an equivalent of C’s “?:” ternary operator? | Programming FAQ](https://docs.python.org/3/faq/programming.html#is-there-an-equivalent-of-c-s-ternary-operator)）
- `strip()` 而不是 `trim()`（好在 Visual Studio 2019 智能提示，会自动把 `trim` 识别为 `strip`）
- `self` 而不是 `this` 😵

其次，作为一个 **非脚本语言用户**，离开了 **编译器的检查** 和 IDE 强大的 **智能提示**，感觉自己不会写代码了：

- Python 是 **运行时强类型** 语言（参考：[Strong versus Weak Typing _(A Conversation with Guido van Rossum)_](https://www.artima.com/intv/strongweak.html#part2)）
- 只有在 **运行时**，才能发现函数的参数（个数/类型）**错误**
- 另外，Python 2 的函数不能 [指定参数/返回值的类型](https://docs.python.org/3/library/typing.html)，IDE 的 **智能提示** 经常失效

> Life is short, you need Python. —— Bruce Eckel

最后，Python 的 **核心语言** _(core language)_ 还算 [比较简单](https://docs.python.org/3/reference/grammar.html)（反例：C++），很多概念都是 **良好定义** _(well-defined)_ 的 —— 只要理解基本原理，就能 **快速上手**（写本文时，我的 Python 代码量未超过 1,000 行）。

如果有什么问题，**欢迎交流**。😄

Delivered under MIT License &copy; 2019, BOT Man
