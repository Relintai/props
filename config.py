

def can_build(env, platform):
  return True


def configure(env):
	pass


def get_doc_classes():
    return [
        "PropDataEntity",
        "PropDataEntry",
        "PropDataLight",
        "PropDataProp",
        "PropDataScene",
        "PropData",

        "PropDataProcessor",

        "GroundClutterFoliage",
        "GroundClutter",

        "PropESSEntity",
        "PropInstance",
        "PropMeshUtils",

        "PropUtils",
    ]

def get_doc_path():
    return "doc_classes"

