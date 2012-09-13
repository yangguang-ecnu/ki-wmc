/*
 * util.h
 *
 *  Created on: Aug 28, 2011
 *      Author: soheil
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <string>
#include <vector>

void
advance_spinner(int step);

void
printProgress(double rat);

std::vector<std::string>
parseModelHeaderFile(const std::string& headerFile);

std::string
fullPath(const std::string& dir, const std::string& file);

template<class TImage>
  void
  showImage(typename TImage::Pointer image);

template<class TImage, int s>
  void
  CreateImage(typename TImage::Pointer image);

#endif /* UTIL_H_ */
