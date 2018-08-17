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

template<typename T>
struct IdArray {
	IdGenerator gen;
	Array<T> list;

	typedef typename Array<T>::iterator iterator;

	IdArray() {}
	IdArray ( std::initializer_list<T> list ) : list ( list ) {
		for ( T& e : list ) e.id = gen.next();
	}
	IdArray ( Array<T> list ) : list ( list ) {
		for ( T& e : list ) e.id = gen.next();
	}

	RId insert ( T& ele ) {
		for ( size_t i = 0; i < list.size(); i++ ) {
			if ( !list[i].id ) {
				ele.id = i + 1;
				list[i] = ele;
				return ele.id;
			}
		}
		ele.id = gen.next();
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
		gen.clear();
		list.clear();
	}
};

template<typename T>
struct IdArrayArray {
	IdGenerator gen;
	Array<T*> list;

	typedef typename Array<T>::iterator iterator;

	IdArrayArray() {}
	IdArrayArray ( std::initializer_list<T*> list ) : list ( list ) {
		for ( T* e : list ) e->id = gen.next();
	}
	IdArrayArray ( Array<T*> list ) : list ( list ) {
		for ( T* e : list ) e->id = gen.next();
	}

	RId insert ( T* ele ) {
		for ( size_t i = 0; i < list.size(); i++ ) {
			if ( !list[i].id ) {
				ele->id = i + 1;
				list[i] = ele;
				return ele->id;
			}
		}
		ele->id = gen.next();
		list.push_back ( ele );
		return ele->id;
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
	void remove ( RId id ) {
		get ( id )->id = 0;
	}
	T* get ( RId id ) {
		return id && id <= list.size() ? list[id - 1] : nullptr;
	}
	T& operator[] ( RId id ) {
		return *get ( id );
	}
	void clear() {
		gen.clear();
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

	RId insert ( T ele ) {
		RId id = 0;
		if ( ele.id && ( id = insert ( ele.id, ele ) ) )
			return id;
		id = gen.next();
		while ( map.find ( id ) != map.end() ) id = gen.next();
		ele.id = id;
		map.insert ( std::make_pair ( id, ele ) );
		return id;
	}
	RId insert ( RId id, T ele ) {
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
	Array<T> data;
	Array<bool> filled_bits;

	u32 create_chunk ( ) {
		for ( u32 i = 0; i < data.size(); i++ ) {
			if ( filled_bits[i] ) {
				filled_bits[i] = false;
				return i;
			}
		}
		data.emplace_back ( );
		filled_bits.push_back ( false );
		return data.size() - 1;
	}
	T* operator[] ( u32 index ) {
		return &data[index];
	}
	void delete_chunk ( u32 index ) {
		filled_bits[index] = true;
		while ( filled_bits.back() ) {
			filled_bits.pop_back();
			data.pop_back();
		}
	}
	void clear ( ) {
		data.clear();
		filled_bits.clear();
	}
};

template<>
struct SparseStore<void> {
	u32 genericdatasize;
	u32 end_index;
	u32 capacity;
	u8* data;
	u8* filled_bits;

	SparseStore<void> ( u32 genericdatasize ) : genericdatasize ( genericdatasize ), end_index ( 0 ), capacity ( 8 ), data ( new u8[8 * genericdatasize] ), filled_bits ( new u8[1] ) {}
	SparseStore<void> ( u32 genericdatasize, u32 initial_capacity ) : genericdatasize ( genericdatasize ), end_index ( 0 ) {
		capacity = initial_capacity;
		u32 overflow = capacity % 8;
		if ( overflow ) {
			capacity += 8 - overflow;
		}
		data = new u8[capacity];
		filled_bits = new u8[capacity / 8];
	}
	~SparseStore<void> ( ) {
		if ( data ) delete[] data;
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

		delete[] data;
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
		} else if ( blocks <= capacity / 4 && capacity / 4 >= 8 ) {
			reallocate ( capacity / 2 );
		}
		end_index = blocks;
	}

	u32 create_chunk () {
		for ( u32 i = 0; i < end_index; i++ ) {
			if ( !is_filled(i) ) {
				set_filled(i);
				return i;
			}
		}
		u32 new_index = end_index;
		resize ( end_index + 1 );
		set_filled ( new_index );
		return new_index;
	}
	u8* operator[] ( u32 index ) {
		return &data[index * genericdatasize];
	}
	void delete_chunk ( u32 index ) {
		set_free ( index );
		u32 last_free_index = end_index;
		for(; last_free_index > 0; last_free_index--){
			if(is_filled(last_free_index - 1))
				break;
		}
		resize(last_free_index);
	}
};
