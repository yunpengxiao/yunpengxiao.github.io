# 高阶函数：消除循环和临时变量

> 2018/10/27
>
> 如何使用高阶函数，实现内部迭代，消除循环和临时变量，让代码更简洁、更清晰。

## 目录 [no-toc]

[TOC]

## 背景知识

[**高阶函数** (higher-order function)](https://en.wikipedia.org/wiki/Higher-order_function) 指的是满足下列条件之一的函数：

- 以函数作为参数
- 返回值是函数

[**迭代器模式** (iterator pattern)](../2017/Design-Patterns-Notes-3.md#Iterator) 分为两种：

- **外部迭代器** (external iterator) 提供 First/Next/IsDone/GetItem 的接口，让使用者在遍历集合时，取出各个元素，实现自己的操作
- **内部迭代器** (internal iterator) 类似于 [访问者模式 (iterator pattern)](../2017/Design-Patterns-Notes-3.md#Visitor)，提供 [依赖注入 (dependency injection)](https://en.wikipedia.org/wiki/Dependency_injection) 的接口，让使用者在遍历集合时，执行传入的操作

使用内部迭代器，可以消除 **显式的循环** 和 **迭代器临时变量**。

## 使用 `every/some` (`all/any`) 合并谓词

很多时候，我们需要对一个序列的各个元素检查同一个 **谓词** (predicate)。例如：

- 检查 **每个元素** 是否 **都** 满足一个条件
- 检查是否 **存在某些元素** 满足一个条件

设这组元素序列为 $S$，谓词为 $P$，这两个操作用离散数学可以表述为：

- $every: \forall{x \in S}, P(x)$
- $some: \exists{x \in S}, P(x)$

对于这两种检查，我们可以用高阶函数表示。

> 数学描述：
>
> $$(E \rightarrow boolean) \times [E] \rightarrow boolean$$
>
> 编程语言实现：
>
> - JavaScript：[`Array.prototype.every`](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/every)/[`Array.prototype.some`](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/some)
> - C++：[`std::any_of`/`std::all_of`](https://en.cppreference.com/w/cpp/algorithm/all_any_none_of)

### 例子：检查日期

检查的目标：

- 检查一天是否为 周一、周三、周五、周日 中的某一天
- 由于不同人配置方法不一样，周日既可以表示为 `0`，也可以表示为 `7`

对应输入的配置：

``` json
{
  "avaliable_days": [ 1, 3, 5, 7 ]
}
```

使用 `for` 实现的 JavaScript 代码（**外部迭代**）：

``` js
function isDayAvaliable(avaliable_days, day_now) {
  for (const day of avaliable_days) {
    if (day % 7 === day_now)
      return true;
  }
  return false;
}
```

使用高阶函数 `some` 化简的 JavaScript 代码（**内部迭代**）：

``` js
function isDayAvaliable(avaliable_days, day_now) {
  return avaliable_days.some(day => day % 7 === day_now);
}
```

相比循环实现的代码，使用高阶函数一方面缩短代码的长度，另一方面可读性更好 —— 代码的风格更接近于自然语言：检查 `avaliable_days` 中是否存在 `some` `day`，满足和 `day_now` 表示同一天的 。

## 使用 `map` 映射结果

图形描述：

[align-center]

[img=max-width:50%]

![Map](Higher-Order-Functions/map.png)

[align-center]

[map](https://atendesigngroup.com/blog/array-map-filter-and-reduce-js) - [source](https://atendesigngroup.com/sites/default/files/array-map.png)

有时候，我们需要对一个序列的各个元素进行同一个 **转换** (transform)。例如：上边图片里，给定 3 个水果，把每个水果都进行切片转换，得到 3 个水果切片。操作的特点是：

- 转换后结果的 **个数** 和转换前 **相同**（都是三个）
- 转换前后元素 **类型** **不一定相同**（从水果到切片）

> 数学描述：
>
> $$(E \rightarrow F) \times [E] \rightarrow [F]$$
>
> 编程语言实现：
>
> - JavaScript：[`Array.prototype.map`](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/map)
> - C++：[`std::transform`](https://en.cppreference.com/w/cpp/algorithm/transform)
>
> [Wikipedia 描述](https://en.wikipedia.org/wiki/Map_%28higher-order_function%29)

### 例子：稳定排序

JavaScript 里，[`Array.prototype.sort`](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/sort) 函数不保证 [排序的稳定性](https://en.wikipedia.org/wiki/Sorting_algorithm#Stability)；而在很多业务场景下，要求排序是稳定的。例如：有一个比赛名单，上面记录了每个人的获胜次数；现在需要选出获胜次数最少的若干个人，获胜次数相同则优先选择名单顺序靠前的人。

假设给定输入：

``` json
[
  {
    "id": "john",
    "win": 2
  },
  {
    "id": "jill",
    "win": 1
  },
  {
    "id": "jack",
    "win": 3
  },
  {
    "id": "jess",
    "win": 2
  }
]
```

根据上述规则，如果要选两个人，我们应该选择 `john` 和 `jill`，而不会选择 `jill` 和 `jess`。

我们可以利用（不稳定排序 + 下标排序）构造一个复合排序，实现稳定排序。

> 代码参考：[Fast stable sorting algorithm implementation in javascript (@stackoverflow)](https://stackoverflow.com/questions/1427608/fast-stable-sorting-algorithm-implementation-in-javascript/48660568#48660568)

``` js
// [ y1, y2, x, y3 ] => [ x, y1, y2, y3 ]
function stableSort(array, compare) {
  return array
    .map((item, index) => ({ item, index }))
    .sort((a, b) => compare(a.item, b.item) || a.index - b.index)
    .map(({ item }) => item);
}
```

- 首先，将每个元素连同其下标组合成 <元素，下标> 序对
- 然后，根据元素的比较函数作为排序依据；如果比较相等，那么就优先排列下标靠前的元素
- 最后，从排好序的 <元素，下标> 序对中取出元素，从而得到结果

### 例子：抓取页面

[cheerio.js](https://cheerio.js.org/) 提供了类似 jQuery 的解析 DOM 树接口，可以用于抓取页面元素。对于每个 DOM 节点的解析结果，都可以从父节点处，使用两种方式访问：

- **外部迭代** `for (const child of elem.children()) ...` 遍历节点 `elem` 的所有子节点，执行需要的操作
- **内部迭代** `elem.map((index, child) => { ... })` 遍历节点 `elem` 的子节点，执行函数，并将所有返回的结果构造为数组

假设抓取的页面格式：

``` html
<div class="myclass">
  <p>section1</p>
  <ul>
    <li>Please visit <a href="http://github.com/a">website1</a>!</li>
    <li>Please visit <a href="http://github.com/b">website2</a>!</li>
  </ul>
  <p>section2</p>
  <ul>
    <li>Please visit <a href="http://github.com/c">website3</a>!</li>
    <li>Please visit <a href="http://github.com/d">website4</a>!</li>
  </ul>
</div>
```

对应抓取的目标结果：

``` json
[
  {
    "titles": [ "website1", "website2" ],
    "links": [ "http://github.com/a", "http://github.com/b" ]
  },
  {
    "titles": [ "website3", "website4" ],
    "links": [ "http://github.com/c", "http://github.com/d" ]
  }
]
```

使用 `for` 实现的代码（**外部迭代**）：

``` js
const result = [];

for (cosnt e_ul of $('.myclass ul').children()) {
  const titles = [];
  const links = [];

  for (const e_a of $('li a', e_ul).children()) {
    const title = $(e_a).text();
    titles.push(title);

    const link = $(e_a).attr('href');
    links.push(link);
  }

  result.push({ titles, links });
}
```

- 需要使用两层 for 循环
- 需要使用 `titles`/`links` 变量保存中间结果

使用 `map` 抓取的代码（**内部迭代**）：

``` js
const result = $('.myclass ul').map(
  (i_ul, e_ul) => ({
    titles: $('li a', e_ul).map(
      (i_a, e_a) => $(e_a).text()
    ),
    links: $('li a', e_ul).map(
      (i_a, e_a) => $(e_a).attr('href')
    )
  })
);
```

这里可以通过高阶函数嵌套的方法，有效的消除循环和临时变量，并更直观的展现出输入/输出的数据结构。例如，把 `() =>` 删掉，把 `$().map(...)` 替换成 `[...]`，我们就可以得到下面的结果（这不就是我们的抓取目标格式吗）：

``` js
result = [
  {
    titles: [
      text()
    ],
    links: [
      attr('href')
    ]
  }
]
```

## 使用 `filter` 过滤结果

图形描述：

[align-center]

[img=max-width:50%]

![Filter](Higher-Order-Functions/filter.png)

[align-center]

[filter](https://atendesigngroup.com/blog/array-map-filter-and-reduce-js) - [source](https://atendesigngroup.com/sites/default/files/array-filter.png)

有时候，我们需要对一个序列的各个元素按照一个规则进行 **筛选** (filter)。例如：上边图片里，给定 3 个水果，筛选球形的水果，得到 1 个水果（橙子）。操作的特点是：

- 筛选后结果的 **个数 ≤** 筛选前的元素总数（1 ≤ 3）
- 筛选前后元素 **类型** **相同**（都是水果）

> 数学描述：
>
> $$(E \rightarrow boolean) \times [E] \rightarrow [E]'$$
>
> 编程语言实现：
>
> - JavaScript：[`Array.prototype.filter`](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/filter)
> - C++：
>   - 拷贝过滤：[`std::copy_if`](https://en.cppreference.com/w/cpp/algorithm/copy)
>   - 移除过滤：[`std::remove_if`](https://en.cppreference.com/w/cpp/algorithm/remove) + [erase–remove idiom](https://en.wikipedia.org/wiki/Erase%E2%80%93remove_idiom)
>
> [Wikipedia 描述](https://en.wikipedia.org/wiki/Filter_%28higher-order_function%29)

### 例子：数组去重

``` js
// [ x, y, y ] => [ x, y ]
function uniqueArray(array) {
  return array.filter((elem, i) => array.indexOf(elem) === i);
}
```

- 使用 `indexOf` 检查 `elem` 首次出现的下标
- 使用 `filter` 过滤出下标和当前 `elem` 下标一致的元素

## 使用 `reduce` (`fold`) 规约结果

图形描述：

[align-center]

[img=max-width:50%]

![Reduce](Higher-Order-Functions/reduce.png)

[align-center]

[reduce](https://atendesigngroup.com/blog/array-map-filter-and-reduce-js) - [source](https://atendesigngroup.com/sites/default/files/array-reduce.png)

有时候，我们需要对一个序列的各个元素按照一个规则先进行 **加工** (process)，然后 **累加** (accumulate) 起来。例如：上边图片里，给定 3 个水果，我们先提供一个空碗，每一步把水果切成小块并放入碗里，最后得到一碗水果块。操作的特点是：

- 规约完成后，最终只得到 **一个结果**（得到一碗水果块）
- 规约得到 **结果和初始值** **类型相同**（都是一碗东西）

> 数学描述：
>
> $$(E \times F \rightarrow F) \times [E] \times F \rightarrow F'$$
>
> 编程语言实现：
>
> - JavaScript：
>   - 最左规约：[`Array.prototype.reduce`](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/reduce)
>   - 最右规约：[`Array.prototype.reduceRight`](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/reduceRight)
> - C++：
>   - 最左规约：[`std::accumulate`](https://en.cppreference.com/w/cpp/algorithm/accumulate)
>   - 乱序规约：[`std::reduce`](https://en.cppreference.com/w/cpp/algorithm/reduce)
>
> [Wikipedia 描述](https://en.wikipedia.org/wiki/Fold_%28higher-order_function%29)

### 例子：数组扁平化

``` js
// [ [x, y], [z] ] => [ x, y, z ]
function flatArray(array) {
  return array.reduce((acc, elem) => acc.concat(elem), []);
}
```

- 首先，规约的初始值是一个空数组，作为 `acc` 进行规约
- 然后，每次从输入数组 `array` 中取出一个元素 `elem`，和数组 `acc` 合并，并作为下一轮规约的 `acc`
- 最后，得到数组 `acc` 作为规约的结果

## 写在最后

说几点自己的想法：

- 对于需要遍历元素的使用者来说：
  - 外部迭代 需要关心集合元素的 **存储实现**，再进行具体的访问操作
  - 内部迭代 只需要关心 **如何操作** 各个元素，不需要关心有多少元素、元素如何存储
- 变量主要是用来存储状态，而如果系统中存在过多的零碎的状态，很难保持各个状态的一致性，所以尽可能：
  - 减少变量的使用：将与当前上下文无关的临时变量封装到新的函数里（小范围可以构造匿名函数，大范围可以提成公共函数）
  - 缩小变量的作用域：使用高阶函数，将临时变量的作用域限制在传入高阶函数的参数函数里，每次执行不会相互干扰
- `every`/`some`/`map`/`filter` 都可以用 `reduce` 实现
- C++ 是强类型编译语言，不同序列的 **类型不同**，所以标准库只能借助 **泛型** 实现高阶函数；JavaScript 是弱类型脚本语言，可以使用 `Array` 存储 **任意序列**，所以可以把这些高阶函数作为 **`Array` 的原型方法**
  - [C++ 版本 demo](Higher-Order-Functions/Higher-Order-Functions.cpp)
  - [JavaScript 版本 demo](Higher-Order-Functions/Higher-Order-Functions.js)

> C++ 库 [range-v3](https://github.com/ericniebler/range-v3) 利用了现代 C++ 的模板技巧，提供一种更方便的函数式编程方法
>
> [例如](http://www.modernescpp.com/index.php/the-new-ranges-library)，找出 **小于 1000 的 [完全平方](https://en.wikipedia.org/wiki/Square_number) 奇数**：
>
> ``` cpp
> auto oddSquaresLessThan1000 =
>     view::ints(1) |
>     view::transform([] (int i) { return i * i; }) |
>     view::remove_if([] (int i) { return i % 2 == 0; }) |
>     view::take_while([] (int i) { return i < 1000; });
> 
> ranges::for_each(oddSquaresLessThan1000,
>                  [] (int i) { std::cout << i << " "; });
> 
> // output:
> // 1 9 25 49 81 121 169 225 289 361 441 529 625 729 841 961
> ```
>
> [再如](https://github.com/ericniebler/range-v3/blob/master/example/comprehensions.cpp)，求 [毕达哥拉斯三元组](https://en.wikipedia.org/wiki/Pythagorean_triple)：
>
> ``` cpp
> auto triples = view::for_each(view::ints(1), [] (int z) {
>   return view::for_each(view::ints(1, z + 1), [=] (int x) {
>     return view::for_each(view::ints(x, z + 1), [=] (int y) {
>       return yield_if(x * x + y * y == z * z,
>                       std::make_tuple(x, y, z));
>     });
>   });
> });
> 
> // output:
> // (3,4,5)
> // (6,8,10)
> // (5,12,13)
> // (9,12,15)
> // ...
> ```
>
> 不过，C++ range 也存在许多问题，参考：[现代 C++ 的哀歌](http://aras-p.info/blog/2018/12/28/Modern-C-Lamentations/) —— 使用 C++ 20 range 实现上述代码的问题（编译慢、运行慢、心智负担）

最后补上一张 [emoji 版本的 map/filter/reduce 的解释](http://www.globalnerdy.com/2016/06/23/map-filter-and-reduce-explained-using-emoji/)：

[align-center]

[img=max-width:50%]

![Emoji Map Filter Reduce](Higher-Order-Functions/emoji-map-filter-reduce.png)

> 延伸阅读：
>
> - [Simplify your JavaScript – Use .map(), .reduce(), and .filter() | Medium](https://medium.com/poka-techblog/simplify-your-javascript-use-map-reduce-and-filter-bd02c593cc2d)
> - [Reading 25: Map, Filter, Reduce | MIT 6.005](http://web.mit.edu/6.005/www/fa15/classes/25-map-filter-reduce/)
> - [Functional-Light JavaScript (FLJSBook) | Github](https://github.com/getify/Functional-Light-JS)

如果有什么问题，**欢迎交流**。😄

Delivered under MIT License &copy; 2018, BOT Man
