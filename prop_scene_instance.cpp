#include "prop_scene_instance.h"

#include "core/version.h"

#if VERSION_MAJOR > 3
#include "core/config/engine.h"
#else
#include "core/engine.h"
#endif

Ref<PackedScene> PropSceneInstance::get_scene() {
	return _scene;
}
void PropSceneInstance::set_scene(const Ref<PackedScene> &data) {
	if (_scene == data)
		return;

	_scene = data;

	build();
}

bool PropSceneInstance::get_snap_to_mesh() const {
	return _snap_to_mesh;
}
void PropSceneInstance::set_snap_to_mesh(const bool value) {
	_snap_to_mesh = value;
}

Vector3 PropSceneInstance::get_snap_axis() const {
	return _snap_axis;
}
void PropSceneInstance::set_snap_axis(const Vector3 &value) {
	_snap_axis = value;
}

void PropSceneInstance::build() {
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

	if (!_scene.is_valid())
		return;

	Node *n = _scene->instance();

	add_child(n);

	//if (Engine::get_singleton()->is_editor_hint())
	//	n->set_owner(get_tree()->get_edited_scene_root());
}

PropSceneInstance::PropSceneInstance() {
	_snap_to_mesh = false;
	_snap_axis = Vector3(0, -1, 0);
}
PropSceneInstance::~PropSceneInstance() {
	_scene.unref();
}

void PropSceneInstance::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			build();
		}
	}
}

void PropSceneInstance::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_scene"), &PropSceneInstance::get_scene);
	ClassDB::bind_method(D_METHOD("set_scene", "value"), &PropSceneInstance::set_scene);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "scene", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"), "set_scene", "get_scene");

	ClassDB::bind_method(D_METHOD("get_snap_to_mesh"), &PropSceneInstance::get_snap_to_mesh);
	ClassDB::bind_method(D_METHOD("set_snap_to_mesh", "value"), &PropSceneInstance::set_snap_to_mesh);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "snap_to_mesh"), "set_snap_to_mesh", "get_snap_to_mesh");

	ClassDB::bind_method(D_METHOD("get_snap_axis"), &PropSceneInstance::get_snap_axis);
	ClassDB::bind_method(D_METHOD("set_snap_axis", "value"), &PropSceneInstance::set_snap_axis);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "snap_axis"), "set_snap_axis", "get_snap_axis");

	ClassDB::bind_method(D_METHOD("build"), &PropSceneInstance::build);
}
