#ifndef DRAWDISPATCHER_H
#define DRAWDISPATCHER_H

#include "RenderEnvironment.h"
#include <assert.h>
#include <vector>

struct ObjId{
	uint32_t id, index;
};

template<uint32_t TYPEID, typename... OBJDATAs> struct ObjStore;

template<uint32_t TYPEID>
struct ObjStore <TYPEID>{
	template<typename TOBJDATA>
	inline ObjId insert(TOBJDATA& data){
		assert(false);//Type not defined
		return {0, 0};
	}
	inline bool delete_obj(ObjId objId){
		return false;
	}
	inline void* get_obj_raw(ObjId objId) const {
		return nullptr;
	}
	inline void clean() {}
	inline void update(ThreadRenderEnvironment* renderEnv) {}
	inline void execute(ThreadRenderEnvironment* renderEnv) {}
};

template<uint32_t TYPEID, typename OBJDATA, typename... OBJDATAs>
struct ObjStore <TYPEID, OBJDATA, OBJDATAs...>{
	std::vector<std::pair<OBJDATA, bool>> obj_list;
	ObjStore<TYPEID + 1, OBJDATAs...> subStore;
	template<typename TOBJDATA>
	inline ObjId insert(TOBJDATA& data){
		return subStore.insert(data);
	}
	inline ObjId insert(OBJDATA& data){
		for(auto it = obj_list.begin(); it != obj_list.end(); it++) {
			if(!it->second){
				it->first = data;
				return {TYPEID, static_cast<uint32_t>(std::distance(obj_list.begin(), it))};
			}
		}
		obj_list.push_back(std::make_pair(data, true));
		return {TYPEID, static_cast<uint32_t>(obj_list.size() - 1)};
	}
	template<typename TOBJDATA>
	inline TOBJDATA* get_obj(ObjId objId) const {
		return subStore.get_obj(objId);
	}
	inline OBJDATA* get_obj(ObjId objId) const {
		return &obj_list[objId.index];
	}
	inline void* get_obj_raw(ObjId objId) const {
		return objId.id == TYPEID ? reinterpret_cast<void*>(&obj_list[objId.index]) : subStore.get_obj_raw(objId);
	}
	inline bool delete_obj(ObjId objId) const {
		return objId.id == TYPEID ? obj_list[objId.index].second = false, true : subStore.delete_obj(objId);
	}
	inline void clean(){
		obj_list.clean();
		subStore.clean();
	}
	inline void update(ThreadRenderEnvironment* renderEnv){
		for(std::pair<OBJDATA, bool>& p : obj_list){
			if(p.second) p.first.update(renderEnv);
		}
		subStore.update(renderEnv);
	}
	inline void execute(ThreadRenderEnvironment* renderEnv){
		for(std::pair<OBJDATA, bool>& p : obj_list){
			if(p.second) p.first.execute(renderEnv);
		}
		subStore.execute(renderEnv);
	}
};

template< typename... OBJDATAs>
struct ObjectStore {
	ObjStore<0, OBJDATAs...> inner_store;

	template<typename OBJDATA>
	inline ObjId insert(OBJDATA& data) {
		return inner_store.insert(data);
	}
	template<typename OBJDATA>
	inline OBJDATA* get_obj(ObjId objId) const {
		return inner_store.get_obj(objId);
	}
	inline void* get_obj_raw(ObjId objId) const {
		return inner_store.get_obj_raw(objId);
	}
	inline bool delete_obj(ObjId objId) const {
		return inner_store.delete_obj(objId);
	}
	inline void clean() {
		inner_store.clean();
	}
	inline void update(ThreadRenderEnvironment* renderEnv) {
		inner_store.update(renderEnv);
	}
	inline void execute(ThreadRenderEnvironment* renderEnv) {
		inner_store.execute(renderEnv);
	}
};


#endif // DRAWDISPATCHER_H
