/*
 * General ITK Modules
 */
#include "itkNumericTraits.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageToVectorImageFilter.h"
#include "itkImageRegionConstIterator.h"
/*
 * Internal ITK Modules. Warning. These files are not inside ITK.
 */
#include "itkSVMImageFilter.h"
#include "itkProgressWatcher.h"
#include "SVM.h"
#include "util.h"
#include "LASVM.h"
/*
 * CPP Headers
 */
#include <algorithm>
#include <string>
#include <vector>
#include <fstream>
#include <tclap/CmdLine.h>

#ifndef DIM
#define DIM 3
#endif

typedef float ResultPixelType;
typedef float ImagePixelType;

typedef itk::Image<ResultPixelType, DIM> ResultImageType;
typedef itk::Image<ImagePixelType, DIM> OrgImageType;
typedef itk::ImageToVectorImageFilter<OrgImageType> CollectorType;
typedef CollectorType::OutputImageType CollectedType;

typedef itk::ImageFileReader<CollectedType> ReaderType;
typedef itk::ImageFileReader<OrgImageType> MaskReaderType;
typedef itk::ImageFileWriter<ResultImageType> ResultWriterType;

typedef itk::SVMImageFilter<CollectedType, ResultImageType> SegmentatorType;

int main(int argc, char *argv[]) {
  using std::vector;
  using std::string;

  TCLAP::CmdLine cmd("Segment White Matter Lesions", ' ', "0.9");

  TCLAP::ValueArg<int> radius("r", "radius",
      "radius of the feature patch in millimeter", false, 1, "integer",
      cmd);

  TCLAP::ValueArg<string> outfile("o", "outpref",
      "Prefix to append to sequence name for output filename", false,
      "out_", "string", cmd);

  TCLAP::MultiArg<string> sequences("i", "input",
      "Stacked Input sequences e.g. Stack_1.nii", true, "string", cmd);

  TCLAP::MultiArg<string> groundTruth("g", "groundtruth",
      "Ground truth sequences e.g. WML.nii", true, "string", cmd);

  TCLAP::ValueArg<string> dir("d", "basedir",
      "Directory where the files lie.", false, ".", "string", cmd);

  TCLAP::ValueArg<string> fn("f", "confidence", "Confidence file.", false,
      "roc.csv", "string", cmd);

  TCLAP::ValueArg<string> modelHeaderFile("m", "model", "Model file", true,
      "model.hd", "string", cmd);

  TCLAP::SwitchArg verb("v", "verbose", "More verbose", cmd, false);

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

  SegmentatorType::Pointer segmentator = SegmentatorType::New();
  ResultWriterType::Pointer writer = ResultWriterType::New();

  /*
   * Argument and setting up the pipeline
   */
  try {
    segmentator->SetRadius(radius.getValue());
    SVM_Exp svm;
    vector<string> modelFileNames = parseModelHeaderFile(
        modelHeaderFile.getValue());
    svm.Load(modelFileNames[0].c_str());
    segmentator->SetClassifier(&svm);

    vector<string>::const_iterator seqNameIt = sequences.getValue().begin();
    vector<string>::const_iterator maskIt = groundTruth.getValue().begin();

    vector<ROC_Item> roc;

    int NN = 0, PP = 0;
    while (seqNameIt != sequences.getValue().end()) {

      MaskReaderType::Pointer mReader = MaskReaderType::New();
      mReader->SetFileName(fullPath(dir.getValue(), *maskIt));
      mReader->Update();

      ReaderType::Pointer reader = ReaderType::New();
      reader->SetFileName(fullPath(dir.getValue(), *seqNameIt));
      segmentator->SetInput(reader->GetOutput());

      writer->SetFileName(outfile.getValue() + *seqNameIt);
      if (verb.getValue()) {
        itk::ProgressWatcher watcher(segmentator,
            (*seqNameIt + " level 1").c_str());
        segmentator->Update();
      } else {
        segmentator->Update();
      }
      writer->SetInput(segmentator->GetOutput());
      writer->Update();

      itk::ImageRegionConstIterator<ResultImageType> compIterator(
          segmentator->GetOutput(),
          segmentator->GetOutput()->GetLargestPossibleRegion());

      itk::ImageRegionConstIterator<OrgImageType> gtIterator(
          mReader->GetOutput(),
          mReader->GetOutput()->GetLargestPossibleRegion());

      while (!compIterator.IsAtEnd()) {
        const ImagePixelType g = gtIterator.Get();
        const ResultPixelType r = compIterator.Get();
        if (r != itk::NumericTraits<ResultPixelType>::min()) {
          if (g > 0) {
            PP++;
            roc.push_back(ROC_Item(1, r));
          } else {
            NN++;
            roc.push_back(ROC_Item(-1, r));
          }
        }
        ++compIterator;
        ++gtIterator;
      }

      ++seqNameIt;
      ++maskIt;
    }
    std::cout << "TOTAL: " << NN+PP << " POS: " << PP << " NEG: " << NN 
              << std::endl;
    std::sort(roc.begin(), roc.end(), std::greater<ROC_Item>());
    std::ofstream rocfile(fn.getValue().c_str());
    int TP_Acc = 0, FP_Acc = 0;
    for (vector<ROC_Item>::const_iterator roc_it = roc.begin();
        roc_it != roc.end(); ++roc_it) {

      TP_Acc += roc_it->label > 0 ? 1 : 0;
      FP_Acc += roc_it->label < 0 ? 1 : 0;

      const double currentTPR = double(TP_Acc) / PP;
      const double currentFPR = double(FP_Acc) / NN;

      rocfile << currentFPR << "\t"
          << currentTPR << "\t"
          << std::setw(2) << roc_it->label << "\t"
          << (roc_it->label > 0 ? 1 : 0) << "\t"
          << (roc_it->label < 0 ? 1 : 0) << "\t"
          << roc_it->confidence
          << std::endl;

    }
    rocfile.close();

    std::stringstream gnuplotCommands;
    gnuplotCommands << "set size square" << std::endl;
    gnuplotCommands << "set title 'ROC Curve'" << std::endl;
    gnuplotCommands << "set xlabel 'False positive rate'" << std::endl;
    gnuplotCommands << "set ylabel 'True positive rate'" << std::endl;
    gnuplotCommands << "set xr [0.0:1.0]" << std::endl;
    gnuplotCommands << "set yr [0.0:1.0]" << std::endl;
    gnuplotCommands << "plot '" << fn.getValue()
                    << "' using 1:2 notitle with lines" << std::endl;

    FILE* pp = popen("gnuplot -persist", "w");
    fprintf(pp, "%s", gnuplotCommands.str().c_str());

  } catch (itk::ExceptionObject & err) {
    std::ostringstream errorMessage;
    errorMessage << "Exception caught!\n" << err << "\n";
    itk::OutputWindowDisplayErrorText(errorMessage.str().c_str());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
