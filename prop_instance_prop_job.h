/*
Copyright (c) 2019-2021 Péter Magyar

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

#ifndef PROP_JOB_H
#define PROP_JOB_H

#include "prop_instance_job.h"

#if TEXTURE_PACKER_PRESENT
class TexturePacker;
#endif

class PropMesher;
class PropInstance;
class PropInstanceMerger;
class PropMesherJobStep;

#if MESH_DATA_RESOURCE_PRESENT
class PropDataMeshData;
#endif

class PropInstancePropJob : public PropInstanceJob {
	GDCLASS(PropInstancePropJob, PropInstanceJob);

public:
#if TEXTURE_PACKER_PRESENT
	Ref<TexturePacker> get_texture_packer();
	void set_texture_packer(const Ref<TexturePacker> &packer);
#endif

	Ref<PropMesherJobStep> get_jobs_step(const int index) const;
	void set_jobs_step(const int index, const Ref<PropMesherJobStep> &step);
	void remove_jobs_step(const int index);
	void add_jobs_step(const Ref<PropMesherJobStep> &step);
	int get_jobs_step_count() const;

	PropInstanceMerger *get_prop_instace();
	void set_prop_instace(PropInstanceMerger *prop);
	void set_prop_instace_bind(Node *prop);

	Ref<PropMesher> get_prop_mesher() const;
	void set_prop_mesher(const Ref<PropMesher> &mesher);

#if MESH_DATA_RESOURCE_PRESENT
	void add_mesh(const Ref<PropDataMeshData> &mesh_data, const Transform &base_transform);
	void clear_meshes();
#endif

	void phase_physics_process();
	void phase_prop();

	void _physics_process(float delta);
	void _execute_phase();
	void _reset();

	PropInstancePropJob();
	~PropInstancePropJob();

protected:
	static void _bind_methods();

protected:
#if MESH_DATA_RESOURCE_PRESENT
	struct PMDREntry {
		Ref<PropDataMeshData> mesh_data;
		Transform base_transform;
	};
#endif

#if TEXTURE_PACKER_PRESENT
	Ref<TexturePacker> _texture_packer;
#endif

	Vector<Ref<PropMesherJobStep>> _job_steps;
	int _current_job_step;
	int _current_mesh;

	Ref<PropMesher> _prop_mesher;
	PropInstanceMerger *_prop_instace;

#if MESH_DATA_RESOURCE_PRESENT
	Vector<PMDREntry> _prop_mesh_datas;
#endif

	Array temp_mesh_arr;
};

#endif
