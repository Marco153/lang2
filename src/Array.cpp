#pragma once
#include "assert.cpp"
#include "Array.h"

template <class T>
FUNC_STRUCT_EXPORT void LangArray<T>::Remove(T *t)
{
	int i = 0;
	auto ar = this;
	FOR((*ar))
	{
		if(it == t)
		{
			RemoveAt(i);
		}
		i++;
	}
}
template <class T>
FUNC_STRUCT_EXPORT void LangArray<T>::AddAt(unsigned int idx, T *v)
{
}
template <class T>
FUNC_STRUCT_EXPORT void LangArray<T>::RemoveAt(int a)
{
	ASSERT(a >= 0 && a < this->count)
	T *t = this->start + a;
	int diff = this->count - a - 1;
	memcpy(t, t + 1, sizeof(T) * diff);
	this->end--;
	this->count--;
}
template <class T>
int LangArray<T>::GetTotalBytes()
{
	return sizeof(T) * count;
}


template <class T>
FUNC_STRUCT_EXPORT T *LangArray<T>::Add(LangArray<T> *a)
{
	ASSERT((this->count + a->count) < this->length)


	T *ret = this->end;
	memcpy(this->end, a->start, a->count * sizeof(T));
	this->end   += a->count;
	this->count += a->count;
	return ret;
}
template <class T>
FUNC_STRUCT_EXPORT T *LangArray<T>::Add(T a)
{
	ASSERT(this->count < this->length && this->length > 0)


	T *ret = this->end;
	memcpy(this->end, &a, sizeof(T));
	this->end++;
	this->count++;
	return ret;
}
template <class T>
FUNC_STRUCT_EXPORT T *LangArray<T>::Add(T *a)
{
	ASSERT(this->count < this->length && this->length > 0)


	T *ret = this->start + this->count;
	memcpy(ret, a, sizeof(T));
	this->end++;
	this->count++;
	return ret;
}

template <class T>
T *LangArray<T>::Peek()
{
	T *ret = this->end - 1;
	return ret;
}
template <class T>
T *LangArray<T>::Pop()
{
	ASSERT(this->count > 0)
	if(count == 0)
	{
		return nullptr;
	}

	T *ret = --this->end;
	this->count--;
	return ret;
}
template <class T>
void LangArray<T>::Clear()
{
	this->end   = start;
	this->count = 0;
}
template <class T>
T *LangArray<T>::AddVal(T a)
{
	ASSERT(this->count < this->length && this->length > 0)


	T *ret = this->end;
	*ret   = a;
	this->end++;
	this->count++;
	return ret;
}

template <class T>
T *LangArray<T>::operator[](int index)
{
	ASSERT(index >= 0 && index <= this->length)
	T *ret = this->start + index;
	return ret;
}


template <class T>
void LangArray<T>::ReplaceLangArrayData(T *a, int count)
{
	ASSERT(count <= this->length)

	memcpy(this->start, a, sizeof(T) * count);
	this->end = this->start + count;
	this->count = count;
}
template <class T>
void InitArWithBuffer(LangArray<T> *ar, char *buffer, int length)
{
	ar->start  = (T *)(buffer);
	ar->end    = ar->start;
	ar->length = length;
	ar->count  = 0;
}

