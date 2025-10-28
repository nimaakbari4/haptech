import xml.etree.ElementTree as ET
from dataclasses import dataclass
from typing import List, Optional
import sys

@dataclass
class Position:
    x: float
    y: float
    z: float

    @staticmethod
    def from_elem(elem: ET.Element) -> "Position":
        return Position(
            x=float(elem.get("x", 0.0)),
            y=float(elem.get("y", 0.0)),
            z=float(elem.get("z", 0.0))
        )

@dataclass
class ObjectParameter:
    key: str
    value: str

@dataclass
class Condition:
    name: str
    success: bool
    priority: int
    parameters: List[ObjectParameter] = None
    
    def __post_init__(self):
        if self.parameters is None:
            self.parameters = []

@dataclass
class StartConditions:
    conditions: List[Condition]

@dataclass
class EndConditions:
    conditions: List[Condition]

@dataclass
class HUDText:
    start_message: str
    success_message: str
    failure_message: str

@dataclass
class BackgroundColor:
    r: float
    g: float
    b: float

@dataclass
class Camera:
    position: Position
    look_at: Position
    up: Position

@dataclass
class SceneSettings:
    background_color: BackgroundColor
    camera: Camera

@dataclass
class ObjectConfig:
    id: str
    type: str
    position: Position
    orientation: Position  # roll, pitch, yaw in degrees
    color: Optional[BackgroundColor] = None
    params: List[ObjectParameter] = None
    
    def __post_init__(self):
        if self.params is None:
            self.params = []

@dataclass
class DeviceDataConfig:
    position: bool = True
    velocity: bool = True
    angular_velocity: bool = True
    orientation: bool = True
    proxy_position: bool = True
    force: bool = True

@dataclass
class ObjectDataConfig:
    position: bool = True
    orientation: bool = True
    contact: bool = True
    contact_point: bool = True
    contact_normal: bool = True
    penetration_depth: bool = True
    force: bool = True
    spring_force: bool = True
    damping_force: bool = True

@dataclass
class DataCollectionConfig:
    device: DeviceDataConfig
    objects: ObjectDataConfig

@dataclass
class TrialConfig:
    start_conditions: StartConditions
    end_conditions: EndConditions
    hud_text: HUDText
    scene_settings: SceneSettings
    objects: List[ObjectConfig]
    data_collection: Optional[DataCollectionConfig] = None


def xml_parser(xml_path: str) -> TrialConfig:
    tree = ET.parse(xml_path)
    root = tree.getroot()

    #StartConditions
    start_conds = []
    for c in root.find("StartConditions").findall("Condition"):
        # Parse parameters from Parameters element (consistent with Objects)
        params = []
        params_elem = c.find("Parameters")
        if params_elem is not None:
            for key, value in params_elem.attrib.items():
                params.append(ObjectParameter(key=key, value=value))
        cond = Condition(
            name = c.get("name"),
            success = c.get("success", "false").lower() == "true",
            priority = int(c.get("priority", "0")),
            parameters = params
        )
        start_conds.append(cond)

    start_conditions = StartConditions(conditions=start_conds)

    # EndConditions
    end_conds = []
    for c in root.find("EndConditions").findall("Condition"):
        # Parse parameters from Parameters element (consistent with Objects)
        params = []
        params_elem = c.find("Parameters")
        if params_elem is not None:
            for key, value in params_elem.attrib.items():
                params.append(ObjectParameter(key=key, value=value))
        
        cond = Condition(
            name = c.get("name"),
            success = c.get("success", "false").lower() == "true",
            priority = int(c.get("priority", "0")),
            parameters = params
        )
        end_conds.append(cond)
    end_conditions = EndConditions(conditions=end_conds)

    # HUDText
    hud = root.find("HUDText")
    hud_text = HUDText(
        start_message   = hud.findtext("StartMessage"),
        success_message = hud.findtext("SuccessMessage"),
        failure_message = hud.findtext("FailureMessage")
    )

    # SceneSettings
    ss = root.find("SceneSettings")
    bg = ss.find("BackgroundColor")
    background_color = BackgroundColor(
        r=float(bg.get("r")),
        g=float(bg.get("g")),
        b=float(bg.get("b"))
    )
    cam = ss.find("Camera")
    camera = Camera(
        position = Position.from_elem(cam.find("Position")),
        look_at = Position.from_elem(cam.find("LookAt")),
        up = Position.from_elem(cam.find("Up"))
    )
    scene_settings = SceneSettings(
        background_color=background_color,
        camera=camera
    )

    # Objects
    objs = []
    for obj in root.find("Objects").findall("Object"):
        position    = Position.from_elem(obj.find("Position"))
        orientation = Position.from_elem(obj.find("Orientation"))
        
        # Convert color
        color_elem = obj.find("Color")
        color = (
            BackgroundColor(
                r=float(color_elem.get("r")),
                g=float(color_elem.get("g")),
                b=float(color_elem.get("b"))
            )
            if color_elem is not None
            else None
        )
        
        # Convert all parameters from the Parameters XML element
        params = []
        
        # Handle the new Parameters element
        params_elem = obj.find("Parameters")
        if params_elem is not None:
            for key, value in params_elem.attrib.items():
                params.append(ObjectParameter(key=key, value=value))

        objs.append(ObjectConfig(
            id          = obj.get("id"),
            type        = obj.get("type"),
            position    = position,
            orientation = orientation,
            color       = color,
            params      = params
        ))

    # Optional DataCollection section
    data_collection = None
    dc_elem = root.find("DataCollection")
    if dc_elem is not None:
        # Device config
        device_cfg = DeviceDataConfig()
        device_elem = dc_elem.find("Device")
        if device_elem is not None:
            for child in device_elem:
                enabled = child.get("enabled", "true").lower() == "true"
                # Convert CamelCase tag to snake_case field
                name = child.tag[0].lower() + child.tag[1:]
                name = ''.join(['_' + c.lower() if c.isupper() else c for c in name]).lstrip('_')
                if hasattr(device_cfg, name):
                    setattr(device_cfg, name, enabled)

        # Objects config
        objects_cfg = ObjectDataConfig()
        objects_elem = dc_elem.find("Objects")
        if objects_elem is not None:
            for child in objects_elem:
                enabled = child.get("enabled", "true").lower() == "true"
                name = child.tag[0].lower() + child.tag[1:]
                name = ''.join(['_' + c.lower() if c.isupper() else c for c in name]).lstrip('_')
                if hasattr(objects_cfg, name):
                    setattr(objects_cfg, name, enabled)

        data_collection = DataCollectionConfig(device=device_cfg, objects=objects_cfg)

    return TrialConfig(
        start_conditions = start_conditions,
        end_conditions = end_conditions,
        hud_text       = hud_text,
        scene_settings = scene_settings,
        objects        = objs,
        data_collection = data_collection
    )


if __name__ == "__main__":
    cfg = xml_parser(sys.argv[1])

    print("\nStart Conditions:")
    for c in cfg.start_conditions.conditions:
        print(f" - {c.name} (success={c.success}, priority={c.priority})")
        for param in c.parameters:
            print(f"   {param.key}: {param.value}")

    print("\nEnd Conditions:")
    for c in cfg.end_conditions.conditions:
        print(f" - {c.name} (success={c.success}, priority={c.priority})")
        for param in c.parameters:
            print(f"   {param.key}: {param.value}")

    print("\nHUD Messages:")
    print(" Start:",   cfg.hud_text.start_message)
    print(" Success:", cfg.hud_text.success_message)
    print(" Failure:", cfg.hud_text.failure_message)

    print("\nScene Settings:")
    print(" Background color:", cfg.scene_settings.background_color)
    print(" Camera pos:",       cfg.scene_settings.camera.position)
    print(" Camera look-at:",   cfg.scene_settings.camera.look_at)
    print(" Camera up:",        cfg.scene_settings.camera.up)

    print("\nObjects:")
    for obj in cfg.objects:
        print(f" • [{obj.type}] {obj.id}")
        print("    Position:",    obj.position)
        print("    Orientation:", obj.orientation)
        if obj.color:    print("    Color:", obj.color)
        if obj.params:
            print("    Parameters:")
            for param in obj.params:
                print(f"      {param.key}: {param.value}")