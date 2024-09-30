#pragma once

template <typename T, typename U>
struct rel_ptr
{
	T offset;
	
	void operator=(own_std::string &ptr)
	{
	}
	void operator=(char *ptr)
	{
		offset = ptr ? (int)(ptr - (char *)&offset):0;
	}
	U *Ptr()
	{
		return offset ? (T *)((char *)&offset + offset) : nullptr;
	}
	U *operator->()
	{
		return offset ? (T *)((char *)&offset + offset) : nullptr;
	}
};

struct rel_str
{
	rel_ptr<int, char *> data;
	int len;
};

