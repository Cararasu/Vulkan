#pragma once
#include "Header.h"

struct Extend2D {
	union{
		float width;
		float x;
		float min;
	};
	union{
		float height;
		float y;
		float max;
	};

	Extend2D (float width_ = 0, float height_ = 0)
		: width (width_), height (height_) {
	}

	Extend2D (Extend2D const & rhs) {
		memcpy (this, &rhs, sizeof (Extend2D));
	}

	Extend2D& operator= (Extend2D const & rhs) {
		memcpy (this, &rhs, sizeof (Extend2D));
		return *this;
	}

	bool operator== (Extend2D const& rhs) const {
		return (width == rhs.width) && (height == rhs.height);
	}
	bool operator!= (Extend2D const& rhs) const {
		return !operator== (rhs);
	}
};
typedef Extend2D Offset2D;
typedef Extend2D Range;

struct Viewport {
	Offset2D offset;
	Extend2D extend;
	Range depth;

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

