#pragma once

template<typename T>
struct CyclingQueue {
	T* data;
	u32 capacity;
	u32 current_bottom;
	u32 current_size;
	
	CyclingQueue() : data((T*)malloc(16 * sizeof(T))), capacity(16), current_bottom(0), current_size(0) {
		
	}
	CyclingQueue(u32 initial_capacity) : data((T*)malloc(initial_capacity * sizeof(T))), capacity(initial_capacity), current_bottom(0), current_size(0) {
		
	}
	void double_capacity() {
		u32 new_capacity = capacity * 2;
		T* new_data = (T*)malloc(new_capacity * sizeof(T));
		
		u32 i = 0;
		for(; (current_bottom + i) < capacity && i < current_size; i++) {
			new_data[i] = std::move(data[current_bottom + i]);
		}
		if(i < current_size) {
			for(u32 j = 0; j < (current_size - i); j++) {
				new_data[i + j] = std::move(data[j]);
			}
		}
		free(data);
		data = new_data;
		capacity = new_capacity;
		current_bottom = 0;
	}
	void pop(T* t_ptr) {
		*t_ptr = data[current_bottom];
		current_bottom = (current_bottom + 1) % capacity;
		current_size--;
	}
	T pop() {
		T t = data[current_bottom];
		current_bottom = (current_bottom + 1) % capacity;
		current_size--;
		return t;
	}
	bool empty() {
		return current_size == 0;
	}
	u32 size() {
		return current_size;
	}
	void push(const T& element) {
		if(current_size == capacity) double_capacity();
		data[(current_bottom + current_size) % capacity] = element;
		current_size++;
	}
};

template<typename T>
struct ConcurrentCyclingQueue {
	std::mutex mutex;
	T* data;
	u32 capacity;
	u32 current_bottom;
	u32 current_size;
	
	ConcurrentCyclingQueue() : data((T*)malloc(16 * sizeof(T))), capacity(16), current_bottom(0), current_size(0) {
		
	}
	ConcurrentCyclingQueue(u32 initial_capacity) : data((T*)malloc(initial_capacity * sizeof(T))), capacity(initial_capacity), current_bottom(0), current_size(0) {
		
	}
	void double_capacity() {
		u32 new_capacity = capacity * 2;
		T* new_data = (T*)malloc(new_capacity * sizeof(T));
		
		u32 i = 0;
		for(; (current_bottom + i) < capacity && i < current_size; i++) {
			new_data[i] = std::move(data[current_bottom + i]);
		}
		if(i < current_size) {
			for(u32 j = 0; j < (current_size - i); j++) {
				new_data[i + j] = std::move(data[j]);
			}
		}
		free(data);
		data = new_data;
		capacity = new_capacity;
		current_bottom = 0;
	}
	bool pop(T* t_ptr) {
		mutex.lock();
		if(!current_size) {
			mutex.unlock();
			return false;
		}
		*t_ptr = data[current_bottom];
		current_bottom = (current_bottom + 1) % capacity;
		current_size--;
		mutex.unlock();
		return true;
	}
	u32 size() {
		mutex.lock();
		u32 size = current_size;
		mutex.unlock();
		return size;
	}
	bool empty() {
		return size() == 0;
	}
	void push(const T& element) {
		mutex.lock();
		if(current_size == capacity) double_capacity();
		data[(current_bottom + current_size) % capacity] = element;
		current_size++;
		mutex.unlock();
	}
};