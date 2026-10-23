# Yc::parent_aware_n_branches_tree

在文件 `"parent_aware_n_branches_tree.h"` 定义

```C++
template<
    size_t N,
    class T,
    class Alloc = std::allocator<T>
>class parent_aware_n_branches_tree;
```

`Yc::parent_aware_n_branches_tree` 是对**反向关联父节点**的 _N 叉树_ 数据结构的封装。分支数 `N` 是编译期常量。

术语与无父指针版本（`Yc::n_branches_tree`）相同：每一个 _N 叉树_ 占据一个 _树位置_ ，一个 _树位置_ 或空闲，或存在一个 _N 叉树_ ；一个 _N 叉树_ 保有 `N` 个 _树位置_ （分支，编号 `0` 到 `N - 1`）以及一个 `T` 类型的元素；每个容器对象内部保有一个 _树位置_ （根 _树位置_ ）。

除此之外，本版本的每个 _N 叉树_ 还保有一个指向**父节点**的 _树位置_ ，因此可以从任意位置向上移动到父节点（[`go_up`](edge_proxy/edge_proxy.md) 一族）。为了让"根"也有父节点，容器内部有一个**哨位节点**：根节点的父节点就是它，[`parent_of_nroot`](parent_of_nroot.md) 返回它的代理。

## 与 `n_branches_tree` 的关系

接口一一对应，区别只在"一条边能记住多少上下文"：

- `n_branches_tree` 的边只指代一个 _树位置_ ，横向只能在兄弟之间逐步移动；
- `parent_aware_n_branches_tree` 的边 = (父节点, 分支编号)，所以兄弟之间可以**随机访问**（`go_to_brother(i)` / `go_to_last_brother()`），并且能向上走（`go_up`）。

两边都有"尾后边"（分支编号为 `N` 的边）：它不是真实分支、不能解引用，但可比较、可哈希，可以当兄弟遍历的哨位。

## 模板参数

| | |
|-:|:-|
|`N`|分支数，编译期常量，必须大于 `0`。|
|`T`|元素的类型。 至少应当满足[_可擦除 (Erasable)_](https://zh.cppreference.com/w/cpp/named_req/Erasable)。|
|`Allocator`|用于获取/释放内存及构造/析构内存中元素的分配器。类型必须满足[_分配器 (Allocator)_](https://zh.cppreference.com/w/cpp/named_req/Allocator)。|

## 成员类型

|成员类型|定义|
|-:|:-|
|`value_type`|`T`|
|`allocator_type`|`Alloc`|
|`reference`|`T&`|
|`pointer`|[`std::allocator_traits<Alloc>::pointer`](https://zh.cppreference.com/w/cpp/memory/allocator_traits)|
|[`edge_proxy`](edge_proxy/edge_proxy.md)|表示 _树位置_ 的轻量级代理对象。|
|[`edge_const_proxy`](edge_proxy/edge_proxy.md)|表示 _树位置_ 的轻量级代理对象，元素只读。|
|[`node_proxy`](node_proxy/node_proxy.md)|表示 _N 叉树_ 的轻量级代理对象。|
|[`node_const_proxy`](node_proxy/node_proxy.md)|表示 _N 叉树_ 的轻量级代理对象，元素只读。|

## 成员函数

|||
|:-|:-|
|[（构造函数）](constructor.md)|构造 `parent_aware_n_branches_tree` <br>（公开成员函数）|
|[（析构函数）](destructor.md)|析构 `parent_aware_n_branches_tree` <br>（公开成员函数）|
|[`operator=`](operator_assign.md)|对内容赋值<br>（公开成员函数）|
|[`get_allocator`](get_allocator.md)|返回关联的分配器<br>（公开成员函数）|

### 代理

|||
|:-|:-|
|[`root`<br>`croot`](root.md)|返回根 _树位置_ 的代理。（公开成员函数）|
|[`nroot`<br>`cnroot`](nroot.md)|返回根 _树位置_ 上的 _N 叉树_ 的代理。（公开成员函数）|
|[`parent_of_nroot`<br>`parent_of_cnroot`](parent_of_nroot.md)|返回哨位节点（根的父节点）的代理。（公开成员函数）|

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
|[`recur_and_write`](recur_and_write.md)|消耗一个输入以在某个 _树位置_ 构造 _N 叉树_ ，递归地在它的各分支继续构造<br>（公开成员函数）|

### 遍历

|||
|:-|:-|
|[`for_each`](for_each.md)|以未指定的顺序遍历并对每个元素调用所提供的函数<br>（公开成员函数）|

### 容量

|||
|:-|:-|
|[`empty`](empty.md)|返回根 _树位置_ 上是否没有 _N 叉树_ <br>（公开成员函数）|

### 通用工具

|||
|:-|:-|
|[`parent_aware_n_branches_tree_functional::get_children`](functional_get_children.md)|定制点对象，可统一对四种代理类型调用。|

### 非成员函数

|||
|:-|:-|
|[`Yc::swap`](adl_swap.md)|交换内容<br>（函数模板）|
