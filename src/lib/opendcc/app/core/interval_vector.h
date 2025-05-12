/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <vector>
#include <numeric>
#include <algorithm>

OPENDCC_NAMESPACE_OPEN

template <class T>
class IntervalVector
{
public:
    struct Interval
    {
        T start;
        T end;
        Interval() {}
        Interval(T val)
            : start(val)
            , end(val)
        {
        }
        Interval(T start, T end)
            : start(start)
            , end(end)
        {
        }

        T length() const { return end - start + 1; }
        bool is_adjacent_left(const Interval& other) const { return end + 1 == other.start; }
        bool is_adjacent_left(const T& other) const { return end + 1 == other; }
        bool is_adjacent_right(const Interval& other) const { return other.is_adjacent_left(*this); }
        bool is_adjacent_right(const T& other) const { return other + 1 == start; }
        bool is_adjacent(const Interval& other) const { return is_adjacent_left(other) || is_adjacent_right(other); }
        bool is_adjacent(T other) const { return is_adjacent_left(other) || is_adjacent_right(other); }

        void extend_left(T val = 1) { start -= val; }
        void shrink_left(T val = 1) { start += val; }
        void extend_right(T val = 1) { end += val; }
        void shrink_right(T val = 1) { end -= val; }
        void split_left(T val) { end = val - 1; }
        void split_right(T val) { start = val + 1; }
        bool operator<(const Interval& other) const { return start < other.start; }
        bool operator<(const T& other) const { return start < other; }
        bool operator==(const Interval& other) const { return start == other.start && end == other.end; }
        bool operator!=(const Interval& other) const { return !(*this == other); }
    };

    class RangeProxy
    {
    public:
        class iterator
        {
        public:
            typedef std::bidirectional_iterator_tag iterator_category;
            using difference_type = T;
            using value_type = T;
            using reference = T&;
            using pointer = T*;
            typedef typename std::vector<Interval>::size_type size_type;

            iterator(const std::vector<Interval>& intervals, size_type interval_id, size_type offset)
                : m_intervals(intervals)
                , m_interval_id(interval_id)
                , m_offset(offset)
            {
            }

            bool operator==(const iterator& other) const { return m_interval_id == other.m_interval_id && m_offset == other.m_offset; }
            bool operator!=(const iterator& other) const { return !(*this == other); }
            value_type operator*() const { return m_intervals[m_interval_id].start + m_offset; }
            iterator& operator++()
            {
                const auto& interval = m_intervals[m_interval_id];
                if (interval.end - interval.start == m_offset)
                {
                    ++m_interval_id;
                    m_offset = 0;
                }
                else
                {
                    ++m_offset;
                }
                return *this;
            }
            iterator& operator--()
            {
                if (m_offset != 0)
                {
                    --m_offset;
                }
                else
                {
                    --m_interval_id;
                    const auto& interval = m_intervals[m_interval_id];
                    m_offset = interval.end - interval.start;
                }
                return *this;
            }

        private:
            const std::vector<Interval>& m_intervals;
            size_type m_interval_id;
            size_type m_offset;
        };

        RangeProxy(const IntervalVector& interval_vector)
            : m_interval_vector(interval_vector)
        {
        }

        iterator begin() const { return iterator(m_interval_vector.m_intervals, 0, 0); }
        iterator end() const { return iterator(m_interval_vector.m_intervals, m_interval_vector.m_intervals.size(), 0); }
        size_t size() const { return m_interval_vector.size(); }
        bool empty() const { return size() == 0; }

    private:
        const IntervalVector& m_interval_vector;
    };
    using iterator = typename std::vector<Interval>::iterator;
    using const_iterator = typename std::vector<Interval>::const_iterator;

    IntervalVector()
        : m_size(0) {};
    IntervalVector(const IntervalVector&) = default;
    IntervalVector(IntervalVector&&) = default;
    IntervalVector& operator=(const IntervalVector&) = default;
    IntervalVector& operator=(IntervalVector&&) = default;

    template <class TCollection>
    static IntervalVector from_collection(TCollection values)
    {
        std::sort(values.begin(), values.end());
        return IntervalVector(values.begin(), values.end());
    }
    template <class TCollection>
    static IntervalVector from_sorted_collection(const TCollection& values)
    {
        return IntervalVector(values.begin(), values.end());
    }

    static IntervalVector from_intervals(const std::vector<Interval>& intervals)
    {
        IntervalVector result;
        result.m_intervals = intervals;
        std::sort(result.m_intervals.begin(), result.m_intervals.end());
        auto iter = result.m_intervals.begin();
        auto cur = iter;
        Interval tmp = *cur;
        while (++iter != result.m_intervals.end())
        {
            if (tmp.end + 1 < iter->start) // [(.tmp.),..., (.iter.)]
            {
                *cur = tmp;
                result.m_size += cur->length();
                tmp = *iter;
                ++cur;
            }
            else if (tmp.end < iter->end)
            {
                tmp.end = iter->end;
            }
        }
        if (cur == result.m_intervals.begin() || *(cur - 1) != tmp)
        {
            *cur = tmp;
            result.m_size += cur->length();
            ++cur;
        }
        result.m_intervals.erase(cur, result.m_intervals.end());
        return std::move(result);
    }

    const_iterator begin() const { return m_intervals.begin(); }

    const_iterator end() const { return m_intervals.end(); }

    size_t size() const { return m_size; }

    size_t interval_count() const { return m_intervals.size(); }

    bool empty() const { return m_size == 0; }

    void insert(const T& val)
    {
        const auto iter = std::upper_bound(m_intervals.begin(), m_intervals.end(), Interval(val));
        if (iter == m_intervals.end())
        {
            if (empty())
            {
                m_intervals.emplace_back(val); // [ V ]
                ++m_size;
                return;
            }
            auto last = std::prev(iter);
            if (val <= last->end)
                return; // [ (...), (.V.) ]
            if (last->is_adjacent_left(val))
            {
                last->extend_right(); // [ (...), (...)V ]
                ++m_size;
                return;
            }
            m_intervals.emplace_back(val); // [ (...), (...), V ]
            ++m_size;
            return;
        }

        if (iter == m_intervals.begin()) // [V(...), (...) ]
        {
            if (iter->is_adjacent_right(val)) // [ V(...), (...) ]
                iter->extend_left();
            else // [ V, (...), (...) ]
                m_intervals.emplace(iter, val);
            ++m_size;
            return;
        }

        const auto prev = std::prev(iter);
        if (iter->is_adjacent_right(val))
        {
            if (prev->is_adjacent_left(val)) // [ (...)V(...) ]
            {
                prev->end = iter->end;
                m_intervals.erase(iter);
            }
            else // [ (...), V(...) ]
            {
                iter->extend_left();
            }
            ++m_size;
        }
        else if (val <= prev->end) // [ (.V.), (...) ]
        {
            return;
        }
        else if (prev->is_adjacent_left(val)) // [ (...)V, (...) ]
        {
            prev->extend_right();
            ++m_size;
        }
        else // [ (...), V, (...) ]
        {
            m_intervals.emplace(iter, val);
            ++m_size;
        }
    }

    // accepts sorted collection
    template <class TIterator>
    void insert(const TIterator& begin, const TIterator& end)
    {
        if (begin == end)
            return;

        std::vector<Interval> updated;
        updated.reserve(std::distance(begin, end) + m_intervals.size());
        auto this_it = m_intervals.begin();
        auto other_it = begin;
        m_size = 0;

        if (m_intervals.empty())
        {
            updated.emplace_back(*other_it);
            ++other_it;
            m_size = 1;
        }
        else if (this_it->start <= *other_it)
        {
            updated.emplace_back(*this_it);
            m_size = this_it->length();
            ++this_it;
        }
        else
        {
            updated.emplace_back(*other_it);
            ++other_it;
            m_size = 1;
        }

        while (this_it != m_intervals.end() && other_it != end)
        {
            if (this_it->start <= *other_it)
            {
                if (updated.back().is_adjacent_left(*this_it))
                    updated.back().end = this_it->end;
                else
                    updated.emplace_back(*this_it);
                m_size += this_it->length();
                ++this_it;
                continue;
            }
            else if (*other_it > updated.back().end)
            {
                if (updated.back().is_adjacent_left(*other_it))
                    updated.back().extend_right();
                else
                    updated.emplace_back(*other_it);
                m_size++;
            }
            ++other_it;
        }

        if (this_it != m_intervals.end() && updated.back().is_adjacent_left(*this_it))
        {
            updated.back().end = this_it->end;
            m_size += this_it->length();
            ++this_it;
        }
        while (this_it != m_intervals.end())
        {
            updated.emplace_back(*this_it);
            m_size += this_it->length();
            ++this_it;
        }

        while (other_it != end)
        {
            if (*other_it > updated.back().end)
            {
                if (updated.back().is_adjacent_left(*other_it))
                    updated.back().extend_right();
                else
                    updated.emplace_back(*other_it);
                m_size++;
            }
            ++other_it;
        }

        m_intervals = std::move(updated);
    }

    void insert(const IntervalVector& other)
    {
        if (other.empty())
            return;
        if (m_intervals.empty())
        {
            m_intervals = other.m_intervals;
            m_size = other.m_size;
            return;
        }

        std::vector<Interval> updated;
        updated.reserve(m_intervals.size() + other.size());
        m_size = 0;

        struct CollectionIterators
        {
            typename std::vector<Interval>::const_iterator it;
            typename std::vector<Interval>::const_iterator end;
        } *left, *right;

        CollectionIterators this_col = { m_intervals.begin(), m_intervals.end() };
        CollectionIterators other_col = { other.m_intervals.begin(), other.m_intervals.end() };
        T start, end;

        if (this_col.it->start < other_col.it->start)
        {
            left = &this_col;
            right = &other_col;
        }
        else
        {
            left = &other_col;
            right = &this_col;
        }
        start = left->it->start;
        end = left->it->end;

        while (left->it != left->end && right->it != right->end)
        {
            if (right->it->start < end + 2)
            {
                if (right->it->end < end)
                {
                    ++right->it;
                }
                else
                {
                    end = right->it->end;
                    ++left->it;
                    std::swap(left, right);
                }
            }
            else
            {
                ++left->it;
                if (left->it != left->end)
                {
                    updated.emplace_back(start, end);
                    m_size += end - start + 1;
                    if (right->it->start < left->it->start)
                        std::swap(left, right);
                    start = left->it->start;
                    end = left->it->end;
                }
            }
        }
        updated.emplace_back(start, end);
        m_size += end - start + 1;
        if (right->it == right->end)
            ++left->it;

        while (this_col.it != this_col.end)
        {
            updated.emplace_back(*this_col.it);
            m_size += updated.back().length();
            this_col.it++;
        }
        while (other_col.it != other_col.end)
        {
            updated.emplace_back(*other_col.it);
            m_size += updated.back().length();
            other_col.it++;
        }
        m_intervals = std::move(updated);
    }

    void erase(const T& val)
    {
        const auto iter = std::upper_bound(m_intervals.begin(), m_intervals.end(), Interval(val));
        if (iter == m_intervals.begin())
            return;

        const auto prev = std::prev(iter);
        if (prev->end < val) // [ (...), V, (...) ]
            return;
        if (prev->end == val) // [ (...V, (...) ]
        {
            if (prev->start == prev->end)
                m_intervals.erase(prev);
            else
                prev->shrink_right();
        }
        else // [ (.V.), (...) ]
        {
            const auto start = prev->start;
            const auto end = prev->end;

            if (start != val)
            {
                prev->split_left(val);
                m_intervals.emplace(iter, val + 1, end);
            }
            else
            {
                prev->shrink_left();
            }
        }
        --m_size;
    }

    // accepts sorted collection
    template <class TIterator>
    void erase(const TIterator& begin, const TIterator& end)
    {
        if (m_intervals.empty())
            return;

        std::vector<Interval> updated;
        m_size = 0;
        updated.reserve(m_intervals.size());
        auto this_it = m_intervals.begin();
        auto other_it = begin;

        while (this_it != m_intervals.end() && other_it != end)
        {
            if (this_it->start == *other_it)
            {
                if (this_it->start == this_it->end)
                    ++this_it;
                else
                    this_it->shrink_left();
            }
            else if (this_it->start < *other_it)
            {
                // insert full interval
                if (this_it->end < *other_it)
                {
                    updated.emplace_back(*this_it);
                    m_size += updated.back().length();
                    ++this_it;
                    continue;
                }
                else
                {
                    // add interval (current->start, *other_it - 1)
                    updated.emplace_back(this_it->start, *other_it - 1);
                    m_size += updated.back().length();
                    if (*other_it != this_it->end)
                        this_it->start = *other_it + 1;
                    else
                        ++this_it;
                }
            }
            ++other_it;
        }

        while (this_it != m_intervals.end())
        {
            updated.emplace_back(*this_it);
            ++this_it;
            m_size += updated.back().length();
        }

        m_intervals = std::move(updated);
    }
    void erase(const IntervalVector& other)
    {
        if (m_intervals.empty())
            return;

        std::vector<Interval> updated;
        m_size = 0;
        updated.reserve(m_intervals.size());
        auto this_it = m_intervals.begin();
        auto other_it = other.m_intervals.begin();
        while (this_it != m_intervals.end() && other_it != other.m_intervals.end())
        {
            if (this_it->start < other_it->start)
            {
                if (this_it->end < other_it->start)
                {
                    updated.emplace_back(*this_it);
                    m_size += updated.back().length();
                    ++this_it;
                }
                else
                {
                    updated.emplace_back(this_it->start, other_it->start - 1);
                    m_size += updated.back().length();
                    if (this_it->end <= other_it->end)
                        ++this_it;
                    else
                        this_it->start = other_it->end + 1;
                }
            }
            else if (this_it->start > other_it->end)
            {
                ++other_it;
            }
            else if (this_it->end > other_it->end)
            {
                this_it->start = other_it->end + 1;
                ++other_it;
            }
            else
            {
                ++this_it;
            }
        }

        while (this_it != m_intervals.end())
        {
            updated.emplace_back(*this_it);
            m_size += updated.back().length();
            ++this_it;
        }

        m_intervals = std::move(updated);
    }

    void clear()
    {
        m_intervals.clear();
        m_size = 0;
    }

    bool contains(const T& val) const
    {
        const auto iter = std::upper_bound(m_intervals.begin(), m_intervals.end(), Interval(val));
        if (iter == m_intervals.begin())
            return false;
        return val <= std::prev(iter)->end;
    }

    template <class TCollection>
    TCollection flatten() const
    {
        TCollection result;
        result.resize(m_size);
        auto data = result.data();
        for (const auto& interval : m_intervals)
        {
            const auto range = interval.length();
            std::iota(data, data + range, interval.start);
            data += range;
        }
        return std::move(result);
    }

    bool operator==(const IntervalVector& other) const { return m_size == other.m_size && m_intervals == other.m_intervals; }
    bool operator!=(const IntervalVector& other) const { return !(*this == other); }

private:
    template <class TIterator>
    IntervalVector(const TIterator& begin, const TIterator& end)
    {
        insert(begin, end);
    }

    std::vector<Interval> m_intervals;
    std::size_t m_size = 0;
};

OPENDCC_NAMESPACE_CLOSE
