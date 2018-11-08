#pragma once

#include "RenderTypes.h"
#include <cstring>
#include <cstdlib>


constexpr uint64_t djb2_hash(const char* ptr) {
	return ptr ? (*ptr ? (djb2_hash(ptr + 1) * 33) ^ *ptr : 5381) : 0;
}

struct String{
	const char* cstr;
	u64 hash;
	
	constexpr String() : cstr(nullptr), hash(0) {}
	constexpr String(const char* str) : cstr(str), hash(djb2_hash(str)) {}
	constexpr String(const char* str, u64 hash) : cstr(str), hash(hash) {}
	constexpr String(const String& str) : cstr(str.cstr), hash(str.hash) {}
	constexpr String(const String&& str) : cstr(str.cstr), hash(str.hash) {}
	
	operator bool(){
		return cstr != nullptr;
	}
	
	void calc_hash() {
		hash = djb2_hash(cstr);
	}
	size_t len() {
		return strlen(cstr);
	}
	String copy(){
		return String();
	}
	void free_str(){
		free(const_cast<char*>(cstr));
	}
};

inline bool operator==(const String& lhs, const String& rhs){
	return lhs.hash == rhs.hash ? strcmp(lhs.cstr, rhs.cstr) == 0 : false;
}
inline bool operator==(const String& lhs, const char* rhs){
	return strcmp(lhs.cstr, rhs) == 0;
}
inline bool operator==(const char* lhs, const String& rhs){
	return strcmp(lhs, rhs.cstr) == 0;
}
inline bool operator!=(const String& lhs, const String& rhs){
	return !(lhs == rhs);
}
inline bool operator!=(const String& lhs, const char* rhs){
	return !(lhs == rhs);
}
inline bool operator!=(const char* lhs, const String& rhs){
	return !(lhs == rhs);
}
inline bool operator<(const String& lhs, const String& rhs){
	return lhs.hash < rhs.hash ? true : strcmp(lhs.cstr, rhs.cstr) < 0;
}
inline bool operator<=(const String& lhs, const String& rhs){
	return lhs.hash <= rhs.hash ? true : strcmp(lhs.cstr, rhs.cstr) <= 0;
}
inline bool operator>(const String& lhs, const String& rhs){
	return lhs.hash > rhs.hash ? true : strcmp(lhs.cstr, rhs.cstr) > 0;
}
inline bool operator>=(const String& lhs, const String& rhs){
	return lhs.hash >= rhs.hash ? true : strcmp(lhs.cstr, rhs.cstr) >= 0;
}

#define STRINGBUFFER_SIZE (1)
struct StringBuilder{
	struct StringBuilderBuffer{
		char buffer[STRINGBUFFER_SIZE];
		StringBuilderBuffer* next = nullptr;
	} head;
	u64 char_index = 0;
	StringBuilderBuffer *last_block;
	
	StringBuilder() : head(), char_index(0), last_block(&head) {}
	~StringBuilder() {
		StringBuilderBuffer* buffer = head.next;
		while(buffer){
			StringBuilderBuffer* tptr = buffer;
			buffer = buffer->next;
			delete tptr;
		}
	}
	
	u64 size(){
		return char_index + 1;
	}
	
	void create_new_block() {
		last_block->next = new StringBuilderBuffer();
		last_block = last_block->next;
	}
	StringBuilder& append_char(char c) {
		last_block->buffer[char_index % STRINGBUFFER_SIZE] = c;
		char_index++;
		if(!(char_index % STRINGBUFFER_SIZE)) create_new_block();
		return *this;
	}
	StringBuilder& append(const char* cstr) {
		return append(cstr, strlen(cstr));
	}
	StringBuilder& append(const char* cstr, s64 length) {
		s64 block_capacity = (((char_index / STRINGBUFFER_SIZE) + 1) * STRINGBUFFER_SIZE);
		s64 charsleft = (char_index + length) - block_capacity;
		if(charsleft >= 0){
			u64 tocopy = length - charsleft;
			memcpy(last_block->buffer + (char_index % STRINGBUFFER_SIZE), cstr, tocopy);
			cstr += tocopy;
			char_index = block_capacity;
			create_new_block();
		}else{
			memcpy(last_block->buffer + (char_index % STRINGBUFFER_SIZE), cstr, length);
			char_index += length;
			return *this;
		}
		while(charsleft >= STRINGBUFFER_SIZE) {
			memcpy(last_block->buffer, cstr, STRINGBUFFER_SIZE);
			cstr += STRINGBUFFER_SIZE;
			char_index += STRINGBUFFER_SIZE;
			charsleft -= STRINGBUFFER_SIZE;
			create_new_block();
		}
		if(charsleft) {
			memcpy(last_block->buffer + (char_index % STRINGBUFFER_SIZE), cstr, charsleft);
			char_index += charsleft;
		}
		return *this;
	}
	StringBuilder& append(String& str) {
		append(str.cstr);
		return *this;
	}
	
	StringBuilder& append(u64 value, int base = 10) {
		assert(base <= 16);
		u32 count = 1;
		u64 tvalue = value / base;
		while(tvalue) {
			tvalue /= base;
			count++;
		}
		char buffer[20];//18,446,744,073,709,551,615 is the highest number and has 20 digits
		for(u32 i = 1; i <= count; i++){
			int digit = value % base;
			value /= base;
			if(digit < 10){
				buffer[count - i] = '0' + digit;
			}else{
				buffer[count - i] = 'a' - 10 + digit;
			}
		}
		append(buffer, count);
		return *this;
	}
	StringBuilder& append(u32 value, int base = 10) {
		assert(base <= 16);
		u32 count = 1;
		u64 tvalue = value / base;
		while(tvalue) {
			tvalue /= base;
			count++;
		}
		char buffer[10];//4,294,967,295 is the highest number and has 10 digits
		for(u32 i = 1; i <= count; i++){
			int digit = value % base;
			value /= base;
			if(digit < 10){
				buffer[count - i] = '0' + digit;
			}else{
				buffer[count - i] = 'a' - 10 + digit;
			}
		}
		append(buffer, count);
		return *this;
	}
	
	StringBuilder& append(s64 value, int base = 10) {
		
		return *this;
	}
	StringBuilder& append(s32 value, int base = 10) {
		
		return *this;
	}
	
	StringBuilder& append(f64 value, int base = 10/*, int afterpoint*/){
		
		return *this;
	}
	StringBuilder& append(f32 value, int base = 10/*, int afterpoint*/){
		
		return *this;
	}
	
	String build() {
		char* ptr = (char*) malloc(char_index + 1);
		char* iptr = ptr;
		s64 bytesleft = char_index;
		StringBuilderBuffer* buffer = &head;
		while(buffer && bytesleft >= 0){
			s64 bytes_to_copy = bytesleft > STRINGBUFFER_SIZE ? STRINGBUFFER_SIZE : bytesleft;
			memcpy(iptr, buffer->buffer, bytes_to_copy);
			iptr += bytes_to_copy;
			bytesleft -= bytes_to_copy;
			buffer = buffer->next;
		}
		ptr[char_index] = '\0';
		assert(!buffer);
		return String(ptr);
	}
};
#undef STRINGBUFFER_SIZE
