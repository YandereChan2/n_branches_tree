# `Yc::n_branches_tree<N, T, Allocator>::splice`

```C++
n_branches_tree splice(edge_const_proxy to, edge_const_proxy from)noexcept;   // (1)
```

```C++
n_branches_tree splice(edge_const_proxy p, n_branches_tree& tree)noexcept;    // (2)
```

```C++
n_branches_tree splice(edge_const_proxy p, n_branches_tree&& tree)noexcept;   // (3)
```

把 `from` 位置上的子树**转移**到 `to` 位置（(2) (3) 转移 `tree` 的根子树），原先挂在 `to`（或 `p`）上的子树被取下来作为返回值返回；`from`（或 `tree` 的根）变空。

不复制、不分配、不销毁任何节点。

## 参数

||||
|-:|-|:-|
|`to` , `p`|-|接收子树的目标 _树位置_ 。可以是空 _树位置_ 。|
|`from`|-|被取走子树的源 _树位置_ 。转移后变空。|
|`tree`|-|被取走根子树的 `n_branches_tree` 。转移后变为空树。|

## 返回值

目标位置原有的整棵 _N 叉树_ （原为空时返回空树），沿用 `*this` 的分配器。

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

    Yc::n_branches_tree<3, int> other;
    other.emplace(other.root(), 9);

    auto old = t.splice(r.get_child(2), other);   // 把 9 移到第 2 个分支，返回原来第 2 个分支的内容（空）
    return (*r.get_child(2) == 9 && other.empty()) ? 0 : 1;
}
```
