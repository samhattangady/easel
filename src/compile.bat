@echo off
"C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe" src\glsl\grass_vertex.glsl -o data\spirv\grass_vertex.spv
"C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe" src\glsl\grass_fragment.glsl -o data\spirv\grass_fragment.spv
"C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe" src\glsl\skybox_vertex.glsl -o data\spirv\skybox_vertex.spv
"C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe" src\glsl\skybox_fragment.glsl -o data\spirv\skybox_fragment.spv
echo "Complete"
