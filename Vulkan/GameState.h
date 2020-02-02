#pragma once

#include <chrono>
#include <queue>
#include <render/Header.h>
#include <render/WindowEvent.h>
#include "Camera.h"

struct CameraPoint {
	CameraOrientation target;
	float time;
};

struct GameState {
	KeyState basic_keystates[ ( u32 ) KeyCode::Max];
	Map<u32, KeyState> utf32_keystates;
	Map<u32, KeyState> keystates;

	u64 current_time_ns = 0;
	u64 delta_real_time_ns = 0;

	double timescale = 1.0;

	double current_time = 0.0;
	double current_real_time = 0.0;
	double delta_real_time = 0.0;
	double delta_time = 0.0;

	bool debug_camera = false;

	Camera camera;

	std::chrono::time_point<std::chrono::high_resolution_clock> current;
	std::chrono::time_point<std::chrono::high_resolution_clock> last;

	std::deque<CameraPoint> camera_points;
	
	void init() {
		last = current = std::chrono::high_resolution_clock::now();
	}

	void update_tick();
	void update_camera();
};