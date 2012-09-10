/*
 * SVM.h
 *
 *  Created on: Apr 23, 2012
 *      Author: soheil
 */

#ifndef SVM_H_
#define SVM_H_

#include "vector"
#include "string"
#include <iostream>

typedef std::vector<double> SVMPointType;

class SupportVector {
public:
  SupportVector() {
    l = 0;
    n = 0;
    w = 0;
  }
  ;
  void read(std::string& in) {
    char tmp;
    unsigned int ii;
    double a;
    {
      std::istringstream s_in(in);
      s_in >> w;
      l = 0;
      while (s_in.good()) {
        s_in >> ii >> tmp >> a;
        if (ii > l) {
          l = ii;
        }
      }
      l++;
    }
    sv.reserve(l);
    sv.assign(l, 0);
    std::istringstream s_in(in);
    s_in >> w;
    while (s_in.good()) {
      s_in >> ii >> tmp >> a;
      sv[ii] = a;
    }
    n = 0;
    for (std::vector<double>::const_iterator it = sv.begin(); it
        != sv.end(); ++it) {
      n += (*it) * (*it);
    }
  }
  double dot(const SVMPointType& p) const {
    double d = 0;
    for (unsigned int i = 0; i < l; ++i) {
      d += sv[i] * p[i];
    }
    return d;
  }
  const double &weight() const {
    return w;
  }
  const double &length() const {
    return l;
  }
  const double& norm() const {
    return n;
  }
  void print() const {
    std::cout << "weight " << weight() << std::endl;
    std::cout << "norm " << norm() << std::endl;
    for (std::vector<double>::const_iterator it = sv.begin(); it
        != sv.end(); ++it) {
      std::cout << (*it) << " ";
    }
    std::cout << std::endl;
  }
private:
  SVMPointType sv;
  double n;
  double w;
  unsigned int l;
};

class SVM {
public:
  SVM();
  void Load(const char*filename);
  virtual double classify(const SVMPointType&point);
  void adjustGamma(double gamma);
  virtual ~SVM();
  double Null(){return null;}
protected:
  std::vector<SupportVector> svs;
  double bias, g;
  unsigned int l;
  double null;
  std::vector<SupportVector>::iterator begin, end;
};

class SVM_Exp: public SVM {
public:
  virtual double classify(const SVMPointType&point);
};

#endif /* SVM_H_ */
