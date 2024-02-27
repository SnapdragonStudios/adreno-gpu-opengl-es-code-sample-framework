// Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ARRAY_SAFE_H
#define ARRAY_SAFE_H

#include"FrmStdLib.h"
#include<initializer_list>
#include<string.h>

template<class T, size_t kSize>
class ArraySafe
{
    ADRENO_STATIC_ASSERT(kSize > 0);

public:
    ArraySafe()
    {
    }
    ArraySafe(const std::initializer_list<T>& initializerList)
    {
        MemcpyFromStart(initializerList.begin(), initializerList.size() * sizeof(T));
    }
    ArraySafe(T* const initializerArray, const size_t size)
    {
        MemcpyFromStart(initializerArray, size * sizeof(T));
    }
    ~ArraySafe() = default;

    ArraySafe(const ArraySafe<T, kSize>& rhs)
    {
        Copy(rhs);
    }
    ArraySafe(ArraySafe<T, kSize>&& rhs)
    {
        Copy(rhs);
    }
    ArraySafe<T, kSize>& operator=(const ArraySafe<T, kSize>& rhs)
    {
        Copy(rhs);
        return *this;
    }
    ArraySafe<T, kSize>& operator=(ArraySafe<T, kSize>&& rhs)
    {
        Copy(rhs);
        return *this;
    }

    const T& operator[](const size_t index) const
    {
        ADRENO_ASSERT(index >= 0, __FILE__, __LINE__);
        ADRENO_ASSERT(index < kSize, __FILE__, __LINE__);

        return mArray[index];
    }
    T& operator[](const size_t index)
    {
        return const_cast<T&>(static_cast<const ArraySafe<T, kSize>&>(*this).operator[](index));
    }
    const T* data() const
    {
        return begin();
    }
    size_t Size() const
    {
        return kSize;
    }
    size_t LastValidIndex() const
    {
        return kSize - 1;
    }
    size_t SizeInBytes() const
    {
        return kSize * sizeof(T);
    }

    //this block supports C++ foreach loop
    typedef T* iterator;
    typedef const T* const_iterator;
    iterator begin() { return &mArray[0]; }
    const_iterator begin() const { return &mArray[0]; }
    iterator end() { return &mArray[kSize]; }
    const_iterator end() const { return &mArray[kSize]; }

    void MemcpyFromStart(const T* const input, const size_t bytesNum)
    {
        MemcpyFromIndex(0, input, bytesNum);
    }
    void MemcpyFromIndex(const size_t index, const T* const input, const size_t bytesNum)
    {
        ADRENO_ASSERT(bytesNum >= 0, __FILE__, __LINE__);
        ADRENO_ASSERT(bytesNum <= SizeInBytes(), __FILE__, __LINE__);
        ADRENO_ASSERT(input, __FILE__, __LINE__);

        memcpy(&mArray[index], input, bytesNum);
    }

    void Snprintf(const char* const formatString, ...)
    {
        ADRENO_STATIC_ASSERT(sizeof(T) == sizeof(char));//only use when *this holds ASCII characters -- @todo NTF: template specialization might be the C++-compiler-approved method for enforcing this
#if ADRENO_DEBUG
        ADRENO_ASSERT(formatString, __FILE__, __LINE__);

        const char bellAsciiKeyCode = 7;
        char* const lastElement = &mArray[LastValidIndex()];
        *lastElement = bellAsciiKeyCode;//waste one byte to ensure the buffer was large enough to hold the entire resultant string
#endif//#if ADRENO_DEBUG
        ADRENO_ASSERT(strnlen(formatString, Size()) > 0, __FILE__, __LINE__);

        va_list args;
        va_start(args, formatString);
        vsnprintf(&mArray[0], Size(), formatString, args);
        va_end(args);

        ADRENO_ASSERT(*lastElement == bellAsciiKeyCode, __FILE__, __LINE__);//vsnprintf may have truncated to stay within the buffer, which would mean it would have overwritten the sentinel and probably lost one or more characters (fitting the buffer perfectly is still considered an error, since there's no way of distinguishing it from the truncation case)
    }


private:
    void Copy(const ArraySafe<T, kSize>& rhs)
    {
        MemcpyFromStart(rhs.data(), rhs.SizeInBytes());
    }

    T mArray[kSize];
};

#endif//ARRAY_SAFE_H