# `Yc::n_branches_tree<N, T, Allocator>::swap_node`

```C++
static void swap_node(edge_proxy l, edge_proxy r)noexcept;
```

交换两个 _N 叉树_ 节点。净效果是**两个 _树位置_ 上的值互换、子树留在原位**（树的形状不变）。

两个节点是否互为父子不需要判断，也不需要调用方给任何提示。

`l` 和 `r` 都必须指代真实节点（`(bool)l` 与 `(bool)r` 为真），否则行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

## 参数

||||
|-:|-|:-|
|`l` , `r`|-|要进行交换的两个节点所在的 _树位置_ 的代理。|

## 返回值

（无）

## 复杂度

常数（与 `N` 成正比，`N` 是编译期常量）。

## 异常

`noexcept` 。

## 示例

```C++
#include "n_branches_tree.h"

int main()
{
    Yc::n_branches_tree<3, int> t;
    auto r = t.root();
    t.emplace(r, 1);                        // 1 的孩子：2、(空)、3
    t.emplace(r.get_child(0), 2);
    t.emplace(r.get_child(2), 3);

    Yc::n_branches_tree<3, int>::swap_node(r, r.get_child(0));   // 父子相邻也照常交换
    // 现在根位置是 2，它的第 0 个分支是 1
    return (*r == 2 && *r.get_child(0) == 1) ? 0 : 1;
}
```
