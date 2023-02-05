

rule("mmg")
    set_extensions(".glap.yaml", ".glap.yml")
    on_config(function(target) 
        import("utils").mmg_update_target(target) 
    end)