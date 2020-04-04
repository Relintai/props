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

#include "register_types.h"

#include "clutter/ground_clutter.h"
#include "clutter/ground_clutter_foliage.h"

#include "prop_mesh_utils.h"

#include "prop_ess_entity.h"
#include "prop_instance.h"
#include "prop_mesh_data_instance.h"
#include "prop_voxelman_light.h"

void register_props_types() {
	ClassDB::register_class<GroundClutter>();
	ClassDB::register_class<GroundClutterFoliage>();

	ClassDB::register_class<PropMeshUtils>();

	ClassDB::register_class<PropInstance>();

	ClassDB::register_class<PropESSEntity>();
	ClassDB::register_class<PropMeshDataInstance>();
	ClassDB::register_class<PropVoxelmanLight>();
}

void unregister_props_types() {
}
