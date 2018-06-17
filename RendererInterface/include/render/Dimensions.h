#pragma once
#include "Header.h"

template<typename T>
struct Extent2D {
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

	Extent2D (T width_ = 0, T height_ = 0)
		: width (width_), height (height_) {
	}

	Extent2D (Extent2D const & rhs) {
		memcpy (this, &rhs, sizeof (Extent2D));
	}

	Extent2D& operator= (Extent2D const & rhs) {
		memcpy (this, &rhs, sizeof (Extent2D));
		return *this;
	}

	inline bool operator== (Extent2D const& rhs) const {
		return (width == rhs.width) && (height == rhs.height);
	}
	inline bool operator!= (Extent2D const& rhs) const {
		return !operator== (rhs);
	}
	
	inline Extent2D<T>& operator+= (Extent2D<T> extend) {
		this->x += extend.x;
		this->y += extend.y;
		return *this;
	}
	inline Extent2D<T>& operator-= (Extent2D<T> extend) {
		this->x -= extend.x;
		this->y -= extend.y;
		return *this;
	}
	inline Extent2D<T>& operator+= (T value) {
		this->x += value;
		this->y += value;
		return *this;
	}
	inline Extent2D<T>& operator-= (T value) {
		this->x -= value;
		this->y -= value;
		return *this;
	}
	inline Extent2D<T>& operator*= (T value) {
		this->x *= value;
		this->y *= value;
		return *this;
	}
	inline Extent2D<T>& operator/= (T value) {
		this->x /= value;
		this->y /= value;
		return *this;
	}
};

template<typename T>
inline Extent2D<T> operator+ (Extent2D<T> extend, T value) {
	return extend += value;
}
template<typename T>
inline Extent2D<T> operator+ (T value, Extent2D<T> extend) {
	return extend + value;
}
template<typename T>
inline Extent2D<T> operator+ (Extent2D<T> lextend, Extent2D<T> rextend) {
	return lextend += rextend;
}
template<typename T>
inline Extent2D<T> operator- (Extent2D<T> extend, T value) {
	return extend -= value;
}
template<typename T>
inline Extent2D<T> operator- (T value, Extent2D<T> extend) {
	extend.x = value - extend.x;
	extend.y = value - extend.y;
	return extend;
}
template<typename T>
inline Extent2D<T> operator- (Extent2D<T>& lextend, Extent2D<T>& rextend) {
	Extent2D<T> ext = lextend;
	return ext -= rextend;
}
template<typename T>
inline Extent2D<T> operator* (Extent2D<T> extend, T value) {
	return extend *= value;
}
template<typename T>
inline Extent2D<T> operator/ (Extent2D<T> extend, T value) {
	return extend /= value;
}

template<typename T>
using Offset2D = Extent2D<T>;
template<typename T>
using Range = Extent2D<T>;

template<typename T>
inline Extent2D<T> max_extend (Extent2D<T> lmode, Extent2D<T> rmode) {
	return Extent2D<T> (std::max (lmode.x, rmode.x), std::max (lmode.y, rmode.y));
}

template<typename T>
struct Viewport {
	Offset2D<T> offset;
	Extent2D<T> extend;
	Range<T> depth;

	Viewport (T x, T y = 0, T width = 0, T height = 0, T minDepth = 0, T maxDepth = 0)
		: offset (x, y), extend (width, height), depth (minDepth, maxDepth) {
	}
	Viewport (Offset2D<T> offset = Offset2D<T>(), Extent2D<T> extend = Extent2D<T>(), Range<T> depth = Range<T>())
		: offset (offset), extend (extend), depth (depth) {
	}

	Viewport (const Viewport<T>& rhs) {
		memcpy (this, &rhs, sizeof (Viewport<T>));
	}

	Viewport& operator= (const Viewport<T> & rhs) {
		memcpy (this, &rhs, sizeof (Viewport<T>));
		return *this;
	}

	bool operator== (Viewport<T> const& rhs) const {
		return (offset == rhs.offset) && (extend == rhs.extend) && (depth == rhs.depth);
	}
	bool operator!= (Viewport<T> const& rhs) const {
		return !operator== (rhs);
	}
};
