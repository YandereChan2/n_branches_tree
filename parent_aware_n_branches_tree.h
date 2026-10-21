// N 叉树库（父节点感知版本）
//
// 与 n_branches_tree.h（无父节点信息版）共用同一套"一条边 = 一条分支"的抽象，
// 区别在一条边能记住多少上下文，以及节点里多了 parent：
//   - 节点用**多继承**引入 N 个槽位：第 I 个分支是基类 pa_slot<I, pointer> 里的一个
//     child 指针，所以"第几个分支"是类型信息，取槽位时不必下标查数组；
//   - 再继承一层引入 parent 和 T（parent 单独一层，是为了让哨位节点可以是"没有 T 的节点"）；
//   - 一条边 = (父节点指针, 分支下标)，因此兄弟之间可以随机访问（go_to_brother(i) /
//     go_to_last_brother()），也能 O(1) 走到"尾后边"（下标 == N）。
//
// 尾后边（下标 == N）：
//   - 它不是真实分支，**不能解引用**（operator* / operator-> / slot / child 都是未定义行为）；
//   - 但它是良定义的边：可复制、可比较、可哈希；valid() 为 true、null() 为 true、is_end() 为 true，
//     所以可以当兄弟遍历的哨位：
//         for (auto e = p.get_brother(0); !e.is_end(); e.go_to_next_brother()) { ... }
//
// 命名约定与其它头文件一致：go_* 原地改自身、返回 *this；get_* 返回新代理、自身不动。
#pragma once
#include <array>
#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include "union_data.h"

namespace Yc
{
    template<size_t N, class T, class Alloc = std::allocator<T>>
    class parent_aware_n_branches_tree;
    namespace details
    {
        // ===================================================================
        // 节点布局：
        //   pa_slot<I, pointer>              第 I 个分支（一个 child 指针）
        //   pa_slots<pointer, 0..N-1>        多继承：N 个分支各是一个基类
        //   ├─ pa_empty                      空类型：分配器统一 rebind 的目标、pointer 的 element_type
        //   │   └─ pa_node_base<N, pointer>  再继承一次，引入 parent（哨位节点用这个类型）
        //   │       └─ pa_node<N, T, pointer> 再继承一次，引入 T
        // ===================================================================
        // 分配器统一 rebind 的目标：非模板的空类型，整条继承链的最底层
        struct pa_placeholder
        {
        };
        // 槽位链：T<0> 继承 pa_placeholder 并引入第 0 个 child；
        // T<1> 继承 T<0> 再引入第 1 个 child；以此类推，每层多一个 child 指针。
        template<size_t I, class pointer>
        struct pa_child : pa_child<I - 1, pointer>
        {
            pointer child{};
        };
        template<class pointer>
        struct pa_child<0, pointer> : pa_placeholder
        {
            pointer child{};
        };
        // 再继承一层：parent（哨位节点用这个类型，没有 T）
        template<size_t N, class pointer>
        struct pa_node_base : pa_child<N - 1, pointer>
        {
            static_assert(N != 0, "N 叉树的 N 必须 >= 1");
            pointer parent{};
        };
        // 再继承一层：T
        template<size_t N, class T, class pointer>
        struct pa_node : pa_node_base<N, pointer>
        {
            data_t<T> data;
        };

        // ---- 成员指针小巧思（原作者设计，勿"顺手优化"成折叠比较）-------------
        // 成员指针的宿主取**链顶** `pa_child<N - 1, pointer>`：它是每一层槽位的派生类，
        // 所以每层 &child 都能隐式转成 `pointer pa_child<N - 1, pointer>::*`
        //（基类成员指针 → 派生类成员指针，标准转换，不需要 reinterpret_cast）。
        // 32 路 switch 返回的全是这一种类型的常量成员指针，"选哪个分支"于是退化成一条
        // 运行期成员指针；N > 32 时按 32 一层剥下去，返回类型始终相同，递归不分叉。
        // 成员指针的宿主统一取 pa_placeholder（所有槽位层的基类）：
        // 每层 &child 用 static_cast 转成 `pointer pa_placeholder::*`
        //（指向派生类成员的指针 → 指向基类成员的指针，标准允许 static_cast）。
        // "选哪个分支"于是变成一条运行期成员指针；Base 是**函数内** constexpr，
        // N > 32 时剥掉低 32 层递归 N - 32，返回类型始终相同。
        template<class pointer>
        using pa_child_mem_ptr = pointer pa_placeholder::*;

        // 每个分支的成员指针做成 constexpr inline 变量模板：case 的右边返回的是变量
        //（其值仍是编译期常量），比内联的 static_cast 表达式更"像常量"，
        // 编译器更容易把整张 switch 折成"基址 + 下标 × 步长"。
        template<size_t I, class pointer>
        constexpr inline pa_child_mem_ptr<pointer> pa_child_ptr =
            static_cast<pa_child_mem_ptr<pointer>>(&pa_child<I, pointer>::child);

        template<size_t N, class pointer>
        constexpr inline pa_child_mem_ptr<pointer> pa_child_visitor_32(size_t i)noexcept
        {
            constexpr size_t Base = (N > 32) ? (N - 32) : 0;
            switch (i)
            {
            case Base + 0:  return pa_child_ptr<Base + 0, pointer>;
            case Base + 1:  return pa_child_ptr<Base + 1, pointer>;
            case Base + 2:  return pa_child_ptr<Base + 2, pointer>;
            case Base + 3:  return pa_child_ptr<Base + 3, pointer>;
            case Base + 4:  return pa_child_ptr<Base + 4, pointer>;
            case Base + 5:  return pa_child_ptr<Base + 5, pointer>;
            case Base + 6:  return pa_child_ptr<Base + 6, pointer>;
            case Base + 7:  return pa_child_ptr<Base + 7, pointer>;
            case Base + 8:  return pa_child_ptr<Base + 8, pointer>;
            case Base + 9:  return pa_child_ptr<Base + 9, pointer>;
            case Base + 10: return pa_child_ptr<Base + 10, pointer>;
            case Base + 11: return pa_child_ptr<Base + 11, pointer>;
            case Base + 12: return pa_child_ptr<Base + 12, pointer>;
            case Base + 13: return pa_child_ptr<Base + 13, pointer>;
            case Base + 14: return pa_child_ptr<Base + 14, pointer>;
            case Base + 15: return pa_child_ptr<Base + 15, pointer>;
            case Base + 16: return pa_child_ptr<Base + 16, pointer>;
            case Base + 17: return pa_child_ptr<Base + 17, pointer>;
            case Base + 18: return pa_child_ptr<Base + 18, pointer>;
            case Base + 19: return pa_child_ptr<Base + 19, pointer>;
            case Base + 20: return pa_child_ptr<Base + 20, pointer>;
            case Base + 21: return pa_child_ptr<Base + 21, pointer>;
            case Base + 22: return pa_child_ptr<Base + 22, pointer>;
            case Base + 23: return pa_child_ptr<Base + 23, pointer>;
            case Base + 24: return pa_child_ptr<Base + 24, pointer>;
            case Base + 25: return pa_child_ptr<Base + 25, pointer>;
            case Base + 26: return pa_child_ptr<Base + 26, pointer>;
            case Base + 27: return pa_child_ptr<Base + 27, pointer>;
            case Base + 28: return pa_child_ptr<Base + 28, pointer>;
            case Base + 29: return pa_child_ptr<Base + 29, pointer>;
            case Base + 30: return pa_child_ptr<Base + 30, pointer>;
            case Base + 31: return pa_child_ptr<Base + 31, pointer>;
            }
            std::unreachable();
        }
        template<size_t N, class pointer>
        constexpr inline pa_child_mem_ptr<pointer> pa_child_visitor(size_t i)noexcept
        {
            if (i >= N)
            {
                std::unreachable();
            }
            if constexpr (N <= 32)
            {
                return pa_child_visitor_32<N, pointer>(i);
            }
            else
            {
                if (i < N - 32)
                {
                    return pa_child_visitor<N - 32, pointer>(i);
                }
                return pa_child_visitor_32<N, pointer>(i);
            }
        }
        // 把"指向占位类型的节点指针"转成节点基类引用（节点一定是从 pa_placeholder 派生的）
        template<size_t N, class pointer>
        pa_node_base<N, pointer>& pa_as_node(pointer p)noexcept
        {
            return static_cast<pa_node_base<N, pointer>&>(*std::to_address(p));
        }
        // 取"第 i 个分支槽位"的引用（前置条件：i < N）
        template<size_t N, class pointer>
        pointer& pa_slot_at(pa_node_base<N, pointer>& n, size_t i)noexcept
        {
            return static_cast<pa_placeholder&>(n).*pa_child_visitor<N, pointer>(i);
        }
        template<size_t N, class pointer>
        const pointer& pa_slot_at(const pa_node_base<N, pointer>& n, size_t i)noexcept
        {
            return static_cast<const pa_placeholder&>(n).*pa_child_visitor<N, pointer>(i);
        }

        template<size_t N, class T, class Alloc>
        class pa_edge_proxy;
        template<size_t N, class T, class Alloc>
        class pa_edge_const_proxy;
        template<size_t N, class T, class Alloc>
        class pa_node_proxy;
        template<size_t N, class T, class Alloc>
        class pa_node_const_proxy;

        // ===================================================================
        // edge_proxy：一条边 = (父节点指针, 分支下标)，下标范围 [0, N]；
        //             下标 N 表示"该节点分支列表的尾后位置"。
        // ===================================================================
        template<size_t N, class T, class Alloc>
        class pa_edge_proxy
        {
        public:
            using pointer = typename std::allocator_traits<Alloc>::pointer;
            using node_type = pa_node<N, T, pointer>;
            using value_type = T;
            using reference = T&;
        private:
            pointer parent{};               // 父节点（哨位也是"父节点"）
            size_t idx{};                   // 分支下标，0..N
            pointer& slot()const noexcept
            {
                return pa_slot_at(pa_as_node<N>(parent), idx);
            }
        public:
            pa_edge_proxy() = default;
            pa_edge_proxy(const pa_edge_proxy&) = default;
            pa_edge_proxy(pointer parent, size_t idx)noexcept :parent{ parent }, idx{ idx }
            {
            }
            pa_edge_proxy& operator=(const pa_edge_proxy&) = default;
            // 边是否有效（父节点指针非空）
            bool valid()const noexcept
            {
                return bool(parent);
            }
            // 该边指向的分支上是否没有节点；尾后边恒为 true
            bool null()const noexcept
            {
                return idx == N || !bool(slot());
            }
            // 有效且指向真实节点（尾后边为 false）
            explicit operator bool()const noexcept
            {
                return valid() && (!null());
            }
            // 分支下标；尾后边返回 N
            size_t index()const noexcept
            {
                return idx;
            }
            // 是否是尾后边（下标 == N）
            bool is_end()const noexcept
            {
                return idx == N;
            }
            bool is_first_brother()const noexcept
            {
                return idx == 0;
            }
            bool is_last_brother()const noexcept
            {
                return idx + 1 == N;
            }
            // 取值：尾后边、空分支上是未定义行为
            T& operator*()const noexcept
            {
                return static_cast<node_type&>(*slot()).data.data;
            }
            T* operator->()const noexcept
            {
                return std::addressof(static_cast<node_type&>(*slot()).data.data);
            }
            // 纵向：移动到当前节点（或哨位）的第 i 个分支的真实边；i 必须 < N
            pa_edge_proxy& go_to_child(size_t i)noexcept
            {
                parent = slot();
                idx = i;
                return *this;
            }
            // 横向：移动到同一父节点的第 i 个分支（i 可以是 N，即尾后边）
            pa_edge_proxy& go_to_brother(size_t i)noexcept
            {
                idx = i;
                return *this;
            }
            pa_edge_proxy& go_to_first_brother()noexcept
            {
                idx = 0;
                return *this;
            }
            pa_edge_proxy& go_to_last_brother()noexcept
            {
                idx = N - 1;
                return *this;
            }
            pa_edge_proxy& go_to_prev_brother()noexcept
            {
                if (idx != 0)
                {
                    --idx;
                }
                return *this;
            }
            pa_edge_proxy& go_to_next_brother()noexcept
            {
                if (idx != N)
                {
                    ++idx;
                }
                return *this;
            }
            // 尾后边
            pa_edge_proxy& go_to_end()noexcept
            {
                idx = N;
                return *this;
            }
            // 向上：移动到"指向本边父节点的边"；父节点是哨位时得到无效边
            pa_edge_proxy& go_up()noexcept
            {
                pointer par = pa_as_node<N>(parent).parent;
                if (!par)
                {
                    parent = {};
                    idx = 0;
                    return *this;
                }
                size_t i = 0;
                for (size_t k = 0; k != N; ++k)
                {
                    if (pa_slot_at(pa_as_node<N>(par), k) == parent)
                    {
                        i = k;
                        break;
                    }
                }
                parent = par;
                idx = i;
                return *this;
            }
            pa_edge_proxy get_child(size_t i)const noexcept
            {
                return { slot(), i };
            }
            pa_edge_proxy get_brother(size_t i)const noexcept
            {
                return { parent, i };
            }
            pa_edge_proxy get_first_brother()const noexcept
            {
                return { parent, 0 };
            }
            pa_edge_proxy get_last_brother()const noexcept
            {
                return { parent, N - 1 };
            }
            pa_edge_proxy get_prev_brother()const noexcept
            {
                return { parent, idx == 0 ? 0 : idx - 1 };
            }
            pa_edge_proxy get_next_brother()const noexcept
            {
                return { parent, idx == N ? N : idx + 1 };
            }
            pa_edge_proxy get_end()const noexcept
            {
                return { parent, N };
            }
            pa_edge_proxy get_up()const noexcept
            {
                return pa_edge_proxy{ *this }.go_up();
            }
            // 全部 N 个真实分支（不含尾后边），第 i 个元素等于 get_brother(i)；定义在类外
            std::array<pa_edge_proxy<N, T, Alloc>, N> get_children()const noexcept;
            friend bool operator==(pa_edge_proxy l, pa_edge_proxy r)noexcept
            {
                return l.parent == r.parent && l.idx == r.idx;
            }
            friend struct std::hash<pa_edge_proxy>;
            friend class pa_edge_const_proxy<N, T, Alloc>;
            friend class pa_node_proxy<N, T, Alloc>;
            friend class pa_node_const_proxy<N, T, Alloc>;
            template<size_t N1, class T1, class Alloc1>
            friend class Yc::parent_aware_n_branches_tree;
        };
    }
}
// std::hash 特化：哈希 (父节点指针, 下标)；无效边返回 0
template<size_t N, class T, class Alloc>
struct std::hash<Yc::details::pa_edge_proxy<N, T, Alloc>>
{
    size_t operator()(const Yc::details::pa_edge_proxy<N, T, Alloc>& e)const noexcept
    {
        if (!e.valid())
        {
            return 0;
        }
        size_t h = std::hash<decltype(e.parent)>{}(e.parent);
        h ^= e.idx + 0x9e3779b97f4a7c15ull + (h << 6) + (h >> 2);
        return h;
    }
};
namespace Yc
{
    namespace details
    {
        // edge_const_proxy：结构相同，取值只读
        template<size_t N, class T, class Alloc>
        class pa_edge_const_proxy
        {
        public:
            using pointer = typename std::allocator_traits<Alloc>::pointer;
            using node_type = pa_node<N, T, pointer>;
            using value_type = T;
            using reference = const T&;
        private:
            pointer parent{};
            size_t idx{};
            pointer& slot()const noexcept
            {
                return pa_slot_at(pa_as_node<N>(parent), idx);
            }
        public:
            pa_edge_const_proxy() = default;
            pa_edge_const_proxy(const pa_edge_const_proxy&) = default;
            pa_edge_const_proxy(const pa_edge_proxy<N, T, Alloc>& p)noexcept :parent{ p.parent }, idx{ p.idx }
            {
            }
            pa_edge_const_proxy(pointer parent, size_t idx)noexcept :parent{ parent }, idx{ idx }
            {
            }
            pa_edge_const_proxy& operator=(const pa_edge_const_proxy&) = default;
            bool valid()const noexcept
            {
                return bool(parent);
            }
            bool null()const noexcept
            {
                return idx == N || !bool(slot());
            }
            explicit operator bool()const noexcept
            {
                return valid() && (!null());
            }
            size_t index()const noexcept
            {
                return idx;
            }
            bool is_end()const noexcept
            {
                return idx == N;
            }
            bool is_first_brother()const noexcept
            {
                return idx == 0;
            }
            bool is_last_brother()const noexcept
            {
                return idx + 1 == N;
            }
            const T& operator*()const noexcept
            {
                return static_cast<node_type&>(*slot()).data.data;
            }
            const T* operator->()const noexcept
            {
                return std::addressof(static_cast<node_type&>(*slot()).data.data);
            }
            pa_edge_const_proxy& go_to_child(size_t i)noexcept
            {
                parent = slot();
                idx = i;
                return *this;
            }
            pa_edge_const_proxy& go_to_brother(size_t i)noexcept
            {
                idx = i;
                return *this;
            }
            pa_edge_const_proxy& go_to_first_brother()noexcept
            {
                idx = 0;
                return *this;
            }
            pa_edge_const_proxy& go_to_last_brother()noexcept
            {
                idx = N - 1;
                return *this;
            }
            pa_edge_const_proxy& go_to_prev_brother()noexcept
            {
                if (idx != 0)
                {
                    --idx;
                }
                return *this;
            }
            pa_edge_const_proxy& go_to_next_brother()noexcept
            {
                if (idx != N)
                {
                    ++idx;
                }
                return *this;
            }
            pa_edge_const_proxy& go_to_end()noexcept
            {
                idx = N;
                return *this;
            }
            pa_edge_const_proxy& go_up()noexcept
            {
                pointer par = pa_as_node<N>(parent).parent;
                if (!par)
                {
                    parent = {};
                    idx = 0;
                    return *this;
                }
                size_t i = 0;
                for (size_t k = 0; k != N; ++k)
                {
                    if (pa_slot_at(pa_as_node<N>(par), k) == parent)
                    {
                        i = k;
                        break;
                    }
                }
                parent = par;
                idx = i;
                return *this;
            }
            pa_edge_const_proxy get_child(size_t i)const noexcept
            {
                return { slot(), i };
            }
            pa_edge_const_proxy get_brother(size_t i)const noexcept
            {
                return { parent, i };
            }
            pa_edge_const_proxy get_first_brother()const noexcept
            {
                return { parent, 0 };
            }
            pa_edge_const_proxy get_last_brother()const noexcept
            {
                return { parent, N - 1 };
            }
            pa_edge_const_proxy get_prev_brother()const noexcept
            {
                return { parent, idx == 0 ? 0 : idx - 1 };
            }
            pa_edge_const_proxy get_next_brother()const noexcept
            {
                return { parent, idx == N ? N : idx + 1 };
            }
            pa_edge_const_proxy get_end()const noexcept
            {
                return { parent, N };
            }
            pa_edge_const_proxy get_up()const noexcept
            {
                return pa_edge_const_proxy{ *this }.go_up();
            }
            std::array<pa_edge_const_proxy<N, T, Alloc>, N> get_children()const noexcept;
            friend bool operator==(pa_edge_const_proxy l, pa_edge_const_proxy r)noexcept
            {
                return l.parent == r.parent && l.idx == r.idx;
            }
            friend struct std::hash<pa_edge_const_proxy>;
            friend class pa_node_const_proxy<N, T, Alloc>;
            template<size_t N1, class T1, class Alloc1>
            friend class Yc::parent_aware_n_branches_tree;
        };
    }
}
template<size_t N, class T, class Alloc>
struct std::hash<Yc::details::pa_edge_const_proxy<N, T, Alloc>>
{
    size_t operator()(const Yc::details::pa_edge_const_proxy<N, T, Alloc>& e)const noexcept
    {
        if (!e.valid())
        {
            return 0;
        }
        size_t h = std::hash<decltype(e.parent)>{}(e.parent);
        h ^= e.idx + 0x9e3779b97f4a7c15ull + (h << 6) + (h >> 2);
        return h;
    }
};
namespace Yc
{
    namespace details
    {
        // ===================================================================
        // node_proxy：直接代表一个节点（不是槽位），因此不带"第几个分支"的信息，
        //             不能横向游走；但可以向上（go_up）和向下（go_to_child）。
        // ===================================================================
        template<size_t N, class T, class Alloc>
        class pa_node_proxy
        {
        public:
            using pointer = typename std::allocator_traits<Alloc>::pointer;
            using node_type = pa_node<N, T, pointer>;
            using value_type = T;
            using reference = T&;
        private:
            pointer node{};
        public:
            pa_node_proxy() = default;
            pa_node_proxy(const pa_node_proxy&) = default;
            pa_node_proxy(pointer node)noexcept :node{ node }
            {
            }
            // 从边构造：取该边指向的子节点；尾后边/无效边得到空代理
            pa_node_proxy(pa_edge_proxy<N, T, Alloc> e)noexcept
                :node{ (e.valid() && e.idx != N) ? pa_slot_at(pa_as_node<N>(e.parent), e.idx) : pointer{} }
            {
            }
            pa_node_proxy& operator=(const pa_node_proxy&) = default;
            bool null()const noexcept
            {
                return !bool(node);
            }
            explicit operator bool()const noexcept
            {
                return !null();
            }
            T& operator*()const noexcept
            {
                return static_cast<node_type&>(*std::to_address(node)).data.data;
            }
            T* operator->()const noexcept
            {
                return std::addressof(static_cast<node_type&>(*std::to_address(node)).data.data);
            }
            pa_node_proxy& go_to_child(size_t i)noexcept
            {
                node = pa_slot_at(pa_as_node<N>(node), i);
                return *this;
            }
            pa_node_proxy& go_up()noexcept
            {
                node = pa_as_node<N>(node).parent;
                return *this;
            }
            pa_node_proxy get_child(size_t i)const noexcept
            {
                return { pa_slot_at(pa_as_node<N>(node), i) };
            }
            pa_node_proxy get_parent()const noexcept
            {
                return { pa_as_node<N>(node).parent };
            }
            // 全部 N 个分支的边（不含尾后边）
            std::array<pa_edge_proxy<N, T, Alloc>, N> get_children()const noexcept;
            friend bool operator==(pa_node_proxy l, pa_node_proxy r)noexcept
            {
                return l.node == r.node;
            }
            friend struct std::hash<pa_node_proxy>;
            friend class pa_node_const_proxy<N, T, Alloc>;
            template<size_t N1, class T1, class Alloc1>
            friend class Yc::parent_aware_n_branches_tree;
        };
    }
}
template<size_t N, class T, class Alloc>
struct std::hash<Yc::details::pa_node_proxy<N, T, Alloc>>
{
    size_t operator()(const Yc::details::pa_node_proxy<N, T, Alloc>& n)const noexcept
    {
        return n.null() ? 0 : std::hash<decltype(n.node)>{}(n.node);
    }
};
namespace Yc
{
    namespace details
    {
        // node_const_proxy：节点代理的只读版本
        template<size_t N, class T, class Alloc>
        class pa_node_const_proxy
        {
        public:
            using pointer = typename std::allocator_traits<Alloc>::pointer;
            using node_type = pa_node<N, T, pointer>;
            using value_type = T;
            using reference = const T&;
        private:
            pointer node{};
        public:
            pa_node_const_proxy() = default;
            pa_node_const_proxy(const pa_node_const_proxy&) = default;
            pa_node_const_proxy(pa_node_proxy<N, T, Alloc> p)noexcept :node{ p.node }
            {
            }
            pa_node_const_proxy(pointer node)noexcept :node{ node }
            {
            }
            // 从边构造：取该边指向的子节点；尾后边/无效边得到空代理
            pa_node_const_proxy(pa_edge_const_proxy<N, T, Alloc> e)noexcept
                :node{ (e.valid() && e.idx != N) ? pa_slot_at(pa_as_node<N>(e.parent), e.idx) : pointer{} }
            {
            }
            pa_node_const_proxy& operator=(const pa_node_const_proxy&) = default;
            bool null()const noexcept
            {
                return !bool(node);
            }
            explicit operator bool()const noexcept
            {
                return !null();
            }
            const T& operator*()const noexcept
            {
                return static_cast<node_type&>(*std::to_address(node)).data.data;
            }
            const T* operator->()const noexcept
            {
                return std::addressof(static_cast<node_type&>(*std::to_address(node)).data.data);
            }
            pa_node_const_proxy& go_to_child(size_t i)noexcept
            {
                node = pa_slot_at(pa_as_node<N>(node), i);
                return *this;
            }
            pa_node_const_proxy& go_up()noexcept
            {
                node = pa_as_node<N>(node).parent;
                return *this;
            }
            pa_node_const_proxy get_child(size_t i)const noexcept
            {
                return { pa_slot_at(pa_as_node<N>(node), i) };
            }
            pa_node_const_proxy get_parent()const noexcept
            {
                return { pa_as_node<N>(node).parent };
            }
            std::array<pa_edge_const_proxy<N, T, Alloc>, N> get_children()const noexcept;
            friend bool operator==(pa_node_const_proxy l, pa_node_const_proxy r)noexcept
            {
                return l.node == r.node;
            }
            friend struct std::hash<pa_node_const_proxy>;
            template<size_t N1, class T1, class Alloc1>
            friend class Yc::parent_aware_n_branches_tree;
        };
    }
}
template<size_t N, class T, class Alloc>
struct std::hash<Yc::details::pa_node_const_proxy<N, T, Alloc>>
{
    size_t operator()(const Yc::details::pa_node_const_proxy<N, T, Alloc>& n)const noexcept
    {
        return n.null() ? 0 : std::hash<decltype(n.node)>{}(n.node);
    }
};
namespace Yc
{
    namespace details
    {
        // get_children 的类外定义（返回类型里用到代理自身，类内定义时类型还不完整）
        template<size_t N, class T, class Alloc>
        inline std::array<pa_edge_proxy<N, T, Alloc>, N>
            pa_edge_proxy<N, T, Alloc>::get_children()const noexcept
        {
            std::array<pa_edge_proxy<N, T, Alloc>, N> ret{};
            pointer child = slot();                 // 本边所指节点的 N 个分支
            for (size_t i = 0; i != N; ++i)
            {
                ret[i] = { child, i };
            }
            return ret;
        }
        template<size_t N, class T, class Alloc>
        inline std::array<pa_edge_const_proxy<N, T, Alloc>, N>
            pa_edge_const_proxy<N, T, Alloc>::get_children()const noexcept
        {
            std::array<pa_edge_const_proxy<N, T, Alloc>, N> ret{};
            pointer child = slot();
            for (size_t i = 0; i != N; ++i)
            {
                ret[i] = { child, i };
            }
            return ret;
        }
        template<size_t N, class T, class Alloc>
        inline std::array<pa_edge_proxy<N, T, Alloc>, N>
            pa_node_proxy<N, T, Alloc>::get_children()const noexcept
        {
            std::array<pa_edge_proxy<N, T, Alloc>, N> ret{};
            for (size_t i = 0; i != N; ++i)
            {
                ret[i] = { node, i };
            }
            return ret;
        }
        template<size_t N, class T, class Alloc>
        inline std::array<pa_edge_const_proxy<N, T, Alloc>, N>
            pa_node_const_proxy<N, T, Alloc>::get_children()const noexcept
        {
            std::array<pa_edge_const_proxy<N, T, Alloc>, N> ret{};
            for (size_t i = 0; i != N; ++i)
            {
                ret[i] = { node, i };
            }
            return ret;
        }
        // 多态 get_children 函数对象
        struct pa_get_children_t
        {
            template<size_t N, class T, class Alloc>
            auto operator()(pa_edge_proxy<N, T, Alloc> p)const noexcept
            {
                return p.get_children();
            }
            template<size_t N, class T, class Alloc>
            auto operator()(pa_edge_const_proxy<N, T, Alloc> p)const noexcept
            {
                return p.get_children();
            }
            template<size_t N, class T, class Alloc>
            auto operator()(pa_node_proxy<N, T, Alloc> p)const noexcept
            {
                return p.get_children();
            }
            template<size_t N, class T, class Alloc>
            auto operator()(pa_node_const_proxy<N, T, Alloc> p)const noexcept
            {
                return p.get_children();
            }
        };
    }
    namespace parent_aware_n_branches_tree_functional
    {
        constexpr inline Yc::details::pa_get_children_t get_children{};
    }
    // ===================================================================
    // parent_aware_n_branches_tree：父节点感知的 N 叉树容器
    //   - 哨位：容器内嵌一个"没有 T 的节点"，它的第 0 个分支指向真根、parent 为空；
    //     真根的 parent 指向哨位，所以 go_up 对整棵树都是良定义的（从根再往上得到无效边）。
    //   - 一条边 = (父节点, 分支下标)；下标 N 是尾后边（不可解引用，可比较、可哈希）。
    // ===================================================================
    template<size_t N, class T, class Alloc>
    class parent_aware_n_branches_tree
    {
    public:
        using allocator_type = Alloc;
        using value_type = T;
        using reference = T&;
        using pointer = std::allocator_traits<Alloc>::pointer;
        using pa_alloc = typename std::allocator_traits<Alloc>::template rebind_alloc<details::pa_placeholder>;
        using edge_proxy = details::pa_edge_proxy<N, T, pa_alloc>;
        using edge_const_proxy = details::pa_edge_const_proxy<N, T, pa_alloc>;
        using node_proxy = details::pa_node_proxy<N, T, pa_alloc>;
        using node_const_proxy = details::pa_node_const_proxy<N, T, pa_alloc>;
    private:
        using node_pointer = std::allocator_traits<Alloc>::template rebind_traits<details::pa_placeholder>::pointer;
        using placeholder_alloc = std::allocator_traits<Alloc>::template rebind_alloc<details::pa_placeholder>;
        using node_base_type = details::pa_node_base<N, node_pointer>;
        using node_type = details::pa_node<N, T, node_pointer>;
        using node_allocator = std::allocator_traits<Alloc>::template rebind_alloc<node_type>;
        using node_alloc_traits = std::allocator_traits<node_allocator>;
        [[no_unique_address]] Alloc alloc{};
        // 哨位节点：第 0 个分支是指向真根的槽位，parent 为空
        node_base_type sentinel{};
        node_pointer sentinel_pointer()noexcept
        {
            return std::pointer_traits<node_pointer>::pointer_to(static_cast<details::pa_placeholder&>(sentinel));
        }
        node_pointer sentinel_pointer()const noexcept
        {
            return std::pointer_traits<node_pointer>::pointer_to
            (const_cast<details::pa_placeholder&>(static_cast<const details::pa_placeholder&>(sentinel)));
        }
        // 取边所指的槽位（容器是代理的友元）
        static node_pointer& slot_of(edge_const_proxy p)noexcept
        {
            return details::pa_slot_at(details::pa_as_node<N>(p.parent), p.idx);
        }
        // 写槽位并维护 parent
        static void set_slot(edge_const_proxy p, node_pointer child)noexcept
        {
            slot_of(p) = child;
            if (child)
            {
                details::pa_as_node<N>(child).parent = p.parent;
            }
        }
        node_pointer allocate()
        {
            node_allocator a{ alloc };
            node_type* n = std::to_address(node_alloc_traits::allocate(a, 1));
            node_alloc_traits::construct(a, n);
            return std::pointer_traits<node_pointer>::pointer_to(static_cast<details::pa_placeholder&>(*n));
        }
        void deallocate(node_pointer p)noexcept
        {
            node_allocator a{ alloc };
            node_type* n = static_cast<node_type*>(std::to_address(p));
            node_alloc_traits::destroy(a, n);
            node_alloc_traits::deallocate(a, std::pointer_traits<typename node_alloc_traits::pointer>::pointer_to(*n), 1);
        }
        static void swap_child_slots(node_pointer a, node_pointer b)noexcept
        {
            for (size_t i = 0; i != N; ++i)
            {
                std::swap(details::pa_slot_at(pa_as_node<N>(a), i), details::pa_slot_at(pa_as_node<N>(b), i));
            }
        }
    private:
        // 递归写入实现：先写本节点，再逐个分支递归（分支处理顺序未指定）
        template<class ValueGetter, class ChildrenGetter, class InitializeHandle>
        void recur_and_write_impl(edge_const_proxy p, ValueGetter& vg, ChildrenGetter& cg, InitializeHandle h)
        {
            if (!h)
            {
                return;
            }
            emplace(p, std::invoke(vg, h));
            auto hs = std::invoke(cg, h);
            for (size_t i = 0; i != N; ++i)
            {
                recur_and_write_impl(p.get_child(i), vg, cg, hs[i]);
            }
        }
        template<class Proxy, class F>
        static void for_each_impl(Proxy p, F& f)
        {
            if (p.null())
            {
                return;
            }
            std::invoke(f, *p);
            for (size_t i = 0; i != N; ++i)
            {
                for_each_impl(p.get_child(i), f);
            }
        }
        // 从别的树接管一棵子树的根：把它的 parent 指向本树哨位
        void adopt(node_pointer r)noexcept
        {
            details::pa_slot_at(sentinel, 0) = r;
            if (r)
            {
                details::pa_as_node<N>(r).parent = sentinel_pointer();
            }
        }
        void take_root_from(parent_aware_n_branches_tree& b)noexcept
        {
            adopt(std::exchange(details::pa_slot_at(b.sentinel, 0), {}));
        }
    public:
        // 递归写入：从外部树形结构写入 p，返回 p 处原有的子树
        template<class ValueGetter, class ChildrenGetter, class InitializeHandle>
        parent_aware_n_branches_tree recur_and_write(edge_const_proxy p, ValueGetter vg, ChildrenGetter cg, InitializeHandle h)
        {
            parent_aware_n_branches_tree ret{ cut(p) };
            struct _guard
            {
                parent_aware_n_branches_tree* t;
                edge_const_proxy p;
                parent_aware_n_branches_tree* r;
                ~_guard()
                {
                    if (t)
                    {
                        t->splice(p, *r);
                    }
                }
            }guard{ this, p, &ret };
            if (h)
            {
                recur_and_write_impl(p, vg, cg, h);
            }
            guard.t = nullptr;
            return ret;
        }
        parent_aware_n_branches_tree() = default;
        explicit parent_aware_n_branches_tree(const Alloc& a)noexcept :alloc{ a }
        {
        }
        template<class ValueGetter, class ChildrenGetter, class InitializeHandle>
        parent_aware_n_branches_tree(ValueGetter vg, ChildrenGetter cg, InitializeHandle h, const Alloc& a = {}) :alloc{ a }
        {
            if (h)
            {
                recur_and_write_impl(root(), vg, cg, h);
            }
        }
        parent_aware_n_branches_tree(const parent_aware_n_branches_tree& b)
            :parent_aware_n_branches_tree{ std::allocator_traits<Alloc>::select_on_container_copy_construction(b.alloc) }
        {
            auto copier = [](edge_const_proxy p) -> const T&
                {
                    return *p;
                };
            recur_and_write(root(), copier, parent_aware_n_branches_tree_functional::get_children, b.croot());
        }
        parent_aware_n_branches_tree& operator=(const parent_aware_n_branches_tree& b)
        {
            if (this == &b)
            {
                return *this;
            }
            Alloc tmp{ alloc };
            parent_aware_n_branches_tree t1{ cut(root()) };
            struct _guard
            {
                parent_aware_n_branches_tree* t{};
                Alloc* tmp{};
                parent_aware_n_branches_tree* other{};
                ~_guard()
                {
                    if (t)
                    {
                        t->alloc = *tmp;
                        t->splice(t->root(), *other);
                    }
                }
            }guard{ this, &tmp, &t1 };
            if constexpr (std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value)
            {
                alloc = b.alloc;
            }
            auto copier = [](edge_const_proxy p) -> const T&
                {
                    return *p;
                };
            recur_and_write(root(), copier, parent_aware_n_branches_tree_functional::get_children, b.croot());
            guard.t = nullptr;
            return *this;
        }
        parent_aware_n_branches_tree(parent_aware_n_branches_tree&& b)noexcept :alloc{ std::move(b.alloc) }
        {
            take_root_from(b);
        }
        parent_aware_n_branches_tree& operator=(parent_aware_n_branches_tree&& b)noexcept(
            std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value ||
            std::allocator_traits<Alloc>::is_always_equal::value)
        {
            if constexpr (std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value)
            {
                std::swap(alloc, b.alloc);
                take_root_from(b);
                return *this;
            }
            else
            {
                if (alloc == b.alloc)
                {
                    take_root_from(b);
                    return *this;
                }
                auto vg = [](edge_proxy p) -> T&&
                    {
                        return (T&&)*p;
                    };
                parent_aware_n_branches_tree tmp{ vg, parent_aware_n_branches_tree_functional::get_children, b.root(), alloc };
                swap(tmp);
                return *this;
            }
        }
        ~parent_aware_n_branches_tree()
        {
            clear();
        }
        bool empty()const noexcept
        {
            return croot().null();
        }
        Alloc get_allocator()const noexcept
        {
            return alloc;
        }
        edge_proxy root()noexcept
        {
            return { sentinel_pointer(), 0 };
        }
        edge_const_proxy root()const noexcept
        {
            return { sentinel_pointer(), 0 };
        }
        edge_const_proxy croot()const noexcept
        {
            return { sentinel_pointer(), 0 };
        }
        node_proxy nroot()noexcept
        {
            return root();
        }
        node_const_proxy nroot()const noexcept
        {
            return root();
        }
        node_const_proxy cnroot()const noexcept
        {
            return root();
        }
        // 哨位节点本身（"根的父节点"）
        node_proxy parent_of_nroot()noexcept
        {
            return { sentinel_pointer() };
        }
        node_const_proxy parent_of_cnroot()const noexcept
        {
            return { sentinel_pointer() };
        }
        // 切割：把 p 处子树摘下来，返回持有它的新树；p 处置空
        parent_aware_n_branches_tree cut(edge_const_proxy p)noexcept
        {
            parent_aware_n_branches_tree ret{ alloc };
            ret.adopt(std::exchange(slot_of(p), {}));
            return ret;
        }
        // 拼接：把 from 子树移到 to 处，返回 to 处原有的子树
        parent_aware_n_branches_tree splice(edge_const_proxy to, edge_const_proxy from)noexcept
        {
            parent_aware_n_branches_tree ret{ cut(to) };
            set_slot(to, std::exchange(slot_of(from), {}));
            return ret;
        }
        parent_aware_n_branches_tree splice(edge_const_proxy p, parent_aware_n_branches_tree& tree)noexcept
        {
            return splice(p, tree.root());
        }
        parent_aware_n_branches_tree splice(edge_const_proxy p, parent_aware_n_branches_tree&& tree)noexcept
        {
            return splice(p, tree.root());
        }
        // 交换两个子树（同时维护 parent）
        static void swap_sub_tree(edge_const_proxy l, edge_const_proxy r)noexcept
        {
            std::swap(slot_of(l), slot_of(r));
            if (slot_of(l))
            {
                details::pa_as_node<N>(slot_of(l)).parent = l.parent;
            }
            if (slot_of(r))
            {
                details::pa_as_node<N>(slot_of(r)).parent = r.parent;
            }
        }
        void swap(parent_aware_n_branches_tree& other)noexcept
        {
            if constexpr (std::allocator_traits<Alloc>::propagate_on_container_swap::value)
            {
                std::swap(alloc, other.alloc);
            }
            swap_sub_tree(root(), other.root());
        }
        // 交换两个节点：两个位置上的值互换、子树留在原位（父子相邻同样成立）
        static void swap_node(edge_proxy l, edge_proxy r)noexcept
        {
            node_pointer a = slot_of(l);
            node_pointer b = slot_of(r);
            swap_sub_tree(l, r);
            swap_child_slots(a, b);
            for (size_t i = 0; i != N; ++i)
            {
                if (details::pa_slot_at(pa_as_node<N>(a), i))
                {
                    details::pa_as_node<N>(details::pa_slot_at(pa_as_node<N>(a), i)).parent = a;
                }
                if (details::pa_slot_at(pa_as_node<N>(b), i))
                {
                    details::pa_as_node<N>(details::pa_slot_at(pa_as_node<N>(b), i)).parent = b;
                }
            }
        }
        // 删除 p 处节点及其整棵子树（迭代实现，深树不压爆调用栈）
        void erase(edge_const_proxy p)noexcept
        {
            if (p.null())
            {
                return;
            }
            while (true)
            {
                node_pointer n = slot_of(p);
                // 反向扫描最后一个非空分支
                size_t last = N;
                while (last != 0 && !details::pa_slot_at(pa_as_node<N>(n), last - 1))
                {
                    --last;
                }
                if (last == 0)
                {
                    deallocate(n);
                    slot_of(p) = nullptr;
                    return;
                }
                for (size_t i = 0; i + 1 != last; ++i)
                {
                    erase(p.get_child(i));
                }
                node_pointer promoted = details::pa_slot_at(pa_as_node<N>(n), last - 1);
                deallocate(n);
                set_slot(p, promoted);
            }
        }
        void clear()noexcept
        {
            erase(root());
        }
        // 遍历（顺序未指定），函数对象按值传入、按值返回
        template<class F>
        F for_each(F f)noexcept(std::is_nothrow_invocable_v<F&, T&>&& std::is_nothrow_move_constructible_v<F>)
        {
            for_each_impl(root(), f);
            return f;
        }
        template<class F>
        F for_each(F f)const noexcept(std::is_nothrow_invocable_v<F&, const T&>&& std::is_nothrow_move_constructible_v<F>)
        {
            for_each_impl(croot(), f);
            return f;
        }
        // 在指定位置构造新节点（原有子树作为返回值）
        template<class... Args>
        parent_aware_n_branches_tree emplace(edge_const_proxy p, Args&&... args)
        {
            node_pointer new_node = allocate();
            if constexpr (std::is_nothrow_constructible_v<T, Args...>)
            {
                std::allocator_traits<Alloc>::
                    construct(alloc, std::addressof(static_cast<node_type&>(*std::to_address(new_node)).data.data), std::forward<Args>(args)...);
            }
            else
            {
                struct guard_t
                {
                    parent_aware_n_branches_tree* t;
                    node_pointer ptr;
                    ~guard_t()
                    {
                        if (ptr != nullptr)
                        {
                            t->deallocate(ptr);
                        }
                    }
                }guard{ this, new_node };
                std::allocator_traits<Alloc>::
                    construct(alloc, std::addressof(static_cast<node_type&>(*std::to_address(new_node)).data.data), std::forward<Args>(args)...);
                guard.ptr = nullptr;
            }
            parent_aware_n_branches_tree ret{ cut(p) };
            set_slot(p, new_node);
            return ret;
        }
        parent_aware_n_branches_tree insert(edge_const_proxy p, const T& v)
        {
            return emplace(p, v);
        }
        parent_aware_n_branches_tree insert(edge_const_proxy p, T&& v)
        {
            return emplace(p, std::move(v));
        }
    };
    // 特化 swap 自由函数
    template<size_t N, class T, class Alloc>
    void swap(parent_aware_n_branches_tree<N, T, Alloc>& l, parent_aware_n_branches_tree<N, T, Alloc>& r)noexcept
    {
        l.swap(r);
    }
}

