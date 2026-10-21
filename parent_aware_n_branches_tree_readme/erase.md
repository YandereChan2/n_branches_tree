# `Yc::parent_aware_n_branches_tree<N, T, Allocator>::erase`

```C++
void erase(edge_const_proxy p)noexcept;
```

销毁 `p` 指代的 _树位置_ 上的整棵 _N 叉树_ （连同它下面所有分支上的子树），`p` 处变空。`p` 是空 _树位置_ 时什么也不做。

## 参数

||||
|-:|-|:-|
|`p`|-|要擦除的 _树位置_ 。|

## 返回值

（无）

## 复杂度

与擦除掉的 _N 叉树_ 数量成正比。

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
    t.emplace(r.get_child(0).get_child(1), 3);

    t.erase(r.get_child(0));
    if (!t.root().get_child(0).null())
    {
        return 1;
    }
    t.erase(r);
    return t.empty() ? 0 : 2;
}
```
