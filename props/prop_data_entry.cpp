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

#include "prop_data_entry.h"

Transform PropDataEntry::get_transform() const {
	return _transform;
}
void PropDataEntry::set_transform(const Transform &value) {
	_transform = value;
}

#if TEXTURE_PACKER_PRESENT
void PropDataEntry::add_textures_into(Ref<TexturePacker> texture_packer) {
	if (has_method("_add_textures_into"))
		call("_add_textures_into", texture_packer);
}
#endif

PropDataEntry::PropDataEntry() {
}
PropDataEntry::~PropDataEntry() {
}

void PropDataEntry::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_transform"), &PropDataEntry::get_transform);
	ClassDB::bind_method(D_METHOD("set_transform", "value"), &PropDataEntry::set_transform);
	ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM, "transform"), "set_transform", "get_transform");

#if TEXTURE_PACKER_PRESENT
	BIND_VMETHOD(MethodInfo("_add_textures_into", PropertyInfo(Variant::OBJECT, "texture_packer", PROPERTY_HINT_RESOURCE_TYPE, "TexturePacker")));

	ClassDB::bind_method(D_METHOD("add_textures_into", "texture_packer"), &PropDataEntry::add_textures_into);
#endif
}
