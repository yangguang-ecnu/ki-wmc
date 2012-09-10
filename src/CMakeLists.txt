cmake_minimum_required(VERSION 2.6)
 
project(WMC)

SET(CMAKE_BUILD_TYPE RelWithDebInfo)
SET(CMAKE_CXX_FLAGS "")

IF(NOT DEFINED ITK_DIR)
  SET(ITK_DIR /home/soheil/lib/cmake/ITK-4.2/)
ENDIF(NOT DEFINED ITK_DIR)

FIND_PACKAGE(ITK REQUIRED NO_DEFAULT_PATH)
include(${ITK_USE_FILE})

add_subdirectory(lasvm-source)
include_directories (${WMC_SOURCE_DIR}/lasvm-source) 
link_directories (${WMC_BINARY_DIR}/lasvm-source) 

add_executable(wmlTest Test-main.cxx SVM.cpp) 
target_link_libraries(wmlTest ${ITK_LIBRARIES})

add_executable(wmlTrain Train-main.cxx) 
target_link_libraries(wmlTrain ${ITK_LIBRARIES} lasvm)

add_executable(wmlSegmentation Segment-main.cxx SVM.cpp) 
target_link_libraries(wmlSegmentation ${ITK_LIBRARIES})

add_executable(wmlCollectFV Collect-main.cxx) 
target_link_libraries(wmlCollectFV ${ITK_LIBRARIES})

add_executable(wmlScaleImage Scale-main.cxx) 
target_link_libraries(wmlScaleImage ${ITK_LIBRARIES})

add_executable(wmlDistribution Distribution-main.cxx) 
target_link_libraries(wmlDistribution ${ITK_LIBRARIES})

install(TARGETS
  wmlTest wmlTrain wmlSegmentation wmlCollectFV wmlScaleImage wmlDistribution
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PERMISSIONS WORLD_EXECUTE
)

# installing scripts
if (UNIX)
  file(GLOB scripts "${CMAKE_CURRENT_SOURCE_DIR}/*.sh")
  foreach (file ${scripts})
    get_filename_component (name_without_extension ${file} NAME_WE)
    install (FILES ${file} 
             DESTINATION bin
             PERMISSIONS WORLD_EXECUTE
             RENAME ${name_without_extension})
  endforeach ()
else (WIN32)
  file(GLOB scripts "${CMAKE_CURRENT_SOURCE_DIR}/*.bat")
  install (FILES ${scripts}
           PERMISSIONS WORLD_EXECUTE
           DESTINATION bin)
endif ()

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
