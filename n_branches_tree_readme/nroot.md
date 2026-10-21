# `Yc::n_branches_tree<N, T, Allocator>::nroot` , `cnroot`

```C++
node_proxy nroot()noexcept;
```

```C++
node_const_proxy nroot()const noexcept;
```

```C++
node_const_proxy cnroot()const noexcept;
```

返回指代 `*this` 内部根 _树位置_ 上的 _N 叉树_ 的代理对象，等价于把 [`root()`](root.md) 的结果转换成节点代理。

空树时返回的节点代理 `null()` 为 `true` 、按语境转换成 `bool` 为 `false` 。

与 `root()` 的区别只在"指代什么"：`root()` 指代 _树位置_ （能横向游走到兄弟），`nroot()` 指代 _N 叉树_ 本身（不随位置调整而变化，但不带位置信息）。

## 参数

（无）

## 返回值

指代根 _N 叉树_ 的 [`node_proxy`](node_proxy/node_proxy.md) 或 [`node_const_proxy`](node_proxy/node_proxy.md) 。

## 复杂度

常数

## 异常

（无）
