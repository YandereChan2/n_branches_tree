# `Yc::n_branches_tree<N, T, Allocator>::n_branches_tree`

```C++
n_branches_tree();                                                              // (1)
```

```C++
explicit n_branches_tree(const Alloc& a)noexcept;                               // (2)
```

```C++
template<class ValueGetter, class ChildrenGetter, class InitializeHandle>
n_branches_tree(ValueGetter vg, ChildrenGetter cg, InitializeHandle h, const Alloc& a = {}); // (3)
```

```C++
n_branches_tree(const n_branches_tree& b);                                      // (4)
```

```C++
n_branches_tree(n_branches_tree&& b)noexcept;                                   // (5)
```

构造 `n_branches_tree` 。

- (1) 构造空树（根 _树位置_ 上没有 _N 叉树_ ），分配器默认构造。
- (2) 构造空树，以 `a` 为分配器。
- (3) 从**外部树形结构**递归构造整棵树：`h` 是初始句柄，`vg(h)` 取得该句柄对应的值，`cg(h)` 取得该句柄的 `N` 个孩子句柄。`h` 可[按语境转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)成 `bool` ，为 `false` 时表示"这里没有节点"，不构造任何东西。各分支的处理顺序未指定。见 [`recur_and_write`](recur_and_write.md) 。
- (4) 拷贝构造：按 `b` 的结构递归复制，元素被拷贝。分配器按 `std::allocator_traits<Alloc>::select_on_container_copy_construction(b.get_allocator())` 取得。
- (5) 移动构造：移动分配器并交换根 _树位置_ ，`b` 变为空树。

## 参数

||||
|-:|-|:-|
|`a`|-|要使用的分配器。|
|`vg`|-|值获取器。可调用对象，接受句柄，返回要构造的元素（或可转换为 `T` 的类型）。|
|`cg`|-|孩子句柄获取器。可调用对象，接受句柄，返回 `N` 个孩子句柄的**随机访问范围**（如 `std::array` / `std::vector` / `std::deque`）。|
|`h`|-|初始句柄。若不是节点则类型需可按语境转换成 `false` 。|
|`b`|-|用作源的 `n_branches_tree` 。|

## 复杂度

- (1) (2) (5)：常数。
- (3) (4)：与构造出的 _N 叉树_ 数量成正比。

## 异常

- (2) (5)：`noexcept` 。
- (3) (4)：`vg` / 元素构造函数抛出的异常会继续传播。

## 示例

```C++
#include "n_branches_tree.h"
#include <array>

struct src
{
    int v;
    const src* c[3];
};

int main()
{
    const src leaf_a{ 2, {nullptr, nullptr, nullptr} };
    const src leaf_b{ 3, {nullptr, nullptr, nullptr} };
    const src root{ 1, {&leaf_a, nullptr, &leaf_b} };

    auto vg = [](const src* h) { return h->v; };
    auto cg = [](const src* h) { return std::array<const src*, 3>{h->c[0], h->c[1], h->c[2]}; };

    Yc::n_branches_tree<3, int> t{ vg, cg, &root };   // 1 的孩子：2、(空)、3
    return t.empty() ? 1 : 0;
}
```
