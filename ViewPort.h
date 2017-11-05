#pragma once

#include <inttypes.h>
#include <VHeader.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>


template<typename T>
struct PerspectiveViewPort {
	glm::tvec3<T> m_viewvector;
	glm::tvec3<T> m_focalpoint;
	glm::tvec3<T> m_upvector;

	T m_distance;

	T m_fov;
	T m_aspect;
	T m_near;
	T m_far;

	PerspectiveViewPort() : m_fov (M_PI * 3 / 4), m_aspect (1.0), m_near (1.0), m_far (2.0) {}
	~PerspectiveViewPort() {}

	glm::tmat4x4<T> createWorldToScreenSpaceMatrix() {
		
		m_viewvector = glm::normalize(m_viewvector);
		
		glm::mat4 projectionMatrix = glm::perspective(m_fov, m_aspect, m_near, m_far);
		
		glm::mat4 viewMatrix = glm::lookAt(m_focalpoint + (m_viewvector * m_distance), m_focalpoint, m_upvector);
		
		return projectionMatrix * viewMatrix;

	}
	
	//https://gamedev.stackexchange.com/questions/15070/orienting-a-model-to-face-a-target
	void lookAt (glm::tvec3<T> position, glm::tvec3<T> target, glm::tvec3<T> up) {
		m_viewvector = glm::normalize(position - target);
		m_focalpoint = target;
		m_upvector = up;
	}

};
template<typename T>
class OrthogonalViewPort {
	T m_near   = 1.0;
	T m_far    = 2.0;
	T width  = 1.0;
	T height = 1.0;
	T startwidth = -0.5;
	T startheight = -0.5;

	virtual glm::tmat4x4<T> createWorldToScreenSpaceMatrix() {
		return glm::tmat4x4<T>();
	}


};
