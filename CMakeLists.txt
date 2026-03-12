INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL OpenGLWidgets)
add_compile_options(-Wno-shift-op-parentheses)

# eksplisit generate moc untuk header .hxx
qt6_wrap_cpp(MOC_FILES include/ArrowPlotWidget.hxx)

add_executable(collatz src/collatz.cxx ${MOC_FILES})

target_link_libraries(collatz PRIVATE discrete Qt6::Core Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets)