# `Yc::parent_aware_n_branches_tree<N, T, Allocator>::clear`

```C++
void clear()noexcept;
```

销毁保有的整棵 _N 叉树_ ，等价于 [`erase(root())`](erase.md) 。之后 [`empty()`](empty.md) 为 `true` 。

只销毁节点，不释放分配器，也不改变分配器。

## 参数

（无）

## 复杂度

与销毁掉的节点数成正比。

## 异常

`noexcept` 。
