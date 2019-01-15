#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE true
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/ext.hpp>

struct CameraOrientation {
	glm::vec3 look_at;
	glm::vec3 view_vector;
	glm::vec3 up_vector;
	CameraOrientation ( glm::vec3 look_at, glm::vec3 view_vector, glm::vec3 up_vector ) :
		look_at ( look_at ), view_vector ( view_vector ), up_vector ( up_vector ) { }

	void turn ( float pitch, float yaw, float roll = 0.0f ) {
		view_vector = glm::rotate ( glm::rotate ( glm::mat4 ( 1.0f ), yaw, up_vector ), pitch, glm::cross ( up_vector, view_vector ) ) * glm::vec4 ( view_vector, 0.0f );
		up_vector = glm::rotate ( glm::mat4 ( 1.0f ), roll, view_vector ) * glm::vec4 ( up_vector, 0.0f );
	}
	void move ( float forward, float sidewards, float upwards = 0.0f ) {
		glm::vec3 sideward_vec = glm::cross ( view_vector, up_vector );
		glm::vec3 forward_vec = glm::cross ( up_vector, sideward_vec );
		glm::vec3 change_vec = glm::normalize ( forward_vec ) * forward + glm::normalize ( sideward_vec ) * sidewards + glm::normalize ( up_vector ) * upwards;
		look_at += change_vec;
	}
	void move ( glm::vec3 move_vec ) {
		glm::vec3 sideward_vec = glm::cross ( view_vector, up_vector );
		glm::vec3 forward_vec = glm::cross ( up_vector, sideward_vec );
		glm::vec3 change_vec = glm::normalize ( forward_vec ) * move_vec.z + glm::normalize ( sideward_vec ) * move_vec.x + glm::normalize ( up_vector ) * move_vec.y;
		look_at += change_vec;
	}
	void move_forward ( float forward, float sidewards, float upwards ) {
		look_at += view_vector * forward;
	}
	void zoom ( float zoom ) {
		view_vector *= std::pow ( 1.1f, zoom );
		if ( glm::length ( view_vector ) < 1.0f ) view_vector = glm::normalize ( view_vector );
	}
	glm::mat4 w2v_mat() {
		return glm::lookAt ( look_at - view_vector, look_at, up_vector );
	}
	glm::mat4 w2v_rot_mat() {
		return glm::lookAt ( glm::vec3 ( 0.0f, 0.0f, 0.0f ), view_vector, up_vector );
	}
};

struct Camera {
	CameraOrientation orientation;
	float fov, aspect;
	float left, right, top, bottom;
	float near, far;
	bool perspective = true;

	Camera() : orientation ( glm::vec3 ( 0.0f, 0.0f, 0.0f ), glm::vec3 ( 0.0f, 0.0f, -1.0f ), glm::vec3 ( 0.0f, 1.0f, 0.0f ) ),
		fov ( 120.0f ), aspect ( 1.0f ), left ( 1.0f ), right ( 1.0f ), top ( 1.0f ), bottom ( 1.0f ), near ( 1.0f ), far ( 1000.0f ), perspective ( true ) { }

	Camera ( glm::vec3 look_at, glm::vec3 view_vector, glm::vec3 up_vector, float fov, float aspect, float near, float far ) :
		orientation ( look_at, view_vector, up_vector ), fov ( fov ), aspect ( aspect ), left ( 1.0f ), right ( 1.0f ), top ( 1.0f ), bottom ( 1.0f ), near ( near ), far ( far ), perspective ( true ) { }

	Camera ( glm::vec3 look_at, glm::vec3 view_vector, glm::vec3 up_vector, float left, float right, float top, float bottom, float near, float far ) :
		orientation ( look_at, view_vector, up_vector ), fov ( 120.0f ), aspect ( 1.0f ), left ( left ), right ( right ), top ( top ), bottom ( bottom ), near ( near ), far ( far ), perspective ( false ) { }

	glm::mat4 v2s_mat() {
		if ( perspective ) {
			return glm::perspective ( fov, aspect, near, far );
		} else {
			return glm::ortho ( left, right, bottom, top, near, far );
		}
	}

};
inline float interp_float ( float src_f, float dst_f, float factor ) {
	return ( src_f * ( 1.0f - factor ) ) + ( dst_f * factor );
}
inline glm::vec3 interp_pos ( glm::vec3 src_pos, glm::vec3 dst_pos, float factor ) {
	return ( src_pos * ( 1.0f - factor ) ) + ( dst_pos * factor );
}
inline glm::vec3 interp_vec ( glm::vec3 src_vec, glm::vec3 dst_vec, float factor ) {
	glm::vec3 norm_src = glm::normalize ( src_vec );
	glm::vec3 norm_dst = glm::normalize ( dst_vec );
	float len = interp_float ( glm::length ( src_vec ), glm::length ( dst_vec ), factor );
	if ( norm_src == norm_dst ) {
		return norm_src * len;
	} else {
		return norm_src * len * glm::angleAxis ( glm::acos ( glm::dot ( norm_src, norm_dst ) ) * factor, glm::normalize ( glm::cross ( dst_vec, src_vec ) ) );
	}
}
inline CameraOrientation interp_camera_orientation ( CameraOrientation src_orientation, CameraOrientation dst_orientation, float factor ) {
	return CameraOrientation (
	           interp_pos ( src_orientation.look_at, dst_orientation.look_at, factor ),
	           interp_vec ( src_orientation.view_vector, dst_orientation.view_vector, factor ),
	           interp_vec ( src_orientation.up_vector, dst_orientation.up_vector, factor )
	       );
}
