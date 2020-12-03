@echo off
"C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe" src\glsl\vertex.glsl -o data\spirv\vertex.spv
"C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe" src\glsl\fragment.glsl -o data\spirv\fragment.spv
"C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe" src\glsl\tree_vertex.glsl -o data\spirv\tree_vertex.spv
"C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe" src\glsl\grass_vertex.glsl -o data\spirv\grass_vertex.spv
"C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe" src\glsl\skybox_fragment.glsl -o data\spirv\skybox_fragment.spv
echo "Complete"
