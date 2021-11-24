//
// Created by netcan on 2021/11/24.
//

#ifndef ASYNCIO_COUNTED_H
#define ASYNCIO_COUNTED_H
struct CountedPolicy {
    bool move_constructable = true;
    bool copy_constructable = true;
    bool move_assignable = true;
    bool copy_assignable = true;
};

inline constexpr CountedPolicy default_count_policy;

template<CountedPolicy policy = default_count_policy>
struct Counted {
    static void reset_count() {
        move_construct_counts = 0;
        copy_construct_counts = 0;
        default_construct_counts = 0;
        copy_assign_counts = 0;
        move_assign_counts = 0;
        destruction_counts = 0;
    }
    Counted() { id_ = default_construct_counts++; }
    ~Counted() { ++destruction_counts; }
    Counted(const Counted&) requires (policy.copy_constructable)
    { ++copy_construct_counts; }

    Counted(Counted&& other) requires (policy.move_constructable)
    { ++move_construct_counts; other.id_ = -1; }

    Counted& operator=(const Counted&) requires(policy.copy_assignable)
    {
        ++copy_assign_counts;
        return *this;
    }

    Counted& operator=(Counted&& other) requires(policy.move_assignable)
    {
        ++move_assign_counts;
        other.id_ = -1;
        return *this;
    }

    static int construct_counts() {
        return move_construct_counts
               + copy_construct_counts
               + default_construct_counts;
    }
    static int alive_counts() {
        return construct_counts() - destruction_counts;
    }

    int id_;
    static int move_construct_counts;
    static int copy_construct_counts;
    static int copy_assign_counts;
    static int move_assign_counts;
    static int default_construct_counts;
    static int destruction_counts;
};

template<CountedPolicy policy>
inline int Counted<policy>::move_construct_counts = 0;
template<CountedPolicy policy>
inline int Counted<policy>::copy_construct_counts = 0;
template<CountedPolicy policy>
inline int Counted<policy>::move_assign_counts = 0;
template<CountedPolicy policy>
inline int Counted<policy>::copy_assign_counts = 0;
template<CountedPolicy policy>
inline int Counted<policy>::default_construct_counts = 0;
template<CountedPolicy policy>
inline int Counted<policy>::destruction_counts = 0;

#endif // ASYNCIO_COUNTED_H
