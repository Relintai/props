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

#include "core/version.h"

#if VERSION_MAYOR > 3
#define GET_WORLD get_world_3d
#else
#define GET_WORLD get_world
#endif

#include "jobs/prop_mesher_job_step.h"
#include "material_cache/prop_material_cache.h"
#include "prop_instance.h"
#include "prop_instance_merger.h"
#include "prop_mesher.h"
#include "singleton/prop_cache.h"

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
#undef PROPS_PRESENT
#endif

Ref<PropMaterialCache> PropInstancePropJob::get_material_cache() {
	return _material_cache;
}
void PropInstancePropJob::set_material_cache(const Ref<PropMaterialCache> &cache) {
	_material_cache = cache;
}

Ref<PropMesherJobStep> PropInstancePropJob::get_jobs_step(int index) const {
	ERR_FAIL_INDEX_V(index, _job_steps.size(), Ref<PropMesherJobStep>());

	return _job_steps.get(index);
}
void PropInstancePropJob::set_jobs_step(int index, const Ref<PropMesherJobStep> &step) {
	ERR_FAIL_INDEX(index, _job_steps.size());

	_job_steps.set(index, step);
}
void PropInstancePropJob::remove_jobs_step(const int index) {
	ERR_FAIL_INDEX(index, _job_steps.size());

	_job_steps.remove(index);
}
void PropInstancePropJob::add_jobs_step(const Ref<PropMesherJobStep> &step) {
	_job_steps.push_back(step);
}
int PropInstancePropJob::get_jobs_step_count() const {
	return _job_steps.size();
}

PropInstanceMerger *PropInstancePropJob::get_prop_instace() {
	return _prop_instace;
}
void PropInstancePropJob::set_prop_instace(PropInstanceMerger *prop) {
	_prop_instace = prop;
}
void PropInstancePropJob::set_prop_instace_bind(Node *prop) {
	set_prop_instace(Object::cast_to<PropInstanceMerger>(prop));
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
	//TODO this should only update the differences
	for (int i = 0; i < _prop_instace->collider_get_num(); ++i) {
		PhysicsServer::get_singleton()->free(_prop_instace->collider_body_get(i));
	}

	_prop_instace->colliders_clear();

#ifdef MESH_DATA_RESOURCE_PRESENT
	for (int i = 0; i < _prop_mesh_datas.size(); ++i) {
		PMDREntry &e = _prop_mesh_datas.write[i];

		Ref<PropDataMeshData> pe = e.mesh_data;

		ERR_CONTINUE(!pe.is_valid());

		Ref<MeshDataResource> mdr = pe->get_mesh();

		ERR_CONTINUE(!mdr.is_valid());

		Transform t = pe->get_transform();
		t *= e.base_transform;

		for (int j = 0; j < mdr->get_collision_shape_count(); ++j) {
			Ref<Shape> shape = mdr->get_collision_shape(j);
			Transform offset = mdr->get_collision_shape_offset(j);

			if (!shape.is_valid()) {
				continue;
			}

			RID body = PhysicsServer::get_singleton()->body_create(PhysicsServer::BODY_MODE_STATIC);

			Transform transform = t;
			transform *= offset;

			PhysicsServer::get_singleton()->body_add_shape(body, shape->get_rid());

			//TODO store the layer mask somewhere
			PhysicsServer::get_singleton()->body_set_collision_layer(body, 1);
			PhysicsServer::get_singleton()->body_set_collision_mask(body, 1);

			if (_prop_instace->is_inside_tree() && _prop_instace->is_inside_world()) {
				Ref<World> world = _prop_instace->GET_WORLD();

				if (world.is_valid() && world->get_space() != RID()) {
					PhysicsServer::get_singleton()->body_set_space(body, world->get_space());
				}
			}

			PhysicsServer::get_singleton()->body_set_state(body, PhysicsServer::BODY_STATE_TRANSFORM, _prop_instace->get_transform() * transform);

			_prop_instace->collider_add(transform, shape, shape->get_rid(), body);
		}
	}
#endif

#if TOOLS_ENABLED
	if (SceneTree::get_singleton()->is_debugging_collisions_hint() && _prop_instace->collider_get_num() > 0) {
		_prop_instace->draw_debug_mdr_colliders();
	}
#endif

	set_build_phase_type(BUILD_PHASE_TYPE_NORMAL);
	next_phase();
}

void PropInstancePropJob::phase_prop() {
#ifdef MESH_DATA_RESOURCE_PRESENT

	if (!_prop_mesher.is_valid()) {
		set_complete(true); //So threadpool knows it's done
		return;
	}

	if (should_do()) {
		if (_prop_mesh_datas.size() == 0) {
			set_complete(true); //So threadpool knows it's done
			return;
		}

		for (int i = 0; i < _prop_mesh_datas.size(); ++i) {
			PMDREntry &e = _prop_mesh_datas.write[i];

			Ref<PropDataMeshData> pmd = e.mesh_data;

			Ref<MeshDataResource> mesh = pmd->get_mesh();
			Ref<Texture> tex = pmd->get_texture();
			Transform t = pmd->get_transform();
			t *= e.base_transform;

			Rect2 uvr = _material_cache->texture_get_uv_rect(tex);

			get_prop_mesher()->add_mesh_data_resource_transform(mesh, t, uvr);
		}

		if (get_prop_mesher()->get_vertex_count() == 0) {
			//reset_stages();

			set_complete(true); //So threadpool knows it's done
			return;
		}

		if (should_return()) {
			return;
		}
	}

	/*
	if (should_do()) {
		if ((chunk->get_build_flags() & TerraChunkDefault::BUILD_FLAG_USE_LIGHTING) != 0) {
			_prop_mesher->bake_colors(_chunk);
		}

		if (should_return()) {
			return;
		}
	}
*/
	/*
	//lights should be added here by the prop instance preemtively
	//Also this system should use it's own lights
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
	*/

	if (get_prop_mesher()->get_vertex_count() != 0) {
		if (should_do()) {
			temp_mesh_arr = get_prop_mesher()->build_mesh();

			if (should_return()) {
				return;
			}
		}

		if (should_do()) {
			if (_prop_instace->mesh_get_num() == 0) {
				_prop_instace->meshes_create(1);
			}

			RID mesh_rid = _prop_instace->mesh_get(0);

			/*
			if (mesh_rid == RID()) {
				if ((chunk->get_build_flags() & TerraChunkDefault::BUILD_FLAG_CREATE_LODS) != 0)
					chunk->meshes_create(TerraChunkDefault::MESH_INDEX_PROP, chunk->get_lod_num() + 1);
				else
					chunk->meshes_create(TerraChunkDefault::MESH_INDEX_PROP, 1);

				mesh_rid = chunk->mesh_rid_get_index(TerraChunkDefault::MESH_INDEX_PROP, TerraChunkDefault::MESH_TYPE_INDEX_MESH, 0);
			}*/

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
			RID mesh_rid = _prop_instace->mesh_get(0);

			VS::get_singleton()->mesh_add_surface_from_arrays(mesh_rid, VisualServer::PRIMITIVE_TRIANGLES, temp_mesh_arr);

			Ref<Material> mat = _prop_instace->material_get(0);

			if (mat.is_valid())
				VS::get_singleton()->mesh_surface_set_material(mesh_rid, 0, mat->get_rid());

			if (should_return()) {
				return;
			}
		}

		/* 
		//skip lod generation for now
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
		*/
	}

#endif

	set_complete(true); //So threadpool knows it's done
	finished();
}

void PropInstancePropJob::_physics_process(float delta) {
	if (_phase == 0)
		phase_physics_process();
}

void PropInstancePropJob::_execute_phase() {
	if (!_material_cache.is_valid()) {
		ERR_PRINT("!PropInstancePropJob::_execute_phase(): _material_cache.is_valid()");
		set_complete(true); //So threadpool knows it's done
		finished();
	}

#ifdef MESH_DATA_RESOURCE_PRESENT
	if (_prop_mesh_datas.size() == 0) {
		set_complete(true);
		finished();
		return;
	}
#endif

	if (_phase == 1) {
		phase_prop();
	} else if (_phase > 1) {
		set_complete(true); //So threadpool knows it's done
		finished();
		ERR_FAIL_MSG("PropInstancePropJob: _phase is too high!");
	}

	//set_complete(true); //So threadpool knows it's done
	//finished();
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

void PropInstancePropJob::phase_steps() {
	ERR_FAIL_COND(!_prop_mesher.is_valid());

	if (should_return()) {
		return;
	}

	if (_prop_mesher->get_vertex_count() == 0) {
		reset_stages();
		//next_phase();
		set_complete(true); //So threadpool knows it's done
		return;
	}

	//set up the meshes
	if (should_do()) {
		if (_prop_instace->mesh_get_num() == 0) {
			//need to allocate the meshes

			//first count how many we need
			int count = 0;
			for (int i = 0; i < _job_steps.size(); ++i) {
				Ref<PropMesherJobStep> step = _job_steps[i];

				ERR_FAIL_COND(!step.is_valid());

				switch (step->get_job_type()) {
					case PropMesherJobStep::TYPE_NORMAL:
						++count;
						break;
					case PropMesherJobStep::TYPE_NORMAL_LOD:
						++count;
						break;
					case PropMesherJobStep::TYPE_DROP_UV2:
						++count;
						break;
					case PropMesherJobStep::TYPE_MERGE_VERTS:
						++count;
						break;
					case PropMesherJobStep::TYPE_BAKE_TEXTURE:
						++count;
						break;
					case PropMesherJobStep::TYPE_SIMPLIFY_MESH:
#ifdef MESH_UTILS_PRESENT
						count += step->get_simplification_steps();
#endif
						break;
					default:
						break;
				}
			}

			//allocate
			if (count > 0) {
				_prop_instace->meshes_create(count);
			}

		} else {
			//we have the meshes, just clear
			int count = _prop_instace->mesh_get_num();

			for (int i = 0; i < count; ++i) {
				RID mesh_rid = _prop_instace->mesh_get(i);

				if (VS::get_singleton()->mesh_get_surface_count(mesh_rid) > 0)
#if !GODOT4
					VS::get_singleton()->mesh_remove_surface(mesh_rid, 0);
#else
					VS::get_singleton()->mesh_clear(mesh_rid);
#endif
			}
		}
	}

	for (; _current_job_step < _job_steps.size();) {
		Ref<PropMesherJobStep> step = _job_steps[_current_job_step];

		ERR_FAIL_COND(!step.is_valid());

		switch (step->get_job_type()) {
			case PropMesherJobStep::TYPE_NORMAL:
				step_type_normal();
				break;
			case PropMesherJobStep::TYPE_NORMAL_LOD:
				step_type_normal_lod();
				break;
			case PropMesherJobStep::TYPE_DROP_UV2:
				step_type_drop_uv2();
				break;
			case PropMesherJobStep::TYPE_MERGE_VERTS:
				step_type_merge_verts();
				break;
			case PropMesherJobStep::TYPE_BAKE_TEXTURE:
				step_type_bake_texture();
				break;
			case PropMesherJobStep::TYPE_SIMPLIFY_MESH:
				step_type_simplify_mesh();
				break;
			case PropMesherJobStep::TYPE_OTHER:
				//do nothing
				break;
		}

		++_current_job_step;

		if (should_return()) {
			return;
		}
	}

	reset_stages();
	//next_phase();

	set_complete(true); //So threadpool knows it's done
}

void PropInstancePropJob::step_type_normal() {
	//TODO add a lighting generation step

	temp_mesh_arr = _prop_mesher->build_mesh();

	RID mesh_rid = _prop_instace->mesh_get(_current_mesh);

	VS::get_singleton()->mesh_add_surface_from_arrays(mesh_rid, VisualServer::PRIMITIVE_TRIANGLES, temp_mesh_arr);

	Ref<Material> lmat;

	/*
	if (chunk->prop_material_cache_key_has()) {
		lmat = chunk->get_library()->prop_material_cache_get(_chunk->prop_material_cache_key_get())->material_lod_get(_current_mesh);
	} else {
		lmat = chunk->get_library()->prop_material_lod_get(_current_mesh);
	}
*/

	if (lmat.is_valid()) {
		VisualServer::get_singleton()->mesh_surface_set_material(mesh_rid, 0, lmat->get_rid());
	}

	++_current_mesh;
}

void PropInstancePropJob::step_type_normal_lod() {
	print_error("Error: step_type_normal_lod doesn't work for TerraPropJobs!");

	++_current_mesh;
}

void PropInstancePropJob::step_type_drop_uv2() {
	RID mesh_rid = _prop_instace->mesh_get(_current_mesh);

	temp_mesh_arr[VisualServer::ARRAY_TEX_UV2] = Variant();

	VisualServer::get_singleton()->mesh_add_surface_from_arrays(mesh_rid, VisualServer::PRIMITIVE_TRIANGLES, temp_mesh_arr);

	Ref<Material> lmat;

	/*
	if (chunk->prop_material_cache_key_has()) {
		lmat = chunk->get_library()->prop_material_cache_get(_chunk->prop_material_cache_key_get())->material_lod_get(_current_mesh);
	} else {
		lmat = chunk->get_library()->prop_material_lod_get(_current_mesh);
	}
*/

	if (lmat.is_valid()) {
		VisualServer::get_singleton()->mesh_surface_set_material(mesh_rid, 0, lmat->get_rid());
	}

	++_current_mesh;
}

void PropInstancePropJob::step_type_merge_verts() {
	Array temp_mesh_arr2 = merge_mesh_array(temp_mesh_arr);
	temp_mesh_arr = temp_mesh_arr2;

	RID mesh_rid = _prop_instace->mesh_get(_current_mesh);

	VisualServer::get_singleton()->mesh_add_surface_from_arrays(mesh_rid, VisualServer::PRIMITIVE_TRIANGLES, temp_mesh_arr);

	Ref<Material> lmat;

	/*
	if (chunk->prop_material_cache_key_has()) {
		lmat = chunk->get_library()->prop_material_cache_get(_chunk->prop_material_cache_key_get())->material_lod_get(_current_mesh);
	} else {
		lmat = chunk->get_library()->prop_material_lod_get(_current_mesh);
	}
*/

	if (lmat.is_valid()) {
		VisualServer::get_singleton()->mesh_surface_set_material(mesh_rid, 0, lmat->get_rid());
	}

	++_current_mesh;
}

void PropInstancePropJob::step_type_bake_texture() {
	Ref<ShaderMaterial> mat; // = chunk->get_library()->material_lod_get(0);
	Ref<SpatialMaterial> spmat; // = chunk->get_library()->material_lod_get(0);
	Ref<Texture> tex;

	if (mat.is_valid()) {
		tex = mat->get_shader_param("texture_albedo");
	} else if (spmat.is_valid()) {
		tex = spmat->get_texture(SpatialMaterial::TEXTURE_ALBEDO);
	}

	if (tex.is_valid()) {
		temp_mesh_arr = bake_mesh_array_uv(temp_mesh_arr, tex);
		temp_mesh_arr[VisualServer::ARRAY_TEX_UV] = Variant();

		RID mesh_rid = _prop_instace->mesh_get(_current_mesh);

		VisualServer::get_singleton()->mesh_add_surface_from_arrays(mesh_rid, VisualServer::PRIMITIVE_TRIANGLES, temp_mesh_arr);

		Ref<Material> lmat;

		/*
		if (chunk->prop_material_cache_key_has()) {
			lmat = chunk->get_library()->prop_material_cache_get(_chunk->prop_material_cache_key_get())->material_lod_get(_current_mesh);
		} else {
			lmat = chunk->get_library()->prop_material_lod_get(_current_mesh);
		}
*/

		if (lmat.is_valid()) {
			VisualServer::get_singleton()->mesh_surface_set_material(mesh_rid, 0, lmat->get_rid());
		}
	}

	++_current_mesh;
}

void PropInstancePropJob::step_type_simplify_mesh() {
#ifdef MESH_UTILS_PRESENT

	Ref<PropMesherJobStep> step = _job_steps[_current_job_step];
	ERR_FAIL_COND(!step.is_valid());
	Ref<FastQuadraticMeshSimplifier> fqms = step->get_fqms();
	ERR_FAIL_COND(!fqms.is_valid());

	fqms->initialize(temp_mesh_arr);

	for (int i = 0; i < step->get_simplification_steps(); ++i) {
		fqms->simplify_mesh(temp_mesh_arr.size() * step->get_simplification_step_ratio(), step->get_simplification_agressiveness());
		temp_mesh_arr = fqms->get_arrays();

		RID mesh_rid = _prop_instace->mesh_get(_current_mesh);

		VisualServer::get_singleton()->mesh_add_surface_from_arrays(mesh_rid, VisualServer::PRIMITIVE_TRIANGLES, temp_mesh_arr);

		Ref<Material> lmat;

		/*
		if (chunk->prop_material_cache_key_has()) {
			lmat = chunk->get_library()->prop_material_cache_get(_chunk->prop_material_cache_key_get())->material_lod_get(_current_mesh);
		} else {
			lmat = chunk->get_library()->prop_material_lod_get(_current_mesh);
		}
*/

		if (lmat.is_valid()) {
			VisualServer::get_singleton()->mesh_surface_set_material(mesh_rid, 0, lmat->get_rid());
		}

		++_current_mesh;
	}

#endif
}

Array PropInstancePropJob::merge_mesh_array(Array arr) const {
	ERR_FAIL_COND_V(arr.size() != VisualServer::ARRAY_MAX, arr);

	PoolVector3Array verts = arr[VisualServer::ARRAY_VERTEX];
	PoolVector3Array normals = arr[VisualServer::ARRAY_NORMAL];
	PoolVector2Array uvs = arr[VisualServer::ARRAY_TEX_UV];
	PoolColorArray colors = arr[VisualServer::ARRAY_COLOR];
	PoolIntArray indices = arr[VisualServer::ARRAY_INDEX];

	bool has_normals = normals.size() > 0;
	bool has_uvs = uvs.size() > 0;
	bool has_colors = colors.size() > 0;

	int i = 0;
	while (i < verts.size()) {
		Vector3 v = verts[i];

		Array equals;
		for (int j = i + 1; j < verts.size(); ++j) {
			Vector3 vc = verts[j];

			if (Math::is_equal_approx(v.x, vc.x) && Math::is_equal_approx(v.y, vc.y) && Math::is_equal_approx(v.z, vc.z))
				equals.push_back(j);
		}

		for (int k = 0; k < equals.size(); ++k) {
			int rem = equals[k];
			int remk = rem - k;

			verts.remove(remk);

			if (has_normals)
				normals.remove(remk);
			if (has_uvs)
				uvs.remove(remk);
			if (has_colors)
				colors.remove(remk);

			for (int j = 0; j < indices.size(); ++j) {
				int indx = indices[j];

				if (indx == remk)
					indices.set(j, i);
				else if (indx > remk)
					indices.set(j, indx - 1);
			}
		}

		++i;
	}

	arr[VisualServer::ARRAY_VERTEX] = verts;

	if (has_normals)
		arr[VisualServer::ARRAY_NORMAL] = normals;
	if (has_uvs)
		arr[VisualServer::ARRAY_TEX_UV] = uvs;
	if (has_colors)
		arr[VisualServer::ARRAY_COLOR] = colors;

	arr[VisualServer::ARRAY_INDEX] = indices;

	return arr;
}
Array PropInstancePropJob::bake_mesh_array_uv(Array arr, Ref<Texture> tex, const float mul_color) const {
	ERR_FAIL_COND_V(arr.size() != VisualServer::ARRAY_MAX, arr);
	ERR_FAIL_COND_V(!tex.is_valid(), arr);

	Ref<Image> img = tex->get_data();

	ERR_FAIL_COND_V(!img.is_valid(), arr);

	Vector2 imgsize = img->get_size();

	PoolVector2Array uvs = arr[VisualServer::ARRAY_TEX_UV];
	PoolColorArray colors = arr[VisualServer::ARRAY_COLOR];

	if (colors.size() < uvs.size())
		colors.resize(uvs.size());

#if !GODOT4
	img->lock();
#endif

	for (int i = 0; i < uvs.size(); ++i) {
		Vector2 uv = uvs[i];
		uv *= imgsize;

		int ux = static_cast<int>(CLAMP(uv.x, 0, imgsize.x - 1));
		int uy = static_cast<int>(CLAMP(uv.y, 0, imgsize.y - 1));

		Color c = img->get_pixel(ux, uy);

		colors.set(i, colors[i] * c * mul_color);
	}

#if !GODOT4
	img->unlock();
#endif

	arr[VisualServer::ARRAY_COLOR] = colors;

	return arr;
}

PropInstancePropJob::PropInstancePropJob() {
	set_build_phase_type(BUILD_PHASE_TYPE_PHYSICS_PROCESS);

	_prop_instace = NULL;
}

PropInstancePropJob::~PropInstancePropJob() {
}

void PropInstancePropJob::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_material_cache"), &PropInstancePropJob::get_material_cache);
	ClassDB::bind_method(D_METHOD("set_material_cache", "packer"), &PropInstancePropJob::set_material_cache);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material_cache", PROPERTY_HINT_RESOURCE_TYPE, "PropMaterialCache", 0), "set_material_cache", "get_material_cache");

	ClassDB::bind_method(D_METHOD("get_jobs_step", "index"), &PropInstancePropJob::get_jobs_step);
	ClassDB::bind_method(D_METHOD("set_jobs_step", "index", "mesher"), &PropInstancePropJob::set_jobs_step);
	ClassDB::bind_method(D_METHOD("remove_jobs_step", "index"), &PropInstancePropJob::remove_jobs_step);
	ClassDB::bind_method(D_METHOD("add_jobs_step", "mesher"), &PropInstancePropJob::add_jobs_step);
	ClassDB::bind_method(D_METHOD("get_jobs_step_count"), &PropInstancePropJob::get_jobs_step_count);

	ClassDB::bind_method(D_METHOD("get_prop_mesher"), &PropInstancePropJob::get_prop_mesher);
	ClassDB::bind_method(D_METHOD("set_prop_mesher", "mesher"), &PropInstancePropJob::set_prop_mesher);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "prop_mesher", PROPERTY_HINT_RESOURCE_TYPE, "PropMesher", 0), "set_prop_mesher", "get_prop_mesher");

	ClassDB::bind_method(D_METHOD("_physics_process", "delta"), &PropInstancePropJob::_physics_process);
}
