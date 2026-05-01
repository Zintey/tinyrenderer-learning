@echo off
cmake --build .
if %errorlevel% equ 0 (
    Debug\tinyrenderer.exe ..\obj\african_head\african_head.obj ..\obj\african_head\african_head_diffuse.tga ..\obj\african_head\african_head_nm.tga
) else (
    echo Build failed, not running the program.
)