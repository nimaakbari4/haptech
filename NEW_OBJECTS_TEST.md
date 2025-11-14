# New Objects Test Configuration

## Purpose
This configuration file (`new_objects_test.xml`) is specifically designed for testing newly added object types to the haptic system. It provides a dedicated testing environment that can be easily extended as new object classes are implemented.

## Current Objects Being Tested

### Torus Objects
The file currently contains 8 different torus configurations to test various parameter combinations:

1. **basic_torus** - Standard medium-sized torus with default quality
2. **thin_ring** - Thin ring-like torus (small difference between inner/outer radius)
3. **thick_donut** - Thick donut-shaped torus (large difference between radii)
4. **large_torus** - Large high-quality torus (high num_sides/num_rings)
5. **low_res_torus** - Low-resolution torus (for performance testing)
6. **tiny_torus** - Very small torus
7. **wire_ring** - Extremely thin torus (wire-like)
8. **elevated_torus** - Torus positioned at a different Z height

### Reference Objects
- **floor_ref** - Thin cylinder acting as a floor reference
- **center_ref** - Small sphere at the origin for spatial reference
- **cursor** - Red haptic cursor for interaction

## Configuration Details
- **Duration:** 45 seconds (longer than simple tests for thorough exploration)
- **Background:** Darker blue-gray for better object visibility
- **Camera:** Positioned for optimal viewing of the test layout
- **Layout:** Objects arranged in a grid pattern around the origin

## Usage
```bash
cd /home/peter/haptech/python_client
python3 parser.py new_objects_test.xml  # Test parsing
python3 main.py new_objects_test.xml    # Run the trial
```

## Adding New Object Types
When adding new object types in the future:

1. Add new object instances to the `<Objects>` section
2. Update the `<StartMessage>` to mention the new object type being tested
3. Consider adding different parameter variations to test edge cases
4. Maintain the organized layout for easy visual identification

## Torus Parameter Testing
The torus objects test various parameter combinations:
- **inner_radius:** 0.01 to 0.04 (range of hole sizes)
- **outer_radius:** 0.025 to 0.09 (range of overall sizes)
- **num_sides:** 6 to 32 (mesh resolution testing)
- **num_rings:** 8 to 40 (radial resolution testing)

This provides good coverage for testing the torus object's parameter space and visual/haptic quality at different resolutions.