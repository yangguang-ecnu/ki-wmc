/*
 * General ITK Modules
 */
#include "itkTimeProbe.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageRegionConstIterator.h"
#include "itkMacro.h"
/*
 * Internal ITK Modules. Warning. These files are not inside ITK.
 */
#include "itkImageToVectorImageFilter.h"
#include "itkImageNeighborLinearAccess.h"
#include "util.h"

/*
 * CPP Headers
 */
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <tclap/CmdLine.h>

#include "buildinfo.h"

#ifndef DIM
#define DIM 3
#endif

typedef unsigned char MaskPixelType;
typedef float ImagePixelType;

typedef itk::Image<MaskPixelType, DIM> MaskImageType;
typedef itk::ImageToVectorImageFilter<itk::Image<ImagePixelType, DIM> > 
        CollectorType;
typedef CollectorType::OutputImageType CollectedType;

typedef itk::ImageFileReader<CollectedType> ReaderType;
typedef itk::ImageFileReader<MaskImageType> MaskReaderType;

typedef itk::ImageNeighborLinearAccess<CollectedType, MaskImageType> AccessType;
/*
 * SVM Stuff
 */
#include "LASVM.h"
typedef LASVM<AccessType> TrainerType;

AccessType* acs;

double kgamma = 0; // Kernel param
double C = 0; // optimizer penalty
std::vector<double> norm2;
double kernel(int i, int j, void *kparam) {
  const double dot = acs->dot(i, j);
  return exp(-kgamma * (norm2[i] + norm2[j] - 2 * dot));
}

int main(int argc, char *argv[]) {
  using std::vector;
  using std::string;

  TCLAP::CmdLine cmd("Train White Matter Lesions" BUILDINFO, ' ', "0.9",
      true);

  TCLAP::MultiSwitchArg verb("V", "", "set verbosity", cmd, false);

  TCLAP::ValueArg<string> outfile("o", "outfile",
      "File to write trained data", false, "model.hd", "string", cmd);

  TCLAP::ValueArg<int> radius("r", "radius",
      "radius of the feature patch in millimeter", false, 1, "integer",
      cmd);

  TCLAP::ValueArg<float> gamma("g", "gamma", "kernel variance", false,
      1.0 / 16, "float", cmd);

  TCLAP::ValueArg<float> cost("c", "cost", "optimization penalty cost", false,
      1, "float", cmd);

  TCLAP::ValueArg<int> levels("l", "levels", "number of levels in hierarchy",
      false, 1, "integer", cmd);

  TCLAP::ValueArg<int> maxsv("s", "maxsv",
      "maximum number of support vectors", false, 1000, "integer", cmd);

  TCLAP::ValueArg<float> totalFPR("t", "fpr", "desired total FPR", false,
      0.05, "float", cmd);

  TCLAP::MultiArg<string> sequences("i", "input",
      "Stacked Input sequences e.g. Stack_1.nii", true, "string", cmd);

  TCLAP::MultiArg<string> masks("m", "masks",
      "Collected Input sequences e.g. MASK_1.nii", true, "string", cmd);

  TCLAP::ValueArg<string> dir("d", "basedir",
      "Directory where the files lie.", false, ".", "string", cmd);

  // Parse the argv array.
  try {
    cmd.parse(argc, argv);
  } catch (TCLAP::ArgException &e) {
    std::ostringstream errorMessage;
    errorMessage << "error: " << e.error() << " for arg " << e.argId()
                 << std::endl;
    itk::OutputWindowDisplayErrorText(errorMessage.str().c_str());
    return EXIT_FAILURE;
  }

  /*
   * Argument and setting up the pipeline
   */
  std::cout.setf(std::ios_base::unitbuf);
  try {
    itk::TimeProbe loading;
    loading.Start();
    std::cout << "Loading accessor." << std::endl;
    AccessType access;
    vector<string>::const_iterator seqNameIt = sequences.getValue().begin();
    while (seqNameIt != sequences.getValue().end()) {
      try {
        ReaderType::Pointer reader = ReaderType::New();
        reader->SetFileName(fullPath(dir.getValue(), *seqNameIt));
        reader->Update();
        CollectedType::Pointer image = reader->GetOutput();
        access.addImage(image);

      } catch (itk::ExceptionObject & err) {
        std::ostringstream errorMessage;
        errorMessage << "Exception caught during reading sequences.\n"                               << err << "\n";
        itk::OutputWindowDisplayErrorText(errorMessage.str().c_str());
      }
      ++seqNameIt;
    }

    access.makeReady(radius.getValue());

    std::vector<char> Y; // Labels

    AccessType::InputImageType::IndexType index;
    size_t p_index;
    vector<string>::const_iterator maskNameIt = masks.getValue().begin();
    while (maskNameIt != masks.getValue().end()) {
      try {
        MaskReaderType::Pointer maskReader = MaskReaderType::New();
        maskReader->SetFileName(fullPath(dir.getValue(), *maskNameIt));
        maskReader->Update();
        MaskImageType::Pointer mask = maskReader->GetOutput();
        for (p_index = 0; p_index < access.getImage_pix(); p_index++) {
          access.calc_index(p_index, index);
          Y.push_back(mask->GetPixel(index) > 0 ? 1 : -1);
        }
      } catch (itk::ExceptionObject & err) {
        std::ostringstream errorMessage;
        errorMessage << "Exception caught during reading masks."
            << std::endl << err << std::endl;
        itk::OutputWindowDisplayErrorText(errorMessage.str().c_str());
      }
      ++maskNameIt;
    }
    acs = &access;

    std::vector<unsigned int> proxy; // Labels
    norm2.resize(acs->getTotal_pix());
    size_t NN(0), PP(0);
    for (size_t aa = 0; aa < acs->getTotal_pix(); aa++) {
      norm2[aa] = acs->norm2(aa);
      if (norm2[aa] > 0) {
        proxy.push_back(aa);
        if(Y[aa] > 0){
          PP++;
        }else{
          NN++;
        }
      }
      advance_percentage(acs->getTotal_pix());
    }
    advance_percentage(-1);
    loading.Stop();
    std::cout << "\rLoaded (Load time " << loading.GetMean() << " seconds)"
              << std::endl;
    std::cout << "TOTAL: " << NN+PP << " POS: " << PP << " NEG: " << NN 
              << std::endl;
    int seed = time(NULL);
    srand(seed);

    TrainerType trainer;

    trainer.set_param(kernel, 1024, NULL);

    kgamma = gamma.getValue();
    C = cost.getValue();

    if (C < 0 || kgamma <= 0) {
      std::ostringstream errorMessage;
      errorMessage << "Invalid kernel value or optimization cost." << std::endl;
      itk::OutputWindowDisplayErrorText(errorMessage.str().c_str());
      return EXIT_FAILURE;
    }

    std::ofstream mdlFile(outfile.getValue().c_str());
    mdlFile << "# Cascade model header" << std::endl;
    mdlFile << "# " << levels.getValue() << " levels" << std::endl;
    mdlFile << "# Radius " << radius.getValue() << " voxels" << std::endl;
    mdlFile << "# " << acs->getDim() << " Dim" << std::endl;

    std::cout << "C:" << setprecision(5) << C << " G:" << setprecision(5)
              << kgamma << " MaxSV:" << maxsv.getValue() << std::endl;

    for (int level = 1; level <= levels.getValue(); level++) {

      const double desiredFPR = totalFPR.getValue();

      // Initial Training
      std::cout << "Training level " << level << "/" << levels.getValue()
                << std::endl;
      
      TrainerType::ContainerType data_container(Y, proxy, acs);
      trainer.set_verbos(verb.getValue());
      trainer.set_max_sv(maxsv.getValue());
      const int num_proce = trainer.train_online(C, data_container);
      std::cout << "Training done. " << num_proce << " points used."
                << std::endl;

      if (desiredFPR > 0) {
        // Tuning the bias for desired true positive rate
        std::cout << "Tune bias (target FPR:" << desiredFPR << ") ";
        trainer.tuneThreshold(desiredFPR);
        std::cout << std::endl;
      }

      // save the model
      std::ostringstream levelFilename;
      levelFilename << outfile.getValue() << "." << level;
      std::cout << "Saving model file: \"" << levelFilename.str() << "\"";
      trainer.save(levelFilename.str().c_str(), kgamma);
      mdlFile << levelFilename.str() << std::endl;
      mdlFile.flush();
      std::cout << std::endl;

      // draw ROC curve
      if (verb.getValue()) {
        std::ostringstream rocDataFilename;
        rocDataFilename << "roc_" << level << "_" << time(NULL) << ".dat";
        trainer.ROC(rocDataFilename.str().c_str());
      }

      // resample for the next level. Samples that are classified as positive in
      // this level will pass through the next level also.
      if (level != levels.getValue()) { // no need to resample in the last level
        std::cout << "Resampling" << std::endl;
        proxy = trainer.resample();
      }
    }
    mdlFile.close();

  } catch (itk::ExceptionObject & err) {
    std::ostringstream errorMessage;
    errorMessage << "Exception caught during pipeline setup.\n" << err << "\n";
    itk::OutputWindowDisplayErrorText(errorMessage.str().c_str());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
