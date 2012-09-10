/*
 * General ITK Modules
 */
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

/*
 * Internal ITK Modules. Warning. These files are not inside ITK.
 */
#include "itkRobustOneScale.h"
#include "itkImageToVectorImageFilter.h"

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

typedef float ImagePixelType;
typedef float IntermediatePixelType;

typedef itk::Image<ImagePixelType, DIM> InputImageType;
typedef itk::Image<IntermediatePixelType, DIM> IntermediateImageType;
typedef itk::ImageToVectorImageFilter<IntermediateImageType> CollectorType;

typedef CollectorType::OutputImageType OutputImageType;
typedef itk::ImageFileReader<InputImageType> ReaderType;
typedef itk::ImageFileWriter<OutputImageType> WriterType;

typedef itk::RobustOneScale<InputImageType, IntermediateImageType> ScaleType;


int main(int argc, char *argv[]) {
  using std::vector;
  using std::string;

  TCLAP::CmdLine cmd("Stack multiple images", ' ', "0.9");

  TCLAP::SwitchArg noscale("", "noscale","Skip scaling", cmd, false);

  TCLAP::ValueArg<string> outfile("o", "outfile",
      "File to write stacked vector image", true, "out.nii", "string",
      cmd);

  TCLAP::MultiArg<string> sequences("i", "input",
      "Input sequences e.g. FLAIR_1.nii", true, "string", cmd);

  TCLAP::ValueArg<string> dir("d", "basedir",
      "Directory where the files lie.", false, ".", "string", cmd);

  // Parse the argv array.
  try{
    cmd.parse(argc, argv);
  }catch(TCLAP::ArgException &e){
    std::ostringstream errorMessage;
    errorMessage << "error: " << e.error() << " for arg " << e.argId()
                 << std::endl;
    itk::OutputWindowDisplayErrorText(errorMessage.str().c_str());
    return EXIT_FAILURE;
  }

  CollectorType::Pointer collector = CollectorType::New();
  WriterType::Pointer writer = WriterType::New();

  /*
   * Argument and setting up the pipeline
   */
  try {
    unsigned int i=0;
    vector<string>::const_iterator seqNameIt = sequences.getValue().begin();
    while (seqNameIt != sequences.getValue().end()) {

      ReaderType::Pointer reader = ReaderType::New();
      reader->SetFileName(fullPath(dir.getValue(), *seqNameIt));
      ScaleType::Pointer scaleFilter = ScaleType::New();

      scaleFilter->SetInput(reader->GetOutput());

      if (noscale.getValue()) {
        scaleFilter->TriggerOff();
      }
      scaleFilter->Update();

      collector->SetInput(i++, scaleFilter->GetOutput());

      ++seqNameIt;
    }
    collector->Update();
    CollectorType::OutputImageType::Pointer output= collector->GetOutput();
    writer->SetFileName(outfile.getValue());
    writer->SetInput(output);
  } catch (itk::ExceptionObject & err) {
    std::ostringstream errorMessage;
    errorMessage << "Exception caught during pipeline setup.\n" << err << "\n";
    itk::OutputWindowDisplayErrorText(errorMessage.str().c_str());
    return EXIT_FAILURE;
  }

  /*
   * This is the main trigger for whole pipeline.
   */
  try {
    writer->Update();
  } catch (itk::ExceptionObject & err) {
    std::ostringstream errorMessage;
    errorMessage << "Exception caught during updating pipeline.\n" << err 
                 << "\n";
    itk::OutputWindowDisplayErrorText(errorMessage.str().c_str());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
