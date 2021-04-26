#include "prop_instance.h"

#include "../mesh_data_resource/nodes/mesh_data_instance.h"

//#include "../thread_pool/thread_pool.h"

#include "core/version.h"

#if VERSION_MAJOR > 3
#include "core/config/engine.h"
#else
#include "core/engine.h"
#endif

Ref<PropData> PropInstance::get_prop_data() {
	return _prop_data;
}
void PropInstance::set_prop_data(const Ref<PropData> &data) {
	if (_prop_data == data)
		return;

	_prop_data = data;

	build();
}

bool PropInstance::get_auto_bake() const {
	return _auto_bake;
}
void PropInstance::set_auto_bake(const bool value) {
	_auto_bake = value;
}

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

void PropInstance::bake() {
		_baking = true;
	_bake_queued = false;

	if (!_job.is_valid()) {
		_job.instance();
	}

	_job->set_prop_instace(this);

}

void PropInstance::bake_finished() {
	_baking = false;

	if (_bake_queued) {
		call_deferred("bake");
	}
}

void PropInstance::queue_bake() {
	if (!_bake_queued) {
		_bake_queued = true;

		if (!_baking) {
			call_deferred("bake");
		}
	}
}

void PropInstance::build() {
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

	for (int i = 0; i < _prop_data->get_prop_count(); ++i) {
		Ref<PropDataEntry> e = _prop_data->get_prop(i);

		if (!e.is_valid())
			continue;

		Node *n = e->processor_get_node_for(get_transform());

		if (n) {
			add_child(n);

			//if (Engine::get_singleton()->is_editor_hint())
			//	n->set_owner(get_tree()->get_edited_scene_root());
		}
	}

}

PropInstance::PropInstance() {
	_baking = false;
	_bake_queued = false;
	_snap_to_mesh = false;
	_snap_axis = Vector3(0, -1, 0);
	//_job.instance();
	//_job->connect("completed", this, "bake_finished", Vector<Variant>(), Object::CONNECT_DEFERRED);
}
PropInstance::~PropInstance() {
	//_job.unref();
	_prop_data.unref();
}

void PropInstance::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			if (_prop_data.is_valid()) {
				build();
			}
		}
	}
}

void PropInstance::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_prop_data"), &PropInstance::get_prop_data);
	ClassDB::bind_method(D_METHOD("set_prop_data", "value"), &PropInstance::set_prop_data);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "prop_data", PROPERTY_HINT_RESOURCE_TYPE, "PropData"), "set_prop_data", "get_prop_data");

	ClassDB::bind_method(D_METHOD("get_auto_bake"), &PropInstance::get_auto_bake);
	ClassDB::bind_method(D_METHOD("set_auto_bake", "value"), &PropInstance::set_auto_bake);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_bake"), "set_auto_bake", "get_auto_bake");

	ClassDB::bind_method(D_METHOD("get_snap_to_mesh"), &PropInstance::get_snap_to_mesh);
	ClassDB::bind_method(D_METHOD("set_snap_to_mesh", "value"), &PropInstance::set_snap_to_mesh);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "snap_to_mesh"), "set_snap_to_mesh", "get_snap_to_mesh");

	ClassDB::bind_method(D_METHOD("get_snap_axis"), &PropInstance::get_snap_axis);
	ClassDB::bind_method(D_METHOD("set_snap_axis", "value"), &PropInstance::set_snap_axis);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "snap_axis"), "set_snap_axis", "get_snap_axis");

	ClassDB::bind_method(D_METHOD("bake"), &PropInstance::bake);
	ClassDB::bind_method(D_METHOD("queue_bake"), &PropInstance::queue_bake);
	ClassDB::bind_method(D_METHOD("bake_finished"), &PropInstance::bake_finished);
}
