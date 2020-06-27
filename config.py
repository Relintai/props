

def can_build(env, platform):
  return True


def configure(env):
	pass


def get_doc_classes():
    return [
        "PropDataEntity",
        "PropDataEntry",
        "PropDataLight",
        "PropDataMesh",
        "PropDataProp",
        "PropDataScene",
        "PropData",

        "GroundClutterFoliage",
        "GroundClutter",

        "PropESSEntity",
        "PropInstance",
        "PropMeshUtils",
    ]

def get_doc_path():
    return "doc_classes"

