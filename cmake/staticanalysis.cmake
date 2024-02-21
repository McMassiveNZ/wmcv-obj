include_guard()

function(target_enable_static_analysis _target)
    set(_VS_CLANG_TIDY "false")
    find_program(CLANGTIDY clang-tidy)
    if(CLANGTIDY)
        set(_VS_CLANG_TIDY "true")
    endif()
    set_target_properties(
        ${_target}
        PROPERTIES
        VS_GLOBAL_EnableMicrosoftCodeAnalysis true
        VS_GLOBAL_CodeAnalysisRuleSet "AllRules.ruleset"
        VS_GLOBAL_EnableClangTidyCodeAnalysis "${_VS_CLANG_TIDY}"
    )
endfunction()