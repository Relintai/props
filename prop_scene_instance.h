/*
Copyright (c) 2020-2022 Péter Magyar

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

#ifndef PROP_SCENE_INSTANCE_H
#define PROP_SCENE_INSTANCE_H

#include "core/version.h"

#if VERSION_MAJOR < 4
#include "scene/3d/spatial.h"
#else
#include "scene/3d/node_3d.h"

#define Spatial Node3D
#endif

#include "core/math/vector3.h"

#include "props/prop_data.h"
#include "scene/resources/packed_scene.h"

class PropSceneInstance : public Spatial {
	GDCLASS(PropSceneInstance, Spatial);

public:
	Ref<PackedScene> get_scene();
	void set_scene(const Ref<PackedScene> &data);

	bool get_snap_to_mesh() const;
	void set_snap_to_mesh(const bool value);

	Vector3 get_snap_axis() const;
	void set_snap_axis(const Vector3 &value);

	void build();

	PropSceneInstance();
	~PropSceneInstance();

protected:
	void _notification(int p_what);
	static void _bind_methods();

private:
	Ref<PackedScene> _scene;
	bool _snap_to_mesh;
	Vector3 _snap_axis;
};

#endif
