非侵入式的 RAII 风格的泛型 N 叉树实现，旨在通过合理的封装实现子树操作无需内存管理，且不泄露。

# 目标

- 和 C++ 标准库保持风格一致
- 零开销原则
- 支持分配器语义
- head-only

# 非目标

- 这不是一个序列容器库，因此不提供迭代器功能，不假定遍历顺序
- 这不是平衡树或者搜索树的公共基座，是普通的 N 叉树操作库
- 当前要求 C++ 版本是 C++23，可能考虑前移到 C++20，但是不会进一步向前兼容

# 主要代码架构

主要发布件为 `n_branches_tree.h` 与 `parent_aware_n_branches_tree.h`，这两个头文件依赖 `union_data.h`，将文件直接放在头文件目录即可使用，无需配置构建选项。

- `n_branches_tree.h`：不反向关联父节点的版本，节点小、一条边只指代一个 _树位置_；
- `parent_aware_n_branches_tree.h`：父节点感知版本，可以从任意位置向上移动到父节点，兄弟分支之间可以随机访问。

两者的分支数 `N` 都是编译期常量，接口一一对应；两条库的文档分别在 `n_branches_tree_readme` 和 `parent_aware_n_branches_tree_readme` 目录下查阅。

# 关于遍历顺序

`for_each` 以及 `recur_and_write` 一类的遍历/递归接口**以未指定的顺序**处理元素，不承诺任何特定顺序；需要按自己的顺序处理时，用 `root` / `nroot` 配合 `go_to_child` / `go_to_brother` 自行游走。

# 分支与尾后边

每个节点有 `N` 个分支，编号 `0` 到 `N - 1`。两个库的边代理还额外支持编号为 `N` 的边（**尾后边**）：

- 它不是真实分支，**不能解引用**；
- 但它是良定义的边，可以复制、判等、哈希，因此可以当作兄弟遍历的哨位。

# 快速开始

```C++
#include <print>
#include "n_branches_tree.h"

int main()
{
    Yc::n_branches_tree<3, int> t;
    auto r = t.root();
    t.emplace(r, 1);                  // 根：1
    t.emplace(r.get_child(0), 2);
    t.emplace(r.get_child(2), 3);

    int sum = 0;
    t.for_each([&](int& v) { sum += v; });
    std::println("{}", sum);          // 6
}
```

父节点感知版本的用法相同，只是多出向上导航：

```C++
#include <print>
#include "parent_aware_n_branches_tree.h"

int main()
{
    Yc::parent_aware_n_branches_tree<3, int> t;
    auto r = t.root();
    t.emplace(r, 1);
    t.emplace(r.get_child(1), 2);

    auto e = r.get_child(1);
    e.go_up();                        // 回到根
    std::println("{}", *e);           // 1
}
```
