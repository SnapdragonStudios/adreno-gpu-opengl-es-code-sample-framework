// Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef QUEUE_CIRCULAR_H
#define QUEUE_CIRCULAR_H

#include "FrmArraySafe.h"

#define QUEUE_CIRCULAR_ARRAY_SIZE (kSize+1)///<this implementation wastes one slot; give user as many slots as she asked for
#define QUEUE_CIRCULAR_MAX_ITEMS (kSize)
template<class T, size_t kSize>
class QueueCircular
{
public:
    QueueCircular();
    bool Full() const;
    size_t Size() const;
    void PushOverwritingOldestIfFull(const T& item);
    inline const T& operator[](const size_t i) const
    {
        ADRENO_ASSERT(i < Size(), __FILE__, __LINE__);
        return m_array[(m_head + i) % QUEUE_CIRCULAR_ARRAY_SIZE];
    }
    inline T& operator[](const size_t i)
    {
        return const_cast<T&>(static_cast<const QueueCircular<T, kSize>*>(this)->operator[](i));
    }

private:
    ArraySafe<T, QUEUE_CIRCULAR_ARRAY_SIZE> m_array;
    size_t m_head, m_tail;

    void PushInternal(const T& item);
};
template<class T, size_t kSize>
QueueCircular<T, kSize>::QueueCircular()
{
    m_head = m_tail = 0;
}
template<class T, size_t kSize>
bool QueueCircular<T, kSize>::Full() const
{
    return (m_head + 1) % QUEUE_CIRCULAR_ARRAY_SIZE == m_tail;
}
template<class T, size_t kSize>
void QueueCircular<T, kSize>::PushInternal(const T& item)
{
    ADRENO_ASSERT(!Full(), __FILE__, __LINE__);
    m_array[m_head++] = item;
    m_head %= QUEUE_CIRCULAR_ARRAY_SIZE;
}
template<class T, size_t kSize>
void QueueCircular<T, kSize>::PushOverwritingOldestIfFull(const T& item)
{
    if (Full())
    {
        //overwrite oldest item to make room for the new item
        ++m_tail;
        m_tail %= QUEUE_CIRCULAR_ARRAY_SIZE;
    }
    PushInternal(item);
}
template<class T, size_t kSize>
size_t QueueCircular<T, kSize>::Size() const
{
    return (m_head >= m_tail) ? (m_head - m_tail) : (QUEUE_CIRCULAR_ARRAY_SIZE - m_tail + m_head);
}

#endif//QUEUE_CIRCULAR_H