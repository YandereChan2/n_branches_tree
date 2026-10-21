# `Yc::parent_aware_n_branches_tree<N, T, Allocator>::operator=`

```C++
parent_aware_n_branches_tree& operator=(const parent_aware_n_branches_tree& b);
parent_aware_n_branches_tree& operator=(parent_aware_n_branches_tree&& b)noexcept(/* 见下文 */);
```

替换 `*this` 的内容。

- 拷贝赋值：清空原内容后按 `b` 的结构复制；自赋值时不做任何事。分配器按 `std::allocator_traits<Alloc>::propagate_on_container_copy_assignment` 决定是否随 `b` 一起复制。
- 移动赋值：分配器可传播时交换分配器与根子树；分配器不可传播但比较相等时只转移根子树；否则用 `b` 的元素逐个移动构造一棵新树再交换。移动后 `b` 的取值是[未指明的](https://zh.cppreference.com/w/cpp/language/as_if)，但对象始终合法。

## 参数

||||
|-:|-|:-|
|`b`|-|用作源的容器。|

## 返回值

`*this` 。

## 复杂度

- 拷贝赋值：与 `*this` 与 `b` 保有的节点数之和成正比。
- 移动赋值：分配器相等（或可传播）时是常数，否则与 `b` 保有的节点数成正比。

## 异常

拷贝赋值在元素复制或分配失败时抛出，此时 `*this` 的内容保持不变。
