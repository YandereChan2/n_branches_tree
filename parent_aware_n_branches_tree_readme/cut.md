# `Yc::parent_aware_n_branches_tree<N, T, Allocator>::cut`

```C++
parent_aware_n_branches_tree cut(edge_const_proxy p)noexcept;
```

把 `p` 指代的 _树位置_ 上挂着的整棵 _N 叉树_ **摘下来**，放进一个新构造的容器返回；`p` 处变空。

摘下来的子树不复制、不分配、不销毁任何节点；它的根节点的父节点被设为新容器的哨位节点，因此在新容器里向上导航依然正确。

## 参数

||||
|-:|-|:-|
|`p`|-|要摘取的 _树位置_ 。可以是空 _树位置_ （此时返回空树）。|

## 返回值

持有被摘下子树的新容器，沿用 `*this` 的分配器。

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

    auto sub = t.cut(r.get_child(0));
    if (!t.root().get_child(0).null() || sub.empty())
    {
        return 1;
    }
    // 摘出来的树：父节点链指向它自己的哨位
    return (*sub.root() == 2 && !sub.root().get_up().valid()) ? 0 : 2;
}
```
