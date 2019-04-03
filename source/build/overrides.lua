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
    
    -- fixing vs-android bugs
    if ANDROID then
        -- add '_PackagingProjectWithoutNativeComponent' to the 'androidApplicationType'
        local android = require("android")
        premake.override(premake.modules.android, "androidApplicationType", function(base, cfg)
            _p(2, "<_PackagingProjectWithoutNativeComponent>true</_PackagingProjectWithoutNativeComponent>")
            base(cfg)
        end)
        
        -- add '_PackagingProjectWithoutNativeComponent' to the 'projectVersion'
        premake.override(premake.modules.android, "projectVersion", function(base, cfg)
            _p(2, "<_PackagingProjectWithoutNativeComponent>true</_PackagingProjectWithoutNativeComponent>")
            base(cfg)
        end)
        
        -- premake overrides original vc2010 'OutDir' function (sets default path) in android module for some reason
        -- overriding it again to the original variant
        local vs = require("vstudio")
        premake.override(premake.vstudio.vc2010, "outDir", function(base, cfg)
            local outdir = premake.vstudio.path(cfg, cfg.buildtarget.directory)
            premake.vstudio.vc2010.element("OutDir", nil, "%s\\", outdir)
        end)
    end
end