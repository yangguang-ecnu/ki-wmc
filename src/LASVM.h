/*
 * LASVM.h
 *
 *  Created on: Apr 17, 2012
 *      Author: soheil
 */

#ifndef LASVM_H_
#define LASVM_H_
#include "lasvm.h"
#include "itkTimeProbe.h"

#include <vector>

class ROC_Item {
public:
  ROC_Item(int l, double c, unsigned int i){
    label = l;
    confidence = c;
    index = i;
  }
  ROC_Item(int l, double c){
    label = l;
    confidence = c;
    index = 0;
  }
  int label;
  unsigned int index;
  double confidence;
  bool operator<(const ROC_Item& other) const {
    return this->confidence < other.confidence;
  }
  bool operator>(const ROC_Item& other) const {
    return this->confidence > other.confidence;
  }
};

#define MEGABYTES 1048576
template<class ACC>
class LASVM_DC {
public:
  LASVM_DC(const std::vector<char>& Y_,
      const std::vector<unsigned int>& proxy_, ACC*acc_) {
    acc = acc_;
    proxy = proxy_;
    Y = Y_;
    m = proxy.size();
    for (int i = 0; i < m; i++) {
      if (Y[proxy[i]] > 0) {
        ipos.push_back(proxy[i]);
      } else {
        ineg.push_back(proxy[i]);
      }
    }
  }
  const char&label(size_t i) const {
    return Y[i];
  }
  const char&label_proxy(size_t i) const {
    return Y[proxy[i]];
  }
  const unsigned int&item(size_t i) const {
    return proxy[i];
  }
  const size_t size() const {
    return m;
  }
  const size_t label_size() const {
    return Y.size();
  }
  const std::vector<unsigned int>& pos() const {
    return ipos;
  }
  const std::vector<unsigned int>& neg() const {
    return ineg;
  }
  double p2n() const {
    return double(ipos.size()) / double(ineg.size());
  }
  const std::vector<unsigned int>& more() const {
    return p2n() > 1 ? ipos : ineg;
  }
  const std::vector<unsigned int>& less() const {
    return p2n() < 1 ? ipos : ineg;
  }
  const std::vector<double> get(size_t index) const{
    return acc->get(index);
  }
private:
  ACC* acc;
  std::vector<unsigned int> proxy;
  std::vector<unsigned int> ipos, ineg;
  std::vector<char> Y; // Labels
  size_t m;
};

#define ONLINE 0
#define ONLINE_WITH_FINISHING 1

#define RANDOM 0
#define GRADIENT 1
#define MARGIN 2
#define MARGINBAL 3

#define ITERATIONS 0
#define SVS 1
#define TIME 2

template<class ACC>
class LASVM {
public:
  typedef LASVM_DC<ACC> ContainerType;
  LASVM();

  void set_param(lasvm_kernel_t kernel, double cache_size, void *closure);

  double predict(size_t index);
  double kernelVal(size_t index);

  int train_online(double cp, double cn);
  int train_online(double c);
  int train_online(double c, const ContainerType&data_container);

  void save(const char* fn, double kgamma);

  double tuneThreshold(double desiredFPR);
  std::vector<unsigned int> resample();
  double ROC(const char* fn);

  void set_verbos(const int & v);
  void set_max_sv(const int & msv);
  void set_data_container(const ContainerType&data_container);
  virtual ~LASVM();
private:
  void touch();
  void predictAll(double ratio);
  void voidPredicted();
  int select();
  void finish();
  void make_old(int val);
  const ContainerType*data_container;
  size_t c_size;
  lasvm_kcache_t *kcache;
  lasvm_t *sv;
  int sv1, sv2;
  double epsgr;
  double deltamax;
  int selection_type;
  int candidates;
  double alpha_tol;
  double b0; // threshold
  int MAX_SV;
  std::vector<unsigned int> iold, inew;
  std::vector<double> alpha; // alpha_i, SV weights
  std::vector<ROC_Item> roc_items;
  int verbosity;

  bool allPredicted;
  int NP, NN;
};

#include "LASVM.txx"
#endif /* LASVM_H_ */
