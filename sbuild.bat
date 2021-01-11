@echo off
del sbuild\shadergen.exe
mkdir sbuild
pushd sbuild
"c:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.26.28801\bin\Hostx64\x64\cl.exe"  /Fe:shadergen.exe /W4 /Za /Zi /I "C:\Users\user\libraries\SDL2-2.0.12\include" /I "C:\VulkanSDK\1.2.154.1\Include" ..\src\es_shader_compiler.c /link "C:\Users\user\libraries\SDL2-2.0.12\lib\x64\SDL2main.lib" "C:\Users\user\libraries\SDL2-2.0.12\lib\x64\SDL2.lib" "C:\VulkanSDK\1.2.154.1\Lib\vulkan-1.lib" "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\um\x64\shell32.lib" /SUBSYSTEM:CONSOLE
popd
