# `Yc::parent_aware_n_branches_tree<N, T, Allocator>::parent_aware_n_branches_tree`

```C++
parent_aware_n_branches_tree();                                                              // (1)
explicit parent_aware_n_branches_tree(const Alloc& a)noexcept;                               // (2)
```

```C++
template<class ValueGetter, class ChildrenGetter, class InitializeHandle>
parent_aware_n_branches_tree(ValueGetter vg, ChildrenGetter cg, InitializeHandle h, const Alloc& a = {}); // (3)
```

```C++
parent_aware_n_branches_tree(const parent_aware_n_branches_tree& b);                          // (4)
parent_aware_n_branches_tree(parent_aware_n_branches_tree&& b)noexcept;                       // (5)
```

构造容器。

- (1) 空树，分配器默认构造。
- (2) 空树，以 `a` 为分配器。
- (3) 从外部树形结构构造整棵树：`h` 是初始句柄，`vg(h)` 取得该句柄对应的值，`cg(h)` 取得该句柄的 `N` 个孩子句柄。`h` 为 `false` 时表示"这里没有节点"。各分支的处理顺序**未指定**。见 [`recur_and_write`](recur_and_write.md) 。
- (4) 拷贝构造：按 `b` 的结构递归复制，元素被拷贝。
- (5) 移动构造：转移根子树，`b` 变为空树。

## 参数

||||
|-:|-|:-|
|`a`|-|要使用的分配器。|
|`vg`|-|值获取器，接受句柄、返回要构造的元素。|
|`cg`|-|孩子句柄获取器，接受句柄、返回 `N` 个孩子句柄的随机访问范围（如 `std::array` / `std::vector` / `std::deque`）。|
|`h`|-|初始句柄，可按语境转换成 `bool` 。|
|`b`|-|用作源的容器。|

## 复杂度

- (1) (2) (5)：常数。
- (3) (4)：与构造出的节点数成正比。

## 异常

- (2) (5)：`noexcept` 。
- (3) (4)：`vg` / 元素构造函数抛出的异常继续传播。

## 示例

```C++
#include "parent_aware_n_branches_tree.h"
#include <array>

struct src
{
    int v;
    const src* c[3];
};

int main()
{
    const src leaf{ 2, {nullptr, nullptr, nullptr} };
    const src root{ 1, {&leaf, nullptr, nullptr} };

    auto vg = [](const src* h) { return h->v; };
    auto cg = [](const src* h) { return std::array<const src*, 3>{h->c[0], h->c[1], h->c[2]}; };

    Yc::parent_aware_n_branches_tree<3, int> t{ vg, cg, &root };
    const Yc::parent_aware_n_branches_tree<3, int> copy{ t };
    return (!t.empty() && *copy.root() == 1 && *copy.root().get_child(0) == 2) ? 0 : 1;
}
```
