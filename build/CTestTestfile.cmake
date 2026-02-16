# CMake generated Testfile for 
# Source directory: /home/abolinaga/Proyectos/ade9000/cJSON
# Build directory: /home/abolinaga/Proyectos/ade9000/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(cJSON_test "/home/abolinaga/Proyectos/ade9000/build/cJSON_test")
set_tests_properties(cJSON_test PROPERTIES  _BACKTRACE_TRIPLES "/home/abolinaga/Proyectos/ade9000/cJSON/CMakeLists.txt;248;add_test;/home/abolinaga/Proyectos/ade9000/cJSON/CMakeLists.txt;0;")
subdirs("tests")
subdirs("fuzzing")
