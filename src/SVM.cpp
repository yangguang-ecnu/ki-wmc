/*
 * SVM.cpp
 *
 *  Created on: Apr 23, 2012
 *      Author: soheil
 */

#include "itkMacro.h"
#include "SVM.h"

#include <iostream>
#include <fstream>
#include <math.h>

SVM::SVM() {
  g = -1;
  bias = 0;
  l = 0;
}

void SVM::Load(const char*filename) {
  std::ifstream ifs(filename);

  std::string temp;
  std::string token;

  g = -1;
  bias = 0;

  while (getline(ifs, temp)) {
    std::istringstream s_in(temp);
    std::string token;
    s_in >> token;
    if (!token.compare("gamma")) {
      s_in >> g;
    } else if (!token.compare("rho")) {
      s_in >> bias;
    } else if (!token.compare("SV")) {
      break;
    }
  }
  svs.erase(svs.begin(), svs.end());
  l = 0;
  while (getline(ifs, temp)) {
    SupportVector sv;
    sv.read(temp);
    if (l == 0) {
      l = sv.length();
    } else {
      if (l != sv.length()) {
        itkGenericExceptionMacro("Error while loading model " << filename << std::endl<< "All support vectors should be of the same dimension " << l << "!=" << sv.length() << ")");
      }
    }
    svs.push_back(sv);
  }

  ifs.close();

  begin = svs.begin();
  end = svs.end();
  SVMPointType p;
  for(unsigned int i=0;i<l;++i){
    p.push_back(0.0);
  }
  null = classify(p);
  std::cout << "Loading " << filename << " completed" << std::endl;
  std::cout << "G: " << g << " bias: " << bias << std::endl;
}

double SVM::classify(const SVMPointType&point) {
  double pn = 0;
  for (unsigned int i = 0; i < l; ++i) {
    pn += point[i] * point[i];
  }
  double val = 0;
  for (std::vector<SupportVector>::iterator it = begin; it != end; ++it) {
    const double kk = g * (2 * it->dot(point) - it->norm() - pn);
    val += it->weight() * exp(kk);
  }
  return val - bias > 0?1:0;
}
void SVM::adjustGamma(double gamma) {
  g = gamma;
}
SVM::~SVM() {
}

double SVM_Exp::classify(const SVMPointType&point) {
  double pn = 0;
  for (unsigned int i = 0; i < l; ++i) {
    pn += point[i] * point[i];
  }
  double val = 0;
  for (std::vector<SupportVector>::iterator it = begin; it != end; ++it) {
    const double kk = g * (2 * it->dot(point) - it->norm() - pn);
    val += it->weight() * exp(kk);
  }
  return val;
}
