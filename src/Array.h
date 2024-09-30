#pragma once
//#include <windows.h>
#include "serializable_pound_defines.h"
#define FOR_ARRAY(index_name, ar_name, type_ar) for(int index_name = 0; index_name < (ar_name).count; index_name++){ type_ar *it = (ar_name)[index_name];
#define FOR(ar_name) for(auto it = (ar_name)[0]; it < (ar_name).end; it++)
#define FOR2(ar_name, idx_name) for(auto idx_name = (ar_name)[0]; idx_name < (ar_name).end; idx_name++)

#ifdef IS_DLL
 #define FUNC_STRUCT_EXPORT  
#else
 #define FUNC_STRUCT_EXPORT 
#endif


template <class T>
struct LangArray
{
	T *start;
	T *end;

	int count;
	int length;
	FUNC_STRUCT_EXPORT T *Add();
	void Init(char *buffer, int count);
	FUNC_STRUCT_EXPORT void Init(int count);

	FUNC_STRUCT_EXPORT T *Add(T *a);
	FUNC_STRUCT_EXPORT T *Add(T a);
	T *AddVal(T a);
	FUNC_STRUCT_EXPORT T *Add(LangArray<T> *a);
	T *Pop();
	T *Dequeue();
	T *Peek();
	FUNC_STRUCT_EXPORT void AddAt(unsigned int, T *);
	FUNC_STRUCT_EXPORT void RemoveAt(int);
	FUNC_STRUCT_EXPORT void Remove(T *);
	void Sort();
	void Copy(LangArray<T> *a);
	void Clear();
	void CopyDataToAr(T *a, int count);
	void CopyCreateAnotherAr(LangArray<T> *a, char *buffer);
	FUNC_STRUCT_EXPORT T *operator[](int index);
	T *operator[](char index);
	void ReplaceLangArrayData(T *a, int count);
	int GetTotalBytes();

};
template <class T>
void InitArWithBuffer(LangArray<T> *ar, char *buffer, int length);

