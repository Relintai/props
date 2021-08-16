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

#include "scene/3d/mesh_instance.h"

int TiledWall::get_width() const {
	return _width;
}
void TiledWall::set_width(const int value) {
	_width = value;
}

int TiledWall::get_heigt() const {
	return _height;
}
void TiledWall::set_heigt(const int value) {
	_height = value;
}

Ref<Texture> TiledWall::get_texture() {
	return _texture;
}
void TiledWall::set_texture(const Ref<Texture> &texture) {
	_texture = texture;

	setup_material_texture();
	refresh();
}

Ref<Material> TiledWall::get_material() {
	return _material;
}
void TiledWall::set_material(const Ref<Material> &mat) {
	_material = mat;

	setup_material_texture();
	refresh();
}

AABB TiledWall::get_aabb() const {
	return AABB();
}

PoolVector<Face3> TiledWall::get_faces(uint32_t p_usage_flags) const {
	PoolVector<Face3> faces;
/*
	if (_mesh.is_valid()) {
		Array arrs = _mesh->get_array_const();

		if (arrs.size() != Mesh::ARRAY_MAX) {
			return faces;
		}

		PoolVector<Vector3> vertices = arrs[Mesh::ARRAY_VERTEX];
		PoolVector<int> indices = arrs[Mesh::ARRAY_INDEX];

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
	}
*/
	return faces;
}

void TiledWall::refresh() {
	if (!is_inside_tree()) {
		return;
	}
/*
	if (_mesh_rid == RID()) {
		_mesh_rid = VisualServer::get_singleton()->mesh_create();

		VS::get_singleton()->instance_set_base(get_instance(), _mesh_rid);
	}

	VisualServer::get_singleton()->mesh_clear(_mesh_rid);

	if (!_mesh.is_valid()) {
		return;
	}

	Array arr = _mesh->get_array();

	if (arr.size() != Mesh::ARRAY_MAX) {
		return;
	}

	PoolVector<Vector3> vertices = arr[Mesh::ARRAY_VERTEX];

	if (vertices.size() == 0) {
		return;
	}

	VisualServer::get_singleton()->mesh_add_surface_from_arrays(_mesh_rid, VisualServer::PRIMITIVE_TRIANGLES, arr);

	if (_material.is_valid()) {
		VisualServer::get_singleton()->mesh_surface_set_material(_mesh_rid, 0, _material->get_rid());
	}
	*/
}

void TiledWall::setup_material_texture() {
	if (!is_inside_tree()) {
		return;
	}

	if (!_texture.is_valid()) {
		if (_material.is_valid()) {
			Ref<SpatialMaterial> sm = _material;

			if (!sm.is_valid()) {
				return;
			}

			sm->set_texture(SpatialMaterial::TEXTURE_ALBEDO, _texture);
		}

		return;
	} else {
		Ref<SpatialMaterial> sm = _material;

		if (!sm.is_valid()) {
			return;
		}

#if TEXTURE_PACKER_PRESENT
		Ref<PackerImageResource> r = _texture;

		if (r.is_valid()) {
			Ref<Image> i = r->get_data();

			Ref<ImageTexture> tex;
			tex.instance();
#if VERSION_MAJOR < 4
			tex->create_from_image(i, 0);
#else
			tex->create_from_image(i);
#endif

			if (sm.is_valid()) {
				sm->set_texture(SpatialMaterial::TEXTURE_ALBEDO, tex);
			}

			return;
		}
#endif

		sm->set_texture(SpatialMaterial::TEXTURE_ALBEDO, _texture);
	}
}

void TiledWall::free_meshes() {
	if (_mesh_rid != RID()) {
		VS::get_singleton()->free(_mesh_rid);
		_mesh_rid = RID();
	}
}

TiledWall::TiledWall() {
	_width = 1;
	_height = 1;

	//temporary
	set_portal_mode(PORTAL_MODE_GLOBAL);

	//set_notify_transform(true);
}
TiledWall::~TiledWall() {
	_texture.unref();
}

void TiledWall::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			setup_material_texture();
			refresh();

			break;
		}
		case NOTIFICATION_EXIT_TREE: {
			free_meshes();
			break;
		}
			/*
		case NOTIFICATION_TRANSFORM_CHANGED: {
			VisualServer *vs = VisualServer::get_singleton();

			vs->instance_set_transform(get_instance(), get_global_transform());

			break;
		}*/
	}
}

void TiledWall::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_width"), &TiledWall::get_width);
	ClassDB::bind_method(D_METHOD("set_width", "value"), &TiledWall::set_width);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "width"), "set_width", "get_width");

	ClassDB::bind_method(D_METHOD("get_heigt"), &TiledWall::get_heigt);
	ClassDB::bind_method(D_METHOD("set_heigt", "value"), &TiledWall::set_heigt);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "heigt"), "set_heigt", "get_heigt");

	ClassDB::bind_method(D_METHOD("get_texture"), &TiledWall::get_texture);
	ClassDB::bind_method(D_METHOD("set_texture", "value"), &TiledWall::set_texture);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_texture", "get_texture");

	ClassDB::bind_method(D_METHOD("get_material"), &TiledWall::get_material);
	ClassDB::bind_method(D_METHOD("set_material", "value"), &TiledWall::set_material);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material", PROPERTY_HINT_RESOURCE_TYPE, "Material"), "set_material", "get_material");

	ClassDB::bind_method(D_METHOD("refresh"), &TiledWall::refresh);
}
