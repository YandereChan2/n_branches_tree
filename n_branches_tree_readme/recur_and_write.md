# `Yc::n_branches_tree<N, T, Allocator>::recur_and_write`

```C++
template<class ValueGetter, class ChildrenGetter, class InitializeHandle>
n_branches_tree recur_and_write(edge_const_proxy p, ValueGetter vg, ChildrenGetter cg, InitializeHandle h);
```

递归地把**外部树形结构**写进 `p` 指代的 _树位置_ ：`h` 是外部结构中当前元素的句柄，`vg(h)` 取得它的值，`cg(h)` 取得它 `N` 个孩子的句柄；对每个孩子句柄递归做同样的事。`p` 处原有的整棵子树被取下来作为返回值。

`h` 可[按语境转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)成 `bool` ，为 `false` 时表示"这里没有节点"：`p` 处不构造任何东西，原有子树被取下来作为返回值（`p` 处变空，调用方可以用 [`splice`](splice.md) 把它拼回去）。各分支的处理顺序未指定。

三参[构造函数](constructor.md)对整棵树做的就是这件事。

## 参数

||||
|-:|-|:-|
|`p`|-|要写入的 _树位置_ ，由 [`edge_const_proxy`](edge_proxy/edge_proxy.md) 指代。可以是空 _树位置_ 。|
|`vg`|-|值获取器。可调用对象，接受句柄，返回要构造的元素。|
|`cg`|-|孩子句柄获取器。可调用对象，接受句柄，返回 `N` 个孩子句柄的**随机访问范围**（如 `std::array` / `std::vector` / `std::deque`）。|
|`h`|-|起始句柄。|

## 返回值

`p` 处原有的整棵 _N 叉树_ （原为空时返回空树）。

## 复杂度

与写出的 _N 叉树_ 数量成正比。

## 异常

`vg` / 元素构造函数抛出的异常继续传播。异常时 `p` 处的内容保持不变。

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
    const src leaf{ 2, {nullptr, nullptr, nullptr} };
    const src root{ 1, {&leaf, nullptr, nullptr} };

    auto vg = [](const src* h) { return h->v; };
    auto cg = [](const src* h) { return std::array<const src*, 3>{h->c[0], h->c[1], h->c[2]}; };

    Yc::n_branches_tree<3, int> t;
    t.recur_and_write(t.root(), vg, cg, &root);

    const src* nobody = nullptr;
    // h 为空：什么也不写，原有子树被取下来作为返回值（目标位置变空）
    auto kept = t.recur_and_write(t.root().get_child(0), vg, cg, nobody);
    return *kept.root() == 2 ? 0 : 1;
}
```
