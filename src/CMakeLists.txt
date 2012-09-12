cmake_minimum_required(VERSION 2.6)
 
project(WMC)

SET(CMAKE_BUILD_TYPE RelWithDebInfo)
SET(CMAKE_CXX_FLAGS "-Wno-format -Wno-unused-result")
IF(NOT DEFINED TARGET_PREFIX)
  SET(TARGET_PREFIX "wmc")
ENDIF(NOT DEFINED TARGET_PREFIX)

IF(NOT DEFINED ITK_DIR)
  SET(ITK_DIR /home/soheil/lib/cmake/ITK-4.2/)
ENDIF(NOT DEFINED ITK_DIR)

FIND_PACKAGE(ITK REQUIRED NO_DEFAULT_PATH)
include(${ITK_USE_FILE})

add_subdirectory(lasvm-source)
include_directories (${WMC_SOURCE_DIR}/lasvm-source) 
link_directories (${WMC_BINARY_DIR}/lasvm-source) 

add_executable(Train Train-main.cxx)
target_link_libraries(Train ${ITK_LIBRARIES} lasvm)

add_executable(Test Test-main.cxx SVM.cpp) 
target_link_libraries(Test ${ITK_LIBRARIES})

add_executable(Segmentation Segment-main.cxx SVM.cpp) 
target_link_libraries(Segmentation ${ITK_LIBRARIES})

add_executable(CollectFV Collect-main.cxx) 
target_link_libraries(CollectFV ${ITK_LIBRARIES})

add_executable(ScaleImage Scale-main.cxx) 
target_link_libraries(ScaleImage ${ITK_LIBRARIES})

add_executable(Distribution Distribution-main.cxx) 
target_link_libraries(Distribution ${ITK_LIBRARIES})

message("Installation root is ${CMAKE_INSTALL_PREFIX}")
foreach(targ Test Train Segmentation CollectFV ScaleImage Distribution)
  message("Install rule: ${targ} as ${TARGET_PREFIX}${targ}")
  set_property(TARGET ${targ} PROPERTY INSTALL_RPATH_USE_LINK_PATH true)
  set_property(TARGET ${targ} PROPERTY OUTPUT_NAME "${TARGET_PREFIX}${targ}")
  install(TARGETS
    ${targ}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )
endforeach()

# installing scripts
if (UNIX)
  file(GLOB scripts "${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.sh")
  foreach (file ${scripts})
    get_filename_component (filename_we ${file} NAME_WE)
    message("Install rule: ${file} as ${TARGET_PREFIX}${filename_we}")
    install (PROGRAMS ${file} 
             DESTINATION bin
             RENAME "${TARGET_PREFIX}${filename_we}")
  endforeach (file)
else (WIN32)
  file(GLOB scripts "${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.bat")
  foreach (file ${scripts})
    message("Install rule: ${file} as ${TARGET_PREFIX}${file}")
    install (PROGRAMS ${file}
             DESTINATION bin
             RENAME "${TARGET_PREFIX}${file}")
  endforeach (file)
endif ()

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
