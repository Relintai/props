# Props Module

This is a c++ engine module for the Godot Engine.

It gives you props, and editor utilities to convert scenes to props.

## Godot Version Support

This branch tries to follow godot's master branch (as much as I have time).

For different godot versions look at the other branches.

Status for this branch: Update for 4.0 is work in progress.

# Pre-built binaries

You can grab a pre-built editor binary from the [Broken Seals](https://github.com/Relintai/broken_seals/releases) 
repo, should you want to. It contains all my modules.

# Optional Dependencies

[Mesh Data Resource](https://github.com/Relintai/mesh_data_resource): Support for merged meshes, even in gles2.\
[Texture Packer](https://github.com/Relintai/texture_packer): Prop Instance will use this to merge textures.\
[Thread Pool](https://github.com/Relintai/thread_pool): Prop Instance will use this for multithreaded generation.

# PropData

Props are basicly 3D scenes in a simple format, so other things can easily process them without instancing.

For example if you create a building from MeshDataInstances, and then convert that scene to a prop, Voxelman 
can spawn it, merge it's meshes, and create lods without any scene instancing.

PropData is the main class you'll use, it's main purpose it to store a list of PropDataEntries.

# PropDataEntries

These are the classes that actually store data.

They contain 4 methods for scene->prop conversion, namely:

```
//Whether or not this PropDataEntry can process the given Node.
virtual bool _processor_handles(Node *node);

//Save the given Node into the given prop_data any way you like, at tranform.
virtual void _processor_process(Ref<PropData> prop_data, Node *node, const Transform &transform);

//Turn PropDataEntry back into a Node
virtual Node *_processor_get_node_for(const Transform &transform);

//Whether the system should skip evaluating the children of a processes Node or not. See PropDataScene, or PropDataProp.
virtual bool _processor_evaluate_children();
```

# PropInstances

PropInstances are not yet finished.

They will be able to merge meshes, texture etc from a Prop, and also generate lods for it.

Essentially they will be a more advanced MeshInstance.

Voxelman implements all of this, just haven't finished porting it yet.

# PropUtils Singleton

The PropUtils singleton helps with scene->prop conversion.

You can register new PropDataEntries as processors, should you need to.

# Scene conversion

You can either click the new "To Prop" button on the menubar of the 3D scene for a quick conversion, 
or look into Project->Tools.

# Building

1. Get the source code for the engine.

If you want Godot 3.2:
```git clone -b 3.2 https://github.com/godotengine/godot.git godot```

If you want Godot 4.0:
```git clone https://github.com/godotengine/godot.git godot```


2. Go into Godot's modules directory.

```
cd ./godot/modules/
```

3. Clone this repository

```
git clone https://github.com/Relintai/props props
```

4. Build Godot. [Tutorial](https://docs.godotengine.org/en/latest/development/compiling/index.html)


