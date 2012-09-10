/*
 * util.h
 *
 *  Created on: Aug 28, 2011
 *      Author: soheil
 */

#ifndef UTIL_H_
#define UTIL_H_

#include "itkImageRegionIterator.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <cassert>
#include <stdio.h>
#include <unistd.h>

void advance_spinner(int step) {
  static int i = 0;
  static char bars[] = { '/', '-', '\\', '|' };
  static int nbars = sizeof(bars) / sizeof(char);
  static int pos = 0;
  i++;
  if (i >= step) {
    i = 0;
  } else {
    return;
  }
  printf("%c\b", bars[pos]);
  fflush(stdout);
  pos = (pos + 1) % nbars;
}

bool advance_percentage(int total) {
  static int i = 0;
  if (total < 0){
    i=0;
    return false;
  }
  i++;
  if (i % (total / 1000) == 0) {
    printf("%5.1f%%\b\b\b\b\b\b", 100.0 * i / total);
    fflush(stdout);
  }
  return true;
}

template<class E>
class F_Arr {
private:
  E *arr;
  int size;
public:
  F_Arr(int s) {
    size = s;
    arr = new E[size];
  }
  E & operator[](int subscript) {
    assert(0 <= subscript && subscript <= size);
    return arr[subscript];
  }

  ~F_Arr() {
    delete arr;
  }
};

std::vector<std::string> parseModelHeaderFile(const std::string& headerFile) {
  std::vector<std::string> modelFiles;
  std::string s_temp;
  std::string directory(headerFile);
  std::string::iterator lastslash = directory.end();
  while (lastslash != directory.begin()) {
    if (*lastslash == '/')
      break;
    --lastslash;
  }
  if (lastslash != directory.begin()) {
    directory.erase(lastslash + 1, directory.end());
  } else {
    directory = "./";
  }
  std::ifstream ifs(headerFile.c_str());
  while (ifs.good()) {
    getline(ifs, s_temp);
    if (!s_temp.empty() && s_temp.at(0) != '#')
      modelFiles.push_back(directory + s_temp);
  }
  ifs.close();
  return modelFiles;
}

std::string fullPath(const std::string& dir, const std::string& file) {
  if (file.empty())
    return dir;
  if (file[0] == '.' || file[0] == '~' || file[0] == '/')
    return file;
  return dir + '/' + file;
}

template<class TImage>
void showImage(typename TImage::Pointer image) {
  using std::cout;
  using std::endl;
  using std::setw;
  image->Update();
  itk::ImageRegionIterator<TImage> it(image,
      image->GetLargestPossibleRegion());
  int i = 1;
  for (it = it.Begin(); !it.IsAtEnd(); ++it, i++) {
    std::cout << std::setw(4) << it.Get() << " ";
    if (i % (image->GetLargestPossibleRegion().GetSize()[0]) == 0) {
      cout << endl;
    }
    if (i % (image->GetLargestPossibleRegion().GetSize()[0]
        * image->GetLargestPossibleRegion().GetSize()[1]) == 0) {
      cout << endl;
    }
  }
}

template<class TImage, int s>
void CreateImage(typename TImage::Pointer image) {

  typename TImage::IndexType start;
  start.Fill(0);

  typename TImage::SizeType size;
  size.Fill(s);

  typename TImage::RegionType region(start, size);

  image->SetRegions(region);
  image->Allocate();

  itk::ImageRegionIterator<TImage> it(image,
      image->GetLargestPossibleRegion());
  it.GoToBegin();

  int i = 0;
  typename TImage::PixelType vv = 1;
  for (it = it.Begin(); !it.IsAtEnd(); ++it) {
    vv++;
    it.Set(i++ % 3 == 0 ? 0 : vv);
  }
}

#endif /* UTIL_H_ */
