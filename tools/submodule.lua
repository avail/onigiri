local module = {}

local submodule_list = {}

function module.define(info, should_include)
    if should_include == nil or should_include then
        table.insert(submodule_list, info)

        print("[NFSCO:Submodule] Registered " .. info.name)
    else
        print("[NFSCO:Submodule] Not registering " .. info.name .. " as conditions fail.")
    end
end

function module.include(required_modules)
    for _, module_name in ipairs(required_modules) do
        local mod = {}

        for i, v in ipairs(submodule_list) do
            if v["name"] == module_name then
                mod = v
                break
            end
        end

        if (mod == nil) then
            error("[NFSCO:Submodule] " .. module_name .. " doesn't exist!!")
        end

        includedirs(mod["includes"])

        if (mod["libmode"] ~= "include") then
            links(mod["name"])
        else
            defines(mod["defines"])
        end
    end
end

function module.register_all()
    for i, v in ipairs(submodule_list) do
        if (v["libmode"] ~= "include") then
            project(v["name"])
                kind(v["libmode"])
                warnings "off"
                language(v["language"])

                links(v["links"])

                if not is_windows then
                    buildoptions(nfsco_buildopts)
                end

                if v["language"] == "c" or v["language"] == "C" then
                    cdialect("c11")
                end

                includedirs(v["includes"])
                files(v["files"])
                removefiles(v["ignores"])
                defines(v["defines"])
        end
    end
end

return module