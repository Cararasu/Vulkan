
#include "Initialization.h"
#include <render/Specialization.h>
#include "World.h"

void register_shaders ( Instance* instance ) {

	//preload-shaders
	instance->resource_manager()->load_shader ( ShaderType::eVertex, "vert_quad_shader", "shader/quad.vert.sprv" );
	instance->resource_manager()->load_shader ( ShaderType::eFragment, "frag_quad_shader", "shader/quad.frag.sprv" );

	instance->resource_manager()->load_shader ( ShaderType::eVertex, "vert_textured_shader", "shader/textured.vert.sprv" );
	instance->resource_manager()->load_shader ( ShaderType::eFragment, "frag_textured_shader", "shader/textured.frag.sprv" );

	instance->resource_manager()->load_shader ( ShaderType::eVertex, "vert_flat_shader", "shader/flat.vert.sprv" );
	instance->resource_manager()->load_shader ( ShaderType::eFragment, "frag_flat_shader", "shader/flat.frag.sprv" );

	instance->resource_manager()->load_shader ( ShaderType::eVertex, "vert_skybox_shader", "shader/skybox.vert.sprv" );
	instance->resource_manager()->load_shader ( ShaderType::eFragment, "frag_skybox_shader", "shader/skybox.frag.sprv" );

	instance->resource_manager()->load_shader ( ShaderType::eVertex, "vert_shot_shader", "shader/shot.vert.sprv" );
	instance->resource_manager()->load_shader ( ShaderType::eFragment, "frag_shot_shader", "shader/shot.frag.sprv" );

	instance->resource_manager()->load_shader ( ShaderType::eVertex, "vert_locallight_shader", "shader/locallight.vert.sprv" );
	instance->resource_manager()->load_shader ( ShaderType::eFragment, "frag_locallight_shader", "shader/locallight.frag.sprv" );

	instance->resource_manager()->load_shader ( ShaderType::eVertex, "vert_billboard_shader", "shader/billboard.vert.sprv" );
	instance->resource_manager()->load_shader ( ShaderType::eFragment, "frag_billboard_shader", "shader/billboard.frag.sprv" );

	instance->resource_manager()->load_shader ( ShaderType::eVertex, "model_passthrough_shader", "shader/model_passthrough.vert.sprv" );
	instance->resource_manager()->load_shader ( ShaderType::eVertex, "passthrough_shader", "shader/passthrough.vert.sprv" );

	instance->resource_manager()->load_shader ( ShaderType::eFragment, "dirlight_shader", "shader/dirlight.frag.sprv" );
	instance->resource_manager()->load_shader ( ShaderType::eFragment, "lightless_shader", "shader/lightless.frag.sprv" );
	instance->resource_manager()->load_shader ( ShaderType::eFragment, "brightness_shader", "shader/brightness.frag.sprv" );
	instance->resource_manager()->load_shader ( ShaderType::eFragment, "hbloom_shader", "shader/hbloom.frag.sprv" );
	instance->resource_manager()->load_shader ( ShaderType::eFragment, "vbloom_shader", "shader/vbloom.frag.sprv" );
	instance->resource_manager()->load_shader ( ShaderType::eFragment, "hdr_shader", "shader/hdr.frag.sprv" );
	instance->resource_manager()->load_shader ( ShaderType::eFragment, "composition_shader", "shader/composition.frag.sprv" );
}
RenderBundle* setup_renderbundle ( Instance* instance, Window* window, World* world, Array<Context>& shadowmap_cameras ) {

	ResourceManager* resource_manager = instance->resource_manager();
	Image* windowimage = window->backed_image ( 0 );

	RenderBundle* bundle = instance->create_main_bundle ( );

	bundle->set_window_dependency ( window );

	u32 renderstage_index = 0;

	Image* shadowmaps = resource_manager->create_texture ( 2048, 2048, 0, 3, 1, ImageFormat::eD32F );
	for ( int i = 0; i < shadowmap_cameras.size; i++ ) {
		Context& sm_context = shadowmap_cameras[i];
		world->shadow_shard[i].cgroup->set_context ( sm_context );
		bundle->get_renderstage ( renderstage_index )->set_renderimage ( 0, shadowmaps, 0, i );
		bundle->get_renderstage ( renderstage_index )->set_contextgroup ( world->shadow_shard[i].cgroup );
		bundle->get_renderstage ( renderstage_index )->set_instancegroup ( world->world_shard.igroup );
		renderstage_index++;
	}
	Sampler* shadowmap_sampler = resource_manager->create_sampler (
			FilterType::eLinear, FilterType::eLinear, FilterType::eNearest,
			EdgeHandling::eClamp, EdgeHandling::eClamp, EdgeHandling::eClamp,
			0.0f, {0.0f, 0.0f}, 0.0f, DepthComparison::eLEquals );
	world->world_shard.shadowmap_context = instance->create_context ( shadowmap_context_base_id );
	instance->update_context_image_sampler ( world->world_shard.shadowmap_context, 0, 0, shadowmaps->create_use ( ImagePart::eDepth, {0, 1}, {0, 3} ), shadowmap_sampler );
	world->world_shard.cgroup->set_context(world->world_shard.shadowmap_context);

	Image* diffuse = resource_manager->create_dependant_image ( windowimage, ImageFormat::e4Unorm8, 1, 1.0f );
	bundle->get_renderstage ( renderstage_index )->set_renderimage ( 0, diffuse ); //ambient + intensity
	bundle->get_renderstage ( renderstage_index )->set_renderimage ( 1, resource_manager->create_dependant_image ( windowimage, ImageFormat::e2F16, 1, 1.0f ) ); //normals
	bundle->get_renderstage ( renderstage_index )->set_renderimage ( 2, resource_manager->create_dependant_image ( windowimage, ImageFormat::e4Unorm8, 1, 1.0f ) ); //specular power + intensity + ??? + ???
	Image* lightaccumulation = resource_manager->create_dependant_image ( windowimage, ImageFormat::e4F16, 1, 1.0f );
	bundle->get_renderstage ( renderstage_index )->set_renderimage ( 3, lightaccumulation ); //light-accumulation + specularintensity
	bundle->get_renderstage ( renderstage_index )->set_renderimage ( 4, resource_manager->create_dependant_image ( windowimage, ImageFormat::eD24Unorm_St8U, 1, 1.0f ) );
	bundle->get_renderstage ( renderstage_index )->set_contextgroup ( world->world_shard.cgroup );
	bundle->get_renderstage ( renderstage_index )->set_instancegroup ( world->world_shard.igroup );

	Image* bloomimage1 = resource_manager->create_dependant_image ( windowimage, ImageFormat::e4F16, 6, 0.5f );//resource_manager->create_texture ( 1024, 1024, 0, 1, 6, ImageFormat::e4F16);
	Image* bloomimage2 = resource_manager->create_dependant_image ( windowimage, ImageFormat::e4F16, 6, 0.5f );//resource_manager->create_texture ( 1024, 1024, 0, 1, 6, ImageFormat::e4F16);

	{
		renderstage_index++;
		Sampler* downscale_sampler = resource_manager->create_sampler (
		                                 FilterType::eLinear, FilterType::eLinear, FilterType::eLinear,
		                                 EdgeHandling::eMirror, EdgeHandling::eMirror, EdgeHandling::eMirror,
		                                 0.0f, {0.0f, 0.0f}, 0.0f, DepthComparison::eNone );
		ContextGroup* brightness_contextgroup = instance->create_contextgroup();
		Context brightness_context = instance->create_context ( postproc_context_base_id );

		instance->update_context_image_sampler ( brightness_context, 0, 0, lightaccumulation->create_use ( ImagePart::eColor, {0, 1}, {0, 1} ), downscale_sampler );

		brightness_contextgroup->set_context ( brightness_context );
		//downscale + brightness low-pass filter
		bundle->get_renderstage ( renderstage_index )->set_renderimage ( 0, bloomimage1, 0 );
		bundle->get_renderstage ( renderstage_index )->set_contextgroup ( brightness_contextgroup );
		bundle->get_renderstage ( renderstage_index )->set_instancegroup ( world->world_shard.igroup );

		renderstage_index++;

		bundle->get_renderstage ( renderstage_index )->set_renderimage ( 0, bloomimage1, 0 );

		Sampler* bloom_sampler = resource_manager->create_sampler (
		                             FilterType::eNearest, FilterType::eNearest, FilterType::eNearest,
		                             EdgeHandling::eMirror, EdgeHandling::eMirror, EdgeHandling::eMirror,
		                             0.0f, {0.0f, 5.0f}, 0.0f, DepthComparison::eNone );
		ContextGroup* vbloom_contextgroup = instance->create_contextgroup();

		Context vbloom_context = instance->create_context ( postproc_context_base_id );
		vbloom_contextgroup->set_context ( vbloom_context );

		instance->update_context_image_sampler ( vbloom_context, 0, 0, bloomimage1->create_use ( ImagePart::eColor, {1, 6}, {0, 1} ), bloom_sampler );

		ContextGroup* hbloom_contextgroup = instance->create_contextgroup();

		Context hbloom_context = instance->create_context ( postproc_context_base_id );
		hbloom_contextgroup->set_context ( hbloom_context );

		instance->update_context_image_sampler ( hbloom_context, 0, 0, bloomimage2->create_use ( ImagePart::eColor, {1, 6}, {0, 1} ), bloom_sampler );

		renderstage_index++;
		//vertical bloom
		bundle->get_renderstage ( renderstage_index )->set_renderimage ( 0, bloomimage2, 1 );
		bundle->get_renderstage ( renderstage_index )->set_contextgroup ( vbloom_contextgroup );
		bundle->get_renderstage ( renderstage_index )->set_instancegroup ( world->world_shard.igroup );

		renderstage_index++;

		//horizontal bloom
		bundle->get_renderstage ( renderstage_index )->set_renderimage ( 0, bloomimage1, 1 );
		bundle->get_renderstage ( renderstage_index )->set_contextgroup ( hbloom_contextgroup );
		bundle->get_renderstage ( renderstage_index )->set_instancegroup ( world->world_shard.igroup );

		ContextGroup* composition_contextgroup = instance->create_contextgroup();

		Sampler* composition_bloom_sampler = resource_manager->create_sampler (
		        FilterType::eLinear, FilterType::eLinear, FilterType::eLinear,
		        EdgeHandling::eMirror, EdgeHandling::eMirror, EdgeHandling::eMirror,
		        0.0f, {0.0f, 5.0f}, 0.0f, DepthComparison::eNone );
		Context composition_context = instance->create_context ( postproc_context_base_id );
		composition_contextgroup->set_context ( composition_context );

		instance->update_context_image_sampler ( composition_context, 0, 0, bloomimage1->create_use ( ImagePart::eColor, {1, 6}, {0, 1} ), composition_bloom_sampler );

		renderstage_index++;
		//final bloom composition
		bundle->get_renderstage ( renderstage_index )->set_renderimage ( 0, lightaccumulation );
		bundle->get_renderstage ( renderstage_index )->set_contextgroup ( composition_contextgroup );
		bundle->get_renderstage ( renderstage_index )->set_instancegroup ( world->world_shard.igroup );
	}
	renderstage_index++;
	{
		ContextGroup* hdr_contextgroup = instance->create_contextgroup();
		
		Sampler* composition_hdr_sampler = resource_manager->create_sampler (
		        FilterType::eLinear, FilterType::eLinear, FilterType::eLinear,
		        EdgeHandling::eMirror, EdgeHandling::eMirror, EdgeHandling::eMirror,
		        0.0f, {0.0f, 5.0f}, 0.0f, DepthComparison::eNone );
		Context hdr_context = instance->create_context ( postproc_context_base_id );
		hdr_contextgroup->set_context ( hdr_context );

		instance->update_context_image_sampler ( hdr_context, 0, 0, lightaccumulation->create_use ( ImagePart::eColor, {0, 1}, {0, 1} ), composition_hdr_sampler );
		
		//hdr composition
		bundle->get_renderstage ( renderstage_index )->set_renderimage ( 0, diffuse );
		bundle->get_renderstage ( renderstage_index )->set_contextgroup ( hdr_contextgroup );
		bundle->get_renderstage ( renderstage_index )->set_instancegroup ( world->world_shard.igroup );
	}
	renderstage_index++;
	bundle->get_renderstage ( renderstage_index )->set_renderimage ( 0, diffuse );
	bundle->get_renderstage ( renderstage_index )->set_renderwindow ( 0, window );

	return bundle;
}
void setup_camerapoints ( GameState* gamestate ) {

	gamestate->camera_points.push_back ( {
		CameraOrientation (
		    glm::vec3 ( -12.0, -14.0, -7.0 ),
		    glm::vec3 ( -50.0, -25.0, -2.0 ),
		    glm::vec3 ( 0.0f, 1.0f, 0.0f )
		),
		0.05f
	} );
	gamestate->camera_points.push_back ( {
		CameraOrientation (
		    glm::vec3 ( -15.0, -12.0, -10.0 ),
		    glm::vec3 ( -6.0, -6.0, 16.0 ),
		    glm::vec3 ( 0.0f, 1.0f, 0.0f )
		),
		0.05f
	} ); //0.1
	gamestate->camera_points.push_back ( {
		CameraOrientation (
		    glm::vec3 ( -24.0, -14.0, -10.0 ),
		    glm::vec3 ( 18.0, 4.0, 0.0 ),
		    glm::vec3 ( 0.0f, 1.0f, 0.0f )
		),
		0.075f
	} );
	gamestate->camera_points.push_back ( {
		CameraOrientation (
		    glm::vec3 ( -18.0, -0.0, 10.0 ),
		    glm::vec3 ( 24.0, -20.0, -41.0 ),
		    glm::vec3 ( 0.0f, 1.0f, 0.0f )
		),
		0.075f
	} ); //0.25
	gamestate->camera_points.push_back ( {
		CameraOrientation (
		    glm::vec3 ( -3.0, -8.0, -5.0 ),
		    glm::vec3 ( -40.0, -30.0, -10.0 ),
		    glm::vec3 ( 0.0f, 1.0f, 0.0f )
		),
		0.1f
	} ); //0.35
	gamestate->camera_points.push_back ( {
		CameraOrientation (
		    glm::vec3 ( 65.0, 0.0, -55.0 ),
		    glm::vec3 ( -90.0, 55.0, 100.0 ),
		    glm::vec3 ( 0.0f, 1.0f, 0.0f )
		),
		0.1f
	} ); //0.45
	gamestate->camera_points.push_back ( {
		CameraOrientation (
		    glm::vec3 ( -30.0, -15.0, 4.0 ),
		    glm::vec3 ( 100.0, 20.0, 24.0 ),
		    glm::vec3 ( 0.0f, 1.0f, 0.0f )
		),
		0.1f
	} ); //0.55
	//circle around the green shots
	gamestate->camera_points.push_back ( {
		CameraOrientation (
		    glm::vec3 ( 5.0, -4.0, -10.0 ),
		    glm::vec3 ( 6.0, -10.0, -30.0 ),
		    glm::vec3 ( 0.0f, 1.0f, 0.0f )
		),
		0.05f
	} ); //0.6
	gamestate->camera_points.push_back ( {
		CameraOrientation (
		    glm::vec3 ( 5.0, -4.0, -10.0 ),
		    glm::vec3 ( -25.0, -18.0, -2.5 ),
		    glm::vec3 ( 0.0f, 1.0f, 0.0f )
		),
		0.05f
	} ); //0.65
	gamestate->camera_points.push_back ( {
		CameraOrientation (
		    glm::vec3 ( 5.0, -4.0, -10.0 ),
		    glm::vec3 ( -10.0, -20.0, 24.0 ),
		    glm::vec3 ( 0.0f, 1.0f, 0.0f )
		),
		0.05f
	} ); //0.7
	//move to the tie which is about to get shot
	gamestate->camera_points.push_back ( {
		CameraOrientation (
		    glm::vec3 ( -80.0, -35.0, -5.0 ),
		    glm::vec3 ( -40.0, -25.0, 15.0 ),
		    glm::vec3 ( 0.0f, 1.0f, 0.0f )
		),
		0.07f
	} ); //0.77
	gamestate->camera_points.push_back ( {
		CameraOrientation (
		    glm::vec3 ( -100.0, -45.0, -0.0 ),
		    glm::vec3 ( -30.0, -20.0, 10.0 ),
		    glm::vec3 ( 0.0f, 1.0f, 0.0f )
		),
		0.04f
	} ); //0.81
	//zoom back during the explosion
	gamestate->camera_points.push_back ( {
		CameraOrientation (
		    glm::vec3 ( 25.0, 5.0, -20.0 ),
		    glm::vec3 ( -250.0, -225.0, 90.0 ),
		    glm::vec3 ( 0.0f, 1.0f, 0.0f )
		),
		0.01f
	} ); //0.815
	gamestate->camera_points.push_back ( {
		CameraOrientation (
		    glm::vec3 ( 25.0, 5.0, -18.0 ),
		    glm::vec3 ( -20.0, -200.0, -300.0 ),
		    glm::vec3 ( 0.0f, 1.0f, 0.0f )
		),
		0.09f
	} ); //0.9
	gamestate->camera_points.push_back ( {
		CameraOrientation (
		    glm::vec3 ( -16.0, -60.0, -32.0 ),
		    glm::vec3 ( 320.0, -70.0, -110.0 ),
		    glm::vec3 ( 0.0f, 1.0f, 0.0f )
		),
		0.1f
	} ); //1.0
	gamestate->camera_points.push_back ( {
		CameraOrientation (
		    glm::vec3 ( -40.0, -60.0, -3.0 ),
		    glm::vec3 ( 200.0, -128.0, 255.0 ),
		    glm::vec3 ( 0.0f, 1.0f, 0.0f )
		),
		0.1f
	} ); //1.1
	gamestate->camera_points.push_back ( {
		CameraOrientation (
		    glm::vec3 ( -80.0, -32.0, -7.5 ),
		    glm::vec3 ( -80.0, -55.0, 0.0 ),
		    glm::vec3 ( 0.0f, 1.0f, 0.0f )
		),
		0.1f
	} ); //1.2
	gamestate->camera_points.push_back ( {
		CameraOrientation (
		    glm::vec3 ( -80.0, -32.0, -7.5 ),
		    glm::vec3 ( 20.0, -40.0, -90.0 ),
		    glm::vec3 ( 0.0f, 1.0f, 0.0f )
		),
		0.1f
	} ); //1.3
	gamestate->camera_points.push_back ( {
		CameraOrientation (
		    glm::vec3 ( -80.0, -32.0, -7.5 ),
		    glm::vec3 ( 40.0, -5.0, 90.0 ),
		    glm::vec3 ( 0.0f, 1.0f, 0.0f )
		),
		0.1f
	} ); //1.4
	gamestate->camera_points.push_back ( {
		CameraOrientation (
		    glm::vec3 ( -40.0, -32.0, -25.0 ),
		    glm::vec3 ( -80.0, -40.0, 40.0 ),
		    glm::vec3 ( 0.0f, 1.0f, 0.0f )
		),
		0.1f
	} ); //1.5
	gamestate->camera_points.push_back ( {
		CameraOrientation (
		    glm::vec3 ( 10.0, -8.0, 20.0 ),
		    glm::vec3 ( -60.0, -45.0, -110.0 ),
		    glm::vec3 ( 0.0f, 1.0f, 0.0f )
		),
		0.1f
	} ); //1.6
}
