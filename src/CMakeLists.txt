include(lexer/CMakeLists.txt)
include(parser/CMakeLists.txt)

add_executable(gracc main.cc)
set_property(TARGET gracc PROPERTY CXX_STANDARD 14)
set_property(TARGET gracc PROPERTY CXX_STANDARD_REQUIRED ON)
include_directories(".")

TARGET_LINK_LIBRARIES(gracc
    PUBLIC
        ${Boost_LIBRARIES}
    PRIVATE
        gracc_parse
    )
