#pragma once

#include "VHeader.h"
#include <render/Resources.h>

struct VInstance;


struct VSampler : public Sampler {
	VInstance* v_instance;
	vk::Sampler sampler;
	
	VSampler(VInstance* instance, FilterType magnification, FilterType minification, FilterType mipmapping, EdgeHandling u, EdgeHandling v, EdgeHandling w, float lodbias, Range<float> lodrange, float anismax, DepthComparison comp);
	virtual ~VSampler();
};
