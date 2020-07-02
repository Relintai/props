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

#ifndef PROP_UTILS_H
#define PROP_UTILS_H

#include "core/object.h"

#include "core/vector.h"

#include "core/reference.h"

class PropDataProcessor;
class PropData;

class PropUtils : public Object {
	GDCLASS(PropUtils, Object);

public:
	static PropUtils *get_singleton();

	Ref<PropData> convert_tree(Node *root);
	void _convert_tree(Ref<PropData> prop_data, Node *node, const Transform &transform);

	static int add_processor(const Ref<PropDataProcessor> &processor);
	static Ref<PropDataProcessor> get_processor(const int index);
	static void swap_processors(const int index1, const int index2);
	static void remove_processor(const int index);
	static int get_processor_count();

	PropUtils();
	~PropUtils();

protected:
	static void _bind_methods();

private:
	int _add_processor_bind(const Ref<PropDataProcessor> &processor);
	Ref<PropDataProcessor> _get_processor_bind(const int index);
	void _swap_processors_bind(const int index1, const int index2);
	void _remove_processor_bind(const int index);
	int _get_processor_count_bind();

	static Vector<Ref<PropDataProcessor> > _processors;
	static PropUtils *_instance;
};

#endif
