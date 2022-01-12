/*
Copyright (c) 2020-2022 Péter Magyar

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

#include "prop_editor_plugin.h"

#include "../props/prop_data.h"
#include "../singleton/prop_utils.h"
#include "core/os/keyboard.h"

#include "core/version.h"

#if VERSION_MAJOR < 4
#include "core/os/input.h"

#define CONNECT(sig, obj, target_method_class, method) connect(sig, obj, #method)
#define DISCONNECT(sig, obj, target_method_class, method) disconnect(sig, obj, #method)

#else
#include "core/input/input.h"

#define CONNECT(sig, obj, target_method_class, method) connect(sig, callable_mp(obj, &target_method_class::method))
#define DISCONNECT(sig, obj, target_method_class, method) disconnect(sig, callable_mp(obj, &target_method_class::method))
#endif

void PropEditorPlugin::convert_active_scene_to_prop_data() {
	SceneTree *st = SceneTree::get_singleton();

	if (st) {
		Node *scene = st->get_edited_scene_root();

		if (scene) {
			EditorData &ed = EditorNode::get_editor_data();
			String path = ed.get_scene_path(ed.get_edited_scene());

			convert_scene(scene, path.get_basename() + ".tres");
		}
	}
}
void PropEditorPlugin::convert_selected_scene_to_prop_data() {
}

void PropEditorPlugin::convert_scene(Node *root, const String &path) {
	Ref<PropData> data = PropUtils::get_singleton()->convert_tree(root);

	ERR_FAIL_COND(!data.is_valid());

	ResourceLoader l;
	if (l.exists(path)) {
		Ref<PropData> res = l.load(path, "PropData");

		ERR_FAIL_COND(!res.is_valid());

		res->copy_from(data);

		ResourceSaver s;
		s.save(path, res);

		res.unref();
	} else {
		ResourceSaver s;
		s.save(path, data);
	}
}

void PropEditorPlugin::find_room_points(Variant param) {
#if VERSION_MINOR >= 4
	SceneTree *st = SceneTree::get_singleton();

	if (st) {
		Node *scene = st->get_edited_scene_root();

		if (scene) {
			PropUtils::get_singleton()->generate_room_points_node(scene);
		}
	}
#endif
}

void PropEditorPlugin::_quick_convert_button_pressed() {
	convert_active_scene_to_prop_data();
}

void PropEditorPlugin::_convert_active_scene_to_prop_data(Variant param) {
	convert_active_scene_to_prop_data();
}
void PropEditorPlugin::_convert_selected_scene_to_prop_data(Variant param) {
	convert_selected_scene_to_prop_data();
}

PropEditorPlugin::PropEditorPlugin(EditorNode *p_node) {
	editor = p_node;

#if VERSION_MAJOR < 4
	editor->add_tool_menu_item("Convert active scene to PropData", this, "convert_active_scene_to_prop_data");
	editor->add_tool_menu_item("Convert selected scene(s) to PropData", this, "convert_selected_scene_to_prop_data");
#if VERSION_MINOR >= 4
	editor->add_tool_menu_item("(Prop) Find room points.", this, "find_room_points");
#endif
#else
#endif

	HBoxContainer *container = memnew(HBoxContainer);

	container->add_child(memnew(VSeparator));

	Button *b = memnew(Button);
	container->add_child(b);
	b->set_flat(true);

	b->CONNECT("pressed", this, PropEditorPlugin, _quick_convert_button_pressed);
	b->set_text("To Prop");
	b->set_shortcut(ED_SHORTCUT("spatial_editor/quick_prop_convert", "Quick convert scene to PropData.", KEY_MASK_ALT + KEY_U));

	add_control_to_container(EditorPlugin::CONTAINER_SPATIAL_EDITOR_MENU, container);
}

PropEditorPlugin::~PropEditorPlugin() {
}

void PropEditorPlugin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("convert_active_scene_to_prop_data"), &PropEditorPlugin::_convert_active_scene_to_prop_data);
	ClassDB::bind_method(D_METHOD("convert_selected_scene_to_prop_data"), &PropEditorPlugin::_convert_active_scene_to_prop_data);

	ClassDB::bind_method(D_METHOD("find_room_points"), &PropEditorPlugin::find_room_points);

	ClassDB::bind_method(D_METHOD("_quick_convert_button_pressed"), &PropEditorPlugin::_quick_convert_button_pressed);
}
