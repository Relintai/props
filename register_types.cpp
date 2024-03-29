/*
Copyright (c) 2019-2022 Péter Magyar

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "register_types.h"

#include "core/version.h"

#include "core/config/engine.h"

#include "tiled_wall/tiled_wall.h"
#include "tiled_wall/tiled_wall_data.h"

#include "props/prop_data.h"
#include "props/prop_data_entry.h"
#include "props/prop_data_light.h"
#include "props/prop_data_prop.h"
#include "props/prop_data_scene.h"
#include "props/prop_data_tiled_wall.h"

#if VERSION_MINOR >= 4
#include "props/prop_data_portal.h"
#endif

#include "clutter/ground_clutter.h"
#include "clutter/ground_clutter_foliage.h"

#include "prop_ess_entity.h"
#include "prop_instance.h"
#include "prop_instance_merger.h"

#include "prop_instance_job.h"
#include "prop_instance_prop_job.h"

#include "jobs/prop_mesher_job_step.h"
#include "jobs/prop_texture_job.h"

#include "prop_scene_instance.h"

#include "singleton/prop_cache.h"
#include "singleton/prop_utils.h"

#include "lights/prop_light.h"

#include "./editor/prop_editor_plugin.h"

#include "prop_mesher.h"

#include "material_cache/prop_material_cache.h"

#ifdef TEXTURE_PACKER_PRESENT
#include "material_cache/prop_material_cache_pcm.h"
#endif

static PropUtils *prop_utils = NULL;
static PropCache *prop_texture_cache = NULL;

void initialize_props_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		GDREGISTER_CLASS(TiledWall);
		GDREGISTER_CLASS(TiledWallData);

		GDREGISTER_CLASS(PropLight);

		GDREGISTER_CLASS(PropData);
		GDREGISTER_CLASS(PropDataEntry);
		GDREGISTER_CLASS(PropDataScene);
		GDREGISTER_CLASS(PropDataLight);
		GDREGISTER_CLASS(PropDataProp);
		GDREGISTER_CLASS(PropDataTiledWall);

#if VERSION_MINOR >= 4
		GDREGISTER_CLASS(PropDataPortal);
#endif

		GDREGISTER_CLASS(GroundClutter);
		GDREGISTER_CLASS(GroundClutterFoliage);

		GDREGISTER_CLASS(PropMesher);
		GDREGISTER_CLASS(PropMesherJobStep);

		GDREGISTER_CLASS(PropInstance);
		GDREGISTER_CLASS(PropInstanceMerger);

		GDREGISTER_CLASS(PropESSEntity);

		GDREGISTER_CLASS(PropInstanceJob);
		GDREGISTER_CLASS(PropInstancePropJob);

		GDREGISTER_CLASS(PropTextureJob);

		GDREGISTER_CLASS(PropSceneInstance);

		GDREGISTER_CLASS(PropMaterialCache);

#ifdef TEXTURE_PACKER_PRESENT
		GDREGISTER_CLASS(PropMaterialCachePCM);
#endif

		prop_utils = memnew(PropUtils);
		GDREGISTER_CLASS(PropUtils);
		Engine::get_singleton()->add_singleton(Engine::Singleton("PropUtils", PropUtils::get_singleton()));

		prop_texture_cache = memnew(PropCache);
		GDREGISTER_CLASS(PropCache);
		Engine::get_singleton()->add_singleton(Engine::Singleton("PropCache", PropCache::get_singleton()));

		Ref<PropDataLight> light_processor = Ref<PropDataLight>(memnew(PropDataLight));
		PropUtils::add_processor(light_processor);

		Ref<PropDataProp> prop_processor = Ref<PropDataProp>(memnew(PropDataProp));
		PropUtils::add_processor(prop_processor);

		Ref<PropDataScene> scene_processor = Ref<PropDataScene>(memnew(PropDataScene));
		PropUtils::add_processor(scene_processor);

#if VERSION_MINOR >= 4
		Ref<PropDataPortal> portal_processor = Ref<PropDataPortal>(memnew(PropDataPortal));
		PropUtils::add_processor(portal_processor);
#endif

		Ref<PropDataTiledWall> tiled_wall_processor = Ref<PropDataTiledWall>(memnew(PropDataTiledWall));
		PropUtils::add_processor(tiled_wall_processor);
	}

#ifdef TOOLS_ENABLED
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		EditorPlugins::add_by_type<PropEditorPlugin>();
	}
#endif
}

void uninitialize_props_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		if (prop_utils) {
			memdelete(prop_utils);
		}

		if (prop_texture_cache) {
			memdelete(prop_texture_cache);
		}
	}
}
