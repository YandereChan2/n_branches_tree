# `Yc::parent_aware_n_branches_tree<N, T, Allocator>::edge_proxy` , `Yc::parent_aware_n_branches_tree<N, T, Allocator>::edge_const_proxy`

指代一个 _树位置_ ，并提供对该 _树位置_ 上的 _N 叉树_ 保有的元素的访问。

它是 [_可默认构造 (DefaultConstructible)_](https://zh.cppreference.com/w/cpp/named_req/DefaultConstructible) 且 [_可平凡复制 (TriviallyCopyable)_](https://zh.cppreference.com/w/cpp/named_req/TriviallyCopyable) 的轻量代理对象。默认构造的代理无效（`valid() == false`），但可以比较、可以哈希。

`edge_proxy` 可以[隐式转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)到 `edge_const_proxy` ,反过来不行。

这种代理对象会在相应的 _树位置_ 失效时失效，长期保有并期间改变树结构可能导致令人困惑的结果。需要跨操作稳定指向某个节点时，转换成 [`node_proxy`](../node_proxy/node_proxy.md) 。

## 分支编号与尾后边

每条边记录"它属于哪个节点的第几个分支"，分支编号范围是 `0` 到 `N`：

- `0` 到 `N - 1` 是真实分支；
- `N` 是**尾后边**：它不是真实分支，**不能解引用**（`operator*`、`operator->`、以及任何取它所指元素的操作都是[未定义行为](https://zh.cppreference.com/w/cpp/language/ub)）；
- 尾后边仍然是**良定义的边**：可复制、可判等、可哈希，`valid()` 为 `true`、`null()` 为 `true`、[`is_end()`](#is_end) 为 `true`。因此它适合当兄弟遍历的哨位：

```C++
auto r = t.root();
for (auto e = r.get_brother(0); !e.is_end(); e.go_to_next_brother())
{
    // 依次访问第 0、1、…、N-1 个分支；空分支用 e.null() 判断
}
```

## 成员类型

|成员类型|定义|
|-:|:-|
|`value_type`|`T`|
|`reference`|`T&` 或 `const T&`|

## 成员函数

下文中用 `proxy` 表示具体是哪个类型。

### valid

```C++
bool valid()const noexcept;
```

该类型是否合法指代一个 _树位置_。只有默认构造的这种对象返回 `false`。

### null

```C++
bool null()const noexcept;
```

该类型指代的 _树位置_ 上是否没有 _N 叉树_。尾后边恒为 `true`。在 `valid() == false` 时调用该函数是[未定义行为](https://zh.cppreference.com/w/cpp/language/ub)。

### operator bool

```C++
explicit operator bool()const noexcept;
```

向 `bool` 的[按语境转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)：既有效、又指向真实节点时为 `true`（尾后边为 `false`）。

### index

```C++
size_t index()const noexcept;
```

返回分支编号，范围 `0` 到 `N`。

### is_end

```C++
bool is_end()const noexcept;
```

该边是否是尾后边（分支编号为 `N`）。

### is_first_brother, is_last_brother

```C++
bool is_first_brother()const noexcept;
bool is_last_brother()const noexcept;
```

该边是否是所在分支列表的第一个 / 最后一个**真实**分支（编号 `0` / `N - 1`）。

### operator*, operator->

```C++
T& operator*()const noexcept;
T* operator->()const noexcept;
```

访问其指代的 _N 叉树_ 保有的元素。边为空或尾后边时是[未定义行为](https://zh.cppreference.com/w/cpp/language/ub)。

### go_to_child

```C++
proxy& go_to_child(size_t i)noexcept;
```

移动到"当前边所指节点"的第 `i` 个分支的真实边。要求当前边指向真实节点、且 `i < N`，否则[未定义行为](https://zh.cppreference.com/w/cpp/language/ub)。返回 `*this` 以支持连续调用。

### go_to_brother, go_to_first_brother, go_to_last_brother

```C++
proxy& go_to_brother(size_t i)noexcept;
proxy& go_to_first_brother()noexcept;
proxy& go_to_last_brother()noexcept;
```

在**同一个父节点**的分支之间移动。`i` 可以是 `N`（走到尾后边）。返回 `*this`。

### go_to_prev_brother, go_to_next_brother

```C++
proxy& go_to_prev_brother()noexcept;
proxy& go_to_next_brother()noexcept;
```

移动到上一个 / 下一个分支。已经在第一个真实分支时 `go_to_prev_brother` 不动；已经在尾后边时 `go_to_next_brother` 不动。返回 `*this`。

### go_to_end

```C++
proxy& go_to_end()noexcept;
```

移动到同一父节点的尾后边。返回 `*this`。

### go_up

```C++
proxy& go_up()noexcept;
```

移动到"指向当前边父节点的边"。如果当前边本身就是根边（其父节点是哨位），则行为未定义：哨位只持有根槽位，没有 `parent` 字段。返回 `*this`。

复杂度与 `N` 成正比。

### get_child

```C++
proxy get_child(size_t i)const noexcept;
```

返回"当前边所指节点"的第 `i` 个分支的真实边，自身不动。要求当前边指向真实节点、`i < N`。

### get_brother, get_first_brother, get_last_brother, get_prev_brother, get_next_brother, get_end

```C++
proxy get_brother(size_t i)const noexcept;
proxy get_first_brother()const noexcept;
proxy get_last_brother()const noexcept;
proxy get_prev_brother()const noexcept;
proxy get_next_brother()const noexcept;
proxy get_end()const noexcept;
```

返回移动后的新代理，自身不动。语义分别与对应的 `go_to_*` 相同。

### get_up

```C++
proxy get_up()const noexcept;
```

返回向上移动后的新代理，自身不动。

### get_children

```C++
std::array<proxy, N> get_children()const noexcept;
```

一次取回"当前边所指节点"的 `N` 个真实分支的边，第 `i` 个元素等于 `get_child(i)`（不含尾后边）。要求当前边指向真实节点。

## 非成员函数

### operator==

```C++
bool operator==(proxy l, proxy r)noexcept;
```

当且仅当两条边相同（同一个父节点、同一个分支编号）或都无效时返回 `true`。

## 辅助类

### std::hash<edge_proxy>, std::hash<edge_const_proxy>

对代理对象[启用](https://zh.cppreference.com/w/cpp/utility/hash)的 [`std::hash`](https://zh.cppreference.com/w/cpp/utility/hash) 特化。相等的边哈希相同；无效边哈希为 `0`。尾后边是有效边，哈希非 `0`。

## 示例

```C++
#include "parent_aware_n_branches_tree.h"
#include <cstdio>

int main()
{
    Yc::parent_aware_n_branches_tree<3, int> t;
    auto r = t.root();
    t.emplace(r, 1);
    t.emplace(r.get_child(0), 2);
    t.emplace(r.get_child(2), 3);

    // 遍历根节点的 3 个分支：第 0 个是 2、第 1 个空、第 2 个是 3
    int n = 0;
    for (auto e = r.get_child(0).get_first_brother(); !e.is_end(); e.go_to_next_brother())
    {
        if (!e.null())
        {
            ++n;
        }
    }
    if (n != 2)
    {
        return 1;
    }
    // 尾后边：可比较、可哈希，但不能解引用
    auto end = r.get_brother(3);
    return (end.is_end() && end.null() && end.valid() && !static_cast<bool>(end)) ? 0 : 1;
}
```
