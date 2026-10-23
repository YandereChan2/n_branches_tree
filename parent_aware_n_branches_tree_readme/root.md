# `Yc::parent_aware_n_branches_tree<N, T, Allocator>::root` , `croot`

```C++
edge_proxy root()noexcept;
edge_const_proxy root()const noexcept;
edge_const_proxy croot()const noexcept;
```

返回指代根 _树位置_ 的代理对象。根 _树位置_ 属于容器内部的哨位（只持有这一个槽位的指针），因此这个边永远有效；对它再调用 [`go_up`](edge_proxy/edge_proxy.md) 是未定义行为（哨位没有父节点）。

空树时该 _树位置_ 为空：`root().valid()` 为 `true` ，但 `root().null()` 为 `true` 、按语境转换成 `bool` 为 `false` 。

## 参数

（无）

## 返回值

指代根 _树位置_ 的 [`edge_proxy`](edge_proxy/edge_proxy.md) 或 [`edge_const_proxy`](edge_proxy/edge_proxy.md) 。

## 复杂度

常数

## 异常

（无）

## 示例

```C++
#include "parent_aware_n_branches_tree.h"

int main()
{
    Yc::parent_aware_n_branches_tree<3, int> t;
    if (t.root().null())
    {
        t.emplace(t.root(), 1);
    }
    return *t.root() == 1 ? 0 : 1;
}
```
