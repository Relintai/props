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

#ifndef TILED_WALL_DATA_H
#define TILED_WALL_DATA_H

#include "core/version.h"

#if VERSION_MAJOR > 3
#include "core/object/reference.h"
#include "core/templates/vector.h"
#else
#include "core/reference.h"
#include "core/vector.h"
#endif

#include "core/math/rect2.h"
#include "core/math/transform.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"

#include "core/version.h"

#include "scene/resources/texture.h"

#if TEXTURE_PACKER_PRESENT
#include "../../texture_packer/texture_packer.h"
#endif

class TiledWallData : public Resource {
	GDCLASS(TiledWallData, Resource);

public:
	//textures
	Ref<Texture> get_texture(const int index) const;
	void set_texture(const int index, const Ref<Texture> &texture);
	void add_texture(const Ref<Texture> &texture);
	void remove_texture(const int index);

	int get_texture_count() const;

	Vector<Variant> get_textures();
	void set_textures(const Vector<Variant> &textures);

	//flavour_textures
	Ref<Texture> get_flavour_texture(const int index) const;
	void set_flavour_texture(const int index, const Ref<Texture> &texture);
	void add_flavour_texture(const Ref<Texture> &texture);
	void remove_flavour_texture(const int index);

	int get_flavour_texture_count() const;

	Vector<Variant> get_flavour_textures();
	void set_flavour_textures(const Vector<Variant> &textures);

#if TEXTURE_PACKER_PRESENT
	void add_textures_into(Ref<TexturePacker> texture_packer);
#endif

	void copy_from(const Ref<TiledWallData> &prop_data);

	TiledWallData();
	~TiledWallData();

protected:
	static void _bind_methods();

private:
	int _id;
	bool _snap_to_mesh;
	Vector3 _snap_axis;

	Vector<Ref<Texture>> _textures;
	Vector<Ref<Texture>> _flavour_textures;

	bool _is_room;
	PoolVector3Array _room_bounds;
};

#endif
