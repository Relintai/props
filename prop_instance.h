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

#ifndef PROP_INSTANCE_H
#define PROP_INSTANCE_H

#include "core/version.h"

#if VERSION_MAJOR < 4
#include "scene/3d/spatial.h"
#else
#include "scene/3d/node_3d.h"

#define Spatial Node3D
#endif

#include "core/math/vector3.h"

#include "prop_instance_prop_job.h"

#include "props/prop_data.h"

class MeshDataInstance;

class PropInstance : public Spatial {
	GDCLASS(PropInstance, Spatial);

public:
	Ref<PropData> get_prop_data();
	void set_prop_data(const Ref<PropData> &data);

	Ref<PropInstanceJob> get_job();
	void set_job(const Ref<PropInstanceJob> &job);

#ifdef TEXTURE_PACKER_PRESENT
	bool get_merge_textures() const;
	void set_merge_textures(const bool value);
#endif

	///Materials
	Ref<Material> material_get(const int index);
	void material_add(const Ref<Material> &value);
	int material_get_num() const;
	void materials_clear();

	Vector<Variant> materials_get();
	void materials_set(const Vector<Variant> &materials);

	//Meshes
	RID mesh_get(const int index);
	void mesh_add(const RID value);
	int mesh_get_num() const;
	void meshs_clear();

	Vector<Variant> meshes_get();
	void meshes_set(const Vector<Variant> &meshes);

	//Colliders
	RID collider_get(const int index);
	void collider_add(const RID value);
	int collider_get_num() const;
	void colliders_clear();

	Vector<Variant> colliders_get();
	void colliders_set(const Vector<Variant> &colliders);

	float get_first_lod_distance_squared();
	void set_first_lod_distance_squared(const float dist);

	float get_lod_reduction_distance_squared();
	void set_lod_reduction_distance_squared(const float dist);

	void free_meshes();
	void free_colliders();

	void init_materials();
	virtual void _init_materials();

	void build();
	void queue_build();
	void build_finished();
	virtual void _build_finished();

	PropInstance();
	~PropInstance();

protected:
	void _notification(int p_what);
	static void _bind_methods();

private:
	Ref<PropData> _prop_data;

	bool _build_queued;
	bool _building;

	Ref<PropInstancePropJob> _job;

#ifdef TEXTURE_PACKER_PRESENT
	bool _merge_textures;
#endif

	Vector<Ref<Material> > _materials;
	Vector<RID> _meshes;
	Vector<RID> _colliders;

	float _first_lod_distance_squared;
	float _lod_reduction_distance_squared;
};

#endif
