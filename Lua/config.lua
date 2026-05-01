---@diagnostic disable: undefined-global
-- local root_path = get_root_path()
local model_config = {
    head = {
        obj_path = root_path .. "\\obj\\african_head\\african_head.obj",
        texture_path = root_path .. "\\obj\\african_head\\african_head_diffuse.tga",
        normaltex_path = root_path .. "\\obj\\african_head\\african_head_nm.tga"
    },
    monster = {
        obj_path = root_path .. "\\obj\\diablo3_pose\\diablo3_pose.obj",
        texture_path = root_path .. "\\obj\\diablo3_pose\\diablo3_pose_diffuse.tga",
        normaltex_path = root_path .. "\\obj\\diablo3_pose\\diablo3_pose_nm.tga"
    }
}
config = {}
config["model"] = model_config["head"]
config["model"]["position"] = {x = 0, y = 0, z = 0}
config["model"]["rotation"] = {x = 0, y = 30, z = 0}
config["height"] = 1024
config["width"] = 1024
config["backcolor"] = {r = 50, g = 50, b = 50, a = 255}

-- renderer.set_view_frustum(0.1, 300, 90)
renderer.add_light({
    color = {r = 255, g = 255, b = 255, a = 255},
    intensity = 8,
    position = {x = 2, y = 2, z = 2}
})
renderer.add_light({
    color = {r = 255, g = 0, b = 0, a = 255},
    -- intensity = 8,
    position = {x = -2, y = -2, z = -2}
})
renderer.start_render(config)


config = {}
config["model"] = model_config["monster"]
config["model"]["position"] = {x = 0, y = 0, z = 0}
config["model"]["rotation"] = {x = 0, y = 30, z = 0}
config["height"] = 1024
config["width"] = 1024
config["backcolor"] = {r = 50, g = 50, b = 50, a = 255}
renderer.start_render(config)

renderer.save_img(root_path .. "//output.tga")
renderer.save_zbuffer(root_path .. "//zbuffer.tga")
renderer.open_img("output.tga")
