# `Yc::parent_aware_n_branches_tree<N, T, Allocator>::insert`

```C++
parent_aware_n_branches_tree insert(edge_const_proxy p, const T& v);
parent_aware_n_branches_tree insert(edge_const_proxy p, T&& v);
```

在 `p` 指代的 _树位置_ 上构造新节点，元素由 `v` 复制或移动构造；等价于 `emplace(p, v)` 或 `emplace(p, std::move(v))`。

`p` 处原有的整棵子树会被取下来作为返回值。

## 参数

||||
|-:|-|:-|
|`p`|-|要写入的 _树位置_ 。|
|`v`|-|元素的值。|

## 返回值

`p` 处原有的整棵 _N 叉树_ （原为空时返回空树）。

## 复杂度

常数（不计分配与元素构造）。

## 异常

元素复制/移动构造抛出的异常继续传播；此时 `p` 处的内容保持不变。

## 示例

```C++
#include "parent_aware_n_branches_tree.h"

int main()
{
    Yc::parent_aware_n_branches_tree<2, int> t;
    auto r = t.root();
    t.insert(r, 1);
    t.insert(r.get_child(0), 2);
    return *r.get_child(0) == 2 ? 0 : 1;
}
```
