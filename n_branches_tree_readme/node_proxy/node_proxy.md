# `Yc::n_branches_tree<N, T, Allocator>::node_proxy` , `Yc::n_branches_tree<N, T, Allocator>::node_const_proxy`

指代一棵 _N 叉树_ （一个节点本身），并提供对其保有的元素的访问。

它是 [_可默认构造 (DefaultConstructible)_](https://zh.cppreference.com/w/cpp/named_req/DefaultConstructible) 且 [_可平凡复制 (TriviallyCopyable)_](https://zh.cppreference.com/w/cpp/named_req/TriviallyCopyable) 的轻量代理对象。

与 [`edge_proxy`](../edge_proxy/edge_proxy.md) 的区别：

- 节点代理指代**节点本身**，不带"这是第几个分支"的位置信息，所以不能横向游走到兄弟；
- 节点代理不会因为树上的位置调整而改变指代对象（还是那个节点），因此比边代理稳定；
- 节点代理可以由边代理[隐式转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)而来，反过来不行；
- 本库不提供向上导航，所以节点代理**没有** `go_up` 一类的接口。

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
proxy(edge_proxy p)noexcept;      // 仅 node_proxy
```

```C++
proxy(node_proxy p)noexcept;      // 仅 node_const_proxy，隐式转换
```

可以从边代理构造：指代同一个节点。默认构造的节点代理指代空。

### null

```C++
bool null()const noexcept;
```

该类型指代的节点是否为空（默认构造的代理、以及由空 _树位置_ 转换而来的代理都返回 `true` ）。

### operator bool

```C++
explicit operator bool()const noexcept;
```

向 `bool` 的[按语境转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)，只有指代真实节点时返回 `true` 。等价于 `!null()` 。

### operator*, operator->

```C++
T& operator*()const noexcept;
```

```C++
T* operator->()const noexcept;
```

运算符重载，以提供访问其指代的 _N 叉树_ 所保有的元素的能力。如果其不[按语境转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)成 `true` ，那么行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

### go_to_child

```C++
proxy& go_to_child(size_t i)noexcept;
```

把代理对象转移到其指代的节点的第 `i` 个分支上挂着的节点。如果其不[按语境转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)成 `true` ，那么行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。前置条件 `i < N` 。返回 `*this` 以支持连续调用。

### get_child

```C++
proxy get_child(size_t i)const noexcept;
```

返回指代其指代的节点的第 `i` 个分支上挂着的节点的新代理对象，自身不动。前置条件 `i < N` 。

### get_children

```C++
std::array<edge_proxy, N> get_children()const noexcept;
```

一次取回 `N` 个分支 _树位置_ 的**边代理**（注意不是节点代理），第 `i` 个元素等于从本节点出发的第 `i` 条边。

### operator==

```C++
bool operator==(proxy l, proxy r)noexcept;
```

当且仅当指代同一个节点（或都为空）时返回 `true` 。

## 辅助类

### std::hash<node_proxy>,std::hash<node_const_proxy>

对于代理对象[启用](https://zh.cppreference.com/w/cpp/utility/hash)的 [`std::hash`](https://zh.cppreference.com/w/cpp/utility/hash) 特化。保证对相等的 `node_proxy` 与 `node_const_proxy` 返回相同值；空代理哈希为 `0` 。
