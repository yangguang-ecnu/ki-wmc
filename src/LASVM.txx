/*
 * LASVM.cpp
 *
 *  Created on: Apr 17, 2012
 *      Author: soheil
 */

#ifndef __LASVM_txx
#define __LASVM_txx

#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include "util.h"
#include "LASVM.h"

template<class ACC>
LASVM<ACC>::LASVM() {
  sv = NULL;
  kcache = NULL;
  epsgr = 1e-3;
  deltamax = 1000;

  selection_type = MARGINBAL;
  candidates = 50;
  verbosity = 0;
  allPredicted = false;
  NP = 0;
  NN = 0;
}

template<class ACC>
void LASVM<ACC>::touch() {
  allPredicted = false;
  NP = 0;
  NN = 0;
}

template<class ACC>
void LASVM<ACC>::set_param(lasvm_kernel_t kernel, double cache_size,
    void *closure) {
  c_size = cache_size * MEGABYTES;
  if (kcache) {
    lasvm_kcache_destroy(kcache);
  }
  kcache = lasvm_kcache_create(kernel, closure);
  lasvm_kcache_set_maximum_size(kcache, c_size);
  touch();
}
template<class ACC>
double LASVM<ACC>::predict(size_t index) {
  return lasvm_predict(sv, index);
}
template<class ACC>
double LASVM<ACC>::kernelVal(size_t index) {
  return lasvm_predict(sv, index);
}
template<class ACC>
int LASVM<ACC>::train_online(double c, const ContainerType&dc) {
  set_data_container(dc);
  return train_online(c, c);
}
template<class ACC>
int LASVM<ACC>::train_online(double c) {
  return train_online(c, c);
}
template<class ACC>
int LASVM<ACC>::train_online(double cp, double cn) {
  touch();
  if (sv) {
    lasvm_destroy(sv);
  }
  sv = lasvm_create(kcache, /*use_b0*/false, cp, cn);
  size_t m = data_container->size();

  inew.resize(0);
  iold.resize(0);

  int num_samples = 0;
  // everything is new when we start
  for (int i = 0; i < m; i++) {
    inew.push_back(data_container->item(i));
  }

  // first add 5 examples of each class, just to balance the initial set
  int c1 = 0, c2 = 0;
  int btr = 0;
  for (int i = 0; i < 5; i++) {
    btr = rand() % data_container->pos().size();
    lasvm_process(sv, data_container->pos()[btr], double(1));
    make_old(btr);

    btr = rand() % data_container->neg().size();
    lasvm_process(sv, data_container->neg()[btr], double(-1));
    make_old(btr);
  }

  itk::TimeProbe tot, sel, pro, rep;
  int epochs = 2;

  int t1, t2, s;
  for (int j = 0; j < epochs; j++) {
    for (int i = 0; i < m; i++) {

      tot.Start();
      if (inew.size() == 0)
        break; // nothing more to select
      num_samples++;
      sel.Start();
      s = select(); // selection strategy, select new point
      sel.Stop();

      pro.Start();
      t1 = lasvm_process(sv, s, (double) data_container->label(s));
      pro.Stop();

      rep.Start();
      if (deltamax <= 1000) // potentially multiple calls to reprocess..
          {
        t2 = lasvm_reprocess(sv, epsgr); // at least one call to reprocess
        while (lasvm_get_delta(sv) > deltamax && deltamax < 1000) {
          t2 = lasvm_reprocess(sv, epsgr);
        }
      }
      rep.Stop();

      const int l = (int) lasvm_get_l(sv);
      if (verbosity == 2) {
        printf("%d\t%d\t", s, data_container->label(s));
        printf("l=%d process=%d reprocess=%d\n", l, t1, t2);
      } else if (verbosity == 1) {
        {
          fprintf(stdout, "%d/%d (%%%2.2f) (nSvs=%d) ", i, m,
              100.0 * i / m, l);
          fprintf(stdout,
              "Sampling %2.2f, Process %2.2f, Reprocess %2.2f ",
              sel.GetMeanTime() * 1000, pro.GetMeanTime() * 1000,
              rep.GetMeanTime() * 1000);
          fprintf(stdout, "Total %2.2f ms\r",
              tot.GetMeanTime() * 1000);
          fflush(stdout);
        }
      } else if (verbosity == 0) {
        advance_spinner(5);
      }
      tot.Stop();

      // termination sterategy
      if (l >= MAX_SV) {
        break;
      }
    }

    inew.resize(0);
    iold.resize(0); // start again for next epoch..
    for (int i = 0; i < m; i++) {
      inew.push_back(data_container->item(i));
    }
  }
  finish();
  if (verbosity > 0) {
    printf("\n");
    printf("nSVs= %d(P) %d(N) %d(Total)\n", sv1, sv2, sv1 + sv2);
    printf("||w||^2=%g\n", lasvm_get_w2(sv));
  }
  return num_samples;
}

template<class ACC>
int LASVM<ACC>::select() {
  int s = -1;
  int t, i, r, j;
  double tmp, best;
  int ind = -1;

  switch (selection_type) {
  case RANDOM: // pick a random candidate
    s = rand() % inew.size();
    break;

  case MARGIN: // pick closest to margin from 50 candidates
    j = candidates;
    if ((int) inew.size() < j)
      j = inew.size();
    best = 1e20;
    for (i = 0; i < j; i++) {
      s = rand() % inew.size();
      r = inew[s];
      tmp = lasvm_predict(sv, r);
      if (tmp < 0)
        tmp = -tmp;
      if (tmp < best) {
        best = tmp;
        ind = s;
      }
    }
    s = ind;
    break;
  case MARGINBAL: // pick closest to margin from 50 candidates
    j = candidates;
    if ((int) inew.size() < j)
      j = inew.size();
    best = 1e20;
    int posc = candidates / 2;
    int negc = candidates / 2;
    for (; posc + negc > 0;) {
      s = rand() % inew.size();
      if (data_container->label(inew[s]) > 0) {
        if (posc > 0) {
          posc--;
        } else {
          continue;
        }
      } else {
        if (negc > 0) {
          negc--;
        } else {
          continue;
        }
      }
      r = inew[s];
      tmp = lasvm_predict(sv, r);
      if (tmp < 0)
        tmp = -tmp;
      if (tmp < best) {
        best = tmp;
        ind = s;
      }
    }
    s = ind;
    break;
  }

  t = inew[s];
  inew[s] = inew[inew.size() - 1];
  inew.pop_back();
  iold.push_back(t);
  return t;
}

template<class ACC>
void LASVM<ACC>::finish() {
  const int m = data_container->label_size();
  {
    int iter = 0;
    do {
      iter += lasvm_finish(sv, epsgr);
    } while (lasvm_get_delta(sv) > epsgr);
  }

  {
    int l = (int) lasvm_get_l(sv);
    int *svind, svs;
    svind = new int[l];
    svs = lasvm_get_sv(sv, svind);
    alpha.resize(m);
    for (int i = 0; i < m; i++)
      alpha[i] = 0;
    double *svalpha;
    svalpha = new double[l];
    lasvm_get_alpha(sv, svalpha);
    for (int i = 0; i < svs; i++)
      alpha[svind[i]] = svalpha[i];
    b0 = lasvm_get_b(sv);
    delete[] svind;
    delete[] svalpha;
  }

  {
    double max_alpha = 0;
    for (int i = 0; i < m; i++) {
      if (alpha[i] > max_alpha)
        max_alpha = alpha[i];
      if (-alpha[i] > max_alpha)
        max_alpha = -alpha[i];
    }
    alpha_tol = max_alpha / 1000.0;
    sv1 = 0;
    sv2 = 0;
    for (int i = 0; i < m; i++) {
      if (data_container->label(i) > 0) {
        if (alpha[i] >= alpha_tol)
          sv1++;
      } else {
        if (-alpha[i] >= alpha_tol)
          sv2++;
      }
    }
  }
}

template<class ACC>
void LASVM<ACC>::save(const char* fn, double kgamma) {
  FILE *fp = fopen(fn, "w");
  if (fp == NULL)
    return;

  fprintf(fp, "svm_type c_svc\n");
  fprintf(fp, "kernel_type RBF\n");

  fprintf(fp, "gamma %g\n", kgamma);

  fprintf(fp, "nr_class %d\n", 2);
  fprintf(fp, "total_sv %d\n", sv1 + sv2);

  fprintf(fp, "rho %g\n", b0);

  fprintf(fp, "label 1 -1\n");
  fprintf(fp, "nr_sv");
  fprintf(fp, " %d %d", sv1, sv2);
  fprintf(fp, "\n");
  fprintf(fp, "SV\n");

  for (int j = 0; j < 2; j++)
    for (int i = 0; i < data_container->label_size(); i++) {
      if (j == 0 && data_container->label(i) < 0)
        continue;
      if (j == 1 && data_container->label(i) > 0)
        continue;
      if ((data_container->label(i) > 0 ? alpha[i] : -alpha[i])
          < alpha_tol)
        continue; // not an SV

      fprintf(fp, "%.16g ", alpha[i]);
      std::vector<double> point = data_container->get(i);
      for (size_t index = 0; index < point.size(); index++) {
        fprintf(fp, "%d:%.8g ", index, point[index]);
      }
      fprintf(fp, "\n");
    }
  fclose(fp);
}

template<class ACC>
void LASVM<ACC>::make_old(int val) {
  int i, ind = -1;
  for (i = 0; i < (int) inew.size(); i++) {
    if (inew[i] == val) {
      ind = i;
      break;
    }
  }

  if (ind >= 0) {
    inew[ind] = inew[inew.size() - 1];
    inew.pop_back();
    iold.push_back(val);
  }
}
template<class ACC>
void LASVM<ACC>::set_data_container(const ContainerType& dc) {
  data_container = &dc;
  touch();
}

template<class ACC>
void LASVM<ACC>::voidPredicted() {
  roc_items.clear();
}

template<class ACC>
void LASVM<ACC>::predictAll(double ratio = 0) {
  static double ratio_s = 0;
  if (allPredicted && ratio_s >= ratio) {
    return;
  }
  ratio_s = ratio;
  touch();
  voidPredicted();

  const size_t containerSize = data_container->size();
  roc_items.reserve(containerSize);
  itk::TimeProbe predictTimer;
  for (int i = 0; i < containerSize; i++) {

#pragma omp critical
    advance_percentage(containerSize);

    if (ratio > 0 && rand() > RAND_MAX * ratio) {
      continue;
    }
    predictTimer.Start();
    const int index = data_container->item(i);

#pragma omp critical
    {
      if (data_container->label(index) > 0) {
        NP++;
      } else {
        NN++;
      }
    }

    const double kernelVals = lasvm_kern_val(sv, index);
    const int label = data_container->label(index);
    roc_items.push_back(ROC_Item(label, kernelVals, index));
    predictTimer.Stop();
  }

  std::sort(roc_items.begin(), roc_items.end(), std::greater<ROC_Item>());
  allPredicted = true;
}

template<class ACC>
double LASVM<ACC>::tuneThreshold(double desiredFPR) {
  if (desiredFPR <= 0) {
    return b0;
  }
  predictAll(1);

  int TP_Acc(0), FP_Acc(0);
  for (std::vector<ROC_Item>::iterator roc_it = roc_items.begin();
      roc_it != roc_items.end(); ++roc_it) {
    TP_Acc += roc_it->label > 0 ? 1 : 0;
    FP_Acc += roc_it->label < 0 ? 1 : 0;
    const double currentTPR = double(TP_Acc) / NP;
    const double currentFPR = double(FP_Acc) / NN;
    if (desiredFPR < currentFPR) {
      b0 = roc_it->confidence;
      return b0;
    }
  }
}

template<class ACC>
double LASVM<ACC>::ROC(const char* fn = "roc.dat") {
  predictAll(1);

  std::vector<double> TPR, FPR;
  double AUC = 0;
  int TP_Acc = 0, FP_Acc = 0;
  double prevTPR = 0;

  double wTPR = -1, wFPR = -1;

  std::ofstream rocfile(fn);
  std::cout << "TOTAL: " << NN+NP << " POS: " << NP << " NEG: " << NN << std::endl;
  for (std::vector<ROC_Item>::iterator roc_it = roc_items.begin();
      roc_it != roc_items.end(); ++roc_it) {
    TP_Acc += roc_it->label > 0 ? 1 : 0;
    FP_Acc += roc_it->label < 0 ? 1 : 0;

    const double currentTPR = double(TP_Acc) / NP;
    const double currentFPR = double(FP_Acc) / NN;

    TPR.push_back(currentTPR);
    FPR.push_back(currentFPR);

    AUC += currentFPR * (currentTPR - prevTPR);
    prevTPR = currentTPR;

    rocfile << currentFPR << "\t"
        << currentTPR << "\t"
        << std::setw(2) << roc_it->label << "\t"
        << (roc_it->label > 0 ? 1 : 0) << "\t"
        << (roc_it->label < 0 ? 1 : 0) << "\t"
        << roc_it->confidence
        << std::endl;

    if (wTPR < 0 && roc_it->confidence <= b0) {
      wTPR = currentTPR;
      wFPR = currentFPR;
    }
  }
  rocfile.close();

  std::stringstream gnuplotCommands;
  gnuplotCommands << "set size square" << std::endl;
  gnuplotCommands << "set title 'ROC Curve'" << std::endl;
  gnuplotCommands << "set xlabel 'False positive rate'" << std::endl;
  gnuplotCommands << "set ylabel 'True positive rate'" << std::endl;
  gnuplotCommands << "set xr [0.0:1.0]" << std::endl;
  gnuplotCommands << "set yr [0.0:1.0]" << std::endl;
  gnuplotCommands << "plot '" << fn
      << "' using 1:2 notitle with lines, \"< echo '" << wFPR << " "
      << wTPR << "'\" notitle" << std::endl;

  FILE* pp = popen("gnuplot -persist", "w");
  fprintf(pp, "%s", gnuplotCommands.str().c_str());
  return AUC;
}

template<class ACC>
std::vector<unsigned int> LASVM<ACC>::resample() {
  std::vector<unsigned int> new_proxy;
  for (std::vector<ROC_Item>::iterator roc_it = roc_items.begin();
      roc_it != roc_items.end(); ++roc_it) {
    if (roc_it->confidence > b0) { // classified as positive, will flow to the next level of cascade.
      new_proxy.push_back(roc_it->index);
    }
  }
  return new_proxy;
}

template<class ACC>
void LASVM<ACC>::set_max_sv(const int& msv) {
  MAX_SV = msv;
  touch();
}

template<class ACC>
void LASVM<ACC>::set_verbos(const int & v) {
  verbosity = v;
}

template<class ACC>
LASVM<ACC>::~LASVM() {
  if (sv) {
    lasvm_destroy(sv);
  }
  if (kcache) {
    lasvm_kcache_destroy(kcache);
  }
}

#endif
