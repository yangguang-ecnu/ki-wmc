cmake_minimum_required(VERSION 2.6)

project(LaSVM)

set(lasvm_source_files vector.c messages.c kernel.c kcache.c lasvm.c vector.h messages.h kernel.h kcache.h lasvm.h)
set_source_files_properties(${lasvm_source_files} PROPERTIES LANGUAGE CXX)

ADD_LIBRARY(lasvm ${lasvm_source_files})
TARGET_LINK_LIBRARIES(lasvm m)

ADD_EXECUTABLE(train la_svm)
TARGET_LINK_LIBRARIES(train lasvm)

