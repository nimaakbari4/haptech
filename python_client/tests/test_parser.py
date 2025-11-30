"""Simple unit test for valid_test_config.xml"""
import pytest
import sys
import os
import xml.etree.ElementTree as ET

# Add the parent directory to the path so we can import our modules
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from parser import xml_parser, Condition, ObjectConfig, ObjectParameter


def test_valid_test_config():
    """Test parsing the valid_test_config.xml file"""
    # Path to the test config file
    config_path = os.path.join(os.path.dirname(__file__), "test_configs", "valid_test_config.xml")
    
    # Parse the configuration
    config = xml_parser(config_path)
    
    # Test basic properties
    assert config.test is True
    
    # Test that we have one object
    assert len(config.objects) == 1
    
    # Test the box object properties
    box = config.objects[0]
    assert box.id == "test_box"
    assert box.type == "box"
    
    # Test position
    assert box.position.x == 0.1
    assert box.position.y == 0.2
    assert box.position.z == 0.3
    
    # Test orientation
    assert box.orientation.x == 30
    assert box.orientation.y == 40
    assert box.orientation.z == 50
    
    # Test color
    assert box.color.r == 0.1
    assert box.color.g == 0.2
    assert box.color.b == 0.3
    
    # Test parameters
    box_params = {p.key: p.value for p in box.params}
    assert "width" in box_params
    assert "height" in box_params  
    assert "depth" in box_params
    assert box_params["width"] == "0.1"
    assert box_params["height"] == "0.2"
    assert box_params["depth"] == "0.3"


def test_condition_post_init():
    """Test that Condition.__post_init__ properly initializes empty parameters"""
    # Create a condition with None parameters
    condition = Condition(name="test", success=True, priority=1, parameters=None)
    
    # The __post_init__ method should have set parameters to an empty list
    assert condition.parameters == []
    assert isinstance(condition.parameters, list)


def test_object_config_post_init():
    """Test that ObjectConfig.__post_init__ properly initializes empty params"""
    from parser import ObjectConfig, Position
    
    # Create an object config with None params
    obj_config = ObjectConfig(
        id="test",
        type="box", 
        position=Position(0, 0, 0),
        orientation=Position(0, 0, 0),
        color=None,
        params=None
    )
    
    # The __post_init__ method should have set params to an empty list
    assert obj_config.params == []
    assert isinstance(obj_config.params, list)


def test_malformed_xml():
    """Test that malformed XML raises appropriate exception"""
    config_path = os.path.join(os.path.dirname(__file__), "test_configs", "malformed_xml.xml")
    
    # Should raise ET.ParseError for malformed XML
    with pytest.raises(ET.ParseError):
        xml_parser(config_path)


def test_missing_position_config():
    """Test parsing config with missing position element"""
    config_path = os.path.join(os.path.dirname(__file__), "test_configs", "missing_position.xml")
    
    # This should raise AttributeError when trying to access position that doesn't exist
    with pytest.raises(AttributeError):
        xml_parser(config_path)


def test_invalid_color_values():
    """Test parsing config with invalid color values"""
    config_path = os.path.join(os.path.dirname(__file__), "test_configs", "invalid_color_values.xml")
    
    # This should raise ValueError when trying to convert "invalid_value" to float
    with pytest.raises(ValueError):
        xml_parser(config_path)


def test_invalid_data_types():
    """Test parsing config with invalid data types"""
    config_path = os.path.join(os.path.dirname(__file__), "test_configs", "invalid_data_types.xml")
    
    # This should raise ValueError when trying to convert "not_a_number" to float
    with pytest.raises(ValueError):
        xml_parser(config_path)


def test_nonexistent_file():
    """Test that parsing a non-existent file raises appropriate exception"""
    config_path = os.path.join(os.path.dirname(__file__), "test_configs", "does_not_exist.xml")
    
    # Should raise FileNotFoundError for non-existent file
    with pytest.raises(FileNotFoundError):
        xml_parser(config_path)

def test_comprehensive_coverage_parsing():
    """Test parsing a comprehensive config that covers various code paths"""
    config_path = os.path.join(os.path.dirname(__file__), "test_configs", "comprehensive_coverage.xml")
    
    # Parse the configuration
    config = xml_parser(config_path)
    
    # Should have 4 objects
    assert len(config.objects) == 4
    
    # Find each object type
    objects_by_id = {obj.id: obj for obj in config.objects}
    
    # Test object with both color and parameters
    full_obj = objects_by_id["full_object"]
    assert full_obj.color is not None
    assert len(full_obj.params) > 0
    
    # Test object without color but with parameters
    no_color_obj = objects_by_id["no_color_object"]
    assert no_color_obj.color is None
    assert len(no_color_obj.params) > 0
    
    # Test object with color but no parameters
    no_params_obj = objects_by_id["no_params_object"]
    assert no_params_obj.color is not None
    assert len(no_params_obj.params) == 0
    
    # Test object with neither color nor parameters
    minimal_obj = objects_by_id["minimal_object"]
    assert minimal_obj.color is None
    assert len(minimal_obj.params) == 0


def test_background_color_methods():
    """Test BackgroundColor class methods"""
    from parser import BackgroundColor
    
    # Test from_rgb_255
    color1 = BackgroundColor.from_rgb_255(128, 64, 255)
    assert abs(color1.r - 0.502) < 0.01  # 128/255 ≈ 0.502
    assert abs(color1.g - 0.251) < 0.01  # 64/255 ≈ 0.251
    assert abs(color1.b - 1.0) < 0.01    # 255/255 = 1.0
    
    # Test from_rgb_float
    color2 = BackgroundColor.from_rgb_float(0.5, 0.25, 1.0)
    assert color2.r == 0.5
    assert color2.g == 0.25
    assert color2.b == 1.0
    
    # Test to_rgb_255
    rgb_255 = color2.to_rgb_255()
    assert rgb_255 == (127, 63, 255)  # int(0.5*255), int(0.25*255), int(1.0*255)
    
    # Test __str__ method
    color_str = str(color2)
    assert "RGB(0.50, 0.25, 1.00)" in color_str
    assert "RGB(127, 63, 255)" in color_str


def test_text_overlays_config():
    """Test parsing config with text overlays"""
    config_path = os.path.join(os.path.dirname(__file__), "test_configs", "text_overlays_config.xml")
    
    # Parse the configuration
    config = xml_parser(config_path)
    
    # Test background color conversion from 255 range
    bg_color = config.scene_settings.background_color
    assert abs(bg_color.r - 0.502) < 0.01  # 128/255
    assert abs(bg_color.g - 0.251) < 0.01  # 64/255
    assert bg_color.b == 1.0  # 255/255
    
    # Test text overlays
    overlays = config.scene_settings.text_overlays
    assert len(overlays) == 2
    
    # First overlay with color
    overlay1 = overlays[0]
    assert overlay1.text == "Test overlay with color"
    assert overlay1.x == 10
    assert overlay1.y == 20
    assert overlay1.font_size == 24
    assert overlay1.color is not None
    assert overlay1.color.r == 1.0
    assert overlay1.color.g == 0.5
    assert overlay1.color.b == 0.0
    
    # Second overlay without color
    overlay2 = overlays[1]
    assert overlay2.text == "Test overlay without color"
    assert overlay2.x == 50
    assert overlay2.y == 60
    assert overlay2.font_size == 16
    assert overlay2.color is None


def test_data_collection_config():
    """Test parsing config with data collection settings"""
    config_path = os.path.join(os.path.dirname(__file__), "test_configs", "data_collection_config.xml")
    
    # Parse the configuration
    config = xml_parser(config_path)
    
    # Test home position
    assert config.home is not None
    assert config.home.position.x == 0.0
    assert config.home.position.y == 0.0
    assert config.home.position.z == 0.1
    assert config.home.tolerance == 0.01
    
    # Test cursor settings (flat format)
    cursor = config.cursor_settings
    assert cursor is not None
    assert cursor.object_type == "spring"
    assert cursor.color is not None
    assert cursor.color.r == 0.8
    assert cursor.color.g == 0.2
    assert cursor.color.b == 0.6
    assert len(cursor.params) == 2
    
    # Test data collection
    dc = config.data_collection
    assert dc is not None
    
    # Test device settings
    device = dc.device
    assert device.position is True
    assert device.velocity is False
    assert device.angular_velocity is True
    assert device.orientation is False
    assert device.proxy_position is True
    assert device.force is True
    
    # Test objects settings
    objects = dc.objects
    assert objects.position is True
    assert objects.orientation is True
    assert objects.contact is False
    assert objects.contact_point is True
    assert objects.contact_normal is False
    assert objects.penetration_depth is True
    assert objects.force is True
    assert objects.spring_force is False
    assert objects.damping_force is True


def test_nested_cursor_config():
    """Test parsing config with nested cursor settings format"""
    config_path = os.path.join(os.path.dirname(__file__), "test_configs", "nested_cursor_config.xml")
    
    # Parse the configuration
    config = xml_parser(config_path)
    
    # Test cursor settings (nested format)
    cursor = config.cursor_settings
    assert cursor is not None
    assert cursor.object_type == "sphere"
    assert cursor.color is not None
    assert cursor.color.r == 1.0
    assert cursor.color.g == 0.0
    assert cursor.color.b == 1.0
    assert len(cursor.params) == 1
    assert cursor.params[0].key == "radius"
    assert cursor.params[0].value == "0.02"


def test_cursor_settings_post_init():
    """Test CursorSettings __post_init__ method"""
    from parser import CursorSettings
    
    # Create cursor settings with None params
    cursor = CursorSettings(object_type="sphere", params=None)
    
    # The __post_init__ method should have set params to an empty list
    assert cursor.params == []
    assert isinstance(cursor.params, list)


def test_scene_settings_post_init():
    """Test SceneSettings __post_init__ method"""
    from parser import SceneSettings, BackgroundColor, Camera, Position
    
    # Create scene settings with None text_overlays
    bg_color = BackgroundColor(r=0.5, g=0.5, b=0.5)
    camera = Camera(
        position=Position(0, 0, 1),
        look_at=Position(0, 0, 0),
        up=Position(0, 1, 0)
    )
    scene = SceneSettings(background_color=bg_color, camera=camera, text_overlays=None)
    
    # The __post_init__ method should have set text_overlays to an empty list
    assert scene.text_overlays == []
    assert isinstance(scene.text_overlays, list)


def test_cli_usage():
    """Test CLI usage of the parser"""
    import subprocess
    import tempfile
    
    # Create a temporary valid config file with comprehensive content
    with tempfile.NamedTemporaryFile(mode='w', suffix='.xml', delete=False) as f:
        f.write("""<?xml version="1.0" encoding="utf-8"?>
<Trial>
    <Test>true</Test>
    <StartConditions>
        <Condition name="start" success="true" priority="1">
            <Parameters timeout="5.0"/>
        </Condition>
    </StartConditions>
    <EndConditions>
        <Condition name="end" success="true" priority="1"/>
    </EndConditions>
    <HomePosition>
        <Position x="0.0" y="0.0" z="0.1"/>
        <Tolerance>0.01</Tolerance>
    </HomePosition>
    <CursorSettings object="spring">
        <Color r="0.8" g="0.2" b="0.6"/>
    </CursorSettings>
    <SceneSettings>
        <BackgroundColor r="0.5" g="0.5" b="0.5"/>
        <Camera>
            <Position x="0" y="0" z="1"/>
            <LookAt x="0" y="0" z="0"/>
            <Up x="0" y="1" z="0"/>
        </Camera>
    </SceneSettings>
    <Objects>
        <Object id="test_box" type="box">
            <Position x="0.1" y="0.2" z="0.3"/>
            <Orientation x="30" y="40" z="50"/>
            <Color r="1.0" g="0.5" b="0.2"/>
            <Parameters width="0.1" height="0.2"/>
        </Object>
    </Objects>
</Trial>""")
        temp_file = f.name
    
    try:
        # Test CLI execution by actually running the main section
        result = subprocess.run([
            'python3', 'parser.py', temp_file
        ], 
        capture_output=True, text=True, cwd='/home/peter/haptech/python_client')
        
        assert result.returncode == 0
        # Check that various sections are printed
        assert "Home Position:" in result.stdout
        assert "Start Conditions:" in result.stdout
        assert "End Conditions:" in result.stdout
        assert "Scene Settings:" in result.stdout
        assert "Cursor Settings:" in result.stdout
        assert "Objects:" in result.stdout
        assert "test_box" in result.stdout
        
    finally:
        # Clean up
        import os
        os.unlink(temp_file)


def test_cursor_default_object_type():
    """Test parsing config with cursor settings that defaults to sphere"""
    config_path = os.path.join(os.path.dirname(__file__), "test_configs", "cursor_default_type_config.xml")
    
    # Parse the configuration
    config = xml_parser(config_path)
    
    # Test cursor settings (should default to "sphere" when no type is specified)
    cursor = config.cursor_settings
    assert cursor is not None
    assert cursor.object_type == "sphere"  # Default fallback
    assert cursor.color is not None
    assert cursor.color.r == 1.0
    assert cursor.color.g == 0.0
    assert cursor.color.b == 1.0


def test_text_overlay_post_init():
    """Test TextOverlay creation with optional color"""
    from parser import TextOverlay, BackgroundColor
    
    # Test without color
    overlay1 = TextOverlay(text="test", x=10, y=20)
    assert overlay1.color is None
    
    # Test with color
    color = BackgroundColor(r=1.0, g=0.5, b=0.0)
    overlay2 = TextOverlay(text="test", x=10, y=20, color=color)
    assert overlay2.color == color


if __name__ == "__main__":
    test_valid_test_config()
    test_condition_post_init()
    test_object_config_post_init()
    test_comprehensive_coverage_parsing()
    test_background_color_methods()
    test_text_overlays_config()
    test_data_collection_config()
    test_nested_cursor_config()
    test_cursor_settings_post_init()
    test_scene_settings_post_init()
    test_cursor_default_object_type()
    test_text_overlay_post_init()
    test_cli_usage()
    
    print("Testing malformed XML...")
    try:
        test_malformed_xml()
        print("✓ Malformed XML test passed")
    except Exception as e:
        print(f"✗ Malformed XML test failed: {e}")
    
    print("Testing missing position...")
    try:
        test_missing_position_config()
        print("✓ Missing position test passed")
    except Exception as e:
        print(f"✗ Missing position test failed: {e}")
    
    print("Testing invalid color values...")
    try:
        test_invalid_color_values()
        print("✓ Invalid color values test passed")
    except Exception as e:
        print(f"✗ Invalid color values test failed: {e}")
    
    print("Testing invalid data types...")
    try:
        test_invalid_data_types()
        print("✓ Invalid data types test passed")
    except Exception as e:
        print(f"✗ Invalid data types test failed: {e}")
    
    print("Testing nonexistent file...")
    try:
        test_nonexistent_file()
        print("✓ Nonexistent file test passed")
    except Exception as e:
        print(f"✗ Nonexistent file test failed: {e}")
    
    print("All tests completed!")