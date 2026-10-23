# `Yc::parent_aware_n_branches_tree<N, T, Allocator>::parent_of_nroot` , `parent_of_cnroot`

```C++
node_proxy parent_of_nroot()noexcept;
node_const_proxy parent_of_cnroot()const noexcept;
```

返回**哨位节点**的节点代理。

本容器为了让根节点也有父节点，内部保留一个不保有任何元素的哨位节点：它的第 0 个分支指向真根，根节点的父节点就是它。因此：

- `nroot().get_parent() == parent_of_nroot()` 恒成立；
- 从根边 `root()` 向上移动（[`go_up`](edge_proxy/edge_proxy.md)）是**未定义行为**：哨位只持有根槽位，没有 `parent` 字段；
- 哨位节点上**没有元素**：对它的代理解引用（`operator*` / `operator->`）是[未定义行为](https://zh.cppreference.com/w/cpp/language/ub)。

## 参数

（无）

## 返回值

哨位节点的 [`node_proxy`](node_proxy/node_proxy.md) 或 [`node_const_proxy`](node_proxy/node_proxy.md) 。它总是非空的。

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
    t.emplace(t.root(), 1);

    auto sentinel = t.parent_of_nroot();
    if (sentinel.null())
    {
        return 1;
    }
    // 哨位的第 0 个分支就是真根
    if (*sentinel.get_child(0) != 1)
    {
        return 2;
    }
    // 从根再往上没有父节点
    return t.root().get_up().valid() ? 3 : 0;
}
```
