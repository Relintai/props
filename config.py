

def can_build(env, platform):
  return True


def configure(env):
	pass


def get_doc_classes():
    return [
        "PropDataEntry",
        "PropDataLight",
        "PropDataProp",
        "PropDataScene",
        "PropDataPortal",
        "PropDataTiledWall",
        "PropData",
        
        "TiledWall",
        "TiledWallData",

        "PropDataProcessor",

        "GroundClutterFoliage",
        "GroundClutter",

        "PropESSEntity",
        "PropInstance",
        "PropMeshUtils",

        "PropSceneInstance",

        "PropUtils",
    ]

def get_doc_path():
    return "doc_classes"

