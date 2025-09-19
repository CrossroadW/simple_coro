# 查找当前目录所有 cpp 文件
file(GLOB CPP_FILES "*.cpp")

foreach(src_file IN LISTS CPP_FILES)
    # 获取文件名（不带扩展名）
    get_filename_component(target ${src_file} NAME_WE)
    message(${target})
    # 添加可执行文件
    add_executable(${target} ${src_file})

    # 设置 C++23 标准
    target_compile_features(${target} PUBLIC cxx_std_23)

    # 默认包含当前目录
    target_include_directories(${target} PUBLIC ${CMAKE_SOURCE_DIR})

    # 链接 spdlog
    target_link_libraries(${target} PUBLIC spdlog::spdlog)

    # 默认编译选项
    target_compile_options(${target} PUBLIC -Werror=return-type -Wall)
endforeach()
