#include "GameState.h"


void GameState::update_tick() {
	last = current;
	current = std::chrono::high_resolution_clock::now();
	std::chrono::nanoseconds ns = std::chrono::duration_cast<std::chrono::nanoseconds> ( current - last );
	current_time_ns = current.time_since_epoch().count();
	delta_real_time_ns = ns.count();

	delta_real_time = static_cast<double> ( ns.count() ) / 1000000000.0;
	delta_time = delta_real_time * timescale;
	current_real_time += delta_real_time;
	current_time += delta_time;
}

void GameState::update_camera() {
	if ( !debug_camera ) {
		float time_remaining = delta_time;
		while ( !camera_points.empty() ) {
			CameraPoint& point = camera_points.front();
			if ( point.time < time_remaining ) {
				camera.orientation = point.target;
				time_remaining -= point.time;
				camera_points.pop_front();
			} else {
				camera.orientation = interp_camera_orientation ( camera.orientation, point.target, time_remaining / point.time );
				point.time -= time_remaining;
				break;
			}
		}
	}
}