//Copyright (c) 2019 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.

#include <gtest/gtest.h>

#include "../src/Application.h" //To set up a slice with settings.
#include "../src/Slice.h" //To set up a scene to slice.
#include "../src/slicer.h" //Starts the slicing phase that we want to test.
#include "../src/utils/floatpoint.h" //For FMatrix3x3 to load STL files.

namespace cura
{

class AdaptiveLayer;

/*
 * Integration test on the slicing phase of CuraEngine. This tests if the
 * slicing algorithm correctly splits a 3D model up into 2D layers.
 */
class SlicePhaseTest : public testing::Test
{
    void SetUp()
    {
        //Set up a scene so that we may request settings.
        Application::getInstance().current_slice = new Slice(1);

        //And a few settings that we want to default.
        Scene& scene = Application::getInstance().current_slice->scene;
        scene.settings.add("slicing_tolerance", "middle");
        scene.settings.add("layer_height_0", "0.2");
        scene.settings.add("layer_height", "0.1");
        scene.settings.add("magic_mesh_surface_mode", "normal");
        scene.settings.add("meshfix_extensive_stitching", "false");
        scene.settings.add("meshfix_keep_open_polygons", "false");
        scene.settings.add("minimum_polygon_circumference", "1");
        scene.settings.add("meshfix_maximum_resolution", "0.04");
        scene.settings.add("xy_offset", "0");
        scene.settings.add("xy_offset_layer_0", "0");
    }
};

TEST_F(SlicePhaseTest, Cube)
{
    Scene& scene = Application::getInstance().current_slice->scene;
    MeshGroup& mesh_group = scene.mesh_groups.back();

    const FMatrix3x3 transformation;
    //Path to cube.stl is relative to CMAKE_CURRENT_SOURCE_DIR/tests.
    ASSERT_TRUE(loadMeshIntoMeshGroup(&mesh_group, "integration/resources/cube.stl", transformation, scene.settings));
    EXPECT_EQ(mesh_group.meshes.size(), 1);
    Mesh& cube_mesh = mesh_group.meshes[0];

    const coord_t layer_thickness = scene.settings.get<coord_t>("layer_height");
    const coord_t initial_layer_thickness = scene.settings.get<coord_t>("layer_height_0");
    constexpr bool variable_layer_height = false;
    constexpr std::vector<AdaptiveLayer>* variable_layer_height_values = nullptr;
    const size_t slice_layer_count = (cube_mesh.getAABB().max.z - initial_layer_thickness) / layer_thickness + 2;
    Slicer slicer(&cube_mesh, layer_thickness, slice_layer_count, variable_layer_height, variable_layer_height_values);
}

} //namespace cura