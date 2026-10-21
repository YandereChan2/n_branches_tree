# `Yc::n_branches_tree<N, T, Allocator>::operator=`

```C++
n_branches_tree& operator=(const n_branches_tree& b);
```

```C++
n_branches_tree& operator=(n_branches_tree&& b)noexcept(/* 见下文 */);
```

替换 `*this` 的内容。

- 拷贝赋值：先清空 `*this` 原有的 _N 叉树_ ，再按 `b` 的结构递归复制。分配器按 `std::allocator_traits<Alloc>::propagate_on_container_copy_assignment` 决定是否随 `b` 一起复制。自赋值时不做任何事。
- 移动赋值：分配器可传播时直接交换分配器与根 _树位置_ ；分配器不可传播但比较相等时只交换根 _树位置_ ；否则用 `b` 的元素逐个移动构造出一棵新树再交换。移动赋值后 `b` 的取值是[未指明的](https://zh.cppreference.com/w/cpp/language/as_if)，但对象始终合法。

## 参数

||||
|-:|-|:-|
|`b`|-|用作源的 `n_branches_tree` 。|

## 返回值

`*this` 。

## 复杂度

- 拷贝赋值：与 `*this` 与 `b` 保有的 _N 叉树_ 数量之和成正比。
- 移动赋值：分配器相等（或可传播）时是常数，否则与 `b` 保有的 _N 叉树_ 数量成正比。

## 异常

拷贝赋值在元素复制或分配失败时抛出，此时 `*this` 的内容保持不变。移动赋值的 `noexcept` 与分配器是否可传播、以及分配器自身移动构造的 `noexcept` 性质一致。
