add_executable(msignal_demo msignal_demo.cpp)
target_link_libraries(msignal_demo PRIVATE msignal)

add_test(NAME msignal_demo COMMAND msignal_demo)
