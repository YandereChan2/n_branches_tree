# `Yc::parent_aware_n_branches_tree<N, T, Allocator>::node_proxy` , `Yc::parent_aware_n_branches_tree<N, T, Allocator>::node_const_proxy`

指代一个节点（一棵 _N 叉树_ ），并提供对其保有的元素的访问。

它是 [_可默认构造 (DefaultConstructible)_](https://zh.cppreference.com/w/cpp/named_req/DefaultConstructible) 且 [_可平凡复制 (TriviallyCopyable)_](https://zh.cppreference.com/w/cpp/named_req/TriviallyCopyable) 的轻量代理对象。

与 [`edge_proxy`](../edge_proxy/edge_proxy.md) 的区别：

- 节点代理指代节点本身，不带"第几个分支"的信息，所以不能在兄弟之间移动；
- 节点代理不随位置调整而改变指代对象，因此比边代理稳定；
- 可以由边代理[隐式转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)而来（尾后边、空边转换成空代理），反过来不行；
- 有父节点信息，所以可以向上（[`go_up`](#go_up)）与向下（[`go_to_child`](#go_to_child)）移动。

`node_proxy` 可以[隐式转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)到 `node_const_proxy` ,反过来不行。

## 成员类型

|成员类型|定义|
|-:|:-|
|`value_type`|`T`|
|`reference`|`T&` 或 `const T&`|

## 成员函数

下文中用 `proxy` 表示具体是哪个类型。

### （构造函数）

```C++
proxy() = default;
```

```C++
proxy(edge_proxy p)noexcept;      // 仅 node_proxy：取该边指向的节点
```

```C++
proxy(node_proxy p)noexcept;      // 仅 node_const_proxy：隐式转换
```

默认构造的节点代理为空。

### null

```C++
bool null()const noexcept;
```

该类型是否指代空（默认构造、由空边或尾后边转换而来的代理都返回 `true`）。

### operator bool

```C++
explicit operator bool()const noexcept;
```

向 `bool` 的[按语境转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)，指代真实节点时为 `true`。

### operator*, operator->

```C++
T& operator*()const noexcept;
T* operator->()const noexcept;
```

访问元素。代理为空时是[未定义行为](https://zh.cppreference.com/w/cpp/language/ub)。

### go_to_child

```C++
proxy& go_to_child(size_t i)noexcept;
```

移动到其指代节点的第 `i` 个分支上挂着的节点。要求代理非空、且 `i < N`。返回 `*this`。

### go_up

```C++
proxy& go_up()noexcept;
```

移动到其指代节点的父节点（根节点的父节点是哨位节点，见 [`parent_of_nroot`](../parent_of_nroot.md)）。代理由哨位节点再向上时变为空。返回 `*this`。

复杂度与 `N` 成正比。

### get_child

```C++
proxy get_child(size_t i)const noexcept;
```

返回其指代节点的第 `i` 个分支上挂着的节点的新代理，自身不动。要求 `i < N`。

### get_parent

```C++
proxy get_parent()const noexcept;
```

返回其指代节点的父节点的新代理，自身不动。

### get_children

```C++
std::array<edge_proxy, N> get_children()const noexcept;
```

返回其指代节点的 `N` 个真实分支的**边**代理（不是节点代理），第 `i` 个元素等于从该节点出发的第 `i` 条边。

## 非成员函数

### operator==

```C++
bool operator==(proxy l, proxy r)noexcept;
```

当且仅当指代同一个节点（或都为空）时返回 `true`。

## 辅助类

### std::hash<node_proxy>, std::hash<node_const_proxy>

对代理对象[启用](https://zh.cppreference.com/w/cpp/utility/hash)的 [`std::hash`](https://zh.cppreference.com/w/cpp/utility/hash) 特化；空代理哈希为 `0`。

## 示例

```C++
#include "parent_aware_n_branches_tree.h"

int main()
{
    Yc::parent_aware_n_branches_tree<3, int> t;
    auto r = t.root();
    t.emplace(r, 1);
    t.emplace(r.get_child(0), 2);

    Yc::parent_aware_n_branches_tree<3, int>::node_proxy n = t.nroot();
    if (n.null() || *n != 1)
    {
        return 1;
    }
    n.go_to_child(0);                      // 到值 2 的节点
    if (*n != 2 || *n.get_parent() != 1)   // 再回到父节点
    {
        return 2;
    }
    return t.nroot().get_children()[0] == t.root().get_child(0) ? 0 : 3;
}
```
