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

#include "prop_texture_cache.h"

#include "../props/prop_data.h"
#include "../props/prop_data_entry.h"

#include "core/version.h"

#if VERSION_MAJOR > 3
#include "core/config/engine.h"
#else
#include "core/engine.h"
#endif

#include "../jobs/prop_texture_job.h"

#if THREAD_POOL_PRESENT
#include "../../thread_pool/thread_pool.h"
#endif

#if TEXTURE_PACKER_PRESENT
#include "../../texture_packer/texture_packer.h"
#endif

PropTextureCache *PropTextureCache::_instance;

PropTextureCache *PropTextureCache::get_singleton() {
	return _instance;
}

#if TEXTURE_PACKER_PRESENT
bool PropTextureCache::has_texture(const Ref<PropData> &prop) {
	for (int i = 0; i < _entries.size(); ++i) {
		if (_entries[i].prop == prop) {
			return true;
		}
	}

	return false;
}

void PropTextureCache::set_texture(const Ref<PropData> &prop, const Ref<TexturePacker> &merger) {
	for (int i = 0; i < _entries.size(); ++i) {
		PropTextureCacheEntry &e = _entries.write[i];

		if (e.prop == prop) {
			e.merger = merger;
		}
	}
}

Ref<TexturePacker> PropTextureCache::get_texture(const Ref<PropData> &prop) {
	for (int i = 0; i < _entries.size(); ++i) {
		PropTextureCacheEntry &e = _entries.write[i];

		if (e.prop == prop) {
			e.refcount++;

			return e.merger;
		}
	}

	return Ref<TexturePacker>();
}

void PropTextureCache::ref_texture(const Ref<PropData> &prop) {
	for (int i = 0; i < _entries.size(); ++i) {
		PropTextureCacheEntry &e = _entries.write[i];

		if (e.prop == prop) {
			e.refcount++;

			return;
		}
	}
}

void PropTextureCache::unref_texture(const Ref<PropData> &prop) {
	for (int i = 0; i < _entries.size(); ++i) {
		PropTextureCacheEntry &e = _entries.write[i];

		if (e.prop == prop) {
			e.refcount--;

			if (e.refcount <= 0) {
				_entries.remove(i);
			}

			return;
		}
	}
}

Ref<TexturePacker> PropTextureCache::create_texture(const Ref<PropData> &prop) {
	ERR_FAIL_COND_V(has_texture(prop), Ref<TexturePacker>());

	Ref<TexturePacker> merger;
	merger.instance();

	for (int i = 0; i < prop->get_prop_count(); ++i) {
		Ref<PropDataEntry> e = prop->get_prop(i);

		e->add_textures_into(merger);
	}

	PropTextureCacheEntry e;
	e.merger = merger;
	e.prop = prop;
	e.refcount = 1;

	_entries.push_back(e);

	return merger;
}

Ref<TexturePacker> PropTextureCache::get_or_create_texture_immediate(const Ref<PropData> &prop) {
	if (!has_texture(prop)) {

		Ref<TexturePacker> merger = create_texture(prop);

		merger->merge();

		return merger;
	}

	return get_texture(prop);
}

Ref<TexturePacker> PropTextureCache::get_or_create_texture_threaded(const Ref<PropData> &prop) {
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

PropTextureCache::PropTextureCache() {
	_instance = this;
}

PropTextureCache::~PropTextureCache() {
	_instance = NULL;
#if TEXTURE_PACKER_PRESENT
	_entries.clear();
#endif
}

void PropTextureCache::_bind_methods() {
#if TEXTURE_PACKER_PRESENT
	ClassDB::bind_method(D_METHOD("has_texture", "prop"), &PropTextureCache::has_texture);
	ClassDB::bind_method(D_METHOD("set_texture", "prop", "merger"), &PropTextureCache::set_texture);

	ClassDB::bind_method(D_METHOD("get_texture", "prop"), &PropTextureCache::get_texture);

	ClassDB::bind_method(D_METHOD("ref_texture", "prop"), &PropTextureCache::ref_texture);
	ClassDB::bind_method(D_METHOD("unref_texture", "prop"), &PropTextureCache::unref_texture);

	ClassDB::bind_method(D_METHOD("create_texture", "prop"), &PropTextureCache::create_texture);
	ClassDB::bind_method(D_METHOD("get_or_create_texture_immediate", "prop"), &PropTextureCache::get_or_create_texture_immediate);
	ClassDB::bind_method(D_METHOD("get_or_create_texture_threaded", "prop"), &PropTextureCache::get_or_create_texture_threaded);
#endif
}
