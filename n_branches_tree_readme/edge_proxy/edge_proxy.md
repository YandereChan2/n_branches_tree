# `Yc::n_branches_tree<N, T, Allocator>::edge_proxy` , `Yc::n_branches_tree<N, T, Allocator>::edge_const_proxy`

指代一个 _树位置_ ，并提供对该 _树位置_ 上的 _N 叉树_ 保有的元素的访问。

它是 [_可默认构造 (DefaultConstructible)_](https://zh.cppreference.com/w/cpp/named_req/DefaultConstructible) 且 [_可平凡复制 (TriviallyCopyable)_](https://zh.cppreference.com/w/cpp/named_req/TriviallyCopyable) 的轻量代理对象。

`edge_proxy` 可以[隐式转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)到 `edge_const_proxy` ,反过来不行。

这种代理对象会在相应的 _树位置_ 失效时失效，这种失效规则更加复杂。长期保有这种代理并且期间树的结构有变动时可能会导致令人困惑的结果。但是各种树操作依赖这种代理对象。如果希望得到更稳定的元素代理，请转换成 [`node_proxy`](../node_proxy/node_proxy.md) 或 [`node_const_proxy`](../node_proxy/node_proxy.md) 。

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

该类型是否合法指代一个 _树位置_ 。只有默认构造的这种对象返回 `false` 。**注意，这里的**_**合法**_**和上文的**_**失效**_**不是一回事。**

### null

```C++
bool null()const noexcept;
```

该类型指代的 _树位置_ 上面是否没有 _N 叉树_ 。注意，在 `valid() == false` 时调用该函数会引发[未定义行为](https://zh.cppreference.com/w/cpp/language/ub)。

### operator bool

```C++
explicit operator bool()const noexcept;
```

向 `bool` 的[按语境转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)，只有在既有效又在所指代的 _树位置_ 上有 _N 叉树_ 的时候返回 `true` 。

#### 可能的实现

```C++
explicit operator bool()const noexcept
{
    return valid() && (!null());
}
```

### go_to_child

```C++
proxy& go_to_child(size_t i)noexcept;
```

把代理对象**纵向**转移到其指代的 _树位置_ 上的 _N 叉树_ 的第 `i` 个分支 _树位置_ 上。如果其不[按语境转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)成 `true` ，那么行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

前置条件：`i < N` 。越界时行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

返回 `*this` 以支持连续调用。

### go_to_next_brother, go_to_prev_brother

```C++
proxy& go_to_next_brother()noexcept;
```

```C++
proxy& go_to_prev_brother()noexcept;
```

```C++
proxy& go_to_next_brother(ptrdiff_t n)noexcept;
```

```C++
proxy& go_to_prev_brother(ptrdiff_t n)noexcept;
```

把代理对象**横向**转移到其指代的 _树位置_ 所在节点里的下一个 / 上一个分支 _树位置_ 上。带 `n` 的重载一次移动 `n` 个分支，`n` 可以为负，负号表示换方向。

前置条件：不许越过当前节点的分支范围（`N` 个分支），否则行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

横向移动**不依赖当前 _树位置_ 是否为空**：当前分支空着也可以照常走到兄弟分支上去（走到的那个分支可能也是空的）。

返回 `*this` 以支持连续调用。

### operator*, operator->

```C++
T& operator*()const noexcept;
```

```C++
T* operator->()const noexcept;
```

运算符重载，以提供访问其指代的 _树位置_ 上的 _N 叉树_ 所保有的元素的能力。如果其不[按语境转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)成 `true` ，那么行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

### get_child

```C++
proxy get_child(size_t i)const noexcept;
```

返回指代其指代的 _树位置_ 上的 _N 叉树_ 的第 `i` 个分支 _树位置_ 的新代理对象。自身不动。前置条件与 `go_to_child` 相同。

### get_next_brother, get_prev_brother

```C++
proxy get_next_brother()const noexcept;
```

```C++
proxy get_prev_brother()const noexcept;
```

```C++
proxy get_next_brother(ptrdiff_t n)const noexcept;
```

```C++
proxy get_prev_brother(ptrdiff_t n)const noexcept;
```

返回横向移动后的新代理对象，自身不动。前置条件与 `go_to_next_brother` 相同。

### get_children

```C++
std::array<proxy, N> get_children()const noexcept;
```

一次取回 `N` 个分支 _树位置_ 的代理对象，第 `i` 个元素等于 `get_child(i)` 。

### get_child_end

```C++
proxy get_child_end()const noexcept;
```

返回"其指代的 _树位置_ 上的 _N 叉树_ 的第 `N` 个分支位置"的代理——即该节点的**尾后边**。

尾后边不是真实分支：

- **不能解引用**：`operator*` / `operator->` / `null` / `go_to_child` 等一切取值的操作都是[未定义行为](https://zh.cppreference.com/w/cpp/language/ub)；
- 但它**是良定义的边**：`valid()` 为 `true`，可复制、可判等、可哈希，因此适合当兄弟遍历的哨位：

```C++
auto first = t.root().get_child(0);
auto end = t.root().get_child_end();
for (auto e = first; e != end; e.go_to_next_brother())
{
    // 处理第 0、1、…、N-1 个分支（空分支用 e.null() 跳过）
}
```

要求该边有效、且其指代的 _树位置_ 上确有 _N 叉树_ ，否则行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

## 非成员函数

### operator==

```C++
bool operator==(proxy l, proxy r)noexcept;
```

当且仅当其都无效，或指代同一个 _树位置_ 时返回 `true` 。

## 辅助类

### std::hash<edge_proxy>,std::hash<edge_const_proxy>

对于代理对象[启用](https://zh.cppreference.com/w/cpp/utility/hash)的 [`std::hash`](https://zh.cppreference.com/w/cpp/utility/hash) 特化。保证对相等的 `edge_proxy` 与 `edge_const_proxy` 返回相同值；无效边（默认构造的代理）哈希为 `0` 。
