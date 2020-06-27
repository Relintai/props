/*
Copyright (c) 2019-2020 Péter Magyar

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

#ifndef PROP_DATA_MESH_H
#define PROP_DATA_MESH_H

#include "core/math/vector3.h"
#include "prop_data_entry.h"

#include "scene/resources/texture.h"

#include "../../mesh_data_resource/mesh_data_resource.h"

class PropDataMesh : public PropDataEntry {
	GDCLASS(PropDataMesh, PropDataEntry);

public:
	Ref<MeshDataResource> get_mesh() const;
	void set_mesh(const Ref<MeshDataResource> mesh);

	Ref<Texture> get_texture() const;
	void set_texture(const Ref<Texture> texture);

	bool get_snap_to_mesh();
	void set_snap_to_mesh(bool value);

	Vector3 get_snap_axis();
	void set_snap_axis(Vector3 value);

	PropDataMesh();
	~PropDataMesh();

protected:
	static void _bind_methods();

private:
	bool _snap_to_mesh;
	Vector3 _snap_axis;
	Ref<MeshDataResource> _mesh;
	Ref<Texture> _texture;
};

#endif