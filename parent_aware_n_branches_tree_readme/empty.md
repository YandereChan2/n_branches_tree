# `Yc::parent_aware_n_branches_tree<N, T, Allocator>::empty`

```C++
bool empty()const noexcept;
```

返回根 _树位置_ 上是否没有 _N 叉树_ 。

## 参数

（无）

## 返回值

空树返回 `true` ，否则返回 `false` 。

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
    if (!t.empty())
    {
        return 1;
    }
    t.emplace(t.root(), 42);
    return t.empty() ? 1 : 0;
}
```
