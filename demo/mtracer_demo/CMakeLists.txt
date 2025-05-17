add_executable(mtracer_demo mtracer_demo.cpp)
target_link_libraries(mtracer_demo PRIVATE mtracer)

add_test(NAME mtracer_demo COMMAND mtracer_demo)
