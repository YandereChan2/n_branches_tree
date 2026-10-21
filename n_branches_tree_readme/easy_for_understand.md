# 导引：怎么用这棵 N 叉树

`Yc::n_branches_tree<N, T>` 是一棵**分支数固定为 `N`** 的树：每个节点保有一个 `T` 元素和 `N` 个 _树位置_ （分支），分支编号为 `0` 到 `N - 1`，每个分支上可以挂一棵子树，也可以空着。树的形状由使用者自己安排，本库不规定任何顺序性质。

## 从根开始

```C++
Yc::n_branches_tree<3, int> t;
auto r = t.root();                 // 根 _树位置_（是边的代理）
if (r.null())                      // 根上还没有节点
{
    t.emplace(r, 1);               // 在根位置放一个节点，值为 1
}
```

- `root()` 返回**边代理**，指代一个 _树位置_ ，可以纵向、横向游走；
- `nroot()` 返回**节点代理**，指代节点本身，不带到兄弟去。

## 纵向与横向

```C++
auto e = t.root();
e.go_to_child(0);                  // 纵向：走到根的第 0 个分支
e.go_to_next_brother();            // 横向：第 1 个分支
e.go_to_prev_brother();            // 横向：退回第 0 个分支
e.get_child(2);                    // 不改自身，返回第 2 个分支的边
e.get_children();                  // 一次取回 N 个分支的边（std::array）
```

`go_*` 改自身并返回 `*this`（可链式），`get_*` 返回新代理、自身不动。

前置条件：`go_to_child(i)` / `get_child(i)` 要求 `i < N`；横向游走要求不越过当前节点的分支范围。越界是[未定义行为](https://zh.cppreference.com/w/cpp/language/ub)。

## 改树

```C++
t.emplace(r.get_child(0), 2);      // 第 0 个分支上放节点（返回原先挂着的东西，空时是空树）
t.insert(r.get_child(1), 3);       // 同上，值由 3 拷贝/移动而来
auto sub = t.cut(r.get_child(0));  // 把第 0 个分支上的子树摘下来
t.splice(r.get_child(2), sub);     // 再挂到第 2 个分支上
t.erase(r.get_child(2));           // 删掉第 2 个分支上的整棵子树
```

所有修改器都返回"被替换下来的旧子树"：不接住它就等于当场销毁。

## 遍历

```C++
std::string order;
t.for_each([&](int& v) { order += std::to_string(v) + ","; });
```

`for_each` **以未指定的顺序**遍历整棵树，对每个元素调用一次回调。它按值接收回调、按值返回回调副本（与 `std::for_each` 一致），所以累加器之类的有状态函数对象可以把状态带出来：

```C++
struct accumulator
{
    int sum = 0;
    void operator()(const int& v) noexcept { sum += v; }
};
auto acc = t.for_each(accumulator{});   // acc.sum 就是全树元素之和
```

需要不依赖顺序地做"整棵树过一遍"时用它；需要按自己的顺序处理时，用 `root()` / `nroot()` 配合 `go_to_child` / `go_to_next_brother` 自己走。

## 与二叉树的对应关系

| 二叉树 | 这里的对应物 |
|---|---|
| `root()` / `nroot()` | 相同 |
| `go_left()` / `go_right()` | `go_to_child(0)` / `go_to_child(1)`（分支用编号指代） |
| `get_left()` / `get_right()` | `get_child(i)` |
| `get_children()`（`std::pair`） | `get_children()`（`std::array<edge_proxy, N>`） |
| 兄弟之间的移动 | `go_to_next_brother` / `go_to_prev_brother`（以及带步数的重载） |
| `left_rotate` / `right_rotate` | 没有对应物（N 叉树没有旋转） |
| `swap_node(l, r)` | 相同（不需要相邻检查） |

其余接口（`cut` / `splice` / `emplace` / `insert` / `erase` / `clear` / `swap` / `swap_sub_tree` / `recur_and_write` / `functional::get_children` / 代理的 `std::hash` 约定）与 `binary_tree` 一一对应、语义相同。

## 一个完整的例子

```C++
#include "n_branches_tree.h"
#include <string>

int main()
{
    Yc::n_branches_tree<3, int> t;
    auto r = t.root();
    t.emplace(r, 1);                         // 根：1
    t.emplace(r.get_child(0), 2);            // 第 0 个分支：2
    t.emplace(r.get_child(2), 3);            // 第 2 个分支：3（第 1 个分支空着）

    int sum = 0;
    t.for_each([&](int& v) { sum += v; });   // 顺序未指定，但每个元素恰好访问一次
    return sum == 6 ? 0 : 1;
}
```
