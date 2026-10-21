# `Yc::n_branches_tree<N, T, Allocator>::emplace`

```C++
template<class... Args>
n_branches_tree emplace(edge_const_proxy p, Args&&... args);
```

在 `p` 指代的 _树位置_ 上原位构造一个新的 _N 叉树_ 节点：元素由 `args...` 构造，它的 `N` 个分支 _树位置_ 全为空。

`p` 可以是空 _树位置_ （包括 `root()` ），也可以是已经挂着子树的 _树位置_ 。原先挂在那里的整棵子树会被**取下来**，作为返回值返回（调用方不接住它就等于当场销毁）。这一点与 [`insert`](insert.md) 、[`cut`](cut.md) 一致：修改器都返回被替换下来的旧子树。

## 参数

||||
|-:|-|:-|
|`p`|-|要写入的 _树位置_ ，由 [`edge_const_proxy`](edge_proxy/edge_proxy.md) 指代。|
|`args`|-|转发给元素构造函数的参数。|

## 返回值

`p` 处原有的整棵 _N 叉树_ （原为空时返回空树）。

## 复杂度

常数（不计分配与元素构造）。

## 异常

元素构造抛出的异常继续传播；此时 `p` 处的内容保持不变。分配本身失败时抛出 [`std::bad_alloc`](https://zh.cppreference.com/w/cpp/memory/new/bad_alloc)。

## 示例

```C++
#include "n_branches_tree.h"

int main()
{
    Yc::n_branches_tree<3, int> t;
    auto r = t.root();
    t.emplace(r, 1);                        // 根：1
    t.emplace(r.get_child(1), 2);           // 第 1 个分支：2

    auto old = t.emplace(r.get_child(1), 3); // 第 1 个分支换成 3，旧的 2 被返回
    return (*r.get_child(1) == 3 && *old.root() == 2) ? 0 : 1;
}
```
