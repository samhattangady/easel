@echo off
"C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe" src\glsl\vertex.glsl -o data\spirv\vertex.spv
"C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe" src\glsl\fragment.glsl -o data\spirv\fragment.spv
echo "Complete"
