#pragma once

#include <atomic>
#include <limits>

#define NO_ID UINT_MAX
#define NO_TAG UINT_MAX



class ITypeTag
{
public:
	uint32_t Tag;

	ITypeTag() : Tag(NO_TAG) {};
	virtual ~ITypeTag() {};

	template <typename T>
	static uint32_t TypeToTag(T* thisType)
	{
		return GetTag(thisType);
	};

protected:

	template <typename T>
	void SetTag(T* thisObj)
	{
		Tag = GetTag(thisObj);
	};

private:

	template <typename T>
	static uint32_t GetTag(T* thisObj)
	{
		static uint32_t s_tag = NO_TAG;
		if (s_tag == NO_TAG)
		{
			s_tag = GetNextTag();
		}
		return s_tag;
	};

	static int GetNextTag()
	{
		static std::atomic<int> s_nextTag = 0;
		return s_nextTag++;
	};
};

class IDistinguishable
{
public:
	IDistinguishable() : Id(NO_ID) {};
	virtual ~IDistinguishable() {};

	uint32_t Id;

protected:

	template <typename T>
	void SetId(T* thisObj)
	{
		Id = GetNextId(thisObj);
	};

private:

	template <typename T>
	uint32_t GetNextId(T* thisObj)
	{
		static std::atomic<uint32_t> s_nextId = 0;
		return s_nextId++;
	};
};

class IUnquielyIdentifiable : private IDistinguishable, private ITypeTag
{
public:
	IUnquielyIdentifiable() {};
	virtual ~IUnquielyIdentifiable() {};

	uint64_t UniqueId() const { return ((uint64_t)Tag() << 32) | Id(); };
	uint32_t Id() const { return IDistinguishable::Id; };
	uint32_t Tag() const { return ITypeTag::Tag; };

protected:

	template <typename T>
	void SetUniqueId(T* thisObj)
	{
		SetId(thisObj);
		SetTag(thisObj);
	};
};

template <typename T>
class MetaTypeTag
{
public:
	inline static uint32_t TypeTag = ITypeTag::TypeToTag((T*)nullptr);
};