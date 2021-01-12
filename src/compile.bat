:: @echo off
:: "C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe" src\glsl\vertex.glsl -o data\spirv\vertex.spv
:: "C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe" src\glsl\fragment.glsl -o data\spirv\fragment.spv
:: "C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe" src\glsl\tree_fragment.glsl -o data\spirv\tree_fragment.spv
:: "C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe" src\glsl\tree_vertex.glsl -o data\spirv\tree_vertex.spv
:: "C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe" src\glsl\grass_vertex.glsl -o data\spirv\grass_vertex.spv
:: "C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe" src\glsl\skybox_fragment.glsl -o data\spirv\skybox_fragment.spv
:: "C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe" src\glsl\ui_fragment.glsl -o data\spirv\ui_fragment.spv
:: "C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe" src\glsl\ui_vertex.glsl -o data\spirv\ui_vertex.spv
:: "C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe" src\glsl\shadow_map_vertex.glsl -o data\spirv\shadow_map_vertex.spv
:: echo "Complete"

@echo off
pushd data\spirv\
"C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe" -c ..\..\shaders\tree_vertex.glsl ..\..\shaders\tree_sm_vertex.glsl ..\..\shaders\tree_fragment.glsl ..\..\shaders\tree_sm_fragment.glsl ..\..\shaders\skybox_vertex.glsl ..\..\shaders\skybox_fragment.glsl ..\..\shaders\grass_vertex.glsl ..\..\shaders\grass_sm_vertex.glsl ..\..\shaders\grass_fragment.glsl ..\..\shaders\grass_sm_fragment.glsl ..\..\shaders\ui_vertex.glsl ..\..\shaders\ui_fragment.glsl ..\..\shaders\base_vertex.glsl ..\..\shaders\base_sm_vertex.glsl ..\..\shaders\base_fragment.glsl ..\..\shaders\base_sm_fragment.glsl
popd
echo "Complete"
