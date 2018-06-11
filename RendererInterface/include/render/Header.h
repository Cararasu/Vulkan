#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtx/normal.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

#include <map>
#include <set>
#include <vector>
#include <inttypes.h>

typedef uint32_t u32;
typedef uint64_t u64;
typedef int32_t s32;
typedef int64_t s64;

typedef uint32_t ResourceId;

typedef float f32;
typedef double f64;

//TODO implement own DynamicArrayClass
template<typename T>
using Array = std::vector<T>;
template<typename T>
using Set = std::set<T>;
template<typename K, typename T>
using Map = std::map<K, T>;

enum class RendResult {
	e_rend_fail = 0,
	e_rend_success = 1,

};

template<typename T>
struct ChangeableValue {
	mutable T wanted;
	T value;

	ChangeableValue (T value) : wanted (value), value(value) { }
	ChangeableValue (T wanted, T value) : wanted (wanted), value(value) { }
	
	void operator=(T&& newvalue) const{
		wanted = newvalue;
	}
	void operator=(const T& newvalue) const{
		wanted = newvalue;
	}
	
	explicit operator T() const{
		return value;
	}
	inline bool changed() const{
		return wanted != value;
	}
	inline bool unchanged() const{
		return wanted == value;
	}
	inline void apply(){
		value = wanted;
	}
	inline void apply(T val){
		wanted = val;
		value = val;
	}
};
