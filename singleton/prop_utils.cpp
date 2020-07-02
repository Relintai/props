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

#include "prop_utils.h"

#include "../processor/prop_data_processor.h"
#include "../props/prop_data.h"

PropUtils *PropUtils::_instance;
Vector<Ref<PropDataProcessor> > PropUtils::_processors;

PropUtils *PropUtils::get_singleton() {
	return _instance;
}

Ref<PropData> PropUtils::convert_tree(Node *root) {
	ERR_FAIL_COND_V(!ObjectDB::instance_validate(root), Ref<PropData>());

	Ref<PropData> data;
	data.instance();
	Transform t;

	_convert_tree(data, root, t);

	return data;
}

void PropUtils::_convert_tree(Ref<PropData> prop_data, Node *node, const Transform &transform) {
	ERR_FAIL_COND(!ObjectDB::instance_validate(node));

	for (int i = 0; i < PropUtils::_processors.size(); ++i) {
		Ref<PropDataProcessor> proc = PropUtils::_processors.get(i);

		ERR_CONTINUE(!proc.is_valid());

		if (proc->handles(node)) {
			proc->process(prop_data, node, transform);
			break;
		}
	}

	Spatial *sp = Object::cast_to<Spatial>(node);

	if (!sp) {
		//reset transform
		Transform t;

		for (int i = 0; i < node->get_child_count(); ++i) {
			_convert_tree(prop_data, node->get_child(i), t);
		}
	} else {
		for (int i = 0; i < node->get_child_count(); ++i) {
			_convert_tree(prop_data, node->get_child(i), transform * sp->get_transform());
		}
	}
}

int PropUtils::add_processor(const Ref<PropDataProcessor> &processor) {
	ERR_FAIL_COND_V(!processor.is_valid(), 0);

	PropUtils::_processors.push_back(processor);

	return PropUtils::_processors.size() - 1;
}
Ref<PropDataProcessor> PropUtils::get_processor(const int index) {
	ERR_FAIL_INDEX_V(index, PropUtils::_processors.size(), Ref<PropDataProcessor>());

	return PropUtils::_processors[index];
}
void PropUtils::swap_processors(const int index1, const int index2) {
	ERR_FAIL_INDEX(index1, PropUtils::_processors.size());
	ERR_FAIL_INDEX(index2, PropUtils::_processors.size());

	Ref<PropDataProcessor> a = PropUtils::_processors.get(index1);
	PropUtils::_processors.set(index1, PropUtils::_processors.get(index2));
	PropUtils::_processors.set(index2, a);
}
void PropUtils::remove_processor(const int index) {
	ERR_FAIL_INDEX(index, PropUtils::_processors.size());

	PropUtils::_processors.remove(index);
}
int PropUtils::get_processor_count() {
	return PropUtils::_processors.size();
}

PropUtils::PropUtils() {
	_instance = this;
}

PropUtils::~PropUtils() {
	_instance = NULL;

	PropUtils::_processors.clear();
}

void PropUtils::_bind_methods() {
	ClassDB::bind_method(D_METHOD("convert_tree", "root"), &PropUtils::convert_tree);

	ClassDB::bind_method(D_METHOD("add_processor", "processor"), &PropUtils::_add_processor_bind);
	ClassDB::bind_method(D_METHOD("get_processor", "index"), &PropUtils::_get_processor_bind);
	ClassDB::bind_method(D_METHOD("swap_processors", "index1", "index2"), &PropUtils::_swap_processors_bind);
	ClassDB::bind_method(D_METHOD("remove_processor", "index"), &PropUtils::_remove_processor_bind);
	ClassDB::bind_method(D_METHOD("get_processor_count"), &PropUtils::_get_processor_count_bind);
}

int PropUtils::_add_processor_bind(const Ref<PropDataProcessor> &processor) {
	return PropUtils::add_processor(processor);
}
Ref<PropDataProcessor> PropUtils::_get_processor_bind(const int index) {
	return PropUtils::get_processor(index);
}
void PropUtils::_swap_processors_bind(const int index1, const int index2) {
	PropUtils::swap_processors(index1, index2);
}
void PropUtils::_remove_processor_bind(const int index) {
	PropUtils::remove_processor(index);
}
int PropUtils::_get_processor_count_bind() {
	return PropUtils::get_processor_count();
}
