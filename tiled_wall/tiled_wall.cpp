#include "tiled_wall.h"

#include "core/version.h"

#include "core/version.h"
#include "scene/resources/texture.h"

#if VERSION_MAJOR < 4
#include "core/image.h"

#define GET_WORLD get_world
#else
#include "core/io/image.h"

#define GET_WORLD get_world_3d
#endif

#if TEXTURE_PACKER_PRESENT
#include "../../texture_packer/texture_resource/packer_image_resource.h"
#endif

#include "../material_cache/prop_material_cache.h"
#include "../prop_mesher.h"
#include "../singleton/prop_cache.h"

#include "core/core_string_names.h"
#include "tiled_wall_data.h"

int TiledWall::get_width() const {
	return _width;
}
void TiledWall::set_width(const int value) {
	_width = value;

	clear_mesh();
	generate_mesh();
}

int TiledWall::get_heigth() const {
	return _height;
}
void TiledWall::set_heigth(const int value) {
	_height = value;

	clear_mesh();
	generate_mesh();
}

Ref<TiledWallData> TiledWall::get_data() {
	return _data;
}
void TiledWall::set_data(const Ref<TiledWallData> &data) {
	if (_data.is_valid()) {
		_data->disconnect(CoreStringNames::get_singleton()->changed, this, "refresh");
	}

	_data = data;

	if (_data.is_valid()) {
		_data->connect(CoreStringNames::get_singleton()->changed, this, "refresh");
	}

	call_deferred("refresh");
}

bool TiledWall::get_collision() const {
	return _collision;
}
void TiledWall::set_collision(const int value) {
	_collision = value;

/*
	if (!is_inside_tree()) {
		return;
	}

	if (_collision) {
		create_colliders();
	} else {
		free_colliders();
	}*/
}

AABB TiledWall::get_aabb() const {
	return AABB();
}

PoolVector<Face3> TiledWall::get_faces(uint32_t p_usage_flags) const {
	PoolVector<Face3> faces;

	if (_mesh_array.size() != Mesh::ARRAY_MAX) {
		return faces;
	}

	PoolVector<Vector3> vertices = _mesh_array[Mesh::ARRAY_VERTEX];
	PoolVector<int> indices = _mesh_array[Mesh::ARRAY_INDEX];

	int ts = indices.size() / 3;
	faces.resize(ts);

	PoolVector<Face3>::Write w = faces.write();
	PoolVector<Vector3>::Read rv = vertices.read();
	PoolVector<int>::Read ri = indices.read();

	for (int i = 0; i < ts; i++) {
		int im3 = (i * 3);

		for (int j = 0; j < 3; j++) {
			w[i].vertex[j] = rv[indices[im3 + j]];
		}
	}

	w.release();

	return faces;
}

void TiledWall::refresh() {
	if (!is_inside_tree()) {
		return;
	}

	clear_mesh();
/*
	if (_physics_shape_rid != RID()) {
		PhysicsServer::get_singleton()->shape_set_data(_physics_shape_rid, Vector3(_width / 2.0, _height / 2.0, 0.01));

		Transform t = get_global_transform();
		t.translate(Vector3(_width / 2.0, _height / 2.0, 0));
		PhysicsServer::get_singleton()->body_set_state(_physics_body_rid, PhysicsServer::BODY_STATE_TRANSFORM, t);
	}
*/
	if (!_data.is_valid()) {
		return;
	}

	if (_mesh_rid == RID()) {
		_mesh_rid = VisualServer::get_singleton()->mesh_create();

		VS::get_singleton()->instance_set_base(get_instance(), _mesh_rid);
	}

	Ref<PropMaterialCache> old_cache;

	old_cache = _cache;

	_cache = PropCache::get_singleton()->tiled_wall_material_cache_get(_data);

	if (old_cache.is_valid() && old_cache != _cache) {
		PropCache::get_singleton()->tiled_wall_material_cache_unref(old_cache);
	}

	if (!_cache->get_initialized()) {
		_cache->mutex_lock();

		//An anouther thread could have initialized it before wo got the mutex!
		if (!_cache->get_initialized()) {
			_cache->initial_setup_default();

			_data->setup_cache(_cache);

			_cache->refresh_rects();
		}

		_cache->mutex_unlock();
	}

	generate_mesh();
}

void TiledWall::generate_mesh() {
	if (!_data.is_valid()) {
		return;
	}

	if (!_cache.is_valid()) {
		return;
	}
/*
	if (_physics_shape_rid != RID()) {
		PhysicsServer::get_singleton()->shape_set_data(_physics_shape_rid, Vector3(_width / 2.0, _height / 2.0, 0.01));

		Transform t = get_global_transform();
		t.translate(Vector3(_width / 2.0, _height / 2.0, 0));
		PhysicsServer::get_singleton()->body_set_state(_physics_body_rid, PhysicsServer::BODY_STATE_TRANSFORM, t);
	}
*/
	//_mesher->tiled wall mesh_wimple(w, h, TWD, cache)
	_mesher->add_tiled_wall_simple(_width, _height, Transform(), _data, _cache);

	_mesh_array = _mesher->build_mesh();

	if (_mesh_array.size() != Mesh::ARRAY_MAX) {
		return;
	}

	PoolVector<Vector3> vertices = _mesh_array[Mesh::ARRAY_VERTEX];

	if (vertices.size() == 0) {
		return;
	}

	VisualServer::get_singleton()->mesh_add_surface_from_arrays(_mesh_rid, VisualServer::PRIMITIVE_TRIANGLES, _mesh_array);

	Ref<Material> material = _cache->material_lod_get(0);

	if (material.is_valid()) {
		VisualServer::get_singleton()->mesh_surface_set_material(_mesh_rid, 0, material->get_rid());
	}

	_aabb.size = Vector3(_width, _height, 0);
}

void TiledWall::clear_mesh() {
	_mesher->reset();
	_aabb = AABB();
	_mesh_array.clear();

	if (_mesh_rid != RID()) {
		if (VS::get_singleton()->mesh_get_surface_count(_mesh_rid) > 0)
#if VERSION_MAJOR < 4
			VS::get_singleton()->mesh_remove_surface(_mesh_rid, 0);
#else
			VS::get_singleton()->mesh_clear(_mesh_rid);
#endif
	}
}

void TiledWall::free_mesh() {
	if (_mesh_rid != RID()) {
		VS::get_singleton()->free(_mesh_rid);
		_mesh_rid = RID();
	}
}

void TiledWall::create_colliders() {
	if (!is_inside_tree()) {
		return;
	}

	free_colliders();

	ERR_FAIL_COND(!get_world().is_valid() && get_world()->get_space() == RID());

	_physics_shape_rid = PhysicsServer::get_singleton()->shape_create(PhysicsServer::SHAPE_BOX);
	_physics_body_rid = PhysicsServer::get_singleton()->body_create(PhysicsServer::BODY_MODE_STATIC);

	PhysicsServer::get_singleton()->shape_set_data(_physics_shape_rid, Vector3(_width / 2.0, _height / 2.0, 0.01));

	//todo layer mask
	PhysicsServer::get_singleton()->body_set_collision_layer(_physics_body_rid, 1);
	PhysicsServer::get_singleton()->body_set_collision_mask(_physics_body_rid, 1);

	PhysicsServer::get_singleton()->body_add_shape(_physics_body_rid, _physics_shape_rid);

	Transform t = get_global_transform();
	t.translate(Vector3(_width / 2.0, _height / 2.0, 0));

	PhysicsServer::get_singleton()->body_set_state(_physics_body_rid, PhysicsServer::BODY_STATE_TRANSFORM, t);

	PhysicsServer::get_singleton()->body_set_space(_physics_body_rid, get_world()->get_space());
}

void TiledWall::free_colliders() {
	if (_physics_body_rid != RID()) {
		PhysicsServer::get_singleton()->free(_physics_body_rid);

		_physics_body_rid = RID();
	}

	if (_physics_shape_rid != RID()) {
		PhysicsServer::get_singleton()->free(_physics_shape_rid);

		_physics_shape_rid = RID();
	}
}

TiledWall::TiledWall() {
	_width = 1;
	_height = 1;
	_collision = true;

	//temporary
	set_portal_mode(PORTAL_MODE_GLOBAL);

	_mesher.instance();
}
TiledWall::~TiledWall() {
	_data.unref();
	_cache.unref();
	_mesher.unref();
}

void TiledWall::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			//if (_collision) {
			//	call_deferred("create_colliders");
			//}

			refresh();

			break;
		}
		case NOTIFICATION_EXIT_TREE: {
			free_mesh();

			//if (_collision) {
			//	free_colliders();
			//}

			break;
		}
	//	case NOTIFICATION_TRANSFORM_CHANGED: {
			/*
			if (_physics_body_rid != RID()) {
				Transform t = get_global_transform();
				t.translate(Vector3(_width / 2.0, _height / 2.0, 0));

				PhysicsServer::get_singleton()->body_set_state(_physics_body_rid, PhysicsServer::BODY_STATE_TRANSFORM, t);
			}
*/
		//	break;
	//	}
	}
}

void TiledWall::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_width"), &TiledWall::get_width);
	ClassDB::bind_method(D_METHOD("set_width", "value"), &TiledWall::set_width);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "width"), "set_width", "get_width");

	ClassDB::bind_method(D_METHOD("get_heigth"), &TiledWall::get_heigth);
	ClassDB::bind_method(D_METHOD("set_heigth", "value"), &TiledWall::set_heigth);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "heigth"), "set_heigth", "get_heigth");

	ClassDB::bind_method(D_METHOD("get_data"), &TiledWall::get_data);
	ClassDB::bind_method(D_METHOD("set_data", "value"), &TiledWall::set_data);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "data", PROPERTY_HINT_RESOURCE_TYPE, "TiledWallData"), "set_data", "get_data");

	ClassDB::bind_method(D_METHOD("get_collision"), &TiledWall::get_collision);
	ClassDB::bind_method(D_METHOD("set_collision", "value"), &TiledWall::set_collision);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "collision"), "set_collision", "get_collision");

	ClassDB::bind_method(D_METHOD("refresh"), &TiledWall::refresh);
	ClassDB::bind_method(D_METHOD("generate_mesh"), &TiledWall::generate_mesh);
	ClassDB::bind_method(D_METHOD("clear_mesh"), &TiledWall::clear_mesh);
	ClassDB::bind_method(D_METHOD("free_mesh"), &TiledWall::free_mesh);

	ClassDB::bind_method(D_METHOD("create_colliders"), &TiledWall::create_colliders);
	ClassDB::bind_method(D_METHOD("free_colliders"), &TiledWall::free_colliders);
}
