#include "prop_instance.h"

bool PropInstance::get_snap_to_mesh() const {
	return _snap_to_mesh;
}
void PropInstance::set_snap_to_mesh(const bool value) {
	_snap_to_mesh = value;
}

Vector3 PropInstance::get_snap_axis() const {
	return _snap_axis;
}
void PropInstance::set_snap_axis(const Vector3 &value) {
	_snap_axis = value;
}

PropInstance::PropInstance() {
	_snap_to_mesh = false;
	_snap_axis = Vector3(0, -1, 0);
}
PropInstance::~PropInstance() {
}

void PropInstance::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_snap_to_mesh"), &PropInstance::get_snap_to_mesh);
	ClassDB::bind_method(D_METHOD("set_snap_to_mesh", "value"), &PropInstance::set_snap_to_mesh);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "snap_to_mesh"), "set_snap_to_mesh", "get_snap_to_mesh");

	ClassDB::bind_method(D_METHOD("get_snap_axis"), &PropInstance::get_snap_axis);
	ClassDB::bind_method(D_METHOD("set_snap_axis", "value"), &PropInstance::set_snap_axis);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "snap_axis"), "set_snap_axis", "get_snap_axis");
}
