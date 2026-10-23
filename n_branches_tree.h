// N 叉树库（无父节点信息版本）
//
// 与 binary_tree.h 共用同一套"一切皆边"的抽象，区别只在节点里的分支数 N 是编译期常量：
//   - 一条边 = 指向某个 child 槽位的二级指针（pointer_to_pointer），与 binary_tree::edge_proxy 同构；
//   - 节点里的 N 个槽位排成一个 pointer[N] 数组，所以二级指针天然支持偏移：
//     ++ptr / --ptr 就是"走到下一个/上一个兄弟"，既不需要父节点指针，也不需要成员指针技巧；
//   - 没有虚拟父节点哨位，所以也不需要多继承魔法：节点基类只继承一个空类型
//     （n_branches_tree_empty_placeholder），它同时是分配器统一 rebind 的目标和
//     pointer 的 element_type，二级指针解引用后 static_cast 成节点基类即可。
//
// 与 parent_aware_n_branches_tree.h 的差别只在"一条边能记住多少上下文"：
//   - 那边：边 = (父节点指针, 分支下标)，横向游走靠改下标，因此能随机访问兄弟
//     （go_to_last_brother / is_last_brother 之类）；
//   - 这边：边 = 槽位地址，横向游走靠指针偏移（只能逐格 go_to_next_brother /
//     go_to_prev_brother），随机访问兄弟要从父边 get_child(i) 重新开始。
//   换来的是代理只有 8 字节、取槽位只有一次取地址（没有父指针、没有下标、没有查表）。
//
// 命名约定与另外三个头文件一致：
//   go_*  原地改自身、返回 *this（可链式调用）
//   get_* 返回新的代理对象，自身不动
//
// 说明：N 叉树没有"左旋/右旋"这种二叉树操作；swap_node 也不需要 binary_tree 里那套
// 相邻形态检查——节点里就是一排等价槽位，交换槽位 + 逐个交换孩子数组即可（父子相邻
// 的情形会自动成立，见实现注释）。
#pragma once

// MSVC 不支持标准 [[no_unique_address]]，要用它的 [[msvc::no_unique_address]]
#ifndef YC_NO_UNIQUE_ADDRESS
#if defined(_MSC_VER)
#define YC_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#define YC_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif
#endif

#include <array>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>
#include "union_data.h"

namespace Yc
{
    template<size_t N, class T, class Alloc = std::allocator<T>>
    class n_branches_tree;
    namespace details
    {
        // ===================================================================
        // 节点层次结构：
        //   n_branches_tree_empty_placeholder       （空类型：分配器统一 rebind 的目标）
        //   └─ n_branches_tree_node_base<N, Alloc>  （pointer child[N] 槽位数组）
        //       └─ n_branches_tree_node<N, T, Alloc>（附加 data_t<T>）
        //
        // 与 binary_tree 的节点相比，唯一区别是 left/right 变成了一排 child 槽位：
        // 槽位是连续数组，所以"兄弟"在二级指针层面就是相邻地址，偏移即可到达。
        // ===================================================================
        struct n_branches_tree_empty_placeholder
        {
        };
        template<size_t N, class Alloc>
        struct n_branches_tree_node_base : n_branches_tree_empty_placeholder
        {
            static_assert(std::is_same_v<typename std::allocator_traits<Alloc>::value_type,
                n_branches_tree_empty_placeholder>);
            static_assert(N != 0, "N 叉树的 N 必须 >= 1");
            using pointer = std::allocator_traits<Alloc>::pointer;
            // 槽位数组：二级指针 ±1 就是兄弟偏移；默认零初始化，空槽位天然是空指针
            pointer child[N]{};
        };
        // 完整节点：继承槽位数组并附加用户数据
        template<size_t N, class T, class Alloc>
        struct n_branches_tree_node : n_branches_tree_node_base<N, Alloc>
        {
            data_t<T> data;
        };

        template<size_t N, class T, class Alloc>
        class n_branches_tree_edge_const_proxy;
        template<size_t N, class T, class Alloc>
        class n_branches_tree_node_proxy;
        template<size_t N, class T, class Alloc>
        class n_branches_tree_node_const_proxy;

        // ===================================================================
        // edge_proxy：一条边 = 指向某个 child 槽位的二级指针
        //
        //   *ptr  ：槽位本身（pointer&），容器做 cut/splice/emplace 时直接读写它；
        //   **ptr ：槽位指向的节点基类（n_branches_tree_empty_placeholder&），
        //           static_cast 成 node_base 后就能取到 child[i]。
        // ===================================================================
        template<size_t N, class T, class Alloc>
        class n_branches_tree_edge_proxy
        {
            using pointer = std::allocator_traits<Alloc>::pointer;
            using pointer_to_pointer = std::allocator_traits<Alloc>::template rebind_traits<pointer>::pointer;
            using node_base = n_branches_tree_node_base<N, Alloc>;
            // ptr：指向节点里某个 child 槽位
            pointer_to_pointer ptr{};
            static void check_index(size_t i)noexcept
            {
                if (i >= N)
                {
                    std::unreachable();
                }
            }
        public:
            n_branches_tree_edge_proxy() = default;
            n_branches_tree_edge_proxy(const n_branches_tree_edge_proxy&) = default;
            n_branches_tree_edge_proxy(pointer_to_pointer ptr)noexcept :ptr{ ptr }
            {
            }
            n_branches_tree_edge_proxy& operator=(const n_branches_tree_edge_proxy&) = default;
            // 边是否有效（槽位地址非空）
            bool valid()const noexcept
            {
                return (bool)ptr;
            }
            // 边所指的子节点是否为空
            bool null()const noexcept
            {
                return !(bool)*ptr;
            }
            // 边有效且子节点非空
            explicit operator bool()const noexcept
            {
                return valid() && (!null());
            }
            // 纵向：走到"当前子节点"的第 i 个孩子（i < N 由契约保证）
            n_branches_tree_edge_proxy& go_to_child(size_t i)noexcept
            {
                check_index(i);
                ptr = std::pointer_traits<pointer_to_pointer>::pointer_to
                (static_cast<node_base&>(**ptr).child[i]);
                return *this;
            }
            // 横向：槽位是连续数组，二级指针 ±1 就跨了一个兄弟
            n_branches_tree_edge_proxy& go_to_next_brother()noexcept
            {
                ++ptr;
                return *this;
            }
            n_branches_tree_edge_proxy& go_to_prev_brother()noexcept
            {
                --ptr;
                return *this;
            }
            // 一次跨 n 个兄弟：n 的符号就是方向（+ 往后、- 往前）；
            // 与单步版一样，调用方负责不越过父节点的那 N 个槽位
            n_branches_tree_edge_proxy& go_to_next_brother(ptrdiff_t n)noexcept
            {
                ptr += n;
                return *this;
            }
            n_branches_tree_edge_proxy& go_to_prev_brother(ptrdiff_t n)noexcept
            {
                ptr -= n;
                return *this;
            }
            // 解引用：获取子节点中存储的值的引用
            T& operator*()const noexcept
            {
                return static_cast<n_branches_tree_node<N, T, Alloc>&>(**ptr).data.data;
            }
            T* operator->()const noexcept
            {
                return std::addressof(static_cast<n_branches_tree_node<N, T, Alloc>&>(**ptr).data.data);
            }
            // 不修改自身，返回第 i 个孩子的边
            n_branches_tree_edge_proxy get_child(size_t i)const noexcept
            {
                check_index(i);
                return { std::pointer_traits<pointer_to_pointer>::pointer_to
                    (static_cast<node_base&>(**ptr).child[i]) };
            }
            n_branches_tree_edge_proxy get_next_brother()const noexcept
            {
                n_branches_tree_edge_proxy ret{ *this };
                ++ret.ptr;
                return ret;
            }
            n_branches_tree_edge_proxy get_prev_brother()const noexcept
            {
                n_branches_tree_edge_proxy ret{ *this };
                --ret.ptr;
                return ret;
            }
            n_branches_tree_edge_proxy get_next_brother(ptrdiff_t n)const noexcept
            {
                n_branches_tree_edge_proxy ret{ *this };
                ret.ptr += n;
                return ret;
            }
            n_branches_tree_edge_proxy get_prev_brother(ptrdiff_t n)const noexcept
            {
                n_branches_tree_edge_proxy ret{ *this };
                ret.ptr -= n;
                return ret;
            }
            // 全部 N 个子边（第 i 个元素等于 get_child(i)）；定义在类外，见下
                        std::array<n_branches_tree_edge_proxy, N> get_children()const noexcept;
            // 尾后边：本边所指节点的第 N 个分支位置。它不能解引用（operator* / operator-> /
            // null / go_to_child 都是未定义行为），但可以复制、判等、哈希，用来当分支遍历的哨位。
            n_branches_tree_edge_proxy get_child_end()const noexcept
            {
                n_branches_tree_edge_proxy ret = get_child(0);
                ret.ptr += static_cast<ptrdiff_t>(N);
                return ret;
            }
            // 比较两条边是否指向同一个槽位
            friend bool operator==(n_branches_tree_edge_proxy l, n_branches_tree_edge_proxy r)noexcept
            {
                return l.ptr == r.ptr;
            }
            friend struct std::hash<n_branches_tree_edge_proxy>;
            friend class n_branches_tree_edge_const_proxy<N, T, Alloc>;
            template<size_t N1, class T1, class Alloc1>
            friend class Yc::n_branches_tree;
            friend class n_branches_tree_node_proxy<N, T, Alloc>;
            // 与迭代器约定一致：const 性由 reference 表达，value_type 不带 const
            using value_type = T;
            using reference = T&;
        };
    }
}
// std::hash 特化：哈希槽位地址；无效边（默认构造的代理）返回 0
template<size_t N, class T, class Alloc>
struct std::hash<Yc::details::n_branches_tree_edge_proxy<N, T, Alloc>>
{
    size_t operator()(const Yc::details::n_branches_tree_edge_proxy<N, T, Alloc>& para)const noexcept
    {
        if (!para.valid())
            return 0;
        return std::hash<decltype(para.ptr)>{}(para.ptr);
    }
};

namespace Yc
{
    namespace details
    {
        // edge_const_proxy：edge_proxy 的常量版本（结构相同，解引用返回 const T&）
        template<size_t N, class T, class Alloc>
        class n_branches_tree_edge_const_proxy
        {
            using pointer = std::allocator_traits<Alloc>::pointer;
            using pointer_to_pointer = std::allocator_traits<Alloc>::template rebind_traits<pointer>::pointer;
            using node_base = n_branches_tree_node_base<N, Alloc>;
            pointer_to_pointer ptr{};
            static void check_index(size_t i)noexcept
            {
                if (i >= N)
                {
                    std::unreachable();
                }
            }
            // 构造函数本身私有，这样 pointer_to_pointer 不会构成一条对外的隐式转换路径
            static n_branches_tree_edge_const_proxy make(pointer_to_pointer p)noexcept
            {
                return n_branches_tree_edge_const_proxy{ p };
            }
            n_branches_tree_edge_const_proxy(pointer_to_pointer ptr)noexcept :ptr{ ptr }
            {
            }
        public:
            n_branches_tree_edge_const_proxy() = default;
            n_branches_tree_edge_const_proxy(const n_branches_tree_edge_const_proxy&) = default;
            n_branches_tree_edge_const_proxy(const n_branches_tree_edge_proxy<N, T, Alloc>& p)noexcept :ptr{ p.ptr }
            {
            }
            n_branches_tree_edge_const_proxy& operator=(const n_branches_tree_edge_const_proxy&) = default;
            bool valid()const noexcept
            {
                return (bool)ptr;
            }
            bool null()const noexcept
            {
                return !(bool)*ptr;
            }
            explicit operator bool()const noexcept
            {
                return valid() && (!null());
            }
            n_branches_tree_edge_const_proxy& go_to_child(size_t i)noexcept
            {
                check_index(i);
                ptr = std::pointer_traits<pointer_to_pointer>::pointer_to
                (static_cast<node_base&>(**ptr).child[i]);
                return *this;
            }
            n_branches_tree_edge_const_proxy& go_to_next_brother()noexcept
            {
                ++ptr;
                return *this;
            }
            n_branches_tree_edge_const_proxy& go_to_prev_brother()noexcept
            {
                --ptr;
                return *this;
            }
            // 一次跨 n 个兄弟（n 的符号就是方向），与可写边代理同义
            n_branches_tree_edge_const_proxy& go_to_next_brother(ptrdiff_t n)noexcept
            {
                ptr += n;
                return *this;
            }
            n_branches_tree_edge_const_proxy& go_to_prev_brother(ptrdiff_t n)noexcept
            {
                ptr -= n;
                return *this;
            }
            const T& operator*()const noexcept
            {
                return static_cast<n_branches_tree_node<N, T, Alloc>&>(**ptr).data.data;
            }
            const T* operator->()const noexcept
            {
                return std::addressof(static_cast<n_branches_tree_node<N, T, Alloc>&>(**ptr).data.data);
            }
            n_branches_tree_edge_const_proxy get_child(size_t i)const noexcept
            {
                check_index(i);
                return make(std::pointer_traits<pointer_to_pointer>::pointer_to
                (static_cast<node_base&>(**ptr).child[i]));
            }
            n_branches_tree_edge_const_proxy get_next_brother()const noexcept
            {
                n_branches_tree_edge_const_proxy ret{ *this };
                ++ret.ptr;
                return ret;
            }
            n_branches_tree_edge_const_proxy get_prev_brother()const noexcept
            {
                n_branches_tree_edge_const_proxy ret{ *this };
                --ret.ptr;
                return ret;
            }
            n_branches_tree_edge_const_proxy get_next_brother(ptrdiff_t n)const noexcept
            {
                n_branches_tree_edge_const_proxy ret{ *this };
                ret.ptr += n;
                return ret;
            }
            n_branches_tree_edge_const_proxy get_prev_brother(ptrdiff_t n)const noexcept
            {
                n_branches_tree_edge_const_proxy ret{ *this };
                ret.ptr -= n;
                return ret;
            }
                        std::array<n_branches_tree_edge_const_proxy, N> get_children()const noexcept;
            // 尾后边（见非 const 版本说明）
            n_branches_tree_edge_const_proxy get_child_end()const noexcept
            {
                n_branches_tree_edge_const_proxy ret = get_child(0);
                ret.ptr += static_cast<ptrdiff_t>(N);
                return ret;
            }
            friend bool operator==(n_branches_tree_edge_const_proxy l, n_branches_tree_edge_const_proxy r)noexcept
            {
                return l.ptr == r.ptr;
            }
            friend struct std::hash<n_branches_tree_edge_const_proxy>;
            template<size_t N1, class T1, class Alloc1>
            friend class Yc::n_branches_tree;
            friend class n_branches_tree_node_const_proxy<N, T, Alloc>;
            using value_type = T;
            using reference = const T&;
        };
    }
}
// std::hash 特化
template<size_t N, class T, class Alloc>
struct std::hash<Yc::details::n_branches_tree_edge_const_proxy<N, T, Alloc>>
{
    size_t operator()(const Yc::details::n_branches_tree_edge_const_proxy<N, T, Alloc>& para)const noexcept
    {
        if (!para.valid())
            return 0;
        return std::hash<decltype(para.ptr)>{}(para.ptr);
    }
};

namespace Yc
{
    namespace details
    {
        // ===================================================================
        // node_proxy：直接代表树节点
        // 与 edge_proxy 不同，这里持的是节点指针本身，支持 go_to_child(i) 导航与取值；
        // 无父节点信息，所以没有 go_up。
        // ===================================================================
        template<size_t N, class T, class Alloc>
        class n_branches_tree_node_proxy
        {
            using pointer = std::allocator_traits<Alloc>::pointer;
            using pointer_to_pointer = std::allocator_traits<Alloc>::template rebind_traits<pointer>::pointer;
            using node_base = n_branches_tree_node_base<N, Alloc>;
            // ptr：指向节点对象（*ptr 得到 n_branches_tree_empty_placeholder&）
            pointer ptr{};
            static void check_index(size_t i)noexcept
            {
                if (i >= N)
                {
                    std::unreachable();
                }
            }
        public:
            n_branches_tree_node_proxy() = default;
            n_branches_tree_node_proxy(pointer ptr)noexcept :ptr{ ptr }
            {
            }
            n_branches_tree_node_proxy(n_branches_tree_edge_proxy<N, T, Alloc> p)noexcept :ptr{ *p.ptr }
            {
            }
            n_branches_tree_node_proxy(const n_branches_tree_node_proxy&) = default;
            n_branches_tree_node_proxy& operator=(const n_branches_tree_node_proxy&) = default;
            bool null()const noexcept
            {
                return ptr == nullptr;
            }
            explicit operator bool()const noexcept
            {
                return !null();
            }
            T& operator*()const noexcept
            {
                return static_cast<n_branches_tree_node<N, T, Alloc>&>(*ptr).data.data;
            }
            T* operator->()const noexcept
            {
                return std::addressof(static_cast<n_branches_tree_node<N, T, Alloc>&>(*ptr).data.data);
            }
            n_branches_tree_node_proxy& go_to_child(size_t i)noexcept
            {
                check_index(i);
                ptr = static_cast<node_base&>(*ptr).child[i];
                return *this;
            }
            n_branches_tree_node_proxy get_child(size_t i)const noexcept
            {
                check_index(i);
                return { static_cast<node_base&>(*ptr).child[i] };
            }
            std::array<n_branches_tree_edge_proxy<N, T, Alloc>, N> get_children()const noexcept;
            // 尾后边：本节点第 N 个分支位置（不可解引用，只能判等/哈希/当哨位）
            n_branches_tree_edge_proxy<N, T, Alloc> get_child_end()const noexcept
            {
                n_branches_tree_edge_proxy<N, T, Alloc> ret = get_children()[0];
                ret.ptr += static_cast<ptrdiff_t>(N);
                return ret;
            }
            friend bool operator==(n_branches_tree_node_proxy l, n_branches_tree_node_proxy r)noexcept
            {
                return l.ptr == r.ptr;
            }
            friend struct std::hash<n_branches_tree_node_proxy>;
            friend class n_branches_tree_node_const_proxy<N, T, Alloc>;
            using value_type = T;
            using reference = T&;
        };
    }
}
// std::hash 特化：基于节点地址
template<size_t N, class T, class Alloc>
struct std::hash<Yc::details::n_branches_tree_node_proxy<N, T, Alloc>>
{
    size_t operator()(const Yc::details::n_branches_tree_node_proxy<N, T, Alloc>& p)const noexcept
    {
        return std::hash<decltype(p.ptr)>{}(p.ptr);
    }
};

namespace Yc
{
    namespace details
    {
        // node_const_proxy：node_proxy 的常量版本
        template<size_t N, class T, class Alloc>
        class n_branches_tree_node_const_proxy
        {
            using pointer = std::allocator_traits<Alloc>::pointer;
            using pointer_to_pointer = std::allocator_traits<Alloc>::template rebind_traits<pointer>::pointer;
            using node_base = n_branches_tree_node_base<N, Alloc>;
            pointer ptr{};
            static void check_index(size_t i)noexcept
            {
                if (i >= N)
                {
                    std::unreachable();
                }
            }
        public:
            n_branches_tree_node_const_proxy() = default;
            n_branches_tree_node_const_proxy(pointer ptr)noexcept :ptr{ ptr }
            {
            }
            n_branches_tree_node_const_proxy(n_branches_tree_node_proxy<N, T, Alloc> p)noexcept :ptr{ p.ptr }
            {
            }
            n_branches_tree_node_const_proxy(n_branches_tree_edge_const_proxy<N, T, Alloc> p)noexcept :ptr{ *p.ptr }
            {
            }
            n_branches_tree_node_const_proxy(const n_branches_tree_node_const_proxy&) = default;
            n_branches_tree_node_const_proxy& operator=(const n_branches_tree_node_const_proxy&) = default;
            bool null()const noexcept
            {
                return ptr == nullptr;
            }
            explicit operator bool()const noexcept
            {
                return !null();
            }
            const T& operator*()const noexcept
            {
                return static_cast<n_branches_tree_node<N, T, Alloc>&>(*ptr).data.data;
            }
            const T* operator->()const noexcept
            {
                return std::addressof(static_cast<n_branches_tree_node<N, T, Alloc>&>(*ptr).data.data);
            }
            n_branches_tree_node_const_proxy& go_to_child(size_t i)noexcept
            {
                check_index(i);
                ptr = static_cast<node_base&>(*ptr).child[i];
                return *this;
            }
            n_branches_tree_node_const_proxy get_child(size_t i)const noexcept
            {
                check_index(i);
                return { static_cast<node_base&>(*ptr).child[i] };
            }
            std::array<n_branches_tree_edge_const_proxy<N, T, Alloc>, N> get_children()const noexcept;
            // 尾后边（见非 const 版本说明）
            n_branches_tree_edge_const_proxy<N, T, Alloc> get_child_end()const noexcept
            {
                n_branches_tree_edge_const_proxy<N, T, Alloc> ret = get_children()[0];
                ret.ptr += static_cast<ptrdiff_t>(N);
                return ret;
            }
            friend bool operator==(n_branches_tree_node_const_proxy l, n_branches_tree_node_const_proxy r)noexcept
            {
                return l.ptr == r.ptr;
            }
            friend struct std::hash<n_branches_tree_node_const_proxy>;
            using value_type = T;
            using reference = const T&;
        };
    }
}
// std::hash 特化：基于节点地址
template<size_t N, class T, class Alloc>
struct std::hash<Yc::details::n_branches_tree_node_const_proxy<N, T, Alloc>>
{
    size_t operator()(const Yc::details::n_branches_tree_node_const_proxy<N, T, Alloc>& p)const noexcept
    {
        return std::hash<decltype(p.ptr)>{}(p.ptr);
    }
};

namespace Yc
{
    namespace details
    {
        // get_children 的类外定义：返回类型里用到代理自身，类内定义时类型还不完整。
        // 填表也走指针偏移：取一次第 0 个孩子的边，之后每次 ++ptr 就是下一个槽位
        // （不重复做 pointer_to；也避免对最后一个元素再加一次而越界）。
        template<size_t N, class T, class Alloc>
        inline std::array<n_branches_tree_edge_proxy<N, T, Alloc>, N>
            n_branches_tree_edge_proxy<N, T, Alloc>::get_children()const noexcept
        {
            std::array<n_branches_tree_edge_proxy, N> ret{};
            n_branches_tree_edge_proxy it = get_child(0);
            n_branches_tree_edge_proxy* out = ret.data();
            n_branches_tree_edge_proxy* const out_end = out + (N - 1);
            for (; out != out_end; ++out)
            {
                *out = it;
                it.go_to_next_brother();
            }
            *out = it;
            return ret;
        }
        template<size_t N, class T, class Alloc>
        inline std::array<n_branches_tree_edge_const_proxy<N, T, Alloc>, N>
            n_branches_tree_edge_const_proxy<N, T, Alloc>::get_children()const noexcept
        {
            std::array<n_branches_tree_edge_const_proxy, N> ret{};
            n_branches_tree_edge_const_proxy it = get_child(0);
            n_branches_tree_edge_const_proxy* out = ret.data();
            n_branches_tree_edge_const_proxy* const out_end = out + (N - 1);
            for (; out != out_end; ++out)
            {
                *out = it;
                it.go_to_next_brother();
            }
            *out = it;
            return ret;
        }
        // 节点代理也按槽位指针偏移填表（slot 一次取到数组首元素，之后 ++slot）
        template<size_t N, class T, class Alloc>
        inline std::array<n_branches_tree_edge_proxy<N, T, Alloc>, N>
            n_branches_tree_node_proxy<N, T, Alloc>::get_children()const noexcept
        {
            using edge = n_branches_tree_edge_proxy<N, T, Alloc>;
            node_base& n = static_cast<node_base&>(*ptr);
            std::array<edge, N> ret{};
            pointer* slot = n.child;
            edge* out = ret.data();
            edge* const out_end = out + (N - 1);
            for (; out != out_end; ++out, ++slot)
            {
                *out = { std::pointer_traits<pointer_to_pointer>::pointer_to(*slot) };
            }
            *out = { std::pointer_traits<pointer_to_pointer>::pointer_to(*slot) };
            return ret;
        }
        template<size_t N, class T, class Alloc>
        inline std::array<n_branches_tree_edge_const_proxy<N, T, Alloc>, N>
            n_branches_tree_node_const_proxy<N, T, Alloc>::get_children()const noexcept
        {
            using edge = n_branches_tree_edge_const_proxy<N, T, Alloc>;
            node_base& n = static_cast<node_base&>(*ptr);
            std::array<edge, N> ret{};
            pointer* slot = n.child;
            edge* out = ret.data();
            edge* const out_end = out + (N - 1);
            for (; out != out_end; ++out, ++slot)
            {
                *out = n_branches_tree_edge_const_proxy<N, T, Alloc>::make
                (std::pointer_traits<pointer_to_pointer>::pointer_to(*slot));
            }
            *out = n_branches_tree_edge_const_proxy<N, T, Alloc>::make
            (std::pointer_traits<pointer_to_pointer>::pointer_to(*slot));
            return ret;
        }

        // 多态 get_children 函数对象
        struct n_branches_tree_get_children_t
        {
            template<size_t N, class T, class Alloc>
            auto operator()(n_branches_tree_edge_proxy<N, T, Alloc> p)const noexcept
            {
                return p.get_children();
            }
            template<size_t N, class T, class Alloc>
            auto operator()(n_branches_tree_edge_const_proxy<N, T, Alloc> p)const noexcept
            {
                return p.get_children();
            }
            template<size_t N, class T, class Alloc>
            auto operator()(n_branches_tree_node_proxy<N, T, Alloc> p)const noexcept
            {
                return p.get_children();
            }
            template<size_t N, class T, class Alloc>
            auto operator()(n_branches_tree_node_const_proxy<N, T, Alloc> p)const noexcept
            {
                return p.get_children();
            }
        };
    }
    namespace n_branches_tree_functional
    {
        constexpr inline Yc::details::n_branches_tree_get_children_t get_children{};
    }
    // ===================================================================
    // n_branches_tree：N 叉树容器（无父节点指针、无哨兵）
    //
    // 与 binary_tree 一一对应，只是把 left/right 换成了 child[N]：
    //   - 导航：root / croot / nroot / cnroot
    //   - 修改：emplace / insert / cut / splice / erase / clear
    //   - 交换：swap / swap_sub_tree / swap_node
    //   - 遍历：for_each（前序；中序对 N 叉树没有定义）
    //   - 递归构造：recur_and_write
    // ===================================================================
    template<size_t N, class T, class Alloc>
    class n_branches_tree
    {
        using node_pointer = std::allocator_traits<Alloc>::template rebind_traits<details::n_branches_tree_empty_placeholder>::pointer;
        using pointer_to_node_pointer = std::allocator_traits<Alloc>::template rebind_traits<node_pointer>::pointer;
        using placeholder_alloc = std::allocator_traits<Alloc>::template rebind_alloc<details::n_branches_tree_empty_placeholder>;
        using node_type = details::n_branches_tree_node<N, T, placeholder_alloc>;
        using node_base_type = details::n_branches_tree_node_base<N, placeholder_alloc>;
        using node_allocator = std::allocator_traits<Alloc>::template rebind_alloc<node_type>;
        // 分配器（YC_NO_UNIQUE_ADDRESS 零开销存储无状态分配器）
        YC_NO_UNIQUE_ADDRESS Alloc alloc{};
        // 根节点指针（无哨兵节点，空树 root_ptr 为 null）
        node_pointer root_ptr{};
        // 获取占位符对象的 fancy pointer
        node_pointer pointer_to(const details::n_branches_tree_empty_placeholder& r)const noexcept
        {
            return std::pointer_traits<node_pointer>::pointer_to((details::n_branches_tree_empty_placeholder&)r);
        }
        // 获取节点指针的指针（即 pointer_to_pointer）
        pointer_to_node_pointer pointer_to(node_pointer& r)const noexcept
        {
            return std::pointer_traits<pointer_to_node_pointer>::pointer_to(r);
        }
        // 分配并构造一个节点（child 数组由 NSDMI 零初始化）
        node_pointer allocate_node()
        {
            node_allocator a{ alloc };
            node_pointer p = std::pointer_traits<node_pointer>::pointer_to
                (static_cast<details::n_branches_tree_empty_placeholder&>(*std::allocator_traits<node_allocator>::allocate(a, 1)));
            std::allocator_traits<node_allocator>::construct(a, (node_type*)std::to_address(p));
            return p;
        }
        // 析构并释放节点
        void deallocate_node(node_pointer p)noexcept
        {
            node_allocator a{ alloc };
            std::allocator_traits<node_allocator>::destroy(a, (node_type*)std::to_address(p));
            return std::allocator_traits<node_allocator>::deallocate(a,
                std::pointer_traits<typename std::allocator_traits<node_allocator>::pointer>::pointer_to(*(node_type*)std::to_address(p)), 1);
        }
    public:
        using allocator_type = Alloc;
        using value_type = T;
        using reference = T&;
        using pointer = std::allocator_traits<Alloc>::pointer;
        using edge_proxy = details::n_branches_tree_edge_proxy<N, T, placeholder_alloc>;
        using edge_const_proxy = details::n_branches_tree_edge_const_proxy<N, T, placeholder_alloc>;
        using node_proxy = details::n_branches_tree_node_proxy<N, T, placeholder_alloc>;
        using node_const_proxy = details::n_branches_tree_node_const_proxy<N, T, placeholder_alloc>;
    private:
        // 递归写入实现：
        //   先反向扫描源句柄（指针往数组头挪），找到最后一个非空孩子；对排在它前面的
        //   孩子递归，对那个孩子迭代（goto 尾递归）。源句柄和目标边都按指针偏移同步前进，
        //   全程不做下标运算。
        template<class ValueGetter, class ChildrenGetter, class InitializeHandle>
        void recur_and_write_impl(edge_const_proxy p, ValueGetter& vg, ChildrenGetter& cg, InitializeHandle h)
        {
        derecur:
            if (!p.null())
            {
                std::unreachable();
            }
            emplace(p, std::invoke(vg, h));
            // auto&&：句柄容器按值或按引用返回都行（按值返回时临时量被延长到本帧结束）
            auto&& hs = std::invoke(cg, h);
            // 反向扫描：hlast 从范围尾部往回挪，直到指向最后一个非空句柄。
            // 这里对 hs 只要求"随机访问范围"（std::array / std::vector / std::deque / 其它
            // 有 begin/end 的容器都行），不做连续性假设，所以用迭代器而不是 &hs[0]。
            auto const hbegin = std::begin(hs);
            auto hlast = std::end(hs);
            bool any = false;
            while (hlast != hbegin)
            {
                --hlast;
                if (*hlast)
                {
                    any = true;
                    break;
                }
            }
            if (!any)
            {
                return;
            }
            // 目标边 it 与源句柄 hp 同步前进，走到 hlast 为止（它自己留给迭代）
            edge_const_proxy it = p.get_child(0);
            for (auto hp = hbegin; hp != hlast; ++hp, it.go_to_next_brother())
            {
                if (*hp)
                {
                    recur_and_write_impl(it, vg, cg, *hp);
                }
            }
            // 最后一个非空孩子迭代（尾递归），这里已经有非空的保证
            p = it;
            h = *hlast;
            goto derecur;
        }
    public:
        // 递归写入：从外部树形结构写入 p，返回 p 处原有的子树
        // vg：值获取器；cg：孩子句柄获取器（返回 N 个句柄的随机访问范围，如 std::array/vector/deque）；h：当前句柄
        template<class ValueGetter, class ChildrenGetter, class InitializeHandle>
        n_branches_tree recur_and_write(edge_const_proxy p, ValueGetter vg, ChildrenGetter cg, InitializeHandle h)
        {
            n_branches_tree ret{ cut(p) };
            // guard：构造过程异常时自动把已构造部分拼回原树
            struct _guard
            {
                n_branches_tree* t;
                edge_const_proxy p;
                n_branches_tree* r;
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
        n_branches_tree() = default;
        explicit n_branches_tree(const Alloc& a)noexcept :alloc{ a }
        {
        }
        // 从树形结构构造整棵树
        template<class ValueGetter, class ChildrenGetter, class InitializeHandle>
        n_branches_tree(ValueGetter vg, ChildrenGetter cg, InitializeHandle h, const Alloc& a = {}) :alloc{ a }
        {
            if (h)
            {
                recur_and_write_impl(root(), vg, cg, h);
            }
        }
        // 拷贝构造：递归复制整棵树的结构和值
        n_branches_tree(const n_branches_tree& b) :n_branches_tree{ std::allocator_traits<Alloc>::select_on_container_copy_construction(b.alloc) }
        {
            auto copier = [](edge_const_proxy p) -> const T&
                {
                    return *p;
                };
            recur_and_write(root(), copier, n_branches_tree_functional::get_children, b.croot());
        }
        // 拷贝赋值
        n_branches_tree& operator=(const n_branches_tree& b)
        {
            if (this == &b)
            {
                return *this;
            }
            Alloc tmp{ alloc };
            n_branches_tree t1{ cut(root()) };
            // guard：复制过程异常时恢复原分配器和旧树
            struct _guard
            {
                n_branches_tree* t{};
                Alloc* tmp{};
                n_branches_tree* other{};
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
            auto copier = [](edge_const_proxy p)->const T&
                {
                    return *p;
                };
            recur_and_write(root(), copier, n_branches_tree_functional::get_children, b.root());
            guard.t = nullptr;
            return *this;
        }
        // 移动构造：移动分配器 + 交换根指针
        // 注意初始化顺序与成员声明顺序一致（alloc 在 root_ptr 前面），否则 -Wreorder 会报
        n_branches_tree(n_branches_tree&& b)noexcept :
            alloc{ std::move(b.alloc) }, root_ptr{ std::exchange(b.root_ptr, {}) }
        {
        }
        // 移动赋值
        n_branches_tree& operator=(n_branches_tree&& b)noexcept(
            std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value ||
            std::allocator_traits<Alloc>::is_always_equal::value)
        {
            if constexpr (std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value)
            {
                std::swap(alloc, b.alloc);
                std::swap(root_ptr, b.root_ptr);
                return *this;
            }
            else
            {
                if (alloc == b.alloc)
                {
                    std::swap(root_ptr, b.root_ptr);
                    return *this;
                }
                auto vg = [](edge_proxy p) -> T&&
                    {
                        return (T&&)*p;
                    };
                n_branches_tree tmp{ vg, n_branches_tree_functional::get_children, b.root(), alloc };
                swap(tmp);
                return *this;
            }
        }
        bool empty()const noexcept
        {
            return root_ptr == nullptr;
        }
        Alloc get_allocator()const noexcept
        {
            return alloc;
        }
        // 获取树根的 edge_proxy（无哨兵，空树 root_ptr 为 null）
        edge_proxy root()noexcept
        {
            return { pointer_to(root_ptr) };
        }
        edge_const_proxy root()const noexcept
        {
            return { pointer_to((node_pointer&)root_ptr) };
        }
        edge_const_proxy croot()const noexcept
        {
            return { pointer_to((node_pointer&)root_ptr) };
        }
        // 获取树根对应的 node_proxy
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
        // 切割：把 p 处子树摘下来，返回持有它的新树；原槽位置空
        n_branches_tree cut(edge_const_proxy p)noexcept
        {
            pointer_to_node_pointer ptr = p.ptr;
            n_branches_tree ret{ alloc };
            ret.root_ptr = std::exchange(*ptr, {});
            return ret;
        }
        // 删除 p 处节点及其整棵子树：
        //   反向扫描（指针从槽位数组尾部往回挪）找到最后一个非空孩子；没有孩子 -> 直接销毁；
        //   有孩子 -> 先递归删掉排在它前面的孩子，把它提升到当前位置，再循环删它
        //   （这样深树不会被递归压爆）。
        void erase(edge_const_proxy p)noexcept
        {
            if (!p.null())
            {
                node_pointer& place = *p.ptr;
                while (true)
                {
                    node_base_type& n = static_cast<node_base_type&>(**p.ptr);
                    // 反向扫描：last 指向"最后一个非空槽位的下一个位置"（全空则回到首元素）
                    node_pointer* const first = n.child;
                    node_pointer* last = n.child + N;
                    while (last != first && !(bool)*(last - 1))
                    {
                        --last;
                    }
                    if (last == first)
                    {
                        // 没有孩子：销毁本节点，槽位置空，结束
                        std::allocator_traits<Alloc>::destroy(alloc, (T*)p.operator->());
                        deallocate_node(place);
                        place = nullptr;
                        return;
                    }
                    // 排在最后一个之前的槽位先递归删掉（空槽位的 erase 会立刻返回）
                    for (node_pointer* it = first; it + 1 != last; ++it)
                    {
                        erase(edge_const_proxy::make
                        (std::pointer_traits<pointer_to_node_pointer>::pointer_to(*it)));
                    }
                    node_pointer promoted = *(last - 1);
                    std::allocator_traits<Alloc>::destroy(alloc, (T*)p.operator->());
                    deallocate_node(place);
                    place = promoted;
                }
            }
        }
        // 清空整棵树
        void clear()noexcept
        {
            erase(root());
        }
        // 前序遍历，对每个节点的值调用 f（N 叉树没有中序）
        // 与 std::for_each 一致：函数对象按值传入、按值返回，所以有状态函数对象
        // （累加器、计数器之类）遍历完能带着状态出来：
        //     auto acc = t.for_each(accumulator{});
        // 约定同 binary_tree：f 不得改变树结构；noexcept 取决于
        // "f 能无异常接受 T&" 且 "f 能无异常地移动构造"。
        template<class F>
        F for_each(F f)noexcept(std::is_nothrow_invocable_v<F&, T&>&& std::is_nothrow_move_constructible_v<F>)
        {
            for_each_impl<edge_proxy>(root(), f);
            return f;
        }
        template<class F>
        F for_each(F f)const noexcept(std::is_nothrow_invocable_v<F&, const T&>&& std::is_nothrow_move_constructible_v<F>)
        {
            for_each_impl<edge_const_proxy>(croot(), f);
            return f;
        }
    private:
        // 前序：先访问本节点；反向扫描（指针往数组头挪）找到最后一个非空孩子，
        // 排在它前面的孩子递归，它自己走 goto 尾递归。子节点一律用边代理的指针偏移
        // 前进/后退，不做下标运算；递归深度 ≤ 树高（纯"最后一个孩子"的单链完全不递归），
        // 尾部整段空槽位也不用逐个判空。
        template<class Proxy, class F>
        static void for_each_impl(Proxy p, F&& f)
            noexcept(std::is_nothrow_invocable_v<F&, decltype(*p)>)
        {
        derecur:
            if (p.null())
            {
                return;
            }
            std::invoke(f, *p);
            // 反向扫描：tail 从最后一个槽位往回挪，直到指向最后一个非空孩子（last 是剩余格数）
            Proxy tail = p.get_child(N - 1);
            size_t last = N;
            while (last != 0 && tail.null())
            {
                tail.go_to_prev_brother();
                --last;
            }
            if (last == 0)
            {
                return;
            }
            // 从第 0 个槽位按指针偏移往前走，走到 tail 为止（tail 自己留给迭代）
            for (Proxy it = p.get_child(0); it != tail; it.go_to_next_brother())
            {
                if (!it.null())
                {
                    for_each_impl<Proxy>(it, f);
                }
            }
            p = tail;
            goto derecur;
        }
    public:
        // 在指定边位置原地构造新节点（已有子树作为切割结果返回）
        template<class... Args>
        n_branches_tree emplace(edge_const_proxy p, Args&&... args)
        {
            node_pointer new_node = allocate_node();
            // 若 T 的构造不抛异常，直接构造；否则用 RAII guard 确保异常安全
            if constexpr (std::is_nothrow_constructible_v<T, Args...>)
            {
                std::allocator_traits<Alloc>::
                    construct(alloc, std::addressof(((node_type&)*new_node).data.data), std::forward<Args>(args)...);
            }
            else
            {
                struct guard_t
                {
                    n_branches_tree* t;
                    node_pointer ptr;
                    ~guard_t()
                    {
                        if (ptr != nullptr)
                        {
                            t->deallocate_node(ptr);
                        }
                    }
                }guard{ this,new_node };
                std::allocator_traits<Alloc>::
                    construct(alloc, std::addressof(((node_type&)*new_node).data.data), std::forward<Args>(args)...);
                guard.ptr = nullptr;
            }
            n_branches_tree ret{ cut(p) };
            *p.ptr = new_node;
            return ret;
        }
        n_branches_tree insert(edge_const_proxy p, const T& v)
        {
            return emplace(p, v);
        }
        n_branches_tree insert(edge_const_proxy p, T&& v)
        {
            return emplace(p, std::move(v));
        }
        // 拼接：把 from 子树移动到 to 位置，原 to 子树作为结果返回
        n_branches_tree splice(edge_const_proxy to, edge_const_proxy from)noexcept
        {
            n_branches_tree ret{ cut(to) };
            *(to.ptr) = std::exchange(*(from.ptr), {});
            return ret;
        }
        // 拼接整棵树的根
        n_branches_tree splice(edge_const_proxy p, n_branches_tree& tree)noexcept
        {
            return splice(p, tree.root());
        }
        n_branches_tree splice(edge_const_proxy p, n_branches_tree&& tree)noexcept
        {
            return splice(p, tree.root());
        }
        // 交换两个子树（只交换槽位里的指针）
        static void swap_sub_tree(edge_const_proxy l, edge_const_proxy r)noexcept
        {
            std::swap(*(l.ptr), *(r.ptr));
        }
        // 交换两棵树
        void swap(n_branches_tree& other)noexcept
        {
            if constexpr (std::allocator_traits<Alloc>::propagate_on_container_swap::value)
            {
                std::swap(alloc, other.alloc);
            }
            swap_sub_tree(root(), other.root());
        }
        // 交换两个节点：两个槽位互换，再逐个互换它们的 child 数组。
        //   净效果是"两个位置上的值互换、子树留在原位"——与 binary_tree::swap_node 的实际
        //   语义一致（那边的实现也是先换槽位、再换各自子槽位）。
        //   父子相邻的情形自动成立：交换槽位时，父节点里指向孩子的那个槽位已经变成它自己，
        //   随后交换数组就把那一格换成了"挪到孩子位置的那个节点"，因此不需要
        //   binary_tree 那套相邻形态专用分支（那边现在也不需要了）。
        //   前置条件：两个边都指代真实节点。
        static void swap_node(edge_proxy l, edge_proxy r)noexcept
        {
            node_type& a = static_cast<node_type&>(**l.ptr);
            node_type& b = static_cast<node_type&>(**r.ptr);
            swap_sub_tree(l, r);
            // 整个槽位数组一起换（std::swap 对裸数组是逐元素 swap，不用下标循环）
            std::swap(a.child, b.child);
        }
        // 析构：清空所有节点
        ~n_branches_tree()
        {
            clear();
        }
    };
    // 特化 swap 自由函数
    template<size_t N, class T, class Alloc>
    void swap(n_branches_tree<N, T, Alloc>& l, n_branches_tree<N, T, Alloc>& r)noexcept
    {
        l.swap(r);
    }
}
