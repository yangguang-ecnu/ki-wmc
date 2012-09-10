/*
 * General ITK Modules
 */
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageToVectorImageFilter.h"
/*
 * Internal ITK Modules. Warning. These files are not inside ITK.
 */
#include "itkSVMImageFilter.h"
#include "itkProgressWatcher.h"
#include "SVM.h"
#include "util.h"

/*
 * CPP Headers
 */
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
typedef itk::ImageToVectorImageFilter<itk::Image<ImagePixelType, DIM> > 
        CollectorType;

typedef CollectorType::OutputImageType CollectedType;

typedef itk::ImageFileReader<CollectedType> ReaderType;
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

  TCLAP::ValueArg<string> dir("d", "basedir",
      "Directory where the files lie.", false, ".", "string", cmd);

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
    SVM svm;
    vector<string> modelFileNames = parseModelHeaderFile(
                                      modelHeaderFile.getValue());
    svm.Load(modelFileNames[0].c_str());
    segmentator->SetClassifier(&svm);

    vector<string>::const_iterator seqNameIt = sequences.getValue().begin();
    while (seqNameIt != sequences.getValue().end()) {

      ReaderType::Pointer reader = ReaderType::New();
      reader->SetFileName(fullPath(dir.getValue(), *seqNameIt));
      segmentator->SetInput(reader->GetOutput());

      writer->SetFileName(outfile.getValue() + *seqNameIt);
      writer->SetInput(segmentator->GetOutput());
      if (verb.getValue()) {
        itk::ProgressWatcher watcher(segmentator, 
                                    (*seqNameIt + " level 1").c_str());
        segmentator->Update();
      }else{
        segmentator->Update();
      }
      writer->Update();

      ++seqNameIt;
    }

  } catch (itk::ExceptionObject & err) {
    std::ostringstream errorMessage;
    errorMessage << "Exception caught!\n" << err << "\n";
    itk::OutputWindowDisplayErrorText(errorMessage.str().c_str());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
