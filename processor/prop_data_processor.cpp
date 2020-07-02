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

#include "prop_data_processor.h"

#include "scene/3d/spatial.h"

bool PropDataProcessor::handles(Node *node) {
	return call("_handles", node);
}
void PropDataProcessor::process(Ref<PropData> prop_data, Node *node, const Transform &transform) {
	call("_process", prop_data, node, transform);
}

bool PropDataProcessor::_handles(Node *node) {
	return false;
}
void PropDataProcessor::_process(Ref<PropData> prop_data, Node *node, const Transform &transform) {
}

PropDataProcessor::PropDataProcessor() {
}
PropDataProcessor::~PropDataProcessor() {
}

void PropDataProcessor::_bind_methods() {
	BIND_VMETHOD(MethodInfo(PropertyInfo(Variant::BOOL, "handles"), "_handles"));
	BIND_VMETHOD(MethodInfo("_process",
			PropertyInfo(Variant::OBJECT, "prop_data", PROPERTY_HINT_RESOURCE_TYPE, "PropData"),
			PropertyInfo(Variant::OBJECT, "node", PROPERTY_HINT_RESOURCE_TYPE, "Node"),
			PropertyInfo(Variant::TRANSFORM, "transform")));

	ClassDB::bind_method(D_METHOD("handles", "node"), &PropDataProcessor::handles);
	ClassDB::bind_method(D_METHOD("process", "prop_data", "node", "transform"), &PropDataProcessor::process);

	ClassDB::bind_method(D_METHOD("_handles", "node"), &PropDataProcessor::_handles);
	ClassDB::bind_method(D_METHOD("_process", "prop_data", "node", "transform"), &PropDataProcessor::_process);
}
