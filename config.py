

def can_build(env, platform):
  return True


def configure(env):
	pass


def get_doc_classes():
    return [

        "GroundClutterFoliage",
        "GroundClutter",

        "PropESSEntity",
        "PropInstance",
        "PropMeshDataInstance",
        "PropMeshUtils",
        "PropVoxelmanLight",
    ]

def get_doc_path():
    return "doc_classes"

