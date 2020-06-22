#include "prop_mesh_data_instance.h"

#include "prop_instance.h"

bool PropMeshDataInstance::get_snap_to_mesh() const {
	return _snap_to_mesh;
}
void PropMeshDataInstance::set_snap_to_mesh(const bool value) {
	_snap_to_mesh = value;
}

Vector3 PropMeshDataInstance::get_snap_axis() const {
	return _snap_axis;
}
void PropMeshDataInstance::set_snap_axis(const Vector3 &value) {
	_snap_axis = value;
}

Ref<MeshDataResource> PropMeshDataInstance::get_mesh() {
	return _mesh;
}
void PropMeshDataInstance::set_mesh(const Ref<MeshDataResource> &mesh) {
	_mesh = mesh;
}

Ref<Texture> PropMeshDataInstance::get_texture() {
	return _texture;
}
void PropMeshDataInstance::set_texture(const Ref<Texture> &texture) {
	_texture = texture;
}

PropMeshDataInstance::PropMeshDataInstance() {
	_snap_to_mesh = false;
	_snap_axis = Vector3(0, -1, 0);
}
PropMeshDataInstance::~PropMeshDataInstance() {
	_mesh.unref();
	_texture.unref();
}

void PropMeshDataInstance::notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			if (get_parent() == NULL)
				return;
		}
		//fallthrough
		case NOTIFICATION_PARENTED: {
			Node *n = this;

			while (n) {

				PropInstance *pi = Object::cast_to<PropInstance>(n);

				if (pi) {
					_owner = pi;
					pi->register_prop_mesh_data_instance(this);
					return;
				}

				n = n->get_parent();
			}

		} break;
		case NOTIFICATION_EXIT_TREE:
		case NOTIFICATION_UNPARENTED: {
			if (_owner) {
				_owner->unregister_prop_mesh_data_instance(this);
			}
			break;
		}
	}
}

void PropMeshDataInstance::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_snap_to_mesh"), &PropMeshDataInstance::get_snap_to_mesh);
	ClassDB::bind_method(D_METHOD("set_snap_to_mesh", "value"), &PropMeshDataInstance::set_snap_to_mesh);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "snap_to_mesh"), "set_snap_to_mesh", "get_snap_to_mesh");

	ClassDB::bind_method(D_METHOD("get_snap_axis"), &PropMeshDataInstance::get_snap_axis);
	ClassDB::bind_method(D_METHOD("set_snap_axis", "value"), &PropMeshDataInstance::set_snap_axis);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "snap_axis"), "set_snap_axis", "get_snap_axis");

	ClassDB::bind_method(D_METHOD("get_mesh"), &PropMeshDataInstance::get_mesh);
	ClassDB::bind_method(D_METHOD("set_mesh", "value"), &PropMeshDataInstance::set_mesh);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh", PROPERTY_HINT_RESOURCE_TYPE, "MeshDataResource"), "set_mesh", "get_mesh");

	ClassDB::bind_method(D_METHOD("get_texture"), &PropMeshDataInstance::get_texture);
	ClassDB::bind_method(D_METHOD("set_texture", "value"), &PropMeshDataInstance::set_texture);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_texture", "get_texture");
}
