# `Yc::parent_aware_n_branches_tree<N, T, Allocator>::swap_sub_tree`

```C++
static void swap_sub_tree(edge_const_proxy l, edge_const_proxy r)noexcept;
```

交换两个 _树位置_ 上挂着的 _N 叉树_ ：不复制、不分配、不销毁任何节点。交换后两个位置上的子树的父节点链都正确。

两个 _树位置_ 可以任意（包括同一个，此时什么也不做；也包括父子关系）。

## 参数

||||
|-:|-|:-|
|`l` , `r`|-|要交换的两个 _树位置_ 的代理。可以是空 _树位置_ 。|

## 复杂度

常数

## 异常

`noexcept` 。

## 示例

```C++
#include "parent_aware_n_branches_tree.h"

int main()
{
    Yc::parent_aware_n_branches_tree<3, int> t;
    auto r = t.root();
    t.emplace(r, 1);
    t.emplace(r.get_child(0), 2);
    t.emplace(r.get_child(1), 3);

    Yc::parent_aware_n_branches_tree<3, int>::swap_sub_tree(r.get_child(0), r.get_child(1));
    return (*r.get_child(0) == 3 && *r.get_child(1) == 2
        && r.get_child(0).get_up() == r) ? 0 : 1;
}
```
