#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE true
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4

#include <glm/gtc/constants.hpp> //pi, ...
#include <glm/gtc/quaternion.hpp> // glm::quat
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

#include <glm/gtx/normal.hpp> // glm::mat4
#include <glm/gtx/quaternion.hpp> // glm::quat

#include <map>
#include <unordered_map>
#include <list>
#include <set>
#include <vector>

#include "RenderTypes.h"
#include "String.h"

#define MAX(lhs, rhs) (lhs > rhs ? lhs : rhs)
#define MIN(lhs, rhs) (lhs < rhs ? lhs : rhs)


template<typename T, typename ALLOC>
inline T* t_allocate ( ALLOC* allocator, u64 count ) {
	return reinterpret_cast<T*> ( allocator->allocate ( count * sizeof ( T ), alignof ( T ) ) );
}
template<typename T, typename ALLOC>
inline T* t_reallocate ( ALLOC* allocator, T* old_ptr, u64 count ) {
	return reinterpret_cast<T*> ( allocator->reallocate ( old_ptr, count * sizeof ( T ), alignof ( T ) ) );
}


//@TODO implement own DynamicArrayClass
template<typename T, typename ALLOC>
struct MyDynArray {

	u64 size = 0, capacity = 0;
	T* data = nullptr;
	ALLOC* allocator = nullptr;

	typedef T* iterator;

	void free() {
		if ( data ) {
			for ( u64 i = 0; i < size; i++ ) {
				data[i].~T();
				allocator->free ( data );
			}
			size = 0;
			capacity = 0;
			data = nullptr;
		}
	}

	MyDynArray ( ALLOC* allocator ) : size ( 0 ), capacity ( 0 ), data ( nullptr ), allocator ( allocator ) {}

	MyDynArray ( u64 size, ALLOC* allocator ) : size ( size ), capacity ( 0 ), data ( nullptr ), allocator ( allocator ) {
		if ( size ) {
			capacity = 8;
			while ( capacity < size ) capacity *= 2;
			data = t_allocate ( allocator, capacity );
			for ( int i = 0; i < size; i++ ) {
				new ( &data[i] ) T;
			}
		}
	}
	MyDynArray ( u64 size, u64 capacity, ALLOC* allocator ) : size ( size ), capacity ( capacity ), data ( nullptr ), allocator ( allocator ) {
		if ( size || capacity ) {
			while ( capacity < size ) capacity *= 2;
			data = t_allocate ( allocator, capacity );
			for ( int i = 0; i < size; i++ ) {
				new ( &data[i] ) T;
			}
		}
	}
	MyDynArray ( u64 size, const T& ele, ALLOC* allocator ) : size ( size ), capacity ( 8 ), data ( nullptr ), allocator ( allocator ) {
		if ( size ) {
			capacity = 8;
			while ( capacity < size ) capacity *= 2;
			data = t_allocate ( allocator, capacity );
			for ( u64 i = 0; i < size; i++ ) {
				new ( &data[i] ) T ( ele );
			}
		}

	}
	MyDynArray ( const MyDynArray<T, ALLOC>& array ) : size ( array.size ), capacity ( array.capacity ), data ( nullptr ), allocator ( array.allocator ) {
		if ( size ) {
			data = t_allocate ( allocator, capacity );
			for ( int i = 0; i < array.size; i++ ) {
				new ( &data[i] ) T ( array.data[i] );
			}
		}
	}
	MyDynArray ( const MyDynArray<T, ALLOC>&& array ) : size ( array.size ), capacity ( array.capacity ), data ( array.data ), allocator ( array.allocator ) {
		array.size = 0;
		array.capacity = 0;
		array.data = nullptr;
	}
	MyDynArray<T, ALLOC>& operator= ( const std::initializer_list<T> init_list ) {
		free();
		capacity = 0;
		size = init_list.size();
		if ( size ) {
			capacity = 8;
			while ( capacity < size ) capacity *= 2;
			data = t_allocate ( allocator, capacity );
			
			for ( int i = 0; i < init_list.size(); i++ ) {
				new ( &data[i] ) T ( init_list[i] );
			}
		}
		return *this;
	}
	MyDynArray<T, ALLOC>& operator= ( const MyDynArray<T, ALLOC>& array ) {
		free();
		data = nullptr;
		capacity = 0;
		size = array.size;
		if ( size ) {
			capacity = 8;
			while ( capacity < size ) capacity *= 2;
			data = t_allocate ( allocator, capacity );
			
			for ( int i = 0; i < array.size(); i++ ) {
				new ( &data[i] ) T ( std::move ( array[i] ) );
				i++;
			}
		}
		return *this;
	}
	MyDynArray<T, ALLOC>& operator= ( MyDynArray<T, ALLOC>&& array ) {
		free();

		data = array.data;
		array.data = nullptr;
		size = array.size;
		array.size = 0;
		capacity = array.capacity;
		array.capacity = 0;
		allocator = array.allocator;
		return *this;
	}
	~MyDynArray() {
		free();
	}

	void reserve ( u64 min_capacity ) {
		if ( min_capacity > this->capacity ) {
			capacity = MAX(capacity, 8);
			while ( capacity < min_capacity ) capacity *= 2;
			data = t_reallocate ( allocator, data, capacity );
		}
	}
	void shrink ( u64 min_capacity = 8 ) {
		if(!size && !min_capacity) {
			allocator->free(data);
			data = nullptr;
			capacity = 0;
		} else if ( min_capacity > this->capacity ) {
			capacity = MAX(capacity, 8);
			while ( capacity < min_capacity ) capacity *= 2;
			data = t_reallocate ( allocator, data, capacity );
		}
	}
	void push_back ( T&& ele, u64 count = 1 ) {
		if ( size + count >= capacity ) {
			capacity = MAX(capacity, 8);
			while ( capacity < size + count ) capacity *= 2;
			data = t_reallocate ( allocator, data, capacity );
		}
		for ( u64 i = 0; i < count; i++ ) {
			data[size++] = ele;
		}
	}
	void push_back ( T& ele, u64 count = 1 ) {
		if ( size + count >= capacity ) {
			capacity = MAX(capacity, 8);
			while ( capacity < size + count ) capacity *= 2;
			data = t_reallocate ( allocator, data, capacity );
		}
		for ( u64 i = 0; i < count; i++ ) {
			data[size++] = ele;
		}
	}
	void pop_back ( u64 count = 1 ) {
		data[--size].~T();
	}
	T& back() {
		return data[size - 1];
	}

	void clear ( ) {
		for(size_t i = 0; i < size; i++) {
			data[i].~T();
		}
		size = 0;
	}
	void resize ( u64 size, const T& ele ) {
		if(size > capacity) {
			capacity = MAX(capacity, 8);
			while ( capacity < size ) capacity *= 2;
			data = t_reallocate ( allocator, data, capacity );
		}
		for(size_t i = this->size; i < size; i++) {
			new (data[i]) T (ele);
		}
		this->size = size;
	}

	T& operator[] ( u64 i ) {
		return data[i];
	}
	const T& operator[] ( u64 i ) const {
		return data[i];
	}
	iterator begin() {
		return data;
	}
	iterator end() {
		return data + size;
	}
	const iterator begin() const {
		return data;
	}
	const iterator end() const {
		return data + size;
	}
};

template<typename T>
struct Array {
	u64 size;
	T* data;

	typedef T* iterator;

	Array() : size ( 0 ), data ( nullptr ) {}

	Array ( u64 size ) : size ( size ), data ( nullptr ) {
		if ( !size ) return;
		data = new T[size];
	}
	Array ( u64 size, const T& ele ) : size ( size ), data ( nullptr ) {
		if ( !size ) return;
		data = new T[size];
		for ( u64 i = 0; i < size; i++ ) {
			data[i] = ele;
		}
	}
	Array ( const Array<T>& array ) : size ( array.size ), data ( nullptr ) {
		if ( !size ) return;
		data = new T[size];
		u64 i = 0;
		for ( const T& ele : array ) {
			data[i++] = ele;
		}
	}
	Array ( const Array<T>&& array ) : size ( array.size ), data ( array.data ) {
		array.data = nullptr;
		array.size = 0;
	}
	Array ( const std::initializer_list<T> init_list ) : size ( init_list.size() ), data ( nullptr ) {
		if ( !size ) return;
		data = new T[size];
		u64 i = 0;
		for ( const T& ele : init_list ) {
			data[i++] = std::move ( ele );
		}
	}
	Array<T>& operator= ( const std::initializer_list<T> init_list ) {
		if ( data ) delete[] data;
		data = nullptr;
		size = init_list.size();
		if ( !size ) return *this;
		data = new T[size];
		u64 i = 0;
		for ( const T& ele : init_list ) {
			data[i++] = std::move ( ele );
		}
		return *this;
	}
	Array<T>& operator= ( const Array<T>& array ) {
		if ( data ) delete[] data;
		data = nullptr;
		size = array.size;
		if ( !size ) return *this;
		data = new T[size];
		u64 i = 0;
		for ( const T& ele : array ) {
			data[i++] = std::move ( ele );
		}
		return *this;
	}
	Array<T>& operator= ( Array<T>&& array ) {
		if ( data ) delete[] data;
		data = array.data;
		array.data = nullptr;
		size = array.size;
		array.size = 0;
		return *this;
	}
	~Array() {
		if ( data ) delete[] data;
		data = nullptr;
	}

	void resize ( u64 size ) {
		T* tmp_data = nullptr;
		if ( size != 0 ) {
			tmp_data = new T[size];
			u64 min = std::min ( size, this->size );
			for ( u64 i = 0; i < min; i++ ) tmp_data[i] = std::move ( data[i] );
		}

		delete[] data;
		data = tmp_data;
		this->size = size;
	}
	void resize ( u64 size, const T& ele ) {
		T* tmp_data = nullptr;
		if ( size != 0 ) {
			tmp_data = new T[size];
			for ( u64 i = 0; i < size; i++ ) tmp_data[i] = ele;
		}

		delete[] data;
		data = tmp_data;
		this->size = size;
	}
	void resize ( u64 size, T&& ele ) {
		T* tmp_data = nullptr;
		if ( size != 0 ) {
			tmp_data = new T[size];
			for ( u64 i = 0; i < size; i++ ) tmp_data[i] = ele;
		}

		delete[] data;
		data = tmp_data;
		this->size = size;
	}

	T& operator[] ( u64 i ) {
		return data[i];
	}
	const T& operator[] ( u64 i ) const {
		return data[i];
	}
	iterator begin() {
		return data;
	}
	iterator end() {
		return data + size;
	}
	const iterator begin() const {
		return data;
	}
	const iterator end() const {
		return data + size;
	}
};

template<typename T, u64 COUNT>
using StaticArray = std::array<T, COUNT>;
template<typename T>
using DynArray = std::vector<T>;
template<typename T>
using List = std::list<T>;
template<typename T>
using Set = std::set<T>;
template<typename K, typename T>
using Map = std::map<K, T>;
template<typename K, typename T>
using HashMap = std::unordered_map<K, T>;

enum class RendResult {
	Fail = 0,
	Success = 1,
	Unmodified = 2,
	AlreadyRegistered = -0x10,
	WrongInstance = -0x11,
	WrongType = -0x12,
	Uninitialized = -0x20
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
	T& operator= ( const T&& newvalue ) const {
		wanted = newvalue;
		return this->wanted;
	}

	operator T() const {
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
		if ( wanted == value ) wanted = val;
		value = val;
	}
};

struct IdHandle {
	union {
		struct {
			u32 id = 0;
			u32 uid = 0;
		};
		u64 hash;
	};
	IdHandle handle() {
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
	size_t operator() ( const IdHandle &handle ) const {
		return handle.hash;
	}
};
