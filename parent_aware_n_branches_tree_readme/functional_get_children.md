# `Yc::parent_aware_n_branches_tree_functional::get_children`

```C++
template<class P>
auto operator()(P p)const noexcept;      // p 为四种代理类型之一
```

定制点对象，统一对 `edge_proxy` 、`edge_const_proxy` 、`node_proxy` 、`node_const_proxy` 四种代理调用 `get_children()` 。

它的用途是把"取孩子"这件事作为**函数对象**传给 [`recur_and_write`](recur_and_write.md) 或[构造函数](constructor.md) 。

## 参数

||||
|-:|-|:-|
|`p`|-|一个代理对象。|

## 返回值

`std::array` ，含 `N` 个分支的边代理（`edge_proxy` 或 `edge_const_proxy`）。

## 复杂度

与 `N` 成正比。

## 异常

`noexcept` 。

## 示例

```C++
#include "parent_aware_n_branches_tree.h"

int main()
{
    Yc::parent_aware_n_branches_tree<3, int> a;
    a.emplace(a.root(), 1);
    a.emplace(a.root().get_child(1), 2);

    Yc::parent_aware_n_branches_tree<3, int> b;
    auto vg = [](auto p) -> const int& { return *p; };
    b.recur_and_write(b.root(), vg, Yc::parent_aware_n_branches_tree_functional::get_children, a.croot());
    return *b.root().get_child(1) == 2 ? 0 : 1;
}
```
