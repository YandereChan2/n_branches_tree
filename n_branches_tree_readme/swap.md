# `Yc::n_branches_tree<N, T, Allocator>::swap`

```C++
void swap(n_branches_tree& other)noexcept;
```

交换 `*this` 与 `other` 的内容。常数时间，不复制任何节点；分配器按 `std::allocator_traits<Alloc>::propagate_on_container_swap` 决定是否交换。

## 参数

||||
|-:|-|:-|
|`other`|-|要交换内容的另一个 `n_branches_tree` 。|

## 返回值

（无）

## 复杂度

常数

## 异常

`noexcept` 。若两个分配器不相等且不传播，则行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

## 示例

```C++
#include "n_branches_tree.h"

int main()
{
    Yc::n_branches_tree<3, int> a;
    Yc::n_branches_tree<3, int> b;
    a.emplace(a.root(), 1);
    b.emplace(b.root(), 2);

    a.swap(b);
    return (*a.root() == 2 && *b.root() == 1) ? 0 : 1;
}
```
