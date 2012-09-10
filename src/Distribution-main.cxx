/*
 * General ITK Modules
 */
#include "itkImage.h"
#include "itkAddImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkTimeProbe.h"
#include "itkNumericTraits.h"
#include "util.h"

/*
 * CPP Headers
 */
#include <string>
#include <vector>
#include <tclap/CmdLine.h>

#ifndef DIM
#define DIM 3
#endif

typedef float ImagePixelType;
typedef float IntermediatePixelType;

typedef itk::Image<ImagePixelType, DIM> InputImageType;
typedef itk::Image<IntermediatePixelType, DIM> DistributionImageType;

typedef itk::NumericTraits<DistributionImageType::PixelType>
    DistributionTrait;

typedef itk::ImageFileReader<InputImageType> ReaderType;
typedef itk::ImageFileWriter<DistributionImageType> WriterType;

typedef itk::BinaryThresholdImageFilter<InputImageType, DistributionImageType>
    BinaryThresholdImageFilterType;

typedef itk::AddImageFilter<DistributionImageType> AddImageType;

int main(int argc, char *argv[]) {
  using std::vector;
  using std::string;

  TCLAP::CmdLine cmd("Scale an image", ' ', "0.9");

  TCLAP::ValueArg<string> outfile("o", "out",
      "file to write the distribution", false, "out.nii", "string", cmd);

  TCLAP::UnlabeledMultiArg<string> inputs("Inputs", "Input files", true,
      "string", cmd);

  // Parse the argv array.
  try{
    cmd.parse(argc, argv);
  }catch(TCLAP::ArgException &e){
    std::ostringstream errorMessage;
    errorMessage << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    itk::OutputWindowDisplayErrorText(errorMessage.str().c_str());
    return EXIT_FAILURE;
  }

  /*
   * Argument and setting up the pipeline
   */
  try {
    ReaderType::Pointer reader = ReaderType::New();
    WriterType::Pointer writer = WriterType::New();

    BinaryThresholdImageFilterType::Pointer thresholdFilter =
        BinaryThresholdImageFilterType::New();
    
    AddImageType::Pointer adder = AddImageType::New();

    DistributionImageType::Pointer distribution =
        DistributionImageType::New();

    thresholdFilter->SetLowerThreshold(DistributionTrait::Zero
                                       + DistributionTrait::epsilon());

    thresholdFilter->SetInsideValue(DistributionTrait::One);
    thresholdFilter->SetOutsideValue(DistributionTrait::Zero);

    bool isFirst = true;
    string outfilename = outfile.getValue();
    vector<string>::const_iterator fileName = inputs.getValue().begin();
    for (; fileName != inputs.getValue().end(); ++fileName) {
      reader->SetFileName(*fileName);
      if (isFirst) {
        reader->Update();
        distribution->SetRegions(
            reader->GetOutput()->GetLargestPossibleRegion());
        distribution->Allocate();
        distribution->FillBuffer(DistributionTrait::Zero);
        isFirst = false;
      }
      thresholdFilter->SetInput(reader->GetOutput());

      adder->SetInput1(distribution);
      adder->SetInput2(thresholdFilter->GetOutput());
      adder->Update();
      distribution = adder->GetOutput();
    }
    writer->SetFileName(outfilename);
    writer->SetInput(distribution);
    writer->Update();

  } catch (itk::ExceptionObject & err) {
    std::ostringstream errorMessage;
    errorMessage << "Exception caught during scale pipeline.\n" << err
                 << "\n";
    itk::OutputWindowDisplayErrorText(errorMessage.str().c_str());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
