target_sources(
    wmcv-obj PRIVATE
        wmcv_obj/wmcv_obj.h
)

target_include_directories(
    wmcv-obj 
        PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/wmcv_obj
        PUBLIC
            ${CMAKE_CURRENT_LIST_DIR}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})