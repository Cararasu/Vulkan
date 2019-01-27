#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Initialization.h"

#include <chrono>
#include <thread>
#include <algorithm>
#include <vector>

#include <render/Window.h>
#include <render/Logger.h>
#include <render/Instance.h>
#include <render/Specialization.h>
#include <render/Timing.h>
#include <render/UTF.h>

#include "Camera.h"
#include "GameState.h"
#include "Vulkan/VMemoryAllocator.h"
#include "World.h"

Logger g_logger ( "main" );

//from http://lukas-polok.cz/tutorial_sphere.htm
float f_rand() {
	return float ( rand() ) / RAND_MAX * 2 - 1;
	// generates random number in range [-1, 1] with uniform distribution
}
float f_rand2() {
	float f = f_rand();
	return f / cos ( f );
}
using namespace std::chrono_literals;

GameState g_state;

#include <render/Queues.h>

//TODO imageCubeArray

//TODO independentBlend

//TODO samplerAnisotropy
//TODO shaderSampledImageArrayDynamicIndexing
//TODO shaderUniformBufferArrayDynamicIndexing

//TODO add bitmask to instances to determine conditional rendering

int main ( int argc, char **argv ) {

	Instance* instance = create_instance ( "Vulkan" );
	instance->initialize ( InstanceOptions() );

	register_shaders ( instance );

	Monitor* primMonitor = instance->get_primary_monitor();

	if ( g_logger.level <= LogLevel::eDebug ) {
		g_logger.log<LogLevel::eDebug> ( "Monitors" );
		for ( Monitor* monitor : instance->monitors ) {
			if ( monitor == primMonitor )
				g_logger.log<LogLevel::eDebug> ( "\tPrimary Monitor | %s %dx%d", monitor->name, monitor->extend.x, monitor->extend.y );
			else
				g_logger.log<LogLevel::eDebug> ( "\t%s %dx%d", monitor->name, monitor->extend.x, monitor->extend.y );
		}
		g_logger.log<LogLevel::eDebug> ( "Devices" );
		for ( Device* device : instance->devices ) {
			g_logger.log<LogLevel::eDebug> ( "\t%s %" PRId32, device->name, device->rating );
		}
	}

	Window* window = instance->create_window();

	window->position() = primMonitor->offset + Extent2D<s32>(50, 50);
	window->showmode() = WindowShowMode::eMaximized;
	window->fullscreen_monitor() = nullptr;
	window->border() = WindowBorder::eNormal;
	window->size() = instance->get_primary_monitor()->extend - Extent2D<s32>(100, 100);
	window->visible() = true;

	window->cursor_mode() = CursorMode::eInvisible;
	window->update();

	Array<Context> sm_contexts = {
		instance->create_context ( camera_context_base_id ),
		instance->create_context ( camera_context_base_id ),
		instance->create_context ( camera_context_base_id )
	};

	World world;

	world.init ( instance );
	world.light_vector = glm::normalize ( glm::vec3 ( -1.0, -1.0, -1.0 ) );

	RenderBundle* bundle = setup_renderbundle ( instance, window, &world, sm_contexts );

	world.xwings.resize ( 2 );
	world.xwings[0].init ( glm::vec3 ( 5.0f, -5.0f, -10.0f ), glm::angleAxis ( glm::pi<float>() * 0.5f, glm::normalize ( glm::vec3 ( 0.3f, -1.0f, 0.0f ) ) ), 6.25f, 10.0f );
	world.xwings[1].init ( glm::vec3 ( 20.0f, -0.0f, -5.0f ), glm::angleAxis ( glm::pi<float>() * 0.5f, glm::normalize ( glm::vec3 ( 0.25f, -1.0f, 0.0f ) ) ), 6.25f, 10.0f );

	world.ties.resize ( 2 );
	world.ties[0].init ( glm::vec3 ( -75.0f, -30.0f, -7.5f ), glm::angleAxis ( glm::pi<float>() * 0.5f, glm::normalize ( glm::vec3 ( 0.27f, -1.0f, 0.0f ) ) ), 4.5f, 10.0f );
	world.ties[1].init ( glm::vec3 ( 0.0f, 100.0f, 0.0f ), glm::angleAxis ( glm::pi<float>() * 0.7f, glm::normalize ( glm::vec3 ( 1.0f, -1.0f, 1.0f ) ) ), 4.5f, 10.0f );

	world.gallofrees.resize ( 1 );
	world.gallofrees[0].init ( glm::vec3 ( 30.0f, -20.0f, -30.0f ), glm::angleAxis ( glm::pi<float>() * 0.5f, glm::vec3 ( 0.0f, -1.0f, 0.0f ) ), 45.0f, 2.0f );

	world.red_shots.resize ( 2 );
	{
		glm::vec3 shot1_translation ( 4.6f, 0.25f, 7.0f );
		glm::vec3 shot2_translation ( -4.6f, -0.4f, 7.0f );
		shot1_translation = glm::rotate ( world.xwings[0].rotation, shot1_translation );
		shot2_translation = glm::rotate ( world.xwings[0].rotation, shot2_translation );
		world.red_shots[0].init ( world.xwings[0].position + shot1_translation, world.xwings[0].rotation, glm::vec3 ( 0.3f, 0.3f, 7.5f ), 100.0f );
		world.red_shots[1].init ( world.xwings[0].position + shot2_translation, world.xwings[0].rotation, glm::vec3 ( 0.3f, 0.3f, 7.5f ), 100.0f );
	}
	world.green_shots.resize ( 2 );
	{
		glm::vec3 shot1_translation ( 0.3f, -0.7f, 40.0f );
		glm::vec3 shot2_translation ( -0.3f, -0.7f, 40.0f );
		shot1_translation = glm::rotate ( world.ties[1].rotation, shot1_translation );
		shot2_translation = glm::rotate ( world.ties[1].rotation, shot2_translation );
		world.green_shots[0].init ( world.ties[1].position + shot1_translation, world.ties[1].rotation, glm::vec3 ( 0.3f, 0.3f, 7.5f ), 100.0f );
		world.green_shots[1].init ( world.ties[1].position + shot2_translation, world.ties[1].rotation, glm::vec3 ( 0.3f, 0.3f, 7.5f ), 100.0f );
	}
	world.billboards.resize ( 0 );

	g_logger.log<LogLevel::eInfo> ( "Starting Main Loop" );

	g_state.current_time = 0.0;

	g_state.camera = Camera ( glm::vec3 ( 0.0f, 0.0f, 0.0f ), glm::vec3 ( 0.0f, -40.0f, -40.0f ), glm::vec3 ( 0.0f, 1.0f, 0.0f ), 120.0f, 1.0f, 1.0f, 10000.0f );
	g_state.init();

	setup_camerapoints ( &g_state );

	bool explosion = false;
	bool shot_despawn = false;

	srand ( time ( NULL ) );

	g_state.timescale = 1.0 / 100.0;
	
	
	g_state.debug_camera = true;
	g_state.timescale = 0.0;

	while ( instance->is_window_open() ) {
		g_state.update_tick();

		g_state.update_camera();

		Timing timer ( &g_logger, "Main-Loop" );

		if ( g_state.current_time >= 0.81 && !explosion ) {
			world.billboards.resize ( 500 );
			for ( size_t i = 0; i < world.billboards.size(); i++ ) {
				float size = f_rand() * 8.0f + 2.0f;
				float speed = f_rand();
				speed *= speed;
				speed *= 300.0f;
				float ttl = f_rand() * 0.5f + 0.5;

				world.billboards[i].init ( world.ties[0].position, glm::vec3 ( f_rand2(), f_rand2(), f_rand2() ), glm::vec2 ( size, size ), speed, ttl, rand() % 32 );
			}
			explosion = true;
		}
		if ( g_state.current_time >= 0.814 && !shot_despawn ) {
			world.red_shots.erase ( world.red_shots.begin() );
			world.ties.erase ( world.ties.begin() );
			shot_despawn = true;
		}
		printf ( "Time %lf\n", g_state.current_time );

		OSEvent event;
		while ( window->eventqueue.pop ( &event ) ) {
			switch ( event.type ) {
			case OSEventType::eButton: {
				bool ispressed = event.button.action != PressAction::eRelease;
				if ( event.button.keycode == KeyCode::eF10 && event.button.action == PressAction::ePress ) {
					window->showmode() = window->showmode() == WindowShowMode::eMaximized ? WindowShowMode::eWindowed : WindowShowMode::eMaximized;
					window->update();
				}
				if ( event.button.keycode != KeyCode::eUnknown ) {
					KeyState& keystate = g_state.basic_keystates[ ( u32 ) event.button.keycode];
					keystate.pressed = ispressed;
					keystate.time_pressed = g_state.current_time;
				}
				if ( event.button.utf8[0] ) {
					KeyState& keystate = g_state.utf32_keystates[utf8_to_utf32 ( event.button.utf8 )];
					keystate.pressed = ispressed;
					keystate.time_pressed = g_state.current_time;
				}
				KeyState& keystate = g_state.keystates[ ( u32 ) event.button.keycode];
				keystate.pressed = ispressed;
				keystate.time_pressed = g_state.current_time;

				if ( event.button.keycode == KeyCode::ePlus || event.button.keycode == KeyCode::eKPAdd ) {
					if ( !g_state.debug_camera ) {
						if ( event.button.action == PressAction::ePress ) {
							g_state.timescale = 1.0 / 10.0;
						} else if ( event.button.action == PressAction::eRelease ) {
							g_state.timescale = 1.0 / 100.0;
						}
					}
				}
				if ( event.button.action == PressAction::ePress && event.button.keycode == KeyCode::eF11 ) {
					if(window->border() == WindowBorder::eNormal) {
						window->position() = primMonitor->offset;
						window->showmode() = WindowShowMode::eWindowed;
						window->border() = WindowBorder::eNone;
						window->size() = primMonitor->extend;
						//window->update();
					} else {
						window->position() = primMonitor->offset + Extent2D<s32>(50, 50);
						window->showmode() = WindowShowMode::eWindowed;
						window->border() = WindowBorder::eNormal;
						window->size() = primMonitor->extend - Extent2D<s32>(100, 100);
						//window->update();
					}
				}
				
				if ( event.button.action == PressAction::ePress && event.button.keycode == KeyCode::eC ) {
					puts ( "Camera:" );
					printf ( "\tPosition: (%f, %f, %f)\n", g_state.camera.orientation.look_at.x, g_state.camera.orientation.look_at.y, g_state.camera.orientation.look_at.z );
					printf ( "\tView-Vec: (%f, %f, %f)\n", g_state.camera.orientation.view_vector.x, g_state.camera.orientation.view_vector.y, g_state.camera.orientation.view_vector.z );
					printf ( "\tUp-Vec: (%f, %f, %f)\n", g_state.camera.orientation.up_vector.x, g_state.camera.orientation.up_vector.y, g_state.camera.orientation.up_vector.z );
				}
				if ( event.button.action == PressAction::ePress && event.button.keycode == KeyCode::eI ) {
					g_state.debug_camera = !g_state.debug_camera;
					if ( g_state.debug_camera ) {
						g_state.timescale = 0.0;
					} else {
						g_state.timescale = 1.0 / 100.0;
					}
				}
			}
			break;
			case OSEventType::eMouse: {
				switch ( event.mouse.action ) {
				case MouseMoveAction::eMoved:
					if ( g_state.basic_keystates[ ( u32 ) KeyCode::eMouseLeft].pressed ) {
						g_state.camera.orientation.turn ( event.mouse.deltay / 1000.0, event.mouse.deltax / 1000.0 );
					}
					break;
				case MouseMoveAction::eEntered:
					break;
				case MouseMoveAction::eLeft:
					break;
				}
			}
			break;
			case OSEventType::eScroll: {
				g_state.camera.orientation.zoom ( -event.scroll.deltay );
			}
			break;
			case OSEventType::eChar: {
				printf ( "eChar\n" );
			}
			break;
			case OSEventType::eWindow: {
				switch ( event.window.action ) {
				case WindowAction::eMoved:
					break;
				case WindowAction::eResized:
					g_state.camera.aspect = ( ( float ) event.window.x ) / ( ( float ) event.window.y );
					break;
				case WindowAction::eIconify:
					break;
				case WindowAction::eMaximize:
					break;
				case WindowAction::eFocused:
					break;
				case WindowAction::eClosed:
					break;
				}
			}
			break;
			}
		}

		float camera_move_factor = glm::length ( g_state.camera.orientation.view_vector ) * g_state.delta_real_time * 0.25f;

		glm::vec3 move_vec ( 0.0f, 0.0f, 0.0f );

		if ( g_state.basic_keystates[ ( u32 ) KeyCode::eA].pressed ) move_vec.x -= camera_move_factor;
		if ( g_state.basic_keystates[ ( u32 ) KeyCode::eD].pressed ) move_vec.x += camera_move_factor;
		if ( g_state.basic_keystates[ ( u32 ) KeyCode::eS].pressed ) move_vec.z -= camera_move_factor;
		if ( g_state.basic_keystates[ ( u32 ) KeyCode::eW].pressed ) move_vec.z += camera_move_factor;

		if ( g_state.basic_keystates[ ( u32 ) KeyCode::eSpace].pressed ) move_vec.y += camera_move_factor;
		if ( g_state.basic_keystates[ ( u32 ) KeyCode::eLCntrl].pressed ) move_vec.y -= camera_move_factor;

		g_state.camera.orientation.move ( move_vec );

		//this should happen internally in a seperate thread
		//or outside in a seperate thread but probably internally is better
		instance->process_events();

		world.world_shard.camera = g_state.camera;
		world.shadow_shard[0].camera = generate_shadowmap_camera ( &g_state.camera, world.light_vector, g_state.camera.near, 100.0f );
		world.shadow_shard[0].camera_near = g_state.camera.near;
		world.shadow_shard[0].camera_far = 100.0f;
		world.shadow_shard[1].camera = generate_shadowmap_camera ( &g_state.camera, world.light_vector, 100.0f, 400.0f );
		world.shadow_shard[1].camera_near = 100.0f;
		world.shadow_shard[1].camera_far = 400.0f;
		world.shadow_shard[2].camera = generate_shadowmap_camera ( &g_state.camera, world.light_vector, 400.0f, 1000.0f );
		world.shadow_shard[2].camera_near = 400.0f;
		world.shadow_shard[2].camera_far = 1500.0f;
		world.update_shards ( g_state.delta_time );

		instance->render_bundles ( {bundle} );
	}
	window->destroy();

	delete bundle;

	destroy_instance ( instance );
	return 0;
}
