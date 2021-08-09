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

#ifndef PROP_CACHE_H
#define PROP_CACHE_H

#include "core/version.h"

#if VERSION_MAJOR > 3
#include "core/object/object.h"
#include "core/object/reference.h"
#include "core/templates/hash_map.h"
#include "core/templates/vector.h"
#else
#include "core/hash_map.h"
#include "core/object.h"
#include "core/reference.h"
#include "core/vector.h"
#endif

#include "../props/prop_data.h"

#if TEXTURE_PACKER_PRESENT
class TexturePacker;
#endif

class PropCache : public Object {
	GDCLASS(PropCache, Object);

#if TEXTURE_PACKER_PRESENT
public:
	struct PropCacheEntry {
		int refcount;
		Ref<TexturePacker> merger;
		Ref<PropData> prop;
	};

public:
	static PropCache *get_singleton();

	String get_default_prop_material_cache_class();
	void set_default_prop_material_cache_class(const String& cls_name);

	bool has_texture(const Ref<PropData> &prop);
	void set_texture(const Ref<PropData> &prop, const Ref<TexturePacker> &merger);

	Ref<TexturePacker> get_texture(const Ref<PropData> &prop);

	void ref_texture(const Ref<PropData> &prop);
	void unref_texture(const Ref<PropData> &prop);

	Ref<TexturePacker> create_texture(const Ref<PropData> &prop);
	Ref<TexturePacker> get_or_create_texture_immediate(const Ref<PropData> &prop);
	Ref<TexturePacker> get_or_create_texture_threaded(const Ref<PropData> &prop);

private:
	Vector<PropCacheEntry> _entries;

	static PropCache *_instance;

#endif

public:
	PropCache();
	~PropCache();

protected:
	static void _bind_methods();

	String _default_prop_material_cache_class;
};

#endif
