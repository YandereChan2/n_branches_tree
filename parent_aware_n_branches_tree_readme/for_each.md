# `Yc::parent_aware_n_branches_tree<N, T, Allocator>::for_each`

```C++
template<class F>
F for_each(F f)noexcept(std::is_nothrow_invocable_v<F&, T&>&& std::is_nothrow_move_constructible_v<F>); //(1)
```

```C++
template<class F>
F for_each(F f)const noexcept(std::is_nothrow_invocable_v<F&, const T&>&& std::is_nothrow_move_constructible_v<F>); //(2)
```

以**未指定的顺序**遍历保有的 _N 叉树_ ，对每个节点保有的元素调用 `f` 。

遍历顺序未指定，调用方不应依赖它；不同的实现、甚至同一实现的不同版本之间都可能不同。

- (1) 非 const 版本，`f` 收到 `T&` 。
- (2) const 版本，`f` 收到 `const T&` 。

与 [`std::for_each`](https://zh.cppreference.com/w/cpp/algorithm/for_each) 一致：函数对象**按值传入、按值返回**，所以有状态函数对象（累加器、计数器之类）能带着状态出来。

`f` 不应改变树的结构，否则行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

## 参数

||||
|-:|-|:-|
|`f`|-|要对每个元素调用的函数对象。按值传入。|

## 返回值

遍历结束后 `f` 的副本。

## 复杂度

与遍历的节点数成正比。

## 异常

条件性 `noexcept` ：仅当 `f` 能无异常地接受对应左值、且能无异常地移动构造时为 `true` 。`f` 抛出时异常向调用方传播（树保持完整）。

## 示例

```C++
#include "parent_aware_n_branches_tree.h"

struct accumulator
{
    int sum = 0;
    void operator()(const int& v) noexcept { sum += v; }
};

int main()
{
    Yc::parent_aware_n_branches_tree<3, int> t;
    auto r = t.root();
    t.emplace(r, 1);
    t.emplace(r.get_child(0), 2);
    t.emplace(r.get_child(2), 3);

    int count = 0;
    t.for_each([&](int&) { ++count; });        // 顺序未指定：每个元素恰好访问一次
    auto acc = t.for_each(accumulator{});      // 1 + 2 + 3
    const auto& ct = t;
    auto cacc = ct.for_each(accumulator{});
    return (count == 3 && acc.sum == 6 && cacc.sum == 6) ? 0 : 1;
}
```
