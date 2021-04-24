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

#include "prop_mesher.h"

const String PropMesher::BINDING_STRING_BUILD_FLAGS = "Use Isolevel,Use Lighting,Use AO,Use RAO,Generate AO,Generate RAO,Bake Lights,Create Collider,Create Lods";

bool PropMesher::Vertex::operator==(const Vertex &p_vertex) const {

	if (vertex != p_vertex.vertex)
		return false;

	if (uv != p_vertex.uv)
		return false;

	if (uv2 != p_vertex.uv2)
		return false;

	if (normal != p_vertex.normal)
		return false;

	if (binormal != p_vertex.binormal)
		return false;

	if (color != p_vertex.color)
		return false;

	if (bones.size() != p_vertex.bones.size())
		return false;

	for (int i = 0; i < bones.size(); i++) {
		if (bones[i] != p_vertex.bones[i])
			return false;
	}

	for (int i = 0; i < weights.size(); i++) {
		if (weights[i] != p_vertex.weights[i])
			return false;
	}

	return true;
}

uint32_t PropMesher::VertexHasher::hash(const Vertex &p_vtx) {

	uint32_t h = hash_djb2_buffer((const uint8_t *)&p_vtx.vertex, sizeof(real_t) * 3);
	h = hash_djb2_buffer((const uint8_t *)&p_vtx.normal, sizeof(real_t) * 3, h);
	h = hash_djb2_buffer((const uint8_t *)&p_vtx.binormal, sizeof(real_t) * 3, h);
	h = hash_djb2_buffer((const uint8_t *)&p_vtx.tangent, sizeof(real_t) * 3, h);
	h = hash_djb2_buffer((const uint8_t *)&p_vtx.uv, sizeof(real_t) * 2, h);
	h = hash_djb2_buffer((const uint8_t *)&p_vtx.uv2, sizeof(real_t) * 2, h);
	h = hash_djb2_buffer((const uint8_t *)&p_vtx.color, sizeof(real_t) * 4, h);
	h = hash_djb2_buffer((const uint8_t *)p_vtx.bones.ptr(), p_vtx.bones.size() * sizeof(int), h);
	h = hash_djb2_buffer((const uint8_t *)p_vtx.weights.ptr(), p_vtx.weights.size() * sizeof(float), h);
	return h;
}

int PropMesher::get_channel_index_type() const {
	return _channel_index_type;
}
void PropMesher::set_channel_index_type(const int value) {
	_channel_index_type = value;
}

int PropMesher::get_channel_index_isolevel() const {
	return _channel_index_isolevel;
}
void PropMesher::set_channel_index_isolevel(const int value) {
	_channel_index_isolevel = value;
}

int PropMesher::get_mesher_index() const {
	return _mesher_index;
}
void PropMesher::set_mesher_index(const int value) {
	_mesher_index = value;
}

int PropMesher::get_format() const {
	return _format;
}
void PropMesher::set_format(const int value) {
	_format = value;
}

int PropMesher::get_texture_scale() const {
	return _texture_scale;
}
void PropMesher::set_texture_scale(const int value) {
	_texture_scale = value;
}

Ref<Material> PropMesher::get_material() {
	return _material;
}
void PropMesher::set_material(const Ref<Material> &material) {
	_material = material;
}

float PropMesher::get_ao_strength() const {
	return _ao_strength;
}
void PropMesher::set_ao_strength(float value) {
	_ao_strength = value;
}

float PropMesher::get_base_light_value() const {
	return _base_light_value;
}
void PropMesher::set_base_light_value(float value) {
	_base_light_value = value;
}

float PropMesher::get_voxel_scale() const {
	return _voxel_scale;
}
void PropMesher::set_voxel_scale(const float voxel_scale) {
	_voxel_scale = voxel_scale;
}

Rect2 PropMesher::get_uv_margin() const {
	return _uv_margin;
}
void PropMesher::set_uv_margin(const Rect2 margin) {
	_uv_margin = margin;
}

_FORCE_INLINE_ int PropMesher::get_build_flags() const {
	return _build_flags;
}
_FORCE_INLINE_ void PropMesher::set_build_flags(const int flags) {
	_build_flags = flags;

	if ((_build_flags & PropMesher::BUILD_FLAG_USE_LIGHTING) != 0) {
		_format |= VisualServer::ARRAY_FORMAT_COLOR;
	} else {
		_format ^= VisualServer::ARRAY_FORMAT_COLOR;
	}
}

Array PropMesher::build_mesh() {
	Array a;
	a.resize(VisualServer::ARRAY_MAX);

	if (_vertices.size() == 0) {
		//Nothing to do
		return a;
	}

	{
		PoolVector<Vector3> array;
		array.resize(_vertices.size());
#if !GODOT4
		PoolVector<Vector3>::Write w = array.write();
#endif

		for (int i = 0; i < _vertices.size(); ++i) {
			array.set(i, _vertices[i].vertex);
		}

#if !GODOT4
		w.release();
#endif

		a[VisualServer::ARRAY_VERTEX] = array;
	}

	if ((_format & VisualServer::ARRAY_FORMAT_NORMAL) == 0) {
		generate_normals();
	}

	{
		PoolVector<Vector3> array;
		array.resize(_vertices.size());
#if !GODOT4
		PoolVector<Vector3>::Write w = array.write();
#endif

		for (int i = 0; i < _vertices.size(); ++i) {
			array.set(i, _vertices[i].normal);
		}

#if !GODOT4
		w.release();
#endif
		a[VisualServer::ARRAY_NORMAL] = array;
	}

	if ((_format & VisualServer::ARRAY_FORMAT_COLOR) != 0) {
		PoolVector<Color> array;
		array.resize(_vertices.size());
#if !GODOT4
		PoolVector<Color>::Write w = array.write();
#endif

		for (int i = 0; i < _vertices.size(); ++i) {
			array.set(i, _vertices[i].color);
		}

#if !GODOT4
		w.release();
#endif
		a[VisualServer::ARRAY_COLOR] = array;
	}

	if ((_format & VisualServer::ARRAY_FORMAT_TEX_UV) != 0) {
		PoolVector<Vector2> array;
		array.resize(_vertices.size());
#if !GODOT4
		PoolVector<Vector2>::Write w = array.write();
#endif

		for (int i = 0; i < _vertices.size(); ++i) {
			array.set(i, _vertices[i].uv);
		}

#if !GODOT4
		w.release();
#endif

		a[VisualServer::ARRAY_TEX_UV] = array;
	}

	if ((_format & VisualServer::ARRAY_FORMAT_TEX_UV2) != 0) {
		PoolVector<Vector2> array;
		array.resize(_vertices.size());
#if !GODOT4
		PoolVector<Vector2>::Write w = array.write();
#endif

		for (int i = 0; i < _vertices.size(); ++i) {
			array.set(i, _vertices[i].uv2);
		}

#if !GODOT4
		w.release();
#endif
		a[VisualServer::ARRAY_TEX_UV2] = array;
	}

	if (_indices.size() > 0) {
		PoolVector<int> array;
		array.resize(_indices.size());
#if !GODOT4
		PoolVector<int>::Write w = array.write();
#endif

		for (int i = 0; i < _indices.size(); ++i) {
			array.set(i, _indices[i]);
		}

#if !GODOT4
		w.release();
#endif
		a[VisualServer::ARRAY_INDEX] = array;
	}

	return a;
}

void PropMesher::build_mesh_into(RID mesh) {
	ERR_FAIL_COND(mesh == RID());

	VS::get_singleton()->mesh_clear(mesh);

	if (_vertices.size() == 0) {
		//Nothing to do
		return;
	}

	Array arr = build_mesh();

	VS::get_singleton()->mesh_add_surface_from_arrays(mesh, VisualServer::PRIMITIVE_TRIANGLES, arr);

	if (_material.is_valid())
		VS::get_singleton()->mesh_surface_set_material(mesh, 0, _material->get_rid());
}

void PropMesher::generate_normals(bool p_flip) {

	_format = _format | VisualServer::ARRAY_FORMAT_NORMAL;

	for (int i = 0; i < _indices.size(); i += 3) {
		int i0 = _indices[i];
		int i1 = _indices[i + 1];
		int i2 = _indices[i + 2];

		ERR_FAIL_INDEX(i0, _vertices.size());
		ERR_FAIL_INDEX(i1, _vertices.size());
		ERR_FAIL_INDEX(i2, _vertices.size());

		Vertex v0 = _vertices.get(i0);
		Vertex v1 = _vertices.get(i1);
		Vertex v2 = _vertices.get(i2);

		Vector3 normal;
		if (!p_flip)
			normal = Plane(v0.vertex, v1.vertex, v2.vertex).normal;
		else
			normal = Plane(v2.vertex, v1.vertex, v0.vertex).normal;

		v0.normal = normal;
		v1.normal = normal;
		v2.normal = normal;

		_vertices.set(i0, v0);
		_vertices.set(i1, v1);
		_vertices.set(i2, v2);
	}
}

void PropMesher::remove_doubles() {
	if (_vertices.size() == 0)
		return;

	//print_error("before " + String::num(_vertices.size()));

	for (int i = 0; i < _vertices.size(); ++i) {
		Vertex vert = _vertices[i];
		PoolVector<int> indices;

		for (int j = i + 1; j < _vertices.size(); ++j) {
			if (_vertices[j] == vert) {
				indices.push_back(j);
			}
		}

		for (int j = 0; j < indices.size(); ++j) {
			int index = indices[j];

			_vertices.remove(index);

			//make all indices that were bigger than the one we replaced one lower
			for (int k = 0; k < _indices.size(); ++k) {
				int indx = _indices[k];

				if (indx == index) {
					_indices.set(k, i);
				} else if (indx > index) {
					_indices.set(k, --indx);
				}
			}

			for (int k = j + 1; k < indices.size(); ++k) {
				int val = indices[k];

				if (val > index) {
					indices.set(k, --val);
				}
			}
		}
	}

	//print_error("after " + String::num(_vertices.size())+ " " + String::num(duration.count()));
}

//lot faster that normal remove_doubles, but false positives can happen curtesy of hash collisions
void PropMesher::remove_doubles_hashed() {
	if (_vertices.size() == 0)
		return;

	//print_error("before " + String::num(_vertices.size()));

	PoolVector<uint32_t> hashes;
	hashes.resize(_vertices.size());
	for (int i = 0; i < _vertices.size(); ++i) {
		hashes.set(i, VertexHasher::hash(_vertices[i]));
	}

	for (int i = 0; i < hashes.size(); ++i) {
		uint32_t hash = hashes[i];
		PoolVector<int> indices;

		for (int j = i + 1; j < hashes.size(); ++j) {
			if (hashes[j] == hash) {
				indices.push_back(j);
			}
		}

		for (int j = 0; j < indices.size(); ++j) {
			int index = indices[j];

			hashes.remove(index);
			_vertices.remove(index);

			//make all indices that were bigger than the one we replaced one lower
			for (int k = 0; k < _indices.size(); ++k) {
				int indx = _indices[k];

				if (indx == index) {
					_indices.set(k, i);
				} else if (indx > index) {
					_indices.set(k, --indx);
				}
			}

			for (int k = j + 1; k < indices.size(); ++k) {
				int val = indices[k];

				if (val > index) {
					indices.set(k, --val);
				}
			}
		}
	}

	//print_error("after " + String::num(_vertices.size()) + " " + String::num(duration.count()));
}

void PropMesher::reset() {
	_vertices.resize(0);
	_indices.resize(0);

	_last_color = Color();
	_last_normal = Vector3();
	_last_uv = Vector2();
	_last_uv2 = Vector2();
	_last_bones.clear();
	_last_weights.clear();
	_last_tangent = Plane();
}

#ifdef MESH_DATA_RESOURCE_PRESENT
void PropMesher::add_mesh_data_resource(Ref<MeshDataResource> mesh, const Vector3 position, const Vector3 rotation, const Vector3 scale, const Rect2 uv_rect) {
	Transform transform = Transform(Basis(rotation).scaled(scale), position);

	add_mesh_data_resource_transform(mesh, transform, uv_rect);
}

void PropMesher::add_mesh_data_resource_transform(Ref<MeshDataResource> mesh, const Transform transform, const Rect2 uv_rect) {
	if (mesh->get_array().size() == 0)
		return;

	const Array &arr = mesh->get_array();

	PoolVector3Array vertices = arr[Mesh::ARRAY_VERTEX];
	PoolVector3Array normals = arr[Mesh::ARRAY_NORMAL];
	PoolVector2Array uvs = arr[Mesh::ARRAY_TEX_UV];
	PoolColorArray colors = arr[Mesh::ARRAY_COLOR];
	PoolIntArray indices = arr[Mesh::ARRAY_INDEX];

	if (vertices.size() == 0)
		return;

	int orig_vert_size = _vertices.size();

	for (int i = 0; i < vertices.size(); ++i) {
		if (normals.size() > 0)
			add_normal(transform.basis.xform(normals[i]));

		if (normals.size() > 0) {
			Vector2 uv = uvs[i];

			uv.x = uv_rect.size.width * uv.x + uv_rect.position.x;
			uv.y = uv_rect.size.height * uv.y + uv_rect.position.y;

			add_uv(uv);
		}

		if (colors.size() > 0)
			add_color(colors[i]);

		add_vertex(transform.xform(vertices[i]));
	}

	int orig_indices_count = _indices.size();
	_indices.resize(_indices.size() + indices.size());

	for (int i = 0; i < indices.size(); ++i) {
		_indices.set(orig_indices_count + i, orig_vert_size + indices[i]);
	}
}

void PropMesher::add_mesh_data_resource_transform_colored(Ref<MeshDataResource> mesh, const Transform transform, const PoolColorArray &colors, const Rect2 uv_rect) {
	if (mesh->get_array().size() == 0)
		return;

	const Array &arr = mesh->get_array();

	PoolVector3Array vertices = arr[Mesh::ARRAY_VERTEX];
	PoolVector3Array normals = arr[Mesh::ARRAY_NORMAL];
	PoolVector2Array uvs = arr[Mesh::ARRAY_TEX_UV];
	PoolIntArray indices = arr[Mesh::ARRAY_INDEX];

	if (vertices.size() == 0)
		return;

	int orig_vert_size = _vertices.size();

	for (int i = 0; i < vertices.size(); ++i) {
		if (normals.size() > 0)
			add_normal(transform.basis.xform(normals[i]));

		if (normals.size() > 0) {
			Vector2 uv = uvs[i];

			uv.x = uv_rect.size.width * uv.x + uv_rect.position.x;
			uv.y = uv_rect.size.height * uv.y + uv_rect.position.y;

			add_uv(uv);
		}

		if (colors.size() > 0)
			add_color(colors[i]);

		add_vertex(transform.xform(vertices[i]));
	}

	int orig_indices_count = _indices.size();
	_indices.resize(_indices.size() + indices.size());

	for (int i = 0; i < indices.size(); ++i) {
		_indices.set(orig_indices_count + i, orig_vert_size + indices[i]);
	}
}
#endif

//Data Management functions
void PropMesher::generate_ao() {
	/*
	ERR_FAIL_COND(!_chunk.is_valid());

	int data_size_x = _chunk->get_data_size_x();
	int data_size_z = _chunk->get_data_size_z();

	ERR_FAIL_COND(data_size_x == 0 || data_size_z == 0);

	int margin_start = _chunk->get_margin_start();
	int margin_end = _chunk->get_margin_end();

	int ssize_x = _chunk->get_size_x();
	int ssize_z = _chunk->get_size_z();

	int size_x = ssize_x + margin_end;
	int size_z = ssize_z + margin_end;

	for (int z = margin_start - 1; z < size_z - 1; ++z) {
		for (int x = margin_start - 1; x < size_x - 1; ++x) {
			int current = _chunk->get_voxel(x, z, TerraChunkDefault::DEFAULT_CHANNEL_ISOLEVEL);

			int sum = _chunk->get_voxel(x + 1, z, TerraChunkDefault::DEFAULT_CHANNEL_ISOLEVEL);
			sum += _chunk->get_voxel(x - 1, z, TerraChunkDefault::DEFAULT_CHANNEL_ISOLEVEL);
			sum += _chunk->get_voxel(x, z + 1, TerraChunkDefault::DEFAULT_CHANNEL_ISOLEVEL);
			sum += _chunk->get_voxel(x, z - 1, TerraChunkDefault::DEFAULT_CHANNEL_ISOLEVEL);

			sum /= 6;

			sum -= current;

			if (sum < 0)
				sum = 0;

			_chunk->set_voxel(sum, x, z, TerraChunkDefault::DEFAULT_CHANNEL_AO);
		}
	}*/
}

void PropMesher::generate_random_ao(int seed, int octaves, int period, float persistence, float scale_factor) {
	/*
	ERR_FAIL_COND(!_chunk.is_valid());

	int margin_start = _chunk->get_margin_start();
	int margin_end = _chunk->get_margin_end();

	int size_x = _chunk->get_size_x();
	int size_z = _chunk->get_size_z();

	int position_x = _chunk->get_position_x();
	int position_z = _chunk->get_position_z();

	Ref<OpenSimplexNoise> noise;
	noise.instance();

	noise->set_seed(seed);
	noise->set_octaves(octaves);
	noise->set_period(period);
	noise->set_persistence(persistence);

	for (int x = -margin_start; x < size_x + margin_end; ++x) {
		for (int z = -margin_start; z < size_z + margin_end; ++z) {

			float val = noise->get_noise_3d(x + (position_x * size_x), 0, z + (position_z * size_z));

			val *= scale_factor;

			if (val > 1)
				val = 1;

			if (val < 0)
				val = -val;

			_chunk->set_voxel(int(val * 255.0), x, z, TerraChunkDefault::DEFAULT_CHANNEL_RANDOM_AO);
		}
	}*/
}

void PropMesher::add_mesher(const Ref<PropMesher> &mesher) {
	call("_add_mesher", mesher);
}
void PropMesher::_add_mesher(const Ref<PropMesher> &mesher) {
	int orig_size = _vertices.size();

	_vertices.append_array(mesher->_vertices);

	int s = mesher->_indices.size();

	if (s == 0)
		return;

	int orig_indices_size = _indices.size();

	_indices.resize(_indices.size() + s);
	for (int i = 0; i < s; ++i) {
		_indices.set(i + orig_indices_size, mesher->_indices[i] + orig_size);
	}
}

PoolVector<Vector3> PropMesher::build_collider() const {
	PoolVector<Vector3> face_points;

	if (_vertices.size() == 0)
		return face_points;

	if (_indices.size() == 0) {

		int len = (_vertices.size() / 4);

		for (int i = 0; i < len; ++i) {

			face_points.push_back(_vertices.get(i * 4).vertex);
			face_points.push_back(_vertices.get((i * 4) + 2).vertex);
			face_points.push_back(_vertices.get((i * 4) + 1).vertex);

			face_points.push_back(_vertices.get(i * 4).vertex);
			face_points.push_back(_vertices.get((i * 4) + 3).vertex);
			face_points.push_back(_vertices.get((i * 4) + 2).vertex);
		}

		return face_points;
	}

	face_points.resize(_indices.size());
	for (int i = 0; i < face_points.size(); i++) {
		face_points.set(i, _vertices.get(_indices.get(i)).vertex);
	}

	return face_points;
}

#ifdef TERRAMAN_PRESENT
void PropMesher::bake_lights(MeshInstance *node, Vector<Ref<TerraLight> > &lights) {
	ERR_FAIL_COND(node == NULL);

	Color darkColor(0, 0, 0, 1);

	for (int v = 0; v < _vertices.size(); ++v) {
		Vertex vertexv = _vertices.get(v);
		Vector3 vet = vertexv.vertex;
		Vector3 vertex = node->to_global(vet);

		//grab normal
		Vector3 normal = vertexv.normal;

		Vector3 v_lightDiffuse;

		//calculate the lights value
		for (int i = 0; i < lights.size(); ++i) {
			Ref<TerraLight> light = lights.get(i);

			Vector3 lightDir = light->get_world_position() - vertex;

			float dist2 = lightDir.dot(lightDir);
			//inverse sqrt
			lightDir *= (1.0 / sqrt(dist2));

			float NdotL = normal.dot(lightDir);

			if (NdotL > 1.0) {
				NdotL = 1.0;
			} else if (NdotL < 0.0) {
				NdotL = 0.0;
			}

			Color cc = light->get_color();
			Vector3 cv(cc.r, cc.g, cc.b);

			Vector3 value = cv * (NdotL / (1.0 + dist2));

			value *= light->get_size();
			v_lightDiffuse += value;

			/*
                    float dist2 = Mathf.Clamp(Vector3.Distance(transformedLights[i], vertices), 0f, 15f);
                    dist2 /= 35f;

                    Vector3 value = Vector3.one;
                    value *= ((float) lights[i].Strength) / 255f;
                    value *= (1 - dist2);
                    v_lightDiffuse += value;*/
		}

		Color f = vertexv.color;
		//Color f = darkColor;

		Vector3 cv2(f.r, f.g, f.b);
		cv2 += v_lightDiffuse;

		if (cv2.x > 1)
			cv2.x = 1;

		if (cv2.y > 1)
			cv2.y = 1;

		if (cv2.y > 1)
			cv2.y = 1;

		// cv2.x = Mathf.Clamp(cv2.x, 0f, 1f);
		//cv2.y = Mathf.Clamp(cv2.y, 0f, 1f);
		// cv2.z = Mathf.Clamp(cv2.z, 0f, 1f);

		f.r = cv2.x;
		f.g = cv2.y;
		f.b = cv2.z;

		//f.r = v_lightDiffuse.x;
		//f.g = v_lightDiffuse.y;
		//f.b = v_lightDiffuse.z;

		vertexv.color = f;
		_vertices.set(v, vertexv);
	}

	//	for (int i = 0; i < _colors->size(); ++i) {
	//		print_error(_colors->get(i));
	//	}
}
#endif

PoolVector<Vector3> PropMesher::get_vertices() const {
	PoolVector<Vector3> arr;

	arr.resize(_vertices.size());
	for (int i = 0; i < _vertices.size(); ++i) {
		arr.set(i, _vertices.get(i).vertex);
	}

	return arr;
}

void PropMesher::set_vertices(const PoolVector<Vector3> &values) {
	ERR_FAIL_COND(values.size() != _vertices.size());

	for (int i = 0; i < _vertices.size(); ++i) {
		Vertex v = _vertices[i];

		v.normal = values[i];

		_vertices.set(i, v);
	}
}

int PropMesher::get_vertex_count() const {
	return _vertices.size();
}

void PropMesher::add_vertex(const Vector3 &vertex) {
	Vertex vtx;
	vtx.vertex = vertex;
	vtx.color = _last_color;
	vtx.normal = _last_normal;
	vtx.uv = _last_uv;
	vtx.uv2 = _last_uv2;
	// Todo?
	//	vtx.weights = _last_weights;
	//	vtx.bones = _last_bones;
	//	vtx.tangent = _last_tangent.normal;
	//	vtx.binormal = _last_normal.cross(_last_tangent.normal).normalized() * _last_tangent.d;

	_vertices.push_back(vtx);
}

Vector3 PropMesher::get_vertex(const int idx) const {
	return _vertices.get(idx).vertex;
}

void PropMesher::remove_vertex(const int idx) {
	_vertices.remove(idx);
}

PoolVector<Vector3> PropMesher::get_normals() const {
	PoolVector<Vector3> arr;

	arr.resize(_vertices.size());
	for (int i = 0; i < _vertices.size(); ++i) {
		arr.set(i, _vertices.get(i).normal);
	}

	return arr;
}

void PropMesher::set_normals(const PoolVector<Vector3> &values) {
	ERR_FAIL_COND(values.size() != _vertices.size());

	for (int i = 0; i < _vertices.size(); ++i) {
		Vertex v = _vertices[i];

		v.normal = values[i];

		_vertices.set(i, v);
	}
}

void PropMesher::add_normal(const Vector3 &normal) {
	_last_normal = normal;
}

Vector3 PropMesher::get_normal(int idx) const {
	return _vertices.get(idx).normal;
}

PoolVector<Color> PropMesher::get_colors() const {
	PoolVector<Color> arr;

	arr.resize(_vertices.size());
	for (int i = 0; i < _vertices.size(); ++i) {
		arr.set(i, _vertices.get(i).color);
	}

	return arr;
}

void PropMesher::set_colors(const PoolVector<Color> &values) {
	ERR_FAIL_COND(values.size() != _vertices.size());

	for (int i = 0; i < _vertices.size(); ++i) {
		Vertex v = _vertices[i];

		v.color = values[i];

		_vertices.set(i, v);
	}
}

void PropMesher::add_color(const Color &color) {
	_last_color = color;
}

Color PropMesher::get_color(const int idx) const {
	return _vertices.get(idx).color;
}

PoolVector<Vector2> PropMesher::get_uvs() const {
	PoolVector<Vector2> arr;

	arr.resize(_vertices.size());
	for (int i = 0; i < _vertices.size(); ++i) {
		arr.set(i, _vertices.get(i).uv);
	}

	return arr;
}

void PropMesher::set_uvs(const PoolVector<Vector2> &values) {
	ERR_FAIL_COND(values.size() != _vertices.size());

	for (int i = 0; i < _vertices.size(); ++i) {
		Vertex v = _vertices[i];

		v.uv = values[i];

		_vertices.set(i, v);
	}
}

void PropMesher::add_uv(const Vector2 &uv) {
	_last_uv = uv;
}

Vector2 PropMesher::get_uv(const int idx) const {
	return _vertices.get(idx).uv;
}

PoolVector<Vector2> PropMesher::get_uv2s() const {
	PoolVector<Vector2> arr;

	arr.resize(_vertices.size());
	for (int i = 0; i < _vertices.size(); ++i) {
		arr.set(i, _vertices.get(i).uv2);
	}

	return arr;
}

void PropMesher::set_uv2s(const PoolVector<Vector2> &values) {
	ERR_FAIL_COND(values.size() != _vertices.size());

	for (int i = 0; i < _vertices.size(); ++i) {
		Vertex v = _vertices[i];

		v.uv2 = values[i];

		_vertices.set(i, v);
	}
}

void PropMesher::add_uv2(const Vector2 &uv) {
	_last_uv2 = uv;
}

Vector2 PropMesher::get_uv2(const int idx) const {
	return _vertices.get(idx).uv2;
}

PoolVector<int> PropMesher::get_indices() const {
	return _indices;
}

void PropMesher::set_indices(const PoolVector<int> &values) {
	_indices = values;
}

int PropMesher::get_indices_count() const {
	return _indices.size();
}

void PropMesher::add_indices(const int index) {
	_indices.push_back(index);
}

int PropMesher::get_index(const int idx) const {
	return _indices.get(idx);
}

void PropMesher::remove_index(const int idx) {
	_indices.remove(idx);
}

PropMesher::PropMesher() {
	_mesher_index = 0;
	_voxel_scale = 1;
	_ao_strength = 0.25;
	_base_light_value = 0.5;
	_uv_margin = Rect2(0, 0, 1, 1);
	_format = 0;
	_channel_index_type = 0;
	_channel_index_isolevel = 0;
	_texture_scale = 1;

	_build_flags = PropMesher::BUILD_FLAG_CREATE_COLLIDER | PropMesher::BUILD_FLAG_CREATE_LODS;

	_format = VisualServer::ARRAY_FORMAT_NORMAL | VisualServer::ARRAY_FORMAT_TEX_UV;
}

PropMesher::~PropMesher() {
}

void PropMesher::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_channel_index_type"), &PropMesher::get_channel_index_type);
	ClassDB::bind_method(D_METHOD("set_channel_index_type", "value"), &PropMesher::set_channel_index_type);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "channel_index_type"), "set_channel_index_type", "get_channel_index_type");

	ClassDB::bind_method(D_METHOD("get_channel_index_isolevel"), &PropMesher::get_channel_index_isolevel);
	ClassDB::bind_method(D_METHOD("set_channel_index_isolevel", "value"), &PropMesher::set_channel_index_isolevel);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "channel_index_isolevel"), "set_channel_index_isolevel", "get_channel_index_isolevel");

	ClassDB::bind_method(D_METHOD("get_mesher_index"), &PropMesher::get_mesher_index);
	ClassDB::bind_method(D_METHOD("set_mesher_index", "value"), &PropMesher::set_mesher_index);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "mesher_index"), "set_mesher_index", "get_mesher_index");

	ClassDB::bind_method(D_METHOD("get_format"), &PropMesher::get_format);
	ClassDB::bind_method(D_METHOD("set_format", "value"), &PropMesher::set_format);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "format"), "set_format", "get_format");

	ClassDB::bind_method(D_METHOD("get_texture_scale"), &PropMesher::get_texture_scale);
	ClassDB::bind_method(D_METHOD("set_texture_scale", "value"), &PropMesher::set_texture_scale);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "texture_scale"), "set_texture_scale", "get_texture_scale");

	ClassDB::bind_method(D_METHOD("get_material"), &PropMesher::get_material);
	ClassDB::bind_method(D_METHOD("set_material", "value"), &PropMesher::set_material);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material", PROPERTY_HINT_RESOURCE_TYPE, "Material"), "set_material", "get_material");

	ClassDB::bind_method(D_METHOD("get_voxel_scale"), &PropMesher::get_voxel_scale);
	ClassDB::bind_method(D_METHOD("set_voxel_scale", "value"), &PropMesher::set_voxel_scale);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "voxel_scale"), "set_voxel_scale", "get_voxel_scale");

	ClassDB::bind_method(D_METHOD("get_ao_strength"), &PropMesher::get_ao_strength);
	ClassDB::bind_method(D_METHOD("set_ao_strength", "value"), &PropMesher::set_ao_strength);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "ao_strength"), "set_ao_strength", "get_ao_strength");

	ClassDB::bind_method(D_METHOD("get_base_light_value"), &PropMesher::get_base_light_value);
	ClassDB::bind_method(D_METHOD("set_base_light_value", "value"), &PropMesher::set_base_light_value);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "base_light_value"), "set_base_light_value", "get_base_light_value");

	ClassDB::bind_method(D_METHOD("get_uv_margin"), &PropMesher::get_uv_margin);
	ClassDB::bind_method(D_METHOD("set_uv_margin", "value"), &PropMesher::set_uv_margin);
	ADD_PROPERTY(PropertyInfo(Variant::RECT2, "uv_margin"), "set_uv_margin", "get_uv_margin");

	ClassDB::bind_method(D_METHOD("get_build_flags"), &PropMesher::get_build_flags);
	ClassDB::bind_method(D_METHOD("set_build_flags", "value"), &PropMesher::set_build_flags);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "build_flags", PROPERTY_HINT_FLAGS, PropMesher::BINDING_STRING_BUILD_FLAGS), "set_build_flags", "get_build_flags");

#ifdef MESH_DATA_RESOURCE_PRESENT
	ClassDB::bind_method(D_METHOD("add_mesh_data_resource", "mesh", "position", "rotation", "scale", "uv_rect"), &PropMesher::add_mesh_data_resource, DEFVAL(Rect2(0, 0, 1, 1)), DEFVAL(Vector3(1.0, 1.0, 1.0)), DEFVAL(Vector3()), DEFVAL(Vector3()));
	ClassDB::bind_method(D_METHOD("add_mesh_data_resource_transform", "mesh", "transform", "uv_rect"), &PropMesher::add_mesh_data_resource_transform, DEFVAL(Rect2(0, 0, 1, 1)));
	ClassDB::bind_method(D_METHOD("add_mesh_data_resource_transform_colored", "mesh", "transform", "colors", "uv_rect"), &PropMesher::add_mesh_data_resource_transform_colored, DEFVAL(Rect2(0, 0, 1, 1)));
#endif

	ClassDB::bind_method(D_METHOD("generate_ao"), &PropMesher::generate_ao);
	ClassDB::bind_method(D_METHOD("generate_random_ao", "seed", "octaves", "period", "persistence", "scale_factor"), &PropMesher::generate_random_ao, DEFVAL(4), DEFVAL(30), DEFVAL(0.3), DEFVAL(0.6));

	BIND_VMETHOD(MethodInfo("_add_mesher", PropertyInfo(Variant::OBJECT, "mesher", PROPERTY_HINT_RESOURCE_TYPE, "PropMesher")));
	ClassDB::bind_method(D_METHOD("add_mesher", "mesher"), &PropMesher::add_mesher);
	ClassDB::bind_method(D_METHOD("_add_mesher", "mesher"), &PropMesher::_add_mesher);

	ClassDB::bind_method(D_METHOD("get_vertices"), &PropMesher::get_vertices);
	ClassDB::bind_method(D_METHOD("set_vertices", "values"), &PropMesher::set_vertices);
	ClassDB::bind_method(D_METHOD("get_vertex_count"), &PropMesher::get_vertex_count);
	ClassDB::bind_method(D_METHOD("get_vertex", "idx"), &PropMesher::get_vertex);
	ClassDB::bind_method(D_METHOD("remove_vertex", "idx"), &PropMesher::remove_vertex);
	ClassDB::bind_method(D_METHOD("add_vertex", "vertex"), &PropMesher::add_vertex);

	ClassDB::bind_method(D_METHOD("get_normals"), &PropMesher::get_normals);
	ClassDB::bind_method(D_METHOD("set_normals", "values"), &PropMesher::set_normals);
	ClassDB::bind_method(D_METHOD("get_normal", "idx"), &PropMesher::get_normal);
	ClassDB::bind_method(D_METHOD("add_normal", "normal"), &PropMesher::add_normal);

	ClassDB::bind_method(D_METHOD("get_colors"), &PropMesher::get_colors);
	ClassDB::bind_method(D_METHOD("set_colors", "values"), &PropMesher::set_colors);
	ClassDB::bind_method(D_METHOD("get_color", "idx"), &PropMesher::get_color);
	ClassDB::bind_method(D_METHOD("add_color", "color"), &PropMesher::add_color);

	ClassDB::bind_method(D_METHOD("get_uvs"), &PropMesher::get_uvs);
	ClassDB::bind_method(D_METHOD("set_uvs", "values"), &PropMesher::set_uvs);
	ClassDB::bind_method(D_METHOD("get_uv", "idx"), &PropMesher::get_uv);
	ClassDB::bind_method(D_METHOD("add_uv", "uv"), &PropMesher::add_uv);

	ClassDB::bind_method(D_METHOD("get_uv2s"), &PropMesher::get_uv2s);
	ClassDB::bind_method(D_METHOD("set_uv2s", "values"), &PropMesher::set_uv2s);
	ClassDB::bind_method(D_METHOD("get_uv2", "idx"), &PropMesher::get_uv2);
	ClassDB::bind_method(D_METHOD("add_uv2", "uv"), &PropMesher::add_uv2);

	ClassDB::bind_method(D_METHOD("get_indices"), &PropMesher::get_indices);
	ClassDB::bind_method(D_METHOD("set_indices", "values"), &PropMesher::set_indices);
	ClassDB::bind_method(D_METHOD("get_indices_count"), &PropMesher::get_indices_count);
	ClassDB::bind_method(D_METHOD("get_index", "idx"), &PropMesher::get_index);
	ClassDB::bind_method(D_METHOD("remove_index", "idx"), &PropMesher::remove_index);
	ClassDB::bind_method(D_METHOD("add_indices", "indice"), &PropMesher::add_indices);

	ClassDB::bind_method(D_METHOD("reset"), &PropMesher::reset);

	//ClassDB::bind_method(D_METHOD("calculate_vertex_ambient_occlusion", "meshinstance_path", "radius", "intensity", "sampleCount"), &PropMesher::calculate_vertex_ambient_occlusion_path);

	ClassDB::bind_method(D_METHOD("build_mesh"), &PropMesher::build_mesh);
	ClassDB::bind_method(D_METHOD("build_mesh_into", "mesh_rid"), &PropMesher::build_mesh_into);
	ClassDB::bind_method(D_METHOD("build_collider"), &PropMesher::build_collider);

	ClassDB::bind_method(D_METHOD("generate_normals", "flip"), &PropMesher::generate_normals, DEFVAL(false));

	ClassDB::bind_method(D_METHOD("remove_doubles"), &PropMesher::remove_doubles);
	ClassDB::bind_method(D_METHOD("remove_doubles_hashed"), &PropMesher::remove_doubles_hashed);
}
