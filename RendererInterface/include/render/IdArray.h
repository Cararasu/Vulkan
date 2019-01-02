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
template<typename T, u32 OFFSET = 0>
struct IdArray {
	DynArray<T> list;
	typedef typename DynArray<T>::iterator iterator;

	IdArray() {}
	IdArray ( std::initializer_list<T> list ) : list ( list ) {
		for ( size_t i = 0; i < list.size(); i++ ) {
			list[i].id = OFFSET + i + 1;
		}
		this->list.insert ( list.begin(), list.end(), this->list.begin() );
	}
	IdArray ( DynArray<T> list ) : list ( list ) {
		for ( size_t i = 0; i < list.size(); i++ ) {
			list[i].id = OFFSET + i + 1;
		}
		this->list.insert ( list.begin(), list.end(), this->list.begin() );
	}

	RId insert ( const T&& ele ) {
		for ( size_t i = 0; i < list.size(); i++ ) {
			if ( !list[i].id ) {
				u32 id = OFFSET + i + 1;
				list[i] = std::move(ele);
				list[i].id = id;
				return id;
			}
		}
		u32 id = OFFSET + list.size() + 1;
		list.push_back ( std::move(ele) );
		list.back().id = id;
		return id;
	}
	RId insert ( T& ele ) {
		for ( size_t i = 0; i < list.size(); i++ ) {
			if ( !list[i].id ) {
				u32 id = OFFSET + i + 1;
				list[i] = ele;
				list[i].id = id;
				return id;
			}
		}
		u32 id = OFFSET + list.size() + 1;
		list.push_back ( ele );
		list.back().id = id;
		return id;
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
		return id > OFFSET && id <= OFFSET + list.size() ? &list[id - 1 - OFFSET] : nullptr;
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
		if ( ptr->uid == handle.uid ) ptr->id = 0;
	}
	T* get ( IdHandle handle ) {
		return handle.id && handle.id <= list.size() ? ( list[handle.id - 1].uid == handle.uid ? &list[handle.id - 1] : nullptr ) : nullptr;
	}
	T& operator[] ( RId id ) {
		return *get ( id );
	}
	void clear() {
		list.clear();
	}
};

template<typename T, u32 OFFSET = 0>
struct IdPtrArray {
	DynArray<T*> list;

	typedef typename Array<T*>::iterator iterator;

	IdPtrArray() {}
	IdPtrArray ( std::initializer_list<T> list ) : list ( list ) {
		for ( size_t i = 0; i < list.size(); i++ ) {
			list[i].id = OFFSET + i + 1;
		}
		this->list.insert ( list.begin(), list.end(), this->list.begin() );
	}
	IdPtrArray ( DynArray<T> list ) : list ( list ) {
		for ( size_t i = 0; i < list.size(); i++ ) {
			list[i].id = OFFSET + i + 1;
		}
		this->list.insert ( list.begin(), list.end(), this->list.begin() );
	}
	~IdPtrArray () {

	}

	T* insert ( T* ele ) {
		for ( size_t i = 0; i < list.size(); i++ ) {
			if ( !list[i] ) {
				ele->id = OFFSET + i + 1;
				list[i] = ele;
				return list[i];
			}
		}
		ele->id = OFFSET + list.size() + 1;
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
		return id > OFFSET && id <= OFFSET + list.size() ? list[id - 1 - OFFSET] : nullptr;
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
		this->list.insert ( list.begin(), list.end(), this->list.begin() );
	}
	UIdPtrArray ( DynArray<T*> list ) : list ( list ) {
		for ( size_t i = 0; i < list.size(); i++ ) {
			list[i]->id = i + 1;
			list[i]->uid = gen.next();
		}
		this->list.insert ( list.begin(), list.end(), this->list.begin() );
	}
	~UIdPtrArray () {
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
		if ( ptr->uid == handle.uid ) ptr->id = 0;
	}
	T* get ( IdHandle handle ) {
		return handle.id && handle.id <= list.size() ? ( list[handle.id - 1]->uid == handle.uid ? list[handle.id - 1] : nullptr ) : nullptr;
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
