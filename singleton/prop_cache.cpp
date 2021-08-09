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

#include "../material_cache/prop_material_cache.h"

#include "core/hashfuncs.h"

PropCache *PropCache::_instance;

PropCache *PropCache::get_singleton() {
	return _instance;
}

StringName PropCache::get_default_prop_material_cache_class() {
	return _default_prop_material_cache_class;
}
void PropCache::set_default_prop_material_cache_class(const StringName &cls_name) {
	_default_prop_material_cache_class = cls_name;
}

Ref<PropMaterialCache> PropCache::material_cache_get(const Ref<PropData> &prop) {
	ERR_FAIL_COND_V(!prop.is_valid(), Ref<PropMaterialCache>());

	//get pointer's value as uint64
	uint64_t k = make_uint64_t<const PropData *>(*prop);

	_material_cache_mutex.lock();

	if (_material_cache.has(k)) {
		Ref<PropMaterialCache> m = _material_cache[k];

		m->inc_ref_count();

		_material_cache_mutex.unlock();

		return m;
	}

	PropMaterialCache *p = Object::cast_to<PropMaterialCache>(ClassDB::instance(_default_prop_material_cache_class));

	if (!p) {
		ERR_PRINT("Can't instance the given PropMaterialCache! class_name: " + String(_default_prop_material_cache_class));
	}

	Ref<PropMaterialCache> m(p);

	_material_cache[k] = m;

	_material_cache_mutex.unlock();

	return m;
}
void PropCache::material_cache_unref(const Ref<PropData> &prop) {
	_material_cache_mutex.lock();

	_material_cache_mutex.unlock();
}

Ref<PropMaterialCache> PropCache::material_cache_custom_key_get(const uint64_t key) {
	_custom_keyed_material_cache_mutex.lock();

	if (_custom_keyed_material_cache.has(key)) {
		Ref<PropMaterialCache> m = _custom_keyed_material_cache[key];

		m->inc_ref_count();

		_custom_keyed_material_cache_mutex.unlock();

		return m;
	}

	PropMaterialCache *p = Object::cast_to<PropMaterialCache>(ClassDB::instance(_default_prop_material_cache_class));

	if (!p) {
		ERR_PRINT("Can't instance the given PropMaterialCache! class_name: " + String(_default_prop_material_cache_class));
	}

	Ref<PropMaterialCache> m(p);

	_custom_keyed_material_cache[key] = m;

	_custom_keyed_material_cache_mutex.unlock();

	return m;
}
void PropCache::material_cache_custom_key_unref(const uint64_t key) {
	_custom_keyed_material_cache_mutex.lock();

	_custom_keyed_material_cache_mutex.unlock();
}

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
}

void PropCache::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_default_prop_material_cache_class"), &PropCache::get_default_prop_material_cache_class);
	ClassDB::bind_method(D_METHOD("set_default_prop_material_cache_class", "cls_name"), &PropCache::set_default_prop_material_cache_class);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "default_prop_material_cache_class"), "set_default_prop_material_cache_class", "get_default_prop_material_cache_class");
}
