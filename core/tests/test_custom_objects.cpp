#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Include custom object headers
#include "../src/objects/custom_object_interface.h"
#include "../src/objects/sphere_object.h"
#include "../src/objects/box_object.h"
#include "../src/objects/spring_object.h"
#include "../src/objects/cylinder_object.h"
#include "../src/objects/line_object.h"
#include "../src/objects/ellipsoid_object.h"
#include "../src/objects/ball_object.h"
#include "../src/objects/cup_object.h"
#include "../src/objects/torus_object.h"

// CHAI3D includes for mock objects
#include <chai3d.h>

using namespace chai3d;
using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;

// Base test fixture for custom objects
class CustomObjectTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset the registry for each test
        // Note: In a real implementation, we might need a way to clear the registry
        // For now, we'll work with the global state
    }

    void TearDown() override {
        // Clean up
    }

    // Helper to create parameter map
    std::map<std::string, std::string> createParams(const std::map<std::string, std::string>& values) {
        return values;
    }

    // Helper to validate CHAI3D object basics
    void validateChaiObject(cGenericObject* obj, double x, double y, double z) {
        ASSERT_NE(obj, nullptr);
        cVector3d pos = obj->getLocalPos();
        EXPECT_DOUBLE_EQ(pos.x(), x);
        EXPECT_DOUBLE_EQ(pos.y(), y);
        EXPECT_DOUBLE_EQ(pos.z(), z);
    }
};

// Tests for CustomObjectRegistry
class CustomObjectRegistryTest : public CustomObjectTest {
};

TEST_F(CustomObjectRegistryTest, RegisterAndCreateCustomType) {
    // Register a mock custom object type
    CustomObjectRegistry::registerType("test_object", []() -> std::unique_ptr<CustomObjectInterface> {
        return std::make_unique<SphereObject>();  // Use sphere as test implementation
    });
    
    // Check if type is registered
    EXPECT_TRUE(CustomObjectRegistry::isTypeRegistered("test_object"));
    
    // Create object
    auto obj = CustomObjectRegistry::createObject("test_object");
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->getTypeName(), "sphere");  // Should be sphere since that's what we registered
}

TEST_F(CustomObjectRegistryTest, CreateUnknownTypeReturnsNull) {
    auto obj = CustomObjectRegistry::createObject("unknown_type");
    EXPECT_EQ(obj, nullptr);
}

TEST_F(CustomObjectRegistryTest, CheckUnregisteredType) {
    EXPECT_FALSE(CustomObjectRegistry::isTypeRegistered("nonexistent_type"));
}

// Tests for SphereObject
class SphereObjectTest : public CustomObjectTest {
protected:
    SphereObject sphereObj;
};

TEST_F(SphereObjectTest, GetTypeName) {
    EXPECT_EQ(sphereObj.getTypeName(), "sphere");
}

TEST_F(SphereObjectTest, CreateObjectWithValidRadius) {
    auto params = createParams({{"radius", "0.1"}});
    
    cGenericObject* obj = sphereObj.createObject(params, 1.0, 2.0, 3.0, 0.8, 0.6, 0.4);
    
    validateChaiObject(obj, 1.0, 2.0, 3.0);
    
    // Check if it's a mesh (sphere creates cMesh)
    cMesh* mesh = dynamic_cast<cMesh*>(obj);
    EXPECT_NE(mesh, nullptr);
    
    // Clean up
    delete obj;
}

TEST_F(SphereObjectTest, CreateObjectWithDefaultRadius) {
    auto params = createParams({});  // No radius parameter
    
    cGenericObject* obj = sphereObj.createObject(params, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    
    ASSERT_NE(obj, nullptr);
    validateChaiObject(obj, 0.0, 0.0, 0.0);
    
    delete obj;
}

TEST_F(SphereObjectTest, CreateObjectWithInvalidRadius) {
    auto params = createParams({{"radius", "invalid_number"}});
    
    // Should handle invalid parameter gracefully and use default
    EXPECT_NO_THROW({
        cGenericObject* obj = sphereObj.createObject(params, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
        EXPECT_NE(obj, nullptr);  // Should still create object with default radius
        delete obj;
    });
}

TEST_F(SphereObjectTest, UpdatePhysics) {
    auto params = createParams({{"radius", "0.05"}});
    cGenericObject* obj = sphereObj.createObject(params, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    
    // updatePhysics should not crash (currently a stub)
    EXPECT_NO_THROW(sphereObj.updatePhysics(obj, 0.016));
    
    delete obj;
}

// Tests for BoxObject
class BoxObjectTest : public CustomObjectTest {
protected:
    BoxObject boxObj;
};

TEST_F(BoxObjectTest, GetTypeName) {
    EXPECT_EQ(boxObj.getTypeName(), "box");
}

TEST_F(BoxObjectTest, CreateObjectWithValidDimensions) {
    auto params = createParams({
        {"width", "0.2"},
        {"height", "0.3"},
        {"depth", "0.4"}
    });
    
    cGenericObject* obj = boxObj.createObject(params, 1.0, 2.0, 3.0, 0.8, 0.6, 0.4);
    
    validateChaiObject(obj, 1.0, 2.0, 3.0);
    
    // Check if it's a mesh
    cMesh* mesh = dynamic_cast<cMesh*>(obj);
    EXPECT_NE(mesh, nullptr);
    
    delete obj;
}

TEST_F(BoxObjectTest, CreateObjectWithMissingDimensions) {
    auto params = createParams({{"width", "0.2"}});  // Missing height and depth
    
    cGenericObject* obj = boxObj.createObject(params, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    
    ASSERT_NE(obj, nullptr);  // Should create with defaults
    validateChaiObject(obj, 0.0, 0.0, 0.0);
    
    delete obj;
}

TEST_F(BoxObjectTest, CreateObjectWithAllDefaults) {
    auto params = createParams({});  // No parameters
    
    cGenericObject* obj = boxObj.createObject(params, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    
    ASSERT_NE(obj, nullptr);
    validateChaiObject(obj, 0.0, 0.0, 0.0);
    
    delete obj;
}

TEST_F(BoxObjectTest, UpdatePhysics) {
    auto params = createParams({
        {"width", "0.1"},
        {"height", "0.1"},
        {"depth", "0.1"}
    });
    cGenericObject* obj = boxObj.createObject(params, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    
    // updatePhysics should not crash
    EXPECT_NO_THROW(boxObj.updatePhysics(obj, 0.016));
    
    delete obj;
}

// Tests for SpringObject
class SpringObjectTest : public CustomObjectTest {
protected:
    SpringObject springObj;
};

TEST_F(SpringObjectTest, GetTypeName) {
    EXPECT_EQ(springObj.getTypeName(), "spring");
}

TEST_F(SpringObjectTest, CreateObjectWithValidParameters) {
    auto params = createParams({
        {"radius", "0.03"},
        {"height", "0.15"},
        {"stiffness", "1500.0"},
        {"damping", "75.0"}
    });
    
    cGenericObject* obj = springObj.createObject(params, 1.0, 2.0, 3.0, 0.8, 0.6, 0.4);
    
    validateChaiObject(obj, 1.0, 2.0, 3.0);
    
    // Check if it's a multi-segment (spring creates cMultiSegment)
    cMultiSegment* multiSeg = dynamic_cast<cMultiSegment*>(obj);
    EXPECT_NE(multiSeg, nullptr);
    
    // Check material properties were set
    ASSERT_NE(multiSeg->m_material, nullptr);
    EXPECT_DOUBLE_EQ(multiSeg->m_material->getStiffness(), 1500.0);
    EXPECT_DOUBLE_EQ(multiSeg->m_material->getDamping(), 75.0);
    
    delete obj;
}

TEST_F(SpringObjectTest, CreateObjectWithDefaultParameters) {
    auto params = createParams({});  // No parameters, should use defaults
    
    cGenericObject* obj = springObj.createObject(params, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    
    ASSERT_NE(obj, nullptr);
    validateChaiObject(obj, 0.0, 0.0, 0.0);
    
    cMultiSegment* multiSeg = dynamic_cast<cMultiSegment*>(obj);
    ASSERT_NE(multiSeg, nullptr);
    
    // Check default values
    EXPECT_DOUBLE_EQ(multiSeg->m_material->getStiffness(), 1000.0);  // Default stiffness
    EXPECT_DOUBLE_EQ(multiSeg->m_material->getDamping(), 50.0);      // Default damping
    
    delete obj;
}

TEST_F(SpringObjectTest, CreateObjectWithPartialParameters) {
    auto params = createParams({
        {"stiffness", "2000.0"},
        {"height", "0.2"}
        // Missing radius and damping - should use defaults
    });
    
    cGenericObject* obj = springObj.createObject(params, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    
    ASSERT_NE(obj, nullptr);
    
    cMultiSegment* multiSeg = dynamic_cast<cMultiSegment*>(obj);
    ASSERT_NE(multiSeg, nullptr);
    
    // Check mixed parameters
    EXPECT_DOUBLE_EQ(multiSeg->m_material->getStiffness(), 2000.0);  // Custom value
    EXPECT_DOUBLE_EQ(multiSeg->m_material->getDamping(), 50.0);      // Default value
    
    delete obj;
}

TEST_F(SpringObjectTest, UpdatePhysics) {
    auto params = createParams({{"radius", "0.02"}});
    cGenericObject* obj = springObj.createObject(params, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    
    // updatePhysics should not crash
    EXPECT_NO_THROW(springObj.updatePhysics(obj, 0.016));
    
    delete obj;
}

// Tests for CylinderObject
class CylinderObjectTest : public CustomObjectTest {
protected:
    CylinderObject cylinderObj;
};

TEST_F(CylinderObjectTest, GetTypeName) {
    EXPECT_EQ(cylinderObj.getTypeName(), "cylinder");
}

TEST_F(CylinderObjectTest, CreateObjectWithValidParameters) {
    auto params = createParams({
        {"radius", "0.05"},
        {"height", "0.2"}
    });
    
    cGenericObject* obj = cylinderObj.createObject(params, 1.0, 2.0, 3.0, 0.8, 0.6, 0.4);
    
    validateChaiObject(obj, 1.0, 2.0, 3.0);
    
    // Should create a mesh
    cMesh* mesh = dynamic_cast<cMesh*>(obj);
    EXPECT_NE(mesh, nullptr);
    
    delete obj;
}

TEST_F(CylinderObjectTest, CreateObjectWithDefaults) {
    auto params = createParams({});
    
    cGenericObject* obj = cylinderObj.createObject(params, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    
    ASSERT_NE(obj, nullptr);
    validateChaiObject(obj, 0.0, 0.0, 0.0);
    
    delete obj;
}

// Tests for LineObject
class LineObjectTest : public CustomObjectTest {
protected:
    LineObject lineObj;
};

TEST_F(LineObjectTest, GetTypeName) {
    EXPECT_EQ(lineObj.getTypeName(), "line");
}

TEST_F(LineObjectTest, CreateObjectWithValidParameters) {
    auto params = createParams({
        {"length", "1.5"},
        {"thickness", "0.01"}
    });
    
    cGenericObject* obj = lineObj.createObject(params, 1.0, 2.0, 3.0, 0.8, 0.6, 0.4);
    
    validateChaiObject(obj, 1.0, 2.0, 3.0);
    
    // Line object creates cMultiSegment
    cMultiSegment* multiSeg = dynamic_cast<cMultiSegment*>(obj);
    EXPECT_NE(multiSeg, nullptr);
    
    delete obj;
}

TEST_F(LineObjectTest, CreateObjectWithDefaults) {
    auto params = createParams({});
    
    cGenericObject* obj = lineObj.createObject(params, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    
    ASSERT_NE(obj, nullptr);
    validateChaiObject(obj, 0.0, 0.0, 0.0);
    
    delete obj;
}

// Tests for EllipsoidObject
class EllipsoidObjectTest : public CustomObjectTest {
protected:
    EllipsoidObject ellipsoidObj;
};

TEST_F(EllipsoidObjectTest, GetTypeName) {
    EXPECT_EQ(ellipsoidObj.getTypeName(), "ellipsoid");
}

TEST_F(EllipsoidObjectTest, CreateObjectWithValidParameters) {
    auto params = createParams({
        {"radius_x", "0.1"},
        {"radius_y", "0.2"},
        {"radius_z", "0.15"}
    });
    
    cGenericObject* obj = ellipsoidObj.createObject(params, 1.0, 2.0, 3.0, 0.8, 0.6, 0.4);
    
    validateChaiObject(obj, 1.0, 2.0, 3.0);
    
    cMesh* mesh = dynamic_cast<cMesh*>(obj);
    EXPECT_NE(mesh, nullptr);
    
    delete obj;
}

TEST_F(EllipsoidObjectTest, CreateObjectWithDefaults) {
    auto params = createParams({});
    
    cGenericObject* obj = ellipsoidObj.createObject(params, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    
    ASSERT_NE(obj, nullptr);
    validateChaiObject(obj, 0.0, 0.0, 0.0);
    
    delete obj;
}

// Tests for TorusObject
class TorusObjectTest : public CustomObjectTest {
protected:
    TorusObject torusObj;
};

TEST_F(TorusObjectTest, GetTypeName) {
    EXPECT_EQ(torusObj.getTypeName(), "torus");
}

TEST_F(TorusObjectTest, CreateObjectWithValidParameters) {
    auto params = createParams({
        {"inner_radius", "0.03"},
        {"outer_radius", "0.08"}
    });
    
    cGenericObject* obj = torusObj.createObject(params, 1.0, 2.0, 3.0, 0.8, 0.6, 0.4);
    
    validateChaiObject(obj, 1.0, 2.0, 3.0);
    
    // Since it's a cShapeTorus, not a cMesh
    cShapeTorus* torus = dynamic_cast<cShapeTorus*>(obj);
    EXPECT_NE(torus, nullptr);
    
    delete obj;
}

TEST_F(TorusObjectTest, CreateObjectWithDefaults) {
    auto params = createParams({});
    
    cGenericObject* obj = torusObj.createObject(params, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    
    ASSERT_NE(obj, nullptr);
    validateChaiObject(obj, 0.0, 0.0, 0.0);
    
    delete obj;
}

// === BALL OBJECT TESTS ===
class BallObjectTest : public CustomObjectTest {
protected:
    BallObject ballObj;
    
    void SetUp() override {
        CustomObjectTest::SetUp();
    }
};

TEST_F(BallObjectTest, GetTypeName) {
    EXPECT_EQ(ballObj.getTypeName(), "ball");
}

TEST_F(BallObjectTest, CreateObjectWithValidParameters) {
    auto params = createParams({
        {"radius", "0.075"},
        {"bounciness", "0.8"},
        {"gravity_scale", "1.5"},
        {"mass", "2.0"}
    });
    
    cGenericObject* obj = ballObj.createObject(params, 1.0, 2.0, 3.0, 0.8, 0.2, 0.6);
    
    ASSERT_NE(obj, nullptr);
    validateChaiObject(obj, 1.0, 2.0, 3.0);
    
    // Ball creates a cMesh sphere
    cMesh* mesh = dynamic_cast<cMesh*>(obj);
    EXPECT_NE(mesh, nullptr);
    
    delete obj;
}

TEST_F(BallObjectTest, CreateObjectWithDefaultParameters) {
    auto params = createParams({
        {"radius", "0.06"}  // Only required parameter
    });
    
    cGenericObject* obj = ballObj.createObject(params, 0.0, 0.0, 0.0, 0.5, 0.5, 0.5);
    
    ASSERT_NE(obj, nullptr);
    validateChaiObject(obj, 0.0, 0.0, 0.0);
    
    delete obj;
}

TEST_F(BallObjectTest, CreateObjectWithMissingRadius) {
    auto params = createParams({
        {"bounciness", "0.3"},
        {"mass", "1.5"}
    });
    
    cGenericObject* obj = ballObj.createObject(params, 0.0, 0.0, 0.0, 0.5, 0.5, 0.5);
    
    ASSERT_NE(obj, nullptr); // Should still create with default radius
    validateChaiObject(obj, 0.0, 0.0, 0.0);
    
    delete obj;
}

TEST_F(BallObjectTest, CreateObjectWithInvalidParameters) {
    auto params = createParams({
        {"radius", "invalid_number"},
        {"bounciness", "not_a_number"},
        {"gravity_scale", "also_invalid"},
        {"mass", "nope"}
    });
    
    // Should handle invalid parameters gracefully
    cGenericObject* obj = ballObj.createObject(params, 0.0, 0.0, 0.0, 0.5, 0.5, 0.5);
    
    ASSERT_NE(obj, nullptr); // Should still create with defaults
    validateChaiObject(obj, 0.0, 0.0, 0.0);
    
    delete obj;
}

TEST_F(BallObjectTest, CreateObjectWithBoundaryValues) {
    // Test very small values
    auto params1 = createParams({
        {"radius", "0.001"},         // Very small radius
        {"bounciness", "0.0"},       // No bounce
        {"gravity_scale", "10.0"},   // High gravity
        {"mass", "0.1"}              // Light mass
    });
    
    cGenericObject* obj1 = ballObj.createObject(params1, 0.0, 0.0, 0.0, 0.5, 0.5, 0.5);
    ASSERT_NE(obj1, nullptr);
    delete obj1;
    
    // Test large values
    auto params2 = createParams({
        {"radius", "1.0"},           // Large radius
        {"bounciness", "1.0"},       // Perfect bounce
        {"gravity_scale", "0.0"},    // No gravity
        {"mass", "100.0"}            // Heavy mass
    });
    
    cGenericObject* obj2 = ballObj.createObject(params2, 0.0, 0.0, 0.0, 0.5, 0.5, 0.5);
    ASSERT_NE(obj2, nullptr);
    delete obj2;
}

// Integration tests
class CustomObjectIntegrationTest : public CustomObjectTest {
};

TEST_F(CustomObjectIntegrationTest, RegisterMultipleTypesAndCreate) {
    // Register multiple types
    CustomObjectRegistry::registerType("sphere_test", []() {
        return std::make_unique<SphereObject>();
    });
    CustomObjectRegistry::registerType("box_test", []() {
        return std::make_unique<BoxObject>();
    });
    CustomObjectRegistry::registerType("spring_test", []() {
        return std::make_unique<SpringObject>();
    });
    
    // Verify all are registered
    EXPECT_TRUE(CustomObjectRegistry::isTypeRegistered("sphere_test"));
    EXPECT_TRUE(CustomObjectRegistry::isTypeRegistered("box_test"));
    EXPECT_TRUE(CustomObjectRegistry::isTypeRegistered("spring_test"));
    
    // Create and test each type
    auto sphere = CustomObjectRegistry::createObject("sphere_test");
    ASSERT_NE(sphere, nullptr);
    EXPECT_EQ(sphere->getTypeName(), "sphere");
    
    auto box = CustomObjectRegistry::createObject("box_test");
    ASSERT_NE(box, nullptr);
    EXPECT_EQ(box->getTypeName(), "box");
    
    auto spring = CustomObjectRegistry::createObject("spring_test");
    ASSERT_NE(spring, nullptr);
    EXPECT_EQ(spring->getTypeName(), "spring");
}

TEST_F(CustomObjectIntegrationTest, CreateObjectsWithVariedParameters) {
    // Test creating various objects with different parameter combinations
    std::vector<std::unique_ptr<CustomObjectInterface>> objects;
    
    // Sphere with custom radius
    objects.push_back(std::make_unique<SphereObject>());
    auto sphereParams = createParams({{"radius", "0.075"}});
    cGenericObject* sphereObj = objects.back()->createObject(sphereParams, 0, 0, 0, 1, 0, 0);
    EXPECT_NE(sphereObj, nullptr);
    
    // Box with custom dimensions
    objects.push_back(std::make_unique<BoxObject>());
    auto boxParams = createParams({{"width", "0.1"}, {"height", "0.2"}, {"depth", "0.15"}});
    cGenericObject* boxObj = objects.back()->createObject(boxParams, 1, 0, 0, 0, 1, 0);
    EXPECT_NE(boxObj, nullptr);
    
    // Spring with custom physics
    objects.push_back(std::make_unique<SpringObject>());
    auto springParams = createParams({{"stiffness", "2500"}, {"damping", "100"}});
    cGenericObject* springObj = objects.back()->createObject(springParams, 0, 1, 0, 0, 0, 1);
    EXPECT_NE(springObj, nullptr);
    
    // Test updatePhysics on all objects
    for (auto& obj : objects) {
        EXPECT_NO_THROW(obj->updatePhysics(nullptr, 0.016));  // Null object should be handled gracefully
    }
    
    // Clean up CHAI3D objects
    delete sphereObj;
    delete boxObj;
    delete springObj;
}

// Error handling and edge case tests
class CustomObjectErrorTest : public CustomObjectTest {
};

TEST_F(CustomObjectErrorTest, HandleInvalidParameterValues) {
    SphereObject sphere;
    
    // Test with various invalid parameter values
    auto invalidParams = createParams({
        {"radius", "not_a_number"},
        {"invalid_param", "some_value"}
    });
    
    // Should not crash and should create object with defaults
    EXPECT_NO_THROW({
        cGenericObject* obj = sphere.createObject(invalidParams, 0, 0, 0, 1, 1, 1);
        EXPECT_NE(obj, nullptr);
        delete obj;
    });
}

TEST_F(CustomObjectErrorTest, HandleExtremeBoundaryValues) {
    SpringObject spring;
    
    // Test with extreme values
    auto extremeParams = createParams({
        {"radius", "0.001"},      // Very small
        {"height", "10.0"},       // Very large
        {"stiffness", "0.1"},     // Very low stiffness
        {"damping", "10000.0"}    // Very high damping
    });
    
    EXPECT_NO_THROW({
        cGenericObject* obj = spring.createObject(extremeParams, 0, 0, 0, 1, 1, 1);
        EXPECT_NE(obj, nullptr);
        
        // Should handle extreme physics parameters
        spring.updatePhysics(obj, 0.001);  // Very small timestep
        spring.updatePhysics(obj, 1.0);    // Large timestep
        
        delete obj;
    });
}

TEST_F(CustomObjectErrorTest, HandleColorBoundaryValues) {
    BoxObject box;
    auto params = createParams({{"width", "0.1"}});
    
    // Test color values outside normal [0,1] range
    EXPECT_NO_THROW({
        cGenericObject* obj = box.createObject(params, 0, 0, 0, -1.0, 2.0, 0.5);
        EXPECT_NE(obj, nullptr);
        delete obj;
    });
    
    // Test normal color range
    EXPECT_NO_THROW({
        cGenericObject* obj = box.createObject(params, 0, 0, 0, 0.0, 1.0, 0.5);
        EXPECT_NE(obj, nullptr);
        delete obj;
    });
}

// Integration tests for CHAI3D compatibility
class IntegrationTest : public ::testing::Test {
public:
    void SetUp() override {
        // Basic setup for integration testing
    }
    
    void TearDown() override {
        // Clean up after integration tests
    }
};

TEST_F(IntegrationTest, CustomObjectsIntegrateWithCHAI3D) {
    // Test that our custom objects can be created without major CHAI3D issues
    EXPECT_NO_THROW({
        // Test that all our custom object types can be instantiated
        SphereObject sphere;
        BoxObject box;
        SpringObject spring;
        CylinderObject cylinder;
        LineObject line;
        EllipsoidObject ellipsoid;
        BallObject ball;
        CupObject cup;
        TorusObject torus;
        
        // Test that they all report their type names correctly
        EXPECT_EQ(sphere.getTypeName(), "sphere");
        EXPECT_EQ(box.getTypeName(), "box");
        EXPECT_EQ(spring.getTypeName(), "spring");
        EXPECT_EQ(cylinder.getTypeName(), "cylinder");
        EXPECT_EQ(line.getTypeName(), "line");
        EXPECT_EQ(ellipsoid.getTypeName(), "ellipsoid");
        EXPECT_EQ(ball.getTypeName(), "ball");
        EXPECT_EQ(cup.getTypeName(), "cup");
        EXPECT_EQ(torus.getTypeName(), "torus");
    });
}

TEST_F(IntegrationTest, CustomObjectCreationLifecycle) {
    // Test complete creation and destruction lifecycle
    EXPECT_NO_THROW({
        std::vector<std::unique_ptr<CustomObjectInterface>> objects;
        
        // Create all types
        objects.push_back(std::make_unique<SphereObject>());
        objects.push_back(std::make_unique<BoxObject>());
        objects.push_back(std::make_unique<SpringObject>());
        objects.push_back(std::make_unique<CylinderObject>());
        objects.push_back(std::make_unique<LineObject>());
        objects.push_back(std::make_unique<EllipsoidObject>());
        objects.push_back(std::make_unique<BallObject>());
        objects.push_back(std::make_unique<CupObject>());
        objects.push_back(std::make_unique<TorusObject>());
        
        // Verify all were created
        EXPECT_EQ(objects.size(), 9);
        
        // Verify polymorphism works
        for (auto& obj : objects) {
            EXPECT_NE(obj.get(), nullptr);
            EXPECT_FALSE(obj->getTypeName().empty());
        }
        
        // Objects will be automatically destroyed when the vector goes out of scope
    });
}