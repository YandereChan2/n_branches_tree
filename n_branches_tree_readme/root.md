# `Yc::n_branches_tree<N, T, Allocator>::root` , `croot`

```C++
edge_proxy root()noexcept;
```

```C++
edge_const_proxy root()const noexcept;
```

```C++
edge_const_proxy croot()const noexcept;
```

返回指代 `*this` 内部那个 _树位置_ （根 _树位置_ ）的代理对象。

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
#include "n_branches_tree.h"

int main()
{
    Yc::n_branches_tree<3, int> t;
    if (t.root().null())                       // 空树
    {
        t.emplace(t.root(), 1);                // 在根位置放一个节点
    }
    return *t.root() == 1 ? 0 : 1;
}
```
