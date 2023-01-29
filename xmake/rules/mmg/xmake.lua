

rule("glap.mmg")
    set_extensions(".glap.yaml", ".glap.yml")
    on_config(function(target) 
        import("utils").mmg_update_target(target) 
    end)
    -- before_build(import("mmg_utils").mmg_update_target)
    before_buildcmd_file(function (target, batchcmds, source_file, opt)
        local output_file = import("utils").mmg_update_file(target, source_file)
    end)