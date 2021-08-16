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

#ifndef TILED_WALL_H
#define TILED_WALL_H

#include "core/version.h"
#include "scene/resources/texture.h"

#if VERSION_MAJOR < 4
#include "scene/3d/visual_instance.h"
#else
#include "scene/3d/node_3d.h"

#define SpatialMaterial StandardMaterial3D
#define Spatial Node3D
#define Texture Texture2D
#endif

#include "core/math/vector3.h"

class PropInstance;

class TiledWall : public GeometryInstance {
	GDCLASS(TiledWall, GeometryInstance);

public:
	int get_width() const;
	void set_width(const int value);

	int get_heigt() const;
	void set_heigt(const int value);

	//array textures
	//array flavour textures
	//this class->add merger
	//merge tex then:
	//mesher->draw_tiled_wall(width, height, transform, array texture_rects, array flavour texture_rects, int tilemode)
	//for propinstancemerger-> same, in diff thread
	//job->add_wall()->add in the propentry + a transform -> can be easily meshed

	//get_faces -> keep mesh in mesher -> query


	Ref<Texture> get_texture();
	void set_texture(const Ref<Texture> &texture);

	Ref<Material> get_material();
	void set_material(const Ref<Material> &mat);

	AABB get_aabb() const;
	PoolVector<Face3> get_faces(uint32_t p_usage_flags) const;

	void refresh();
	void setup_material_texture();
	void free_meshes();

	TiledWall();
	~TiledWall();

protected:
	void _notification(int p_what);
	static void _bind_methods();

private:
	int _width;
	int _height;
	
	Ref<Texture> _texture;
	Ref<Material> _material;

	RID _mesh_rid;
};

#endif
