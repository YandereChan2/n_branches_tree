# `Yc::n_branches_tree_functional::get_children`

```C++
template<class P>
auto operator()(P p)const noexcept;      // p 为四种代理类型之一
```

定制点对象（customization point object），统一对 [`edge_proxy`](edge_proxy/edge_proxy.md) 、`edge_const_proxy` 、[`node_proxy`](node_proxy/node_proxy.md) 、`node_const_proxy` 四种代理调用 `get_children()` 。

它的用途是把"取孩子"这件事作为**函数对象**传给 [`recur_and_write`](recur_and_write.md) 或三参[构造函数](constructor.md) ：在不同代理、不同 const 性之间不必手写 lambda。

## 参数

||||
|-:|-|:-|
|`p`|-|一个代理对象。|

## 返回值

- 边代理：`std::array<edge_proxy, N>` 或 `std::array<edge_const_proxy, N>`；
- 节点代理：`std::array<edge_proxy, N>` 或 `std::array<edge_const_proxy, N>`。

## 复杂度

常数（与 `N` 成正比）。

## 异常

`noexcept` 。

## 示例

```C++
#include "n_branches_tree.h"

int main()
{
    Yc::n_branches_tree<3, int> a;
    a.emplace(a.root(), 1);
    a.emplace(a.root().get_child(1), 2);

    // 用定制点对象代替 lambda 取孩子
    Yc::n_branches_tree<3, int> b;
    auto vg = [](auto p) -> const int& { return *p; };
    b.recur_and_write(b.root(), vg, Yc::n_branches_tree_functional::get_children, a.root());
    return *b.root().get_child(1) == 2 ? 0 : 1;
}
```
