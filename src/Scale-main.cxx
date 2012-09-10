/*
 * General ITK Modules
 */
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkTimeProbe.h"
/*
 * Internal ITK Modules. Warning. These files are not inside ITK.
 */
#include "itkRobustOneScale.h"

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
typedef itk::Image<IntermediatePixelType, DIM> IntermediateImageType;

typedef itk::ImageFileReader<InputImageType> ReaderType;
typedef itk::ImageFileWriter<IntermediateImageType> WriterType;

typedef itk::RobustOneScale<InputImageType, IntermediateImageType> ScaleType;

int main(int argc, char *argv[]) {
  using std::vector;
  using std::string;

  TCLAP::CmdLine cmd("Scale an image", ' ', "0.9");

  TCLAP::ValueArg<float> radius("r", "radius", "radius of noise reduction",
      false, 1, "float", cmd);

  TCLAP::ValueArg<string> prefix("p", "prefix", "Output scaled file", false,
      "scaled-", "string", cmd);

  TCLAP::ValueArg<string> outdir("o", "outdir", "Directory to write output.",
      false, ".", "string", cmd);

  TCLAP::MultiArg<string> inputs("i", "input",
      "Input sequences e.g. FLAIR_1.nii", true, "string", cmd);

  TCLAP::ValueArg<string> basedir("d", "basedir",
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

  /*
   * Argument and setting up the pipeline
   */
  try {
    itk::TimeProbe clock;

    vector<string>::const_iterator seqNameIt = inputs.getValue().begin();
    while (seqNameIt != inputs.getValue().end()) {
      clock.Start();

      ReaderType::Pointer reader = ReaderType::New();
      reader->SetFileName(fullPath(basedir.getValue(), *seqNameIt));
      ScaleType::Pointer scaleFilter = ScaleType::New();

      scaleFilter->SetInput(reader->GetOutput());

      WriterType::Pointer writer = WriterType::New();
      writer->SetFileName(fullPath(outdir.getValue(), prefix.getValue())
          + *seqNameIt);
      writer->SetInput(scaleFilter->GetOutput());
      writer->Update();
      clock.Stop();

      std::ostringstream logMessage;
      logMessage << "Mean: " << clock.GetMean() << "\tTotal: " 
                 << clock.GetTotal() << "\t seconds\t\t\r";

      itk::OutputWindowDisplayText(logMessage.str().c_str());

      ++seqNameIt;
    }
  } catch (itk::ExceptionObject & err) {
    std::ostringstream errorMessage;
    errorMessage << "Exception caught during scale pipeline.\n" << err << "\n";
    itk::OutputWindowDisplayErrorText(errorMessage.str().c_str());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
