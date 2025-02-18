#pragma once

#include <typeinfo>
#include <typeindex>
#include <atomic>
#include <limits>
#include <array>
#include <string>

#define NO_ID UINT_MAX

class ITypeTag
{
public:
	ITypeTag() : _typeInfo(typeid(ITypeTag)) {};
	virtual ~ITypeTag() {};
	const std::string Name() const noexcept { return _typeInfo.name(); };
	const std::size_t Tag() const noexcept { return _typeInfo.hash_code(); };

	const bool operator==(const ITypeTag& other) const noexcept { return _typeInfo == other._typeInfo; };
	const bool operator!=(const ITypeTag& other) const noexcept { return _typeInfo != other._typeInfo; };

	template<typename U>
	const bool IsThisA() const noexcept { return _typeInfo == std::type_index(typeid(U)); };

	template<typename U>
	static const bool IsThisNameA(const std::string& name) noexcept { return name == std::string(std::type_index(typeid(U)).name()); };

	const ITypeTag& TypeTag() const noexcept { return *this; };

protected:

	template <typename T>
	void SetTypeInfo(T* thisObj) noexcept
	{
		_typeInfo = std::type_index(typeid(T));
	};

private:
	std::type_index _typeInfo;
};

class IDistinguishable
{
public:
	IDistinguishable() : Id(NO_ID) {};
	virtual ~IDistinguishable() {};

	std::size_t Id;

protected:

	template <typename T> 
	void SetId(T* thisObj) noexcept
	{
		Id = GetNextId(thisObj);
	};

private:

	template <typename T>
	std::size_t GetNextId(T* thisObj) noexcept
	{
		static std::atomic<std::size_t> s_nextId = 0;
		return s_nextId++;
	};
};

class IUnquielyIdentifiable : private IDistinguishable, private ITypeTag
{
public:
	IUnquielyIdentifiable() {};
	virtual ~IUnquielyIdentifiable() {};

	std::array<std::size_t, 2> UniqueId() const noexcept { return { Tag(), Id() }; };

	std::size_t Id() const noexcept { return IDistinguishable::Id; };


	std::size_t Tag() const noexcept { return ITypeTag::Tag(); };
	const std::string TypeName() const noexcept { return ITypeTag::Name(); };

	template<typename U>
	const bool IsThisA() const noexcept { return ITypeTag::IsThisA<U>(); };

	const bool operator==(const IUnquielyIdentifiable& other) const noexcept { return UniqueId() == other.UniqueId(); };
	const bool operator!=(const IUnquielyIdentifiable& other) const noexcept { return UniqueId() != other.UniqueId(); };

	const bool TypeEquals(const IUnquielyIdentifiable& other) const noexcept { return Tag() == other.Tag(); };


protected:

	template <typename T>
	void SetUniqueId(T* thisObj) noexcept
	{
		SetId(thisObj);
		SetTypeInfo(thisObj);
	};
};

