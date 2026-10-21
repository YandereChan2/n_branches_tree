# `Yc::n_branches_tree<N, T, Allocator>::cut`

```C++
n_branches_tree cut(edge_const_proxy p)noexcept;
```

把 `p` 指代的 _树位置_ 上挂着的整棵 _N 叉树_ **摘下来**，放进一个新构造的 `n_branches_tree` 返回；`p` 处变空。

摘下来的子树连同它下面所有节点一起转移，不复制、不分配、不销毁任何节点。

## 参数

||||
|-:|-|:-|
|`p`|-|要摘取的 _树位置_ 。可以是空 _树位置_ （此时返回空树，什么也不做）。|

## 返回值

持有被摘下子树的新 `n_branches_tree` 。新树沿用 `*this` 的分配器。

## 复杂度

常数

## 异常

`noexcept` 。

## 示例

```C++
#include "n_branches_tree.h"

int main()
{
    Yc::n_branches_tree<3, int> t;
    auto r = t.root();
    t.emplace(r, 1);
    t.emplace(r.get_child(0), 2);

    Yc::n_branches_tree<3, int> sub = t.cut(r.get_child(0));   // 摘下 2 那棵子树
    if (!t.root().get_child(0).null())                          // 原位置变空
    {
        return 1;
    }
    return *sub.root() == 2 ? 0 : 1;
}
```
