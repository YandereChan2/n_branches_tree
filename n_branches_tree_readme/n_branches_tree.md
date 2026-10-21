# Yc::n_branches_tree

在文件 `"n_branches_tree.h"` 定义

```C++
template<
    size_t N,
    class T,
    class Alloc = std::allocator<T>
>class n_branches_tree;
```

`Yc::n_branches_tree` 是对**不反向关联父节点**的 _N 叉树_ 数据结构的封装。分支数 `N` 是编译期常量。

此处 _N 叉树_ 的定义如下：
每一个 _N 叉树_ 占据一个 _树位置_ ，一个 _树位置_ 或空闲，或存在一个 _N 叉树_ 。一个 _N 叉树_ 保有 `N` 个 _树位置_ （称为它的**分支**，编号 `0` 到 `N - 1`）以及一个 `T` 类型的元素，这 `N` 个 _树位置_ 上可以有也可以没有其他 _N 叉树_ 。每个 `Yc::n_branches_tree` 对象内部保有一个 _树位置_ （根 _树位置_ ）。

当一个 _树位置_ 生存期结束时，其上的 _N 叉树_ 要么提前转移至其他 _树位置_ ，要么提前结束。换言之， _树位置_ 只能在空闲时结束其生存期，否则行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

在本库中，使用 [`edge_proxy`](edge_proxy/edge_proxy.md) 指代一个 _树位置_ ，使用 [`node_proxy`](node_proxy/node_proxy.md) 指代一个 _N 叉树_ 。

如果上面的定义让人难以理解，可以先查看[该文档](easy_for_understand.md)。

## 与 `binary_tree` 的关系

两者的一套"树位置 / 代理"接口一一对应，区别来自每条边指向的分支数：

- `binary_tree` 的分支数是 2，用 `go_left` / `go_right`（`get_left` / `get_right`）指代具体的分支；
- `n_branches_tree` 的分支数是编译期常量 `N`，用**分支编号**指代：`go_to_child(i)` / `get_child(i)`，`get_children()` 一次取回全部 `N` 个分支。

由此产生的差异：

- 没有"左旋/右旋"这类二叉树专属操作，本库不提供 `rotate` 一族；
- 提供在兄弟分支之间移动的 `go_to_next_brother` / `go_to_prev_brother`（以及带步数的重载）；
- [`swap_node`](swap_node.md) 只接受两个参数，不需要任何相邻形态的检查；
- 遍历顺序未指定（见 [`for_each`](for_each.md)）。

## 模板参数

| | |
|-:|:-|
|`N`|分支数，编译期常量，必须大于 `0`。|
|`T`|元素的类型。 至少应当满足[_可擦除 (Erasable)_](https://zh.cppreference.com/w/cpp/named_req/Erasable)，但是一些成员函数提出了更高的要求。|
|`Allocator`|用于获取/释放内存及构造/析构内存中元素的分配器。类型必须满足[_分配器 (Allocator)_](https://zh.cppreference.com/w/cpp/named_req/Allocator)。|

## 成员类型

|成员类型|定义|
|-:|:-|
|`value_type`|`T`|
|`allocator_type`|`Alloc`|
|`reference`|`T&`|
|`pointer`|[`std::allocator_traits<Alloc>::pointer`](https://zh.cppreference.com/w/cpp/memory/allocator_traits)|
|[`edge_proxy`](edge_proxy/edge_proxy.md)|表示 _树位置_ 的轻量级代理对象。|
|[`edge_const_proxy`](edge_proxy/edge_proxy.md)|表示 _树位置_ 的轻量级代理对象。对这个位置的树的元素提供只读访问|
|[`node_proxy`](node_proxy/node_proxy.md)|表示 _N 叉树_ 的轻量级代理对象。|
|[`node_const_proxy`](node_proxy/node_proxy.md)|表示 _N 叉树_ 的轻量级代理对象。对这个树的元素提供只读访问|

## 成员函数

|||
|:-|:-|
|[（构造函数）](constructor.md)|构造 `n_branches_tree` <br>（公开成员函数）|
|[（析构函数）](destructor.md)|析构 `n_branches_tree` <br>（公开成员函数）|
|[`operator=`](operator_assign.md)|对内容赋值<br>（公开成员函数）|
|[`get_allocator`](get_allocator.md)|返回关联的分配器<br>（公开成员函数）|

### 代理

|||
|:-|:-|
|[`root`<br>`croot`](root.md)|返回 `n_branches_tree` 对象内部的 _树位置_ 的代理。（公开成员函数）|
|[`nroot`<br>`cnroot`](nroot.md)|返回处于 `n_branches_tree` 对象内部的 _树位置_ 上的 _N 叉树_ 的代理。（公开成员函数）|

### 修改器

|||
|:-|:-|
|[`clear`](clear.md)|清除内容<br>（公开成员函数）|
|[`insert`](insert.md)|在某 _树位置_ 插入元素，产生新的 _N 叉树_ <br>（公开成员函数）|
|[`emplace`](emplace.md)|在某 _树位置_ 原位构造元素，产生新的 _N 叉树_ <br>（公开成员函数）|
|[`erase`](erase.md)|擦除 _N 叉树_ <br>（公开成员函数）|
|[`swap`](swap.md)|交换内容<br>（公开成员函数）|
|[`cut`](cut.md)|取出某个 _树位置_ 上的 _N 叉树_ <br>（公开成员函数）|
|[`swap_sub_tree`](swap_sub_tree.md)（静态）|交换两个 _树位置_ 上的 _N 叉树_ <br>（公开成员函数）|
|[`swap_node`](swap_node.md)（静态）|交换两个 _N 叉树_ 节点，子树留在原位<br>（公开成员函数）|
|[`splice`](splice.md)|从其他 _树位置_ 转移 _N 叉树_ <br>（公开成员函数）|
|[`recur_and_write`](recur_and_write.md)|消耗一个输入以在某个 _树位置_ 构造 _N 叉树_ ，将递归的在其保有的 _树位置_ 继续构造 _N 叉树_ <br>（公开成员函数）|

### 遍历

|||
|:-|:-|
|[`for_each`](for_each.md)|以未指定的顺序遍历并对每个元素调用所提供的函数<br>（公开成员函数）|

### 容量

|||
|:-|:-|
|[`empty`](empty.md)|返回 `n_branches_tree` 内部的 _树位置_ 上是否没有 _N 叉树_ <br>（公开成员函数）|

### 通用工具

|||
|:-|:-|
|[`n_branches_tree_functional::get_children`](functional_get_children.md)|定制点对象，可统一对四种代理类型调用。|

### 非成员函数

|||
|:-|:-|
|[`Yc::swap`](adl_swap.md)|交换内容<br>（函数模板）|
