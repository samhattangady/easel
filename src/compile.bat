@echo off
"C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe" assets\glsl\vertex.glsl -o data\spirv\vertex.spv
"C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe" assets\glsl\fragment.glsl -o data\spirv\fragment.spv
echo "Complete"
