/*
Copyright (c) 2019-2021 Péter Magyar

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

#include "prop_instance_prop_job.h"

#include "prop_instance.h"
#include "prop_mesher.h"

#ifdef MESH_DATA_RESOURCE_PRESENT
#include "../mesh_data_resource/mesh_data_resource.h"
#endif

#ifdef MESH_UTILS_PRESENT
#include "../mesh_utils/fast_quadratic_mesh_simplifier.h"
#endif

#if TEXTURE_PACKER_PRESENT
#include "../texture_packer/texture_packer.h"
#endif

#if MESH_DATA_RESOURCE_PRESENT
//define PROPS_PRESENT, so things compile. That module's scsub will define this too while compiling, 
//but not when included from here.
#define PROPS_PRESENT 1
#include "../mesh_data_resource/props/prop_data_mesh_data.h"
#endif 

#if TEXTURE_PACKER_PRESENT
Ref<TexturePacker> PropInstancePropJob::get_texture_packer() {
	return _texture_packer;
}
void PropInstancePropJob::set_texture_packer(const Ref<TexturePacker> &packer) {
	_texture_packer = packer;
}
#endif

PropInstance *PropInstancePropJob::get_prop_instace() {
	return _prop_instace;
}
void PropInstancePropJob::set_prop_instace(PropInstance *prop) {
	_prop_instace = prop;
}
void PropInstancePropJob::set_prop_instace_bind(Node *prop) {
	set_prop_instace(Object::cast_to<PropInstance>(prop));
}

Ref<PropMesher> PropInstancePropJob::get_prop_mesher() const {
	return _prop_mesher;
}
void PropInstancePropJob::set_prop_mesher(const Ref<PropMesher> &mesher) {
	_prop_mesher = mesher;
}

#if MESH_DATA_RESOURCE_PRESENT
void PropInstancePropJob::add_mesh(const Ref<PropDataMeshData> &mesh_data, const Transform &base_transform) {
	PMDREntry e;
	e.mesh_data = mesh_data;
	e.base_transform = base_transform;

	_prop_mesh_datas.push_back(e);
}

void PropInstancePropJob::clear_meshes() {
	_prop_mesh_datas.clear();
}
#endif

void PropInstancePropJob::phase_physics_process() {
	/*
	Ref<TerraChunkDefault> chunk = _chunk;

	//TODO this should only update the differences
	for (int i = 0; i < chunk->collider_get_count(); ++i) {
		PhysicsServer::get_singleton()->free(chunk->collider_get_body(i));
	}

	chunk->colliders_clear();

#ifdef MESH_DATA_RESOURCE_PRESENT
	for (int i = 0; i < chunk->mesh_data_resource_get_count(); ++i) {
		Ref<MeshDataResource> mdr = chunk->mesh_data_resource_get(i);

		for (int j = 0; j < mdr->get_collision_shape_count(); ++j) {
			Ref<Shape> shape = mdr->get_collision_shape(j);
			Transform offset = mdr->get_collision_shape_offset(j);

			if (!shape.is_valid()) {
				continue;
			}

			RID body = PhysicsServer::get_singleton()->body_create(PhysicsServer::BODY_MODE_STATIC);

			Transform transform = chunk->mesh_data_resource_get_transform(i);
			transform *= offset;

			PhysicsServer::get_singleton()->body_add_shape(body, shape->get_rid());

			//TODO store the layer mask somewhere
			PhysicsServer::get_singleton()->body_set_collision_layer(body, 1);
			PhysicsServer::get_singleton()->body_set_collision_mask(body, 1);

			if (chunk->get_voxel_world()->is_inside_tree() && chunk->get_voxel_world()->is_inside_world()) {
				Ref<World> world = chunk->get_voxel_world()->GET_WORLD();

				if (world.is_valid() && world->get_space() != RID()) {
					PhysicsServer::get_singleton()->body_set_space(body, world->get_space());
				}
			}

			PhysicsServer::get_singleton()->body_set_state(body, PhysicsServer::BODY_STATE_TRANSFORM, chunk->get_transform() * transform);

			chunk->collider_add(transform, shape, shape->get_rid(), body);
		}
	}
#endif

#if TOOLS_ENABLED
	if (SceneTree::get_singleton()->is_debugging_collisions_hint() && chunk->collider_get_count() > 0) {
		chunk->draw_debug_mdr_colliders();
	}
#endif

	set_build_phase_type(BUILD_PHASE_TYPE_NORMAL);
	next_phase();
}

void PropInstancePropJob::phase_prop() {
#ifdef MESH_DATA_RESOURCE_PRESENT
	Ref<TerraChunkDefault> chunk = _chunk;

	if (!get_prop_mesher().is_valid()) {
		set_complete(true); //So threadpool knows it's done
		next_job();
		return;
	}

	if (should_do()) {
		if (chunk->mesh_data_resource_get_count() == 0) {
			set_complete(true); //So threadpool knows it's done
			next_job();
			return;
		}

		for (int i = 0; i < chunk->mesh_data_resource_get_count(); ++i) {
			if (chunk->mesh_data_resource_get_is_inside(i)) {
				get_prop_mesher()->add_mesh_data_resource_transform(chunk->mesh_data_resource_get(i), chunk->mesh_data_resource_get_transform(i), chunk->mesh_data_resource_get_uv_rect(i));
			}
		}

		if (get_prop_mesher()->get_vertex_count() == 0) {
			//reset_stages();

			set_complete(true); //So threadpool knows it's done
			next_job();
			return;
		}

		if (should_return()) {
			return;
		}
	}

	if (should_do()) {
		if ((chunk->get_build_flags() & TerraChunkDefault::BUILD_FLAG_USE_LIGHTING) != 0) {
			get_prop_mesher()->bake_colors(_chunk);
		}

		if (should_return()) {
			return;
		}
	}

	if (should_do()) {
		if ((chunk->get_build_flags() & TerraChunkDefault::BUILD_FLAG_USE_LIGHTING) != 0) {
			TerraWorldDefault *world = Object::cast_to<TerraWorldDefault>(chunk->get_voxel_world());

			if (world) {
				for (int i = 0; i < chunk->mesh_data_resource_get_count(); ++i) {
					if (!chunk->mesh_data_resource_get_is_inside(i)) {
						Ref<MeshDataResource> mdr = chunk->mesh_data_resource_get(i);

						ERR_CONTINUE(!mdr.is_valid());

						Transform trf = chunk->mesh_data_resource_get_transform(i);

						Array arr = mdr->get_array();

						if (arr.size() <= Mesh::ARRAY_VERTEX) {
							continue;
						}

						PoolVector3Array varr = arr[Mesh::ARRAY_VERTEX];

						if (varr.size() == 0) {
							continue;
						}

						PoolColorArray carr = world->get_vertex_colors(trf, varr);

						get_prop_mesher()->add_mesh_data_resource_transform_colored(mdr, trf, carr, chunk->mesh_data_resource_get_uv_rect(i));
					}
				}
			}
		}

		if (should_return()) {
			return;
		}
	}

	if (get_prop_mesher()->get_vertex_count() != 0) {
		if (should_do()) {
			temp_mesh_arr = get_prop_mesher()->build_mesh();

			if (should_return()) {
				return;
			}
		}

		RID mesh_rid = chunk->mesh_rid_get_index(TerraChunkDefault::MESH_INDEX_PROP, TerraChunkDefault::MESH_TYPE_INDEX_MESH, 0);

		if (should_do()) {
			if (mesh_rid == RID()) {
				if ((chunk->get_build_flags() & TerraChunkDefault::BUILD_FLAG_CREATE_LODS) != 0)
					chunk->meshes_create(TerraChunkDefault::MESH_INDEX_PROP, chunk->get_lod_num() + 1);
				else
					chunk->meshes_create(TerraChunkDefault::MESH_INDEX_PROP, 1);

				mesh_rid = chunk->mesh_rid_get_index(TerraChunkDefault::MESH_INDEX_PROP, TerraChunkDefault::MESH_TYPE_INDEX_MESH, 0);
			}

			if (VS::get_singleton()->mesh_get_surface_count(mesh_rid) > 0)
#if !GODOT4
				VS::get_singleton()->mesh_remove_surface(mesh_rid, 0);
#else
				VS::get_singleton()->mesh_clear(mesh_rid);
#endif

			if (should_return()) {
				return;
			}
		}

		if (should_do()) {

			VS::get_singleton()->mesh_add_surface_from_arrays(mesh_rid, VisualServer::PRIMITIVE_TRIANGLES, temp_mesh_arr);

			if (chunk->get_library()->prop_material_get(0).is_valid())
				VS::get_singleton()->mesh_surface_set_material(mesh_rid, 0, chunk->get_library()->prop_material_get(0)->get_rid());

			if (should_return()) {
				return;
			}
		}

		if ((chunk->get_build_flags() & TerraChunkDefault::BUILD_FLAG_CREATE_LODS) != 0) {
			if (should_do()) {

				if (chunk->get_lod_num() >= 1) {
					//for lod 1 just remove uv2
					temp_mesh_arr[VisualServer::ARRAY_TEX_UV2] = Variant();

					VisualServer::get_singleton()->mesh_add_surface_from_arrays(chunk->mesh_rid_get_index(TerraChunkDefault::MESH_INDEX_PROP, TerraChunkDefault::MESH_TYPE_INDEX_MESH, 1), VisualServer::PRIMITIVE_TRIANGLES, temp_mesh_arr);

					if (chunk->get_library()->prop_material_get(1).is_valid())
						VisualServer::get_singleton()->mesh_surface_set_material(chunk->mesh_rid_get_index(TerraChunkDefault::MESH_INDEX_PROP, TerraChunkDefault::MESH_TYPE_INDEX_MESH, 1), 0, chunk->get_library()->prop_material_get(1)->get_rid());
				}

				if (should_return()) {
					return;
				}
			}

			if (should_do()) {
				if (chunk->get_lod_num() >= 2) {
					Array temp_mesh_arr2 = merge_mesh_array(temp_mesh_arr);
					temp_mesh_arr = temp_mesh_arr2;

					VisualServer::get_singleton()->mesh_add_surface_from_arrays(chunk->mesh_rid_get_index(TerraChunkDefault::MESH_INDEX_PROP, TerraChunkDefault::MESH_TYPE_INDEX_MESH, 2), VisualServer::PRIMITIVE_TRIANGLES, temp_mesh_arr2);

					if (chunk->get_library()->prop_material_get(2).is_valid())
						VisualServer::get_singleton()->mesh_surface_set_material(chunk->mesh_rid_get_index(TerraChunkDefault::MESH_INDEX_PROP, TerraChunkDefault::MESH_TYPE_INDEX_MESH, 2), 0, chunk->get_library()->prop_material_get(2)->get_rid());
				}
				if (should_return()) {
					return;
				}
			}

			//	if (should_do()) {
			if (chunk->get_lod_num() >= 3) {
				Ref<ShaderMaterial> mat = chunk->get_library()->prop_material_get(0);
				Ref<SpatialMaterial> spmat = chunk->get_library()->prop_material_get(0);
				Ref<Texture> tex;

				if (mat.is_valid()) {
					tex = mat->get_shader_param("texture_albedo");
				} else if (spmat.is_valid()) {
					tex = spmat->get_texture(SpatialMaterial::TEXTURE_ALBEDO);
				}

				if (tex.is_valid()) {
					temp_mesh_arr = bake_mesh_array_uv(temp_mesh_arr, tex);
					temp_mesh_arr[VisualServer::ARRAY_TEX_UV] = Variant();

					VisualServer::get_singleton()->mesh_add_surface_from_arrays(chunk->mesh_rid_get_index(TerraChunkDefault::MESH_INDEX_PROP, TerraChunkDefault::MESH_TYPE_INDEX_MESH, 3), VisualServer::PRIMITIVE_TRIANGLES, temp_mesh_arr);

					if (chunk->get_library()->prop_material_get(3).is_valid())
						VisualServer::get_singleton()->mesh_surface_set_material(chunk->mesh_rid_get_index(TerraChunkDefault::MESH_INDEX_PROP, TerraChunkDefault::MESH_TYPE_INDEX_MESH, 3), 0, chunk->get_library()->prop_material_get(3)->get_rid());
				}
			}

#ifdef MESH_UTILS_PRESENT
			if (should_do()) {
				if (chunk->get_lod_num() > 4) {
					Ref<FastQuadraticMeshSimplifier> fqms;
					fqms.instance();
					fqms->set_preserve_border_edges(true);
					fqms->initialize(temp_mesh_arr);

					for (int i = 4; i < chunk->get_lod_num(); ++i) {
						fqms->simplify_mesh(temp_mesh_arr.size() * 0.8, 7);
						temp_mesh_arr = fqms->get_arrays();

						VisualServer::get_singleton()->mesh_add_surface_from_arrays(
								chunk->mesh_rid_get_index(TerraChunkDefault::MESH_INDEX_TERRARIN, TerraChunkDefault::MESH_TYPE_INDEX_MESH, i),
								VisualServer::PRIMITIVE_TRIANGLES, temp_mesh_arr);

						if (chunk->get_library()->prop_material_get(i).is_valid())
							VisualServer::get_singleton()->mesh_surface_set_material(
									chunk->mesh_rid_get_index(TerraChunkDefault::MESH_INDEX_TERRARIN, TerraChunkDefault::MESH_TYPE_INDEX_MESH, i), 0,
									chunk->get_library()->prop_material_get(i)->get_rid());
					}
				}

				if (should_return()) {
					return;
				}
			}
#endif
		}
	}

#endif
*/
	set_complete(true); //So threadpool knows it's done
	finished();
}

void PropInstancePropJob::_physics_process(float delta) {
	if (_phase == 0)
		phase_physics_process();
}

void PropInstancePropJob::_execute_phase() {
	/*
	ERR_FAIL_COND(!_chunk.is_valid());

	Ref<TerramanLibrary> library = _chunk->get_library();

	ERR_FAIL_COND(!library.is_valid());

	Ref<TerraChunkDefault> chunk = _chunk;

	if (!chunk.is_valid()
#ifdef MESH_DATA_RESOURCE_PRESENT
			|| chunk->mesh_data_resource_get_count() == 0
#endif
	) {
		set_complete(true);
		finished();
		return;
	}
*/
	if (_phase == 1) {
		//phase_prop();
	} else if (_phase > 1) {
		set_complete(true); //So threadpool knows it's done
		finished();
		ERR_FAIL_MSG("PropInstancePropJob: _phase is too high!");
	}

	set_complete(true); //So threadpool knows it's done
	finished();
}

void PropInstancePropJob::_reset() {
	PropInstanceJob::_reset();

	_build_done = false;
	_phase = 0;

	if (get_prop_mesher().is_valid()) {
		get_prop_mesher()->reset();
	}

	set_build_phase_type(BUILD_PHASE_TYPE_PHYSICS_PROCESS);
}

PropInstancePropJob::PropInstancePropJob() {
	set_build_phase_type(BUILD_PHASE_TYPE_PHYSICS_PROCESS);
}

PropInstancePropJob::~PropInstancePropJob() {
}

void PropInstancePropJob::_bind_methods() {
#if TEXTURE_PACKER_PRESENT
	ClassDB::bind_method(D_METHOD("get_texture_packer"), &PropInstancePropJob::get_texture_packer);
	ClassDB::bind_method(D_METHOD("set_texture_packer", "packer"), &PropInstancePropJob::set_texture_packer);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture_packer", PROPERTY_HINT_RESOURCE_TYPE, "TexturePacker", 0), "set_texture_packer", "get_texture_packer");
#endif

	ClassDB::bind_method(D_METHOD("get_prop_mesher"), &PropInstancePropJob::get_prop_mesher);
	ClassDB::bind_method(D_METHOD("set_prop_mesher", "mesher"), &PropInstancePropJob::set_prop_mesher);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "prop_mesher", PROPERTY_HINT_RESOURCE_TYPE, "PropMesher", 0), "set_prop_mesher", "get_prop_mesher");

	ClassDB::bind_method(D_METHOD("_physics_process", "delta"), &PropInstancePropJob::_physics_process);
}
