/*
Copyright (c) 2020 Péter Magyar

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

#ifndef PROP_INSTANCE_JOB_H
#define PROP_INSTANCE_JOB_H

#include "core/version.h"

#if VERSION_MAJOR > 3
#include "core/templates/vector.h"
#else
#include "core/vector.h"
#endif

#include "core/math/vector3.h"

#include "../thread_pool/thread_pool_job.h"

#include "../mesh_data_resource/mesh_data_resource.h"
#include "scene/resources/texture.h"

class PropMeshDataInstance;

class PropInstanceJob : public ThreadPoolJob {
	GDCLASS(PropInstanceJob, ThreadPoolJob);

protected:
	struct PropMeshInstanceEntry {
		Transform transform;
		Ref<MeshDataResource> mesh;
		Ref<Texture> texture;
	};

public:
	Transform get_base_transform() const;
	void set_base_transform(const Transform &value);

	void add_mesh_instance(const Transform &transform, const Ref<MeshDataResource> &mesh, const Ref<Texture> &texture);
	int get_mesh_instance_count();

	void clear();

	void _execute();

	PropInstanceJob();
	~PropInstanceJob();

protected:
	static void _bind_methods();

private:
	Transform _base_transform;
	Vector<PropMeshInstanceEntry> _mesh_instances;
};

#endif
