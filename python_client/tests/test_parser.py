"""Simple unit test for valid_test_config.xml"""
import pytest
import sys
import os

# Add the parent directory to the path so we can import our modules
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from parser import xml_parser


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

if __name__ == "__main__":
    test_valid_test_config()
    print("Test completed successfully!")