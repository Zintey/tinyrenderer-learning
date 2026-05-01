@echo off
cmake --build .
if %errorlevel% equ 0 (
    Debug\tinyrenderer.exe ..\obj\diablo3_pose\diablo3_pose.obj ..\obj\diablo3_pose\diablo3_pose_diffuse.tga ..\obj\diablo3_pose\diablo3_pose_nm.tga
) else (
    echo Build failed, not running the program.
)