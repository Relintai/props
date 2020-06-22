#include "prop_instance_job.h"

Transform PropInstanceJob::get_base_transform() const {
	return _base_transform;
}
void PropInstanceJob::set_base_transform(const Transform &value) {
	_base_transform = value;
}

void PropInstanceJob::add_mesh_instance(const Transform &transform, const Ref<MeshDataResource> &mesh, const Ref<Texture> &texture) {
	PropMeshInstanceEntry m;
	m.mesh = mesh;
	m.texture = texture;
	m.transform = transform;

	_mesh_instances.push_back(m);
}

int PropInstanceJob::get_mesh_instance_count() {
	return _mesh_instances.size();
}

void PropInstanceJob::clear() {
	_mesh_instances.clear();
}

void PropInstanceJob::_execute() {

	emit_signal("completed");
}

PropInstanceJob::PropInstanceJob() {
}
PropInstanceJob::~PropInstanceJob() {
	_mesh_instances.clear();
}

void PropInstanceJob::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_base_transform"), &PropInstanceJob::get_base_transform);
	ClassDB::bind_method(D_METHOD("set_base_transform", "value"), &PropInstanceJob::set_base_transform);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "base_transform"), "set_base_transform", "get_base_transform");

	ClassDB::bind_method(D_METHOD("_execute"), &PropInstanceJob::_execute);
}
