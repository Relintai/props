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

#include "prop_cache.h"

#include "../props/prop_data.h"
#include "../props/prop_data_entry.h"

#include "core/version.h"

#if VERSION_MAJOR > 3
#include "core/config/engine.h"
#include "core/config/project_settings.h"
#else
#include "core/engine.h"
#include "core/project_settings.h"
#endif

#include "../jobs/prop_texture_job.h"

#if THREAD_POOL_PRESENT
#include "../../thread_pool/thread_pool.h"
#endif

#if TEXTURE_PACKER_PRESENT
#include "../../texture_packer/texture_packer.h"
#endif

PropCache *PropCache::_instance;

PropCache *PropCache::get_singleton() {
	return _instance;
}

String PropCache::get_default_prop_material_cache_class() {
	return _default_prop_material_cache_class;
}
void PropCache::set_default_prop_material_cache_class(const String &cls_name) {
	_default_prop_material_cache_class = cls_name;
}

#if TEXTURE_PACKER_PRESENT
bool PropCache::has_texture(const Ref<PropData> &prop) {
	for (int i = 0; i < _entries.size(); ++i) {
		if (_entries[i].prop == prop) {
			return true;
		}
	}

	return false;
}

void PropCache::set_texture(const Ref<PropData> &prop, const Ref<TexturePacker> &merger) {
	for (int i = 0; i < _entries.size(); ++i) {
		PropCacheEntry &e = _entries.write[i];

		if (e.prop == prop) {
			e.merger = merger;
		}
	}
}

Ref<TexturePacker> PropCache::get_texture(const Ref<PropData> &prop) {
	for (int i = 0; i < _entries.size(); ++i) {
		PropCacheEntry &e = _entries.write[i];

		if (e.prop == prop) {
			e.refcount++;

			return e.merger;
		}
	}

	return Ref<TexturePacker>();
}

void PropCache::ref_texture(const Ref<PropData> &prop) {
	for (int i = 0; i < _entries.size(); ++i) {
		PropCacheEntry &e = _entries.write[i];

		if (e.prop == prop) {
			e.refcount++;

			return;
		}
	}
}

void PropCache::unref_texture(const Ref<PropData> &prop) {
	for (int i = 0; i < _entries.size(); ++i) {
		PropCacheEntry &e = _entries.write[i];

		if (e.prop == prop) {
			e.refcount--;

			if (e.refcount <= 0) {
				_entries.remove(i);
			}

			return;
		}
	}
}

Ref<TexturePacker> PropCache::create_texture(const Ref<PropData> &prop) {
	ERR_FAIL_COND_V(has_texture(prop), Ref<TexturePacker>());

	Ref<TexturePacker> merger;
	merger.instance();

	for (int i = 0; i < prop->get_prop_count(); ++i) {
		Ref<PropDataEntry> e = prop->get_prop(i);

		e->add_textures_into(merger);
	}

	PropCacheEntry e;
	e.merger = merger;
	e.prop = prop;
	e.refcount = 1;

	_entries.push_back(e);

	return merger;
}

Ref<TexturePacker> PropCache::get_or_create_texture_immediate(const Ref<PropData> &prop) {
	if (!has_texture(prop)) {
		Ref<TexturePacker> merger = create_texture(prop);

		merger->merge();

		return merger;
	}

	return get_texture(prop);
}

Ref<TexturePacker> PropCache::get_or_create_texture_threaded(const Ref<PropData> &prop) {
#if THREAD_POOL_PRESENT

	if (!has_texture(prop)) {
		Ref<TexturePacker> merger = create_texture(prop);

		Ref<PropTextureJob> job;
		job.instance();
		job->set_merger(merger);
		ThreadPool::get_singleton()->add_job(job);

		return merger;
	}

	return get_texture(prop);

#else
	return get_or_create_texture_immediate(prop);
#endif
}

#endif

PropCache::PropCache() {
	_instance = this;

#if TEXTURE_PACKER_PRESENT
	_default_prop_material_cache_class = GLOBAL_DEF("props/default_prop_material_cache_class", "PropMaterialCachePCM");
#else
	_default_prop_material_cache_class = GLOBAL_DEF("props/default_prop_material_cache_class", "PropMaterialCache");
#endif
}

PropCache::~PropCache() {
	_instance = NULL;
#if TEXTURE_PACKER_PRESENT
	_entries.clear();
#endif
}

void PropCache::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_default_prop_material_cache_class"), &PropCache::get_default_prop_material_cache_class);
	ClassDB::bind_method(D_METHOD("set_default_prop_material_cache_class", "cls_name"), &PropCache::set_default_prop_material_cache_class);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "default_prop_material_cache_class"), "set_default_prop_material_cache_class", "get_default_prop_material_cache_class");

#if TEXTURE_PACKER_PRESENT
	ClassDB::bind_method(D_METHOD("has_texture", "prop"), &PropCache::has_texture);
	ClassDB::bind_method(D_METHOD("set_texture", "prop", "merger"), &PropCache::set_texture);

	ClassDB::bind_method(D_METHOD("get_texture", "prop"), &PropCache::get_texture);

	ClassDB::bind_method(D_METHOD("ref_texture", "prop"), &PropCache::ref_texture);
	ClassDB::bind_method(D_METHOD("unref_texture", "prop"), &PropCache::unref_texture);

	ClassDB::bind_method(D_METHOD("create_texture", "prop"), &PropCache::create_texture);
	ClassDB::bind_method(D_METHOD("get_or_create_texture_immediate", "prop"), &PropCache::get_or_create_texture_immediate);
	ClassDB::bind_method(D_METHOD("get_or_create_texture_threaded", "prop"), &PropCache::get_or_create_texture_threaded);
#endif
}
