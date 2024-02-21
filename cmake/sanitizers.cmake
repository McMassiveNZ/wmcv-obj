include_guard()

function( target_enable_sanitizers _target )
    if( MSVC )
        string(FIND "$ENV{PATH}" "$ENV{VSINSTALLDIR}" index_of_vs_install_dir)

        if("${index_of_vs_install_dir}" STREQUAL "-1")
            message(
                SEND_ERROR
                "Using MSVC sanitizers requires setting the MSVC environment before building the project. Please manually open the MSVC command prompt and rebuild the project."
            )
        endif()

        target_compile_options(${_target} PUBLIC /fsanitize=address /Zi /INCREMENTAL:NO)
        target_link_options(${_target} PUBLIC /INCREMENTAL:NO)
    else()
        target_compile_options(${_target} INTERFACE -fsanitize=address,leak,undefined)
        target_link_options(${_target} INTERFACE -fsanitize=address,leak,undefined)
    endif()
endfunction()