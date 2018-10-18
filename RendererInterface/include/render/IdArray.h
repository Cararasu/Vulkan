#pragma once

#include "Header.h"

class IdGenerator {
private:
	RId val = 0;
public:
	RId next() {
		return ++val;
	}
	void clear() {
		val = 0;
	}
};

//maybe move to 2 arrays where the 1 references the second and we can implement a freelist in the mapping array
template<typename T>
struct IdArray {
	DynArray<T> list;
	typedef typename DynArray<T>::iterator iterator;

	IdArray() {}
	IdArray ( std::initializer_list<T> list ) : list ( list ) {
		for ( size_t i = 0; i < list.size(); i++ ) {
			list[i].id = i + 1;
		}
		this->list.insert(list.begin(), list.end(), this->list.begin());
	}
	IdArray ( DynArray<T> list ) : list ( list ) {
		for ( size_t i = 0; i < list.size(); i++ ) {
			list[i].id = i + 1;
		}
		this->list.insert(list.begin(), list.end(), this->list.begin());
	}

	RId insert ( T& ele ) {
		for ( size_t i = 0; i < list.size(); i++ ) {
			if ( !list[i].id ) {
				ele.id = i + 1;
				list[i] = ele;
				return i + 1;
			}
		}
		ele.id = list.size() + 1;
		list.push_back ( ele );
		return ele.id;
	}
	/*template< class... Args >
	RId emplace_back ( Args&&... args ) {
		list.emplace_back ( args... );
		list.back().id = gen.next();
		return list.back().id;
	}*/
	auto begin() -> decltype ( list.begin() ) {
		return list.begin();
	}
	auto end() -> decltype ( list.end() ) {
		return list.end();
	}
	auto size() -> decltype ( list.size() ) {
		return list.size();
	}
	auto back() -> decltype ( list.back() ) {
		return list.back();
	}
	iterator erase ( iterator it ) {
		it->id = 0;
		return ++it;
	}
	void remove ( RId id ) {
		get ( id )->id = 0;
	}
	T* get ( RId id ) {
		return id && id <= list.size() ? &list[id - 1] : nullptr;
	}
	T& operator[] ( RId id ) {
		return *get ( id );
	}
	void clear() {
		list.clear();
	}
};

template<typename T>
struct UIdArray {
	IdGenerator gen;
	DynArray<T> list;

	typedef typename DynArray<T>::iterator iterator;

	UIdArray() {}
	UIdArray ( std::initializer_list<T> init_list ) : list ( init_list ) {
		for ( size_t i = 0; i < list.size(); i++ ) {
			this->list[i].id = i + 1;
			this->list[i].uid = gen.next();
		}
	}
	UIdArray ( DynArray<T>& dynarray ) : list ( dynarray.list ) {
		for ( size_t i = 0; i < list.size(); i++ ) {
			this->list[i].id = i + 1;
			this->list[i].uid = gen.next();
		}
	}

	IdHandle insert ( T& ele ) {
		for ( size_t i = 0; i < list.size(); i++ ) {
			if ( !list[i].id ) {
				ele.id = i + 1;
				ele.uid = gen.next();
				list[i] = ele;
				return ele.id;
			}
		}
		ele.id = list.size() + 1;
		ele.uid = gen.next();
		list.push_back ( ele );
		return {ele.id, ele.uid};
	}
	/*template< class... Args >
	RId emplace_back ( Args&&... args ) {
		list.emplace_back ( args... );
		list.back().id = gen.next();
		return list.back().id;
	}*/
	auto begin() -> decltype ( list.begin() ) {
		return list.begin();
	}
	auto end() -> decltype ( list.end() ) {
		return list.end();
	}
	auto size() -> decltype ( list.size() ) {
		return list.size();
	}
	auto back() -> decltype ( list.back() ) {
		return list.back();
	}
	iterator erase ( iterator it ) {
		it->id = 0;
		return ++it;
	}
	void remove ( IdHandle handle ) {
		T* ptr = get ( handle );
		if(ptr->uid == handle.uid) ptr->id = 0;
	}
	T* get ( IdHandle handle ) {
		return handle.id && handle.id <= list.size() ? (list[handle.id - 1].uid == handle.uid ? &list[handle.id - 1] : nullptr) : nullptr;
	}
	T& operator[] ( RId id ) {
		return *get ( id );
	}
	void clear() {
		list.clear();
	}
};

template<typename T>
struct IdPtrArray {
	DynArray<T*> list;

	typedef typename Array<T*>::iterator iterator;

	IdPtrArray() {}
	IdPtrArray ( std::initializer_list<T> list ) : list ( list ) {
		for ( size_t i = 0; i < list.size(); i++ ) {
			list[i].id = i + 1;
		}
		this->list.insert(list.begin(), list.end(), this->list.begin());
	}
	IdPtrArray ( DynArray<T> list ) : list ( list ) {
		for ( size_t i = 0; i < list.size(); i++ ) {
			list[i].id = i + 1;
		}
		this->list.insert(list.begin(), list.end(), this->list.begin());
	}
	~IdPtrArray ( ) {
		
	}

	T* insert ( T* ele ) {
		for ( size_t i = 0; i < list.size(); i++ ) {
			if ( !list[i] ) {
				ele->id = i + 1;
				list[i] = ele;
				return list[i];
			}
		}
		ele->id = list.size() + 1;
		list.push_back ( ele );
		return list.back();
	}
	/*template< class... Args >
	RId emplace_back ( Args&&... args ) {
		list.emplace_back ( args... );
		list.back().id = gen.next();
		return list.back().id;
	}*/
	auto begin() -> decltype ( list.begin() ) {
		return list.begin();
	}
	auto end() -> decltype ( list.end() ) {
		return list.end();
	}
	auto size() -> decltype ( list.size() ) {
		return list.size();
	}
	auto back() -> decltype ( list.back() ) {
		return list.back();
	}
	iterator erase ( iterator it ) {
		( *it ) = nullptr;
		return ++it;
	}
	T* remove ( RId id ) {
		T* ptr = get ( id );
		list[id - 1] = nullptr;
		return ptr;
	}
	T* get ( RId id ) {
		return id && id <= list.size() ? list[id - 1] : nullptr;
	}
	T* operator[] ( RId id ) {
		return get ( id );
	}
	void clear() {
		list.clear();
	}
};
template<typename T>
struct UIdPtrArray {
	IdGenerator gen;
	DynArray<T*> list;

	typedef typename Array<T*>::iterator iterator;

	UIdPtrArray() {}
	UIdPtrArray ( std::initializer_list<T*> list ) : list ( list ) {
		for ( size_t i = 0; i < list.size(); i++ ) {
			list[i]->id = i + 1;
			list[i]->uid = gen.next();
		}
		this->list.insert(list.begin(), list.end(), this->list.begin());
	}
	UIdPtrArray ( DynArray<T*> list ) : list ( list ) {
		for ( size_t i = 0; i < list.size(); i++ ) {
			list[i]->id = i + 1;
			list[i]->uid = gen.next();
		}
		this->list.insert(list.begin(), list.end(), this->list.begin());
	}
	~UIdPtrArray ( ) {
	}

	T* insert ( T* ele ) {
		for ( size_t i = 0; i < list.size(); i++ ) {
			if ( !list[i]->id ) {
				ele->id = i + 1;
				ele->uid = gen.next();
				list[i] = ele;
				return list[i];
			}
		}
		ele->id = list.size() + 1;
		ele->uid = gen.next();
		list.push_back ( ele );
		return list.back();
	}
	/*template< class... Args >
	RId emplace_back ( Args&&... args ) {
		list.emplace_back ( args... );
		list.back().id = gen.next();
		return list.back().id;
	}*/
	auto begin() -> decltype ( list.begin() ) {
		return list.begin();
	}
	auto end() -> decltype ( list.end() ) {
		return list.end();
	}
	auto size() -> decltype ( list.size() ) {
		return list.size();
	}
	auto back() -> decltype ( list.back() ) {
		return list.back();
	}
	iterator erase ( iterator it ) {
		( *it )->id = 0;
		return ++it;
	}
	void remove ( IdHandle handle ) {
		T* ptr = get ( handle );
		if(ptr->uid == handle.uid) ptr->id = 0;
	}
	T* get ( IdHandle handle ) {
		return handle.id && handle.id <= list.size() ? (list[handle.id - 1]->uid == handle.uid ? list[handle.id - 1] : nullptr) : nullptr;
	}
	T* operator[] ( RId id ) {
		return get ( id );
	}
	void clear() {
		list.clear();
	}
};


template<typename T>
struct IdStore {
	IdGenerator gen;
	Map<RId, T> map;

	typedef typename Map<RId, T>::iterator iterator;

	IdStore() {}
	IdStore ( std::initializer_list<T> list ) {
		for ( T e : list ) insert ( e );
	}

	RId insert ( T& ele ) {
		RId id = 0;
		if ( ele.id && ( id = insert ( ele.id, ele ) ) )
			return id;
		id = gen.next();
		while ( map.find ( id ) != map.end() ) id = gen.next();
		ele.id = id;
		map.insert ( std::make_pair ( id, ele ) );
		return id;
	}
	RId insert ( RId id, T& ele ) {
		if ( map.find ( id ) != map.end() ) return 0;
		ele.id = id;
		map.insert ( std::make_pair ( id, ele ) );
		return id;
	}
	iterator remove ( iterator it ) {
		return map.erase ( it );
	}
	void remove ( RId id ) {
		map.erase ( id );
	}
	auto begin() -> decltype ( map.begin() ) {
		return map.begin();
	}
	auto end() -> decltype ( map.end() ) {
		return map.end();
	}
	auto size() -> decltype ( map.size() ) {
		return map.size();
	}
	const T* get ( RId id ) {
		return &map[id];
	}
	const T& operator[] ( RId id ) {
		return *get ( id );
	}
	void clear() {
		gen.clear();
		map.clear();
	}
};

const u32 FILLED_U32_BITFIELD = 0xFFFFFFFF;
const u64 FILLED_U64_BITFIELD = 0xFFFFFFFFFFFFFFFF;

template<typename T>
struct SparseStore {
	u32 size;
	u32 capacity;
	T* data;
	u8* filled_bits;

	SparseStore ( u32 initial_capacity = 0 ) : size ( 0 ) {
		capacity = 8;
		while(capacity < initial_capacity){
			capacity *= 2;
		}
		data = new T[capacity];
		filled_bits = new u8[capacity / 8];
	}
	~SparseStore ( ) {
		if ( data ) delete[] data;
		if ( filled_bits ) delete[] filled_bits;
	}
	void set_filled ( u32 index ) {
		filled_bits[index / 8] |= 1 << ( index % 8 );
	}
	void set_free ( u32 index ) {
		filled_bits[index / 8] &= ~ ( 1 << ( index % 8 ) );
	}
	bool is_filled ( u32 index ) {
		return ( filled_bits[index / 8] & ( 1 << ( index % 8 ) ) ) != 0;
	}
	void reallocate ( u32 nextblockcount ) {
		T* newdata = new T[nextblockcount];
		u32 nextbitblockcount = nextblockcount / 8;
		u8* newbits = new u8[nextbitblockcount];
		
		memcpy ( newdata, data, std::min ( nextblockcount, capacity ) * sizeof(T) );
		memcpy ( newbits, filled_bits, std::min ( nextbitblockcount, capacity / 8 ) * sizeof ( u8 ) );

		delete[] data;
		delete[] filled_bits;

		data = newdata;
		capacity = nextblockcount;
		filled_bits = newbits;
	}

	u32 create_chunk () {
		for ( u32 i = 0; i < size; i++ ) {
			if ( !is_filled(i) ) {
				set_filled(i);
				return i;
			}
		}
		if ( capacity <= size ) {
			u32 newblockcount = capacity;
			newblockcount *= 2;
			reallocate ( newblockcount );
		}
		set_filled ( size );
		return size++;
	}
	T* operator[] ( u32 index ) {
		return &data[index];
	}
	void delete_chunk ( u32 index ) {
		set_free ( index );
		u32 last_free_index = size;
		for(; last_free_index > 0; last_free_index--){
			if(is_filled(last_free_index - 1))
				break;
		}
		if ( last_free_index <= capacity / 4 && capacity / 2 >= 8 ) {
			reallocate ( capacity / 2 );
		}
		size = last_free_index;
	}
};

template<>
struct SparseStore<void> {
	u32 genericdatasize;
	u32 size;
	u32 capacity;
	void* data;
	u8* filled_bits;

	SparseStore<void> ( u32 genericdatasize, u32 initial_capacity = 0 ) : genericdatasize ( genericdatasize ), size ( 0 ) {
		capacity = 8;
		while(capacity < initial_capacity){
			capacity *= 2;
		}
		data = malloc(capacity * genericdatasize);
		filled_bits = new u8[capacity / 8];
	}
	~SparseStore<void> ( ) {
		if ( data ) free(data);
		if ( filled_bits ) delete[] filled_bits;
	}
	void set_filled ( u32 index ) {
		filled_bits[index / 8] |= 1 << ( index % 8 );
	}
	void set_free ( u32 index ) {
		filled_bits[index / 8] &= ~ ( 1 << ( index % 8 ) );
	}
	bool is_filled ( u32 index ) {
		return ( filled_bits[index / 8] & ( 1 << ( index % 8 ) ) ) != 0;
	}
	void reallocate ( u32 nextblockcount ) {
		u8* newdata = new u8[nextblockcount * genericdatasize];
		u32 nextbitblockcount = nextblockcount / 8;
		u8* newbits = new u8[nextbitblockcount];

		memcpy ( newdata, data, std::min ( nextblockcount, capacity ) * genericdatasize );
		memcpy ( newbits, filled_bits, std::min ( nextbitblockcount, capacity / 8 ) * sizeof ( u8 ) );

		free(data);
		delete[] filled_bits;

		data = newdata;
		capacity = nextblockcount;
		filled_bits = newbits;
	}
	void resize ( u32 blocks ) {
		if ( capacity < blocks ) {
			u32 newblockcount = capacity;
			while ( newblockcount <= blocks ) newblockcount *= 2;
			reallocate ( newblockcount );
		} else if ( blocks <= capacity / 4 && capacity / 2 >= 8 ) {
			reallocate ( capacity / 2 );
		}
		size = blocks;
	}

	u32 create_chunk () {
		for ( u32 i = 0; i < size; i++ ) {
			if ( !is_filled(i) ) {
				set_filled(i);
				return i;
			}
		}
		u32 new_index = size;
		resize ( size + 1 );
		set_filled ( new_index );
		return new_index;
	}
	u8* operator[] ( u32 index ) {
		return &((u8*)data)[index * genericdatasize];
	}
	void delete_chunk ( u32 index ) {
		set_free ( index );
		u32 last_free_index = size;
		for(; last_free_index > 0; last_free_index--){
			if(is_filled(last_free_index - 1))
				break;
		}
		resize(last_free_index);
	}
};
