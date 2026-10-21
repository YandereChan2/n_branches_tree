# `Yc::swap`

```C++
template<size_t N, class T, class Alloc>
void swap(n_branches_tree<N, T, Alloc>& l, n_branches_tree<N, T, Alloc>& r)noexcept;
```

特化的 `swap` 自由函数，等价于 `l.swap(r)` 。它使 `n_branches_tree` 满足[_可交换 (Swappable)_](https://zh.cppreference.com/w/cpp/named_req/Swappable)，可以参与 ADL 两段式 `using std::swap; swap(a, b);` 。

## 参数

||||
|-:|-|:-|
|`l` , `r`|-|要交换内容的两个 `n_branches_tree` 。|

## 返回值

（无）

## 复杂度

常数

## 异常

`noexcept` 。

## 示例

```C++
#include "n_branches_tree.h"
#include <utility>

int main()
{
    Yc::n_branches_tree<3, int> a;
    Yc::n_branches_tree<3, int> b;
    a.emplace(a.root(), 1);
    b.emplace(b.root(), 2);

    using std::swap;
    swap(a, b);                            // 走 ADL，命中 Yc::swap
    return (*a.root() == 2 && *b.root() == 1) ? 0 : 1;
}
```
