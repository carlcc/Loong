cmake_minimum_required(VERSION 3.2)

project(Loong)

add_subdirectory(thirdparty)

set(CMAKE_CXX_STANDARD 17)

function(set_loong_common_target_properties TARGET)

    if(COMMAND custom_configure_target)
        custom_configure_target(${TARGET})
        if(TARGET_CONFIGURATION_COMPLETE)
            return()
        endif()
    endif()

    get_target_property(TARGET_TYPE ${TARGET} TYPE)

    if(MSVC)
        # For msvc, enable link-time code generation for release builds (I was not able to
        # find any way to set these settings through interface library BuildSettings)
        if(TARGET_TYPE STREQUAL STATIC_LIBRARY)

            foreach(REL_CONFIG ${RELEASE_CONFIGURATIONS})
                set_target_properties(${TARGET} PROPERTIES
                    STATIC_LIBRARY_FLAGS_${REL_CONFIG} /LTCG
                )
            endforeach()

        else()

            foreach(REL_CONFIG ${RELEASE_CONFIGURATIONS})
                set_target_properties(${TARGET} PROPERTIES
                    LINK_FLAGS_${REL_CONFIG} "/LTCG /OPT:REF /INCREMENTAL:NO"
                )
            endforeach()

            if(PLATFORM_UNIVERSAL_WINDOWS)
                # On UWP, disable incremental link to avoid linker warnings
                foreach(DBG_CONFIG ${DEBUG_CONFIGURATIONS})
                    set_target_properties(${TARGET} PROPERTIES
                        LINK_FLAGS_${DBG_CONFIG} "/INCREMENTAL:NO"
                    )
                endforeach()
            endif()
        endif()
    else()
        set_target_properties(${TARGET} PROPERTIES
            CXX_VISIBILITY_PRESET hidden # -fvisibility=hidden
            C_VISIBILITY_PRESET hidden # -fvisibility=hidden
            VISIBILITY_INLINES_HIDDEN TRUE

            # Without -fPIC option GCC fails to link static libraries into dynamic library:
            #  -fPIC
            #      If supported for the target machine, emit position-independent code, suitable for
            #      dynamic linking and avoiding any limit on the size of the global offset table.
            POSITION_INDEPENDENT_CODE ON
        )

        if(NOT MINGW_BUILD)
            # Do not disable extensions when building with MinGW!
            set_target_properties(${TARGET} PROPERTIES
                CXX_EXTENSIONS OFF
            )
        endif()
    endif()

endfunction()

add_subdirectory(Loong)
