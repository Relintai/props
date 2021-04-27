#include "prop_instance.h"

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

#else
#include "core/engine.h"

#define VARIANT_ARRAY_GET(arr)             \
	Vector<Variant> r;                     \
	for (int i = 0; i < arr.size(); i++) { \
		r.push_back(arr[i].get_ref_ptr()); \
	}                                      \
	return r;

#include "servers/visual_server.h"

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

Ref<PropData> PropInstance::get_prop_data() {
	return _prop_data;
}
void PropInstance::set_prop_data(const Ref<PropData> &data) {
	if (_prop_data == data)
		return;

	_prop_data = data;

	queue_build();
}

Ref<PropInstanceJob> PropInstance::get_job() {
	return _job;
}
void PropInstance::set_job(const Ref<PropInstanceJob> &job) {
	_job = job;
}

#ifdef TEXTURE_PACKER_PRESENT
bool PropInstance::get_merge_textures() const {
	return _merge_textures;
}

void PropInstance::set_merge_textures(const bool value) {
	_merge_textures = value;
}
#endif

//Materials
Ref<Material> PropInstance::material_get(const int index) {
	ERR_FAIL_INDEX_V(index, _materials.size(), Ref<Material>(NULL));

	return _materials[index];
}

void PropInstance::material_add(const Ref<Material> &value) {
	ERR_FAIL_COND(!value.is_valid());

	_materials.push_back(value);
}

int PropInstance::material_get_num() const {
	return _materials.size();
}

void PropInstance::materials_clear() {
	_materials.clear();
}

Vector<Variant> PropInstance::materials_get() {
	VARIANT_ARRAY_GET(_materials);
}

void PropInstance::materials_set(const Vector<Variant> &materials) {
	_materials.clear();

	for (int i = 0; i < materials.size(); i++) {
		Ref<Material> material = Ref<Material>(materials[i]);

		_materials.push_back(material);
	}
}

//Meshes
RID PropInstance::mesh_get(const int index) {
	ERR_FAIL_INDEX_V(index, _meshes.size(), RID());

	return _meshes[index];
}

void PropInstance::mesh_add(const RID value) {
	_meshes.push_back(value);
}

int PropInstance::mesh_get_num() const {
	return _meshes.size();
}

void PropInstance::meshs_clear() {
	_meshes.clear();
}

Vector<Variant> PropInstance::meshes_get() {
	Vector<Variant> r;
	for (int i = 0; i < _meshes.size(); i++) {
		r.push_back(_meshes[i]);
	}
	return r;
}

void PropInstance::meshes_set(const Vector<Variant> &meshs) {
	_meshes.clear();

	for (int i = 0; i < _meshes.size(); i++) {
		RID mesh = RID(meshs[i]);

		_meshes.push_back(mesh);
	}
}

//Collider
RID PropInstance::collider_get(const int index) {
	ERR_FAIL_INDEX_V(index, _colliders.size(), RID());

	return _colliders[index];
}

void PropInstance::collider_add(const RID value) {
	_colliders.push_back(value);
}

int PropInstance::collider_get_num() const {
	return _colliders.size();
}

void PropInstance::colliders_clear() {
	_colliders.clear();
}

Vector<Variant> PropInstance::colliders_get() {
	Vector<Variant> r;
	for (int i = 0; i < _colliders.size(); i++) {
		r.push_back(_colliders[i]);
	}
	return r;
}

void PropInstance::colliders_set(const Vector<Variant> &colliders) {
	_colliders.clear();

	for (int i = 0; i < colliders.size(); i++) {
		RID collider = (colliders[i]);

		_colliders.push_back(collider);
	}
}

float PropInstance::get_first_lod_distance_squared() {
	return _first_lod_distance_squared;
}
void PropInstance::set_first_lod_distance_squared(const float dist) {
	_first_lod_distance_squared = dist;
}

float PropInstance::get_lod_reduction_distance_squared() {
	return _lod_reduction_distance_squared;
}
void PropInstance::set_lod_reduction_distance_squared(const float dist) {
	_lod_reduction_distance_squared = dist;
}

void PropInstance::free_meshes() {
}
void PropInstance::free_colliders() {
}

void PropInstance::init_materials() {
	call("_init_materials");
}
void PropInstance::_init_materials() {
}

void PropInstance::build() {
	call("_build");
}

void PropInstance::queue_build() {
}

void PropInstance::build_finished() {
	_building = false;

	if (_build_queued) {
		call_deferred("build");
	}
}

void PropInstance::_build() {
	_building = true;
	_build_queued = false;

	if (!_job.is_valid()) {
		_job = Ref<PropInstanceJob>(memnew(PropInstancePropJob()));
	}

	_job->set_prop_instace(this);

	if (!is_inside_tree()) {
		return;
	}

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
}

void PropInstance::_build_finished() {
}

void PropInstance::prop_preprocess(Transform transform, const Ref<PropData> &prop) {
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

			//add to job
			//job could merge textures if needed
			//chunk->mesh_data_resource_add(t, mdr, mesh_data->get_texture());

			continue;
		}
#endif
	}
}

PropInstance::PropInstance() {
	_build_queued = false;
	_building = false;

#ifdef TEXTURE_PACKER_PRESENT
	_merge_textures = true;
#endif

	_first_lod_distance_squared = 20;
	_lod_reduction_distance_squared = 10;
}

PropInstance::~PropInstance() {
	_job.unref();

	_prop_data.unref();

	_materials.clear();
}

void PropInstance::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			if (_prop_data.is_valid()) {
				build();
			}
		}
		case NOTIFICATION_EXIT_TREE: {
			if (_job.is_valid()) {
				_job->set_cancelled(true);
			}

			free_meshes();
			free_colliders();
		}
	}
}

void PropInstance::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_prop_data"), &PropInstance::get_prop_data);
	ClassDB::bind_method(D_METHOD("set_prop_data", "value"), &PropInstance::set_prop_data);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "prop_data", PROPERTY_HINT_RESOURCE_TYPE, "PropData"), "set_prop_data", "get_prop_data");

	ClassDB::bind_method(D_METHOD("get_job"), &PropInstance::get_job);
	ClassDB::bind_method(D_METHOD("set_job", "value"), &PropInstance::set_job);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "job", PROPERTY_HINT_RESOURCE_TYPE, "PropInstanceJob", 0), "set_job", "get_job");

#ifdef TEXTURE_PACKER_PRESENT
	ClassDB::bind_method(D_METHOD("get_merge_textures"), &PropInstance::get_merge_textures);
	ClassDB::bind_method(D_METHOD("set_merge_textures", "value"), &PropInstance::set_merge_textures);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "merge_textures"), "set_merge_textures", "get_merge_textures");
#endif

	///Materials
	ClassDB::bind_method(D_METHOD("material_get", "index"), &PropInstance::material_get);
	ClassDB::bind_method(D_METHOD("material_add", "value"), &PropInstance::material_add);
	ClassDB::bind_method(D_METHOD("material_get_num"), &PropInstance::material_get_num);
	ClassDB::bind_method(D_METHOD("materials_clear"), &PropInstance::materials_clear);

	ClassDB::bind_method(D_METHOD("materials_get"), &PropInstance::materials_get);
	ClassDB::bind_method(D_METHOD("materials_set"), &PropInstance::materials_set);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "materials", PROPERTY_HINT_NONE, "17/17:Material", PROPERTY_USAGE_DEFAULT, "Material"), "materials_set", "materials_get");

	//Meshes
	ClassDB::bind_method(D_METHOD("mesh_get", "index"), &PropInstance::mesh_get);
	ClassDB::bind_method(D_METHOD("mesh_add", "value"), &PropInstance::mesh_add);
	ClassDB::bind_method(D_METHOD("mesh_get_num"), &PropInstance::mesh_get_num);
	ClassDB::bind_method(D_METHOD("meshs_clear"), &PropInstance::meshs_clear);

	ClassDB::bind_method(D_METHOD("meshes_get"), &PropInstance::meshes_get);
	ClassDB::bind_method(D_METHOD("meshes_set"), &PropInstance::meshes_set);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "meshes", PROPERTY_HINT_NONE, "", 0), "meshes_set", "meshes_get");

	//Colliders
	ClassDB::bind_method(D_METHOD("collider_get", "index"), &PropInstance::collider_get);
	ClassDB::bind_method(D_METHOD("collider_add", "value"), &PropInstance::collider_add);
	ClassDB::bind_method(D_METHOD("collider_get_num"), &PropInstance::collider_get_num);
	ClassDB::bind_method(D_METHOD("colliders_clear"), &PropInstance::colliders_clear);

	ClassDB::bind_method(D_METHOD("colliders_get"), &PropInstance::colliders_get);
	ClassDB::bind_method(D_METHOD("colliders_set"), &PropInstance::colliders_set);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "colliders", PROPERTY_HINT_NONE, "", 0), "colliders_set", "colliders_get");

	//---
	ClassDB::bind_method(D_METHOD("get_first_lod_distance_squared"), &PropInstance::get_first_lod_distance_squared);
	ClassDB::bind_method(D_METHOD("set_first_lod_distance_squared", "value"), &PropInstance::set_first_lod_distance_squared);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "first_lod_distance_squared"), "set_first_lod_distance_squared", "get_first_lod_distance_squared");

	ClassDB::bind_method(D_METHOD("get_lod_reduction_distance_squared"), &PropInstance::get_lod_reduction_distance_squared);
	ClassDB::bind_method(D_METHOD("set_lod_reduction_distance_squared", "value"), &PropInstance::set_lod_reduction_distance_squared);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "lod_reduction_distance_squared"), "set_lod_reduction_distance_squared", "get_lod_reduction_distance_squared");

	//---
	ClassDB::bind_method(D_METHOD("free_meshes"), &PropInstance::free_meshes);
	ClassDB::bind_method(D_METHOD("free_colliders"), &PropInstance::free_colliders);

	//---
	BIND_VMETHOD(MethodInfo("_init_materials"));

	ClassDB::bind_method(D_METHOD("init_materials"), &PropInstance::init_materials);
	ClassDB::bind_method(D_METHOD("_init_materials"), &PropInstance::_init_materials);

	//---
	ClassDB::bind_method(D_METHOD("build"), &PropInstance::build);
	ClassDB::bind_method(D_METHOD("queue_build"), &PropInstance::queue_build);
	ClassDB::bind_method(D_METHOD("build_finished"), &PropInstance::build_finished);

	BIND_VMETHOD(MethodInfo("_build"));
	BIND_VMETHOD(MethodInfo("_build_finished"));

	ClassDB::bind_method(D_METHOD("_build"), &PropInstance::_build);
	ClassDB::bind_method(D_METHOD("_build_finished"), &PropInstance::_build_finished);
}
