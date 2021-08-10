#include "prop_instance_merger.h"

#include "../mesh_data_resource/nodes/mesh_data_instance.h"

//#include "../thread_pool/thread_pool.h"

#include "core/version.h"

#if VERSION_MAJOR > 3
#include "core/config/engine.h"

#define VARIANT_ARRAY_GET(arr)             \
	Vector<Variant> r;                     \
	for (int i = 0; i < arr.size(); i++) { \
		r.push_back(arr[i]);               \
	}                                      \
	return r;

#include "servers/rendering_server.h"
typedef class RenderingServer VS;

#define GET_WORLD get_world_3d

#else
#include "core/engine.h"

#define VARIANT_ARRAY_GET(arr)             \
	Vector<Variant> r;                     \
	for (int i = 0; i < arr.size(); i++) { \
		r.push_back(arr[i].get_ref_ptr()); \
	}                                      \
	return r;

#include "servers/visual_server.h"

#define GET_WORLD get_world

#endif

#if MESH_DATA_RESOURCE_PRESENT
//define PROPS_PRESENT, so things compile. That module's scsub will define this too while compiling,
//but not when included from here.
#define PROPS_PRESENT 1
#include "../mesh_data_resource/props/prop_data_mesh_data.h"
#endif

#include "./props/prop_data_entry.h"
#include "./props/prop_data_light.h"
#include "./props/prop_data_prop.h"
#include "./props/prop_data_scene.h"

#if TEXTURE_PACKER_PRESENT
#include "./singleton/prop_cache.h"
#endif

#if THREAD_POOL_PRESENT
#include "../thread_pool/thread_pool.h"
#endif

Ref<PropInstanceJob> PropInstanceMerger::get_job() {
	return _job;
}
void PropInstanceMerger::set_job(const Ref<PropInstanceJob> &job) {
	_job = job;
}

#ifdef TEXTURE_PACKER_PRESENT
bool PropInstanceMerger::get_merge_textures() const {
	return _merge_textures;
}

void PropInstanceMerger::set_merge_textures(const bool value) {
	_merge_textures = value;
}
#endif

//Materials
Ref<Material> PropInstanceMerger::material_get(const int index) {
	ERR_FAIL_INDEX_V(index, _materials.size(), Ref<Material>(NULL));

	return _materials[index];
}

void PropInstanceMerger::material_add(const Ref<Material> &value) {
	ERR_FAIL_COND(!value.is_valid());

	_materials.push_back(value);
}

int PropInstanceMerger::material_get_num() const {
	return _materials.size();
}

void PropInstanceMerger::materials_clear() {
	_materials.clear();
}

Vector<Variant> PropInstanceMerger::materials_get() {
	VARIANT_ARRAY_GET(_materials);
}

void PropInstanceMerger::materials_set(const Vector<Variant> &materials) {
	_materials.clear();

	for (int i = 0; i < materials.size(); i++) {
		Ref<Material> material = Ref<Material>(materials[i]);

		_materials.push_back(material);
	}
}

//Meshes
RID PropInstanceMerger::mesh_get(const int index) {
	ERR_FAIL_INDEX_V(index, _meshes.size(), RID());

	return _meshes[index].mesh;
}

RID PropInstanceMerger::mesh_instance_get(const int index) {
	ERR_FAIL_INDEX_V(index, _meshes.size(), RID());

	return _meshes[index].mesh_instance;
}

void PropInstanceMerger::mesh_add(const RID mesh_instance, const RID mesh) {
	MeshEntry e;
	e.mesh = mesh;
	e.mesh_instance = mesh_instance;

	_meshes.push_back(e);
}

int PropInstanceMerger::mesh_get_num() const {
	return _meshes.size();
}

void PropInstanceMerger::meshes_clear() {
	_meshes.clear();
}

void PropInstanceMerger::meshes_create(const int num) {
	free_meshes();

	for (int i = 0; i < num; ++i) {
		RID mesh_instance_rid = VS::get_singleton()->instance_create();

		if (GET_WORLD().is_valid())
			VS::get_singleton()->instance_set_scenario(mesh_instance_rid, GET_WORLD()->get_scenario());

		RID mesh_rid = VS::get_singleton()->mesh_create();

		VS::get_singleton()->instance_set_base(mesh_instance_rid, mesh_rid);

		VS::get_singleton()->instance_set_transform(mesh_instance_rid, get_transform());

		if (i != 0)
			VS::get_singleton()->instance_set_visible(mesh_instance_rid, false);

		MeshEntry e;
		e.mesh = mesh_rid;
		e.mesh_instance = mesh_instance_rid;

		_meshes.push_back(e);
	}
}

Vector<Variant> PropInstanceMerger::meshes_get() {
	Vector<Variant> r;
	for (int i = 0; i < _meshes.size(); i++) {
		Array a;

		a.push_back(_meshes[i].mesh);
		a.push_back(_meshes[i].mesh_instance);

		r.push_back(a);
	}
	return r;
}

void PropInstanceMerger::meshes_set(const Vector<Variant> &meshs) {
	_meshes.clear();

	for (int i = 0; i < _meshes.size(); i++) {
		Array arr = Array(meshs[i]);

		ERR_CONTINUE(arr.size() != 2);

		MeshEntry e;
		e.mesh = RID(arr[0]);
		e.mesh_instance = RID(arr[1]);

		_meshes.push_back(e);
	}
}

//Collider

Transform PropInstanceMerger::collider_local_transform_get(const int index) {
	ERR_FAIL_INDEX_V(index, _colliders.size(), Transform());

	return _colliders[index].transform;
}

RID PropInstanceMerger::collider_body_get(const int index) {
	ERR_FAIL_INDEX_V(index, _colliders.size(), RID());

	return _colliders[index].body;
}

Ref<Shape> PropInstanceMerger::collider_shape_get(const int index) {
	ERR_FAIL_INDEX_V(index, _colliders.size(), Ref<Shape>());

	return _colliders[index].shape;
}

RID PropInstanceMerger::collider_shape_rid_get(const int index) {
	ERR_FAIL_INDEX_V(index, _colliders.size(), RID());

	return _colliders[index].shape_rid;
}

int PropInstanceMerger::collider_add(const Transform &local_transform, const Ref<Shape> &shape, const RID &shape_rid, const RID &body) {
	ERR_FAIL_COND_V(!shape.is_valid() && shape_rid == RID(), 0);

	int index = _colliders.size();

	ColliderBody e;
	e.transform = local_transform;
	e.body = body;
	e.shape = shape;
	e.shape_rid = shape_rid;

	_colliders.push_back(e);

	return index;
}

int PropInstanceMerger::collider_get_num() const {
	return _colliders.size();
}

void PropInstanceMerger::colliders_clear() {
	_colliders.clear();
}

Vector<Variant> PropInstanceMerger::colliders_get() {
	Vector<Variant> r;
	for (int i = 0; i < _colliders.size(); i++) {
		r.push_back(_colliders[i].body);
	}
	return r;
}

void PropInstanceMerger::colliders_set(const Vector<Variant> &colliders) {
	_colliders.clear();

	for (int i = 0; i < colliders.size(); i++) {
		RID collider = (colliders[i]);

		ColliderBody c;
		c.body = collider;

		_colliders.push_back(c);
	}
}

void PropInstanceMerger::debug_mesh_allocate() {
	if (_debug_mesh_rid == RID()) {
		_debug_mesh_rid = VisualServer::get_singleton()->mesh_create();
	}

	if (_debug_mesh_instance == RID()) {
		_debug_mesh_instance = VisualServer::get_singleton()->instance_create();

		if (GET_WORLD().is_valid())
			VS::get_singleton()->instance_set_scenario(_debug_mesh_instance, GET_WORLD()->get_scenario());

		VS::get_singleton()->instance_set_base(_debug_mesh_instance, _debug_mesh_rid);
		VS::get_singleton()->instance_set_transform(_debug_mesh_instance, get_transform());
		VS::get_singleton()->instance_set_visible(_debug_mesh_instance, true);
	}
}
void PropInstanceMerger::debug_mesh_free() {
	if (_debug_mesh_instance != RID()) {
		VisualServer::get_singleton()->free(_debug_mesh_instance);
	}

	if (_debug_mesh_rid != RID()) {
		VisualServer::get_singleton()->free(_debug_mesh_rid);
	}
}
bool PropInstanceMerger::debug_mesh_has() {
	return _debug_mesh_rid != RID();
}
void PropInstanceMerger::debug_mesh_clear() {
	if (_debug_mesh_rid != RID()) {
		VisualServer::get_singleton()->mesh_clear(_debug_mesh_rid);
	}
}
void PropInstanceMerger::debug_mesh_array_clear() {
	_debug_mesh_array.resize(0);
}
void PropInstanceMerger::debug_mesh_add_vertices_to(const PoolVector3Array &arr) {
	_debug_mesh_array.append_array(arr);

	if (_debug_mesh_array.size() % 2 == 1) {
		_debug_mesh_array.append(_debug_mesh_array[_debug_mesh_array.size() - 1]);
	}
}
void PropInstanceMerger::debug_mesh_send() {
	debug_mesh_allocate();
	debug_mesh_clear();

	if (_debug_mesh_array.size() == 0)
		return;

	SceneTree *st = SceneTree::get_singleton();

	Array arr;
	arr.resize(VisualServer::ARRAY_MAX);
	arr[VisualServer::ARRAY_VERTEX] = _debug_mesh_array;

	VisualServer::get_singleton()->mesh_add_surface_from_arrays(_debug_mesh_rid, VisualServer::PRIMITIVE_LINES, arr);

	if (st) {
		VisualServer::get_singleton()->mesh_surface_set_material(_debug_mesh_rid, 0, SceneTree::get_singleton()->get_debug_collision_material()->get_rid());
	}

	debug_mesh_array_clear();
}

void PropInstanceMerger::draw_debug_mdr_colliders() {
	if (!debug_mesh_has()) {
		debug_mesh_allocate();
	}

	for (int i = 0; i < collider_get_num(); ++i) {
		Ref<Shape> shape = collider_shape_get(i);

		if (!shape.is_valid())
			continue;

		Transform t = collider_local_transform_get(i);

		shape->add_vertices_to_array(_debug_mesh_array, t);
	}
}

float PropInstanceMerger::get_first_lod_distance_squared() {
	return _first_lod_distance_squared;
}
void PropInstanceMerger::set_first_lod_distance_squared(const float dist) {
	_first_lod_distance_squared = dist;
}

float PropInstanceMerger::get_lod_reduction_distance_squared() {
	return _lod_reduction_distance_squared;
}
void PropInstanceMerger::set_lod_reduction_distance_squared(const float dist) {
	_lod_reduction_distance_squared = dist;
}

void PropInstanceMerger::free_meshes() {
	RID rid;

	for (int i = 0; i < _meshes.size(); ++i) {
		MeshEntry &e = _meshes.write[i];

		if (e.mesh_instance != rid) {
			VS::get_singleton()->free(e.mesh_instance);
		}

		if (e.mesh != rid) {
			VS::get_singleton()->free(e.mesh);
		}

		e.mesh_instance = rid;
		e.mesh = rid;
	}
}

void PropInstanceMerger::free_colliders() {
	for (int i = 0; i < _colliders.size(); ++i) {
		PhysicsServer::get_singleton()->free(_colliders[i].body);

		_colliders.write[i].body = RID();
	}
}

void PropInstanceMerger::_init_materials() {
}

void PropInstanceMerger::_build() {
	_building = true;
	_build_queued = false;

	if (!_prop_data.is_valid()) {
		_building = false;
		return;
	}

	if (!_job.is_valid()) {
		_job = Ref<PropInstanceJob>(memnew(PropInstancePropJob()));
		_job->set_prop_instace(this);
	}

	if (!is_inside_tree()) {
		_building = false;
		_build_queued = true;
		return;
	}

//!
//job->cache
//this->cache
//job->cache = cache
//job ->  if !has cache query -> buildings can use this to only have one material per building

//move this to job
//#if TEXTURE_PACKER_PRESENT
	//Ref<TexturePacker> packer; // = PropCache::get_singleton()->get_or_create_texture_threaded(_prop_data);
	/*
	if (packer->get_generated_texture_count() == 0) {
		_building = false;
		_build_queued = true;
		return;
	}
*/
//	_job->set_texture_packer(packer);
//#endif

	for (int i = 0; i < get_child_count(); ++i) {
		Node *n = get_child(i);

		//this way we won't delete the user's nodes
		if (n->get_owner() == NULL) {
			n->queue_delete();
		}
	}

	if (!_prop_data.is_valid())
		return;

	prop_preprocess(Transform(), _prop_data);

#if THREAD_POOL_PRESENT
	ThreadPool::get_singleton()->add_job(_job);
#else
	_job->execute();
#endif
}

void PropInstanceMerger::_build_finished() {
}

void PropInstanceMerger::_prop_preprocess(Transform transform, const Ref<PropData> &prop) {
	ERR_FAIL_COND(!prop.is_valid());

	int count = prop->get_prop_count();
	for (int i = 0; i < count; ++i) {
		Ref<PropDataEntry> e = prop->get_prop(i);

		if (!e.is_valid())
			continue;

		Transform t = transform * e->get_transform();

		Ref<PropDataProp> prop_entry_data = e;

		if (prop_entry_data.is_valid()) {
			Ref<PropData> p = prop_entry_data->get_prop();

			if (!p.is_valid())
				continue;

			prop_preprocess(t, p);

			continue;
		}

		Ref<PropDataScene> scene_data = e;

		if (scene_data.is_valid()) {
			Ref<PackedScene> sc = scene_data->get_scene();

			if (!sc.is_valid())
				continue;

			Node *n = sc->instance();
			add_child(n);
			n->set_owner(this);

			Spatial *sp = Object::cast_to<Spatial>(n);

			if (sp) {
				sp->set_transform(t);
			}

			continue;
		}

		/*
		//Will create a Terralight node, and prop
		//PropDataLight could use standard godot light nodes
		Ref<PropDataLight> light_data = entry;

		if (light_data.is_valid()) {
			Ref<VoxelLight> light;
			light.instance();

			light->set_world_position(wp.x / get_voxel_scale(), wp.y / get_voxel_scale(), wp.z / get_voxel_scale());
			light->set_color(light_data->get_light_color());
			light->set_size(light_data->get_light_size());

			light_add(light);

			continue;
		}
		*/

#if MESH_DATA_RESOURCE_PRESENT
		Ref<PropDataMeshData> mesh_data = e;

		if (mesh_data.is_valid()) {
			Ref<MeshDataResource> mdr = mesh_data->get_mesh();

			if (!mdr.is_valid())
				continue;

			_job->add_mesh(mesh_data, transform);

			continue;
		}
#endif
	}
}

PropInstanceMerger::PropInstanceMerger() {
	_build_queued = false;
	_building = false;

#ifdef TEXTURE_PACKER_PRESENT
	_merge_textures = true;
#endif

	_first_lod_distance_squared = 20;
	_lod_reduction_distance_squared = 10;
}

PropInstanceMerger::~PropInstanceMerger() {
	_job.unref();

	_prop_data.unref();

	_materials.clear();
}

void PropInstanceMerger::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			if (_prop_data.is_valid()) {
				build();
			}

			set_physics_process_internal(true);
			set_process_internal(true);

			break;
		}
		case NOTIFICATION_EXIT_TREE: {
			if (_job.is_valid()) {
				_job->set_cancelled(true);
			}

			free_meshes();
			free_colliders();
			break;
		}
		case NOTIFICATION_INTERNAL_PHYSICS_PROCESS: {
			//todo turn this on and off properly

			if (_building) {
				if (_job->get_build_phase_type() == PropInstanceJob::BUILD_PHASE_TYPE_PHYSICS_PROCESS) {
					_job->physics_process(get_physics_process_delta_time());
				}
			}

			break;
		}
		case NOTIFICATION_INTERNAL_PROCESS: {
			//todo turn this on and off properly

			if (_building) {
				if (_job->get_build_phase_type() == PropInstanceJob::BUILD_PHASE_TYPE_PROCESS) {
					_job->process(get_process_delta_time());
				}
			}

			break;
		}
	}
}

void PropInstanceMerger::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_job"), &PropInstanceMerger::get_job);
	ClassDB::bind_method(D_METHOD("set_job", "value"), &PropInstanceMerger::set_job);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "job", PROPERTY_HINT_RESOURCE_TYPE, "PropInstanceJob", 0), "set_job", "get_job");

#ifdef TEXTURE_PACKER_PRESENT
	ClassDB::bind_method(D_METHOD("get_merge_textures"), &PropInstanceMerger::get_merge_textures);
	ClassDB::bind_method(D_METHOD("set_merge_textures", "value"), &PropInstanceMerger::set_merge_textures);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "merge_textures"), "set_merge_textures", "get_merge_textures");
#endif

	///Materials
	ClassDB::bind_method(D_METHOD("material_get", "index"), &PropInstanceMerger::material_get);
	ClassDB::bind_method(D_METHOD("material_add", "value"), &PropInstanceMerger::material_add);
	ClassDB::bind_method(D_METHOD("material_get_num"), &PropInstanceMerger::material_get_num);
	ClassDB::bind_method(D_METHOD("materials_clear"), &PropInstanceMerger::materials_clear);

	ClassDB::bind_method(D_METHOD("materials_get"), &PropInstanceMerger::materials_get);
	ClassDB::bind_method(D_METHOD("materials_set"), &PropInstanceMerger::materials_set);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "materials", PROPERTY_HINT_NONE, "17/17:Material", PROPERTY_USAGE_DEFAULT, "Material"), "materials_set", "materials_get");

	//Meshes
	ClassDB::bind_method(D_METHOD("mesh_get", "index"), &PropInstanceMerger::mesh_get);
	ClassDB::bind_method(D_METHOD("mesh_instance_get", "index"), &PropInstanceMerger::mesh_instance_get);
	ClassDB::bind_method(D_METHOD("mesh_add", "mesh_instance", "mesh"), &PropInstanceMerger::mesh_add);
	ClassDB::bind_method(D_METHOD("mesh_get_num"), &PropInstanceMerger::mesh_get_num);
	ClassDB::bind_method(D_METHOD("meshes_clear"), &PropInstanceMerger::meshes_clear);

	ClassDB::bind_method(D_METHOD("meshes_get"), &PropInstanceMerger::meshes_get);
	ClassDB::bind_method(D_METHOD("meshes_set"), &PropInstanceMerger::meshes_set);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "meshes", PROPERTY_HINT_NONE, "", 0), "meshes_set", "meshes_get");

	//Colliders
	ClassDB::bind_method(D_METHOD("collider_local_transform_get", "index"), &PropInstanceMerger::collider_local_transform_get);
	ClassDB::bind_method(D_METHOD("collider_body_get", "index"), &PropInstanceMerger::collider_body_get);
	ClassDB::bind_method(D_METHOD("collider_shape_get", "index"), &PropInstanceMerger::collider_shape_get);
	ClassDB::bind_method(D_METHOD("collider_shape_rid_get", "index"), &PropInstanceMerger::collider_shape_rid_get);
	ClassDB::bind_method(D_METHOD("collider_add", "local_transform", "shape", "shape_rid", "body"), &PropInstanceMerger::collider_add);
	ClassDB::bind_method(D_METHOD("collider_get_num"), &PropInstanceMerger::collider_get_num);
	ClassDB::bind_method(D_METHOD("colliders_clear"), &PropInstanceMerger::colliders_clear);
	ClassDB::bind_method(D_METHOD("meshes_create", "num"), &PropInstanceMerger::meshes_create);

	//Colliders
	ClassDB::bind_method(D_METHOD("debug_mesh_allocate"), &PropInstanceMerger::debug_mesh_allocate);
	ClassDB::bind_method(D_METHOD("debug_mesh_free"), &PropInstanceMerger::debug_mesh_free);
	ClassDB::bind_method(D_METHOD("debug_mesh_has"), &PropInstanceMerger::debug_mesh_has);
	ClassDB::bind_method(D_METHOD("debug_mesh_clear"), &PropInstanceMerger::debug_mesh_clear);
	ClassDB::bind_method(D_METHOD("debug_mesh_array_clear"), &PropInstanceMerger::debug_mesh_array_clear);
	ClassDB::bind_method(D_METHOD("debug_mesh_add_vertices_to", "arr"), &PropInstanceMerger::debug_mesh_add_vertices_to);
	ClassDB::bind_method(D_METHOD("debug_mesh_send"), &PropInstanceMerger::debug_mesh_send);
	ClassDB::bind_method(D_METHOD("draw_debug_mdr_colliders"), &PropInstanceMerger::draw_debug_mdr_colliders);

	//---
	ClassDB::bind_method(D_METHOD("get_first_lod_distance_squared"), &PropInstanceMerger::get_first_lod_distance_squared);
	ClassDB::bind_method(D_METHOD("set_first_lod_distance_squared", "value"), &PropInstanceMerger::set_first_lod_distance_squared);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "first_lod_distance_squared"), "set_first_lod_distance_squared", "get_first_lod_distance_squared");

	ClassDB::bind_method(D_METHOD("get_lod_reduction_distance_squared"), &PropInstanceMerger::get_lod_reduction_distance_squared);
	ClassDB::bind_method(D_METHOD("set_lod_reduction_distance_squared", "value"), &PropInstanceMerger::set_lod_reduction_distance_squared);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "lod_reduction_distance_squared"), "set_lod_reduction_distance_squared", "get_lod_reduction_distance_squared");

	//---
	ClassDB::bind_method(D_METHOD("free_meshes"), &PropInstanceMerger::free_meshes);
	ClassDB::bind_method(D_METHOD("free_colliders"), &PropInstanceMerger::free_colliders);
}
