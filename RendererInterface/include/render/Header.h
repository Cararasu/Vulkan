#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtx/normal.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <map>
#include <unordered_map>
#include <list>
#include <set>
#include <vector>

#include "RenderTypes.h"
#include "String.h"

//@TODO implement own DynamicArrayClass
template<typename T>
struct Array {
	u64 count;
	T* data;

	typedef T* iterator;

	Array() : count ( 0 ), data ( nullptr ) {}

	Array ( const Array<T>& array ) : count ( array.size() ), data ( new T[count] ) {
		u64 i = 0;
		for ( const T& ele : array){
			data[i++] = ele;
		}
	}
	Array ( Array<T>&& array ) : count ( array.size() ), data ( new T[count] ) {
		u64 i = 0;
		for ( const T& ele : array){
			data[i++] = std::move ( ele );
		}
	}
	Array ( std::initializer_list<T> init_list ) : count ( init_list.size() ), data ( new T[count] ) {
		u64 i = 0;
		for ( const T& ele : init_list){
			data[i++] = std::move ( ele );
		}
	}
	Array<T>& operator= ( std::initializer_list<T> init_list ) {
		count = init_list.size();
		data = new T[count];
		u64 i = 0;
		for ( const T& ele : init_list){
			data[i++] = std::move ( ele );
		}
		return *this;
	}
	~Array() {
		if(data) delete[] data;
		data = nullptr;
	}

	void resize ( u64 size ) {
		T* tmp_data = nullptr;
		if(size != 0){
			tmp_data = new T[size];
			u64 min = std::min ( size, count );
			for ( u64 i = 0; i < min; i++ ) tmp_data[i] = std::move ( data[i] );
		}
		
		delete[] data;
		data = tmp_data;
		count = size;
	}

	T& operator[] ( u64 i ) {
		return data[i];
	}
	const T& operator[] ( u64 i ) const {
		return data[i];
	}
	u64 size() const {
		return count;
	}
	iterator begin() const {
		return data;
	}
	iterator end() const {
		return data + count;
	}
};

template<typename T>
using DynArray = std::vector<T>;
template<typename T>
using List = std::list<T>;
template<typename T>
using StaticArray = std::vector<T>;
template<typename T>
using Set = std::set<T>;
template<typename K, typename T>
using Map = std::map<K, T>;
template<typename K, typename T>
using HashMap = std::unordered_map<K, T>;

enum class RendResult {
	eFail = 0,
	eSuccess = 1,
	eUnmodified = 2,
	eAlreadyRegistered = -0x10,
	eWrongInstance = -0x11,
	eWrongType = -0x12,
	eUninitialized = -0x20
};

template<typename T>
struct ChangeableValue {
	mutable T wanted;
	T value;

	ChangeableValue ( T value ) : wanted ( value ), value ( value ) { }
	ChangeableValue ( T wanted, T value ) : wanted ( wanted ), value ( value ) { }

	ChangeableValue<T>& operator= ( const ChangeableValue<T>& newvalue ) = delete;
	T& operator= ( const T& newvalue ) const {
		wanted = newvalue;
		return this->wanted;
	}
	T& operator= ( T&& newvalue ) const {
		wanted = newvalue;
		return this->wanted;
	}

	explicit operator T() const {
		return value;
	}
	inline bool changed() const {
		return wanted != value;
	}
	inline bool unchanged() const {
		return wanted == value;
	}
	inline void apply() {
		value = wanted;
	}
	inline void apply ( T val ) {
		wanted = val;
		value = val;
	}
};

struct IdHandle {
	union{
		struct{
			u32 id = 0;
			u32 uid = 0;
		};
		uint64_t hash;
	};
	IdHandle handle(){
		return *this;
	}
};
inline bool operator== ( IdHandle lh, IdHandle rh ) {
	return lh.id == rh.id && lh.uid == rh.uid;
}
inline bool operator< ( IdHandle lh, IdHandle rh ) {
	return lh.id == rh.id ? lh.uid < rh.uid : lh.id < rh.id;
}
inline bool operator> ( IdHandle lh, IdHandle rh ) {
	return lh.id == rh.id ? lh.uid > rh.uid : lh.id > rh.id;
}
template<>
struct std::hash<IdHandle> {
    size_t operator()(const IdHandle &handle) const {
        return handle.hash;
    }
};