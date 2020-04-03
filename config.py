

def can_build(env, platform):
  return True


def configure(env):
	pass


def get_doc_classes():
    return [

        "GroundClutterFoliage",
        "GroundClutter",

        "PropDataEntity",
        "PropDataEntry",
        "PropDataLight",
        "PropDataMesh",
        "PropDataProp",
        "PropDataScene",
        "PropData",

        "PropTool",
        "PropToolEntity",
        "PropToolLight",
        "PropToolMesh",
        "PropToolProp",
        "PropToolScene",
    ]

def get_doc_path():
    return "doc_classes"

