# `Yc::parent_aware_n_branches_tree<N, T, Allocator>::emplace`

```C++
template<class... Args>
parent_aware_n_branches_tree emplace(edge_const_proxy p, Args&&... args);
```

在 `p` 指代的 _树位置_ 上原位构造一个新的节点：元素由 `args...` 构造，它的 `N` 个分支 _树位置_ 全为空；该节点的父节点被设为 `p` 的父节点。

`p` 可以是空 _树位置_ ，也可以已经有子树：原先挂在那里的整棵子树会被**取下来**作为返回值（不接住它就等于当场销毁）。

## 参数

||||
|-:|-|:-|
|`p`|-|要写入的 _树位置_ 。|
|`args`|-|转发给元素构造函数的参数。|

## 返回值

`p` 处原有的整棵 _N 叉树_ （原为空时返回空树）。

## 复杂度

常数（不计分配与元素构造）。

## 异常

元素构造抛出的异常继续传播；此时 `p` 处的内容保持不变。分配失败时抛出 [`std::bad_alloc`](https://zh.cppreference.com/w/cpp/memory/new/bad_alloc)。

## 示例

```C++
#include "parent_aware_n_branches_tree.h"

int main()
{
    Yc::parent_aware_n_branches_tree<3, int> t;
    auto r = t.root();
    t.emplace(r, 1);
    t.emplace(r.get_child(1), 2);

    auto old = t.emplace(r.get_child(1), 3);   // 换掉 2，旧的子树被返回
    return (*r.get_child(1) == 3 && *old.root() == 2) ? 0 : 1;
}
```
