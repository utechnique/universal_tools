function utOverridePremakeFunctions()
    -- fixing vs2008 bug (manifest is not generated)
    if _ACTION == "vs2008" then
        local vs = require("vstudio")
        premake.override(premake.vstudio.vc200x, "generateManifest", function(base, cfg, toolset)
            if cfg.flags.NoManifest then
                premake.w('GenerateManifest="false"')
            else
                premake.w('GenerateManifest="true"')
            end
        end)
    end
end