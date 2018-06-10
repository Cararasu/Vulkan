#pragma once
#include "Header.h"

template<typename T>
struct Extend2D {
	union {
		T width;
		T x;
		T min;
	};
	union {
		T height;
		T y;
		T max;
	};

	Extend2D (T width_ = 0, T height_ = 0)
		: width (width_), height (height_) {
	}

	Extend2D (Extend2D const & rhs) {
		memcpy (this, &rhs, sizeof (Extend2D));
	}

	Extend2D& operator= (Extend2D const & rhs) {
		memcpy (this, &rhs, sizeof (Extend2D));
		return *this;
	}

	inline bool operator== (Extend2D const& rhs) const {
		return (width == rhs.width) && (height == rhs.height);
	}
	inline bool operator!= (Extend2D const& rhs) const {
		return !operator== (rhs);
	}
	
	inline Extend2D<T>& operator+= (Extend2D<T> extend) {
		this->x += extend.x;
		this->y += extend.y;
		return *this;
	}
	inline Extend2D<T>& operator-= (Extend2D<T> extend) {
		this->x -= extend.x;
		this->y -= extend.y;
		return *this;
	}
	inline Extend2D<T>& operator+= (T value) {
		this->x += value;
		this->y += value;
		return *this;
	}
	inline Extend2D<T>& operator-= (T value) {
		this->x -= value;
		this->y -= value;
		return *this;
	}
	inline Extend2D<T>& operator*= (T value) {
		this->x *= value;
		this->y *= value;
		return *this;
	}
	inline Extend2D<T>& operator/= (T value) {
		this->x /= value;
		this->y /= value;
		return *this;
	}
};

template<typename T>
inline Extend2D<T> operator+ (Extend2D<T> extend, T value) {
	Extend2D<T> ext = extend;
	return ext += value;
}
template<typename T>
inline Extend2D<T> operator+ (T value, Extend2D<T> extend) {
	return extend + value;
}
template<typename T>
inline Extend2D<T> operator+ (Extend2D<T>& lextend, Extend2D<T>& rextend) {
	Extend2D<T> ext = lextend;
	return ext += rextend;
}
template<typename T>
inline Extend2D<T> operator+ (Extend2D<T> lextend, Extend2D<T> rextend) {
	return lextend += rextend;
}
template<typename T>
inline Extend2D<T> operator- (Extend2D<T> extend, T value) {
	Extend2D<T> ext = extend;
	return ext -= value;
}
template<typename T>
inline Extend2D<T> operator- (T value, Extend2D<T> extend) {
	Extend2D<T> ext = extend;
	ext.x = value - extend.x;
	ext.y = value - extend.y;
	return ext;
}
template<typename T>
inline Extend2D<T> operator- (Extend2D<T>& lextend, Extend2D<T>& rextend) {
	Extend2D<T> ext = lextend;
	return ext -= rextend;
}
template<typename T>
inline Extend2D<T> operator* (Extend2D<T> extend, T value) {
	Extend2D<T> ext = extend;
	return ext *= value;
}
template<typename T>
inline Extend2D<T> operator/ (Extend2D<T> extend, T value) {
	Extend2D<T> ext = extend;
	return ext /= value;
}

template<typename T>
using Offset2D = Extend2D<T>;
template<typename T>
using Range = Extend2D<T>;

template<typename T>
inline Extend2D<T> max_extend (Extend2D<T> lmode, Extend2D<T> rmode) {
	return Extend2D<T> (std::max (lmode.x, rmode.x), std::max (lmode.y, rmode.y));
}

struct Viewport {
	Offset2D<f32> offset;
	Extend2D<f32> extend;
	Range<f32> depth;

	Viewport (float x = 0, float y = 0, float width = 0, float height = 0, float minDepth = 0, float maxDepth = 0)
		: offset (x, y), extend (width, height), depth (minDepth, maxDepth) {
	}

	Viewport (const Viewport & rhs) {
		memcpy (this, &rhs, sizeof (Viewport));
	}

	Viewport& operator= (const Viewport & rhs) {
		memcpy (this, &rhs, sizeof (Viewport));
		return *this;
	}

	bool operator== (Viewport const& rhs) const {
		return (offset == rhs.offset) && (extend == rhs.extend) && (depth == rhs.depth);
	}
	bool operator!= (Viewport const& rhs) const {
		return !operator== (rhs);
	}
};
