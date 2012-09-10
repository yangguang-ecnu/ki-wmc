/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef __itkSVMImageFilter_hxx
#define __itkSVMImageFilter_hxx
#include "itkSVMImageFilter.h"

#include "itkNumericTraits.h"
#include "itkNeighborhoodInnerProduct.h"
#include "itkImageRegionIterator.h"
#include "itkNeighborhoodAlgorithm.h"
#include "itkOffset.h"
#include "itkProgressReporter.h"

#include "itkShapedNeighborhoodIterator.h"
#include "itkInsideBinaryBallStructuringElement.h"

#include <vector>
#include <algorithm>

namespace itk {
template<class TInputImage, class TOutputImage>
SVMImageFilter<TInputImage, TOutputImage>::SVMImageFilter() {
}

template<class TInputImage, class TOutputImage>
void SVMImageFilter<TInputImage, TOutputImage>::BeforeThreadedGenerateData() {

}
template<class TInputImage, class TOutputImage>
void SVMImageFilter<TInputImage, TOutputImage>::SetClassifier(SVM* cls) {
  classifier = cls;
}
template<class TInputImage, class TOutputImage>
void SVMImageFilter<TInputImage, TOutputImage>::ThreadedGenerateData(
    const OutputImageRegionType & outputRegionForThread,
    ThreadIdType threadId) {
  // Allocate output
  typename OutputImageType::Pointer output = this->GetOutput();
  typename InputImageType::ConstPointer input = this->GetInput();

  // Find the data-set boundary "faces"
  NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType> bC;
  typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType>::FaceListType
      faceList = bC(input, outputRegionForThread, this->GetRadius());

  // support progress methods/callbacks
  ProgressReporter progress(this, threadId,
      outputRegionForThread.GetNumberOfPixels());

  typename StructuringElementType::ConstIterator nit;
  typename StructuringElementType::SizeValueType idx;

  StructuringElementType structuringElement;
  structuringElement.SetRadius(this->GetRadius());
  structuringElement.CreateStructuringElement();

  unsigned int neighborhoodSize;
  // All of our neighborhoods have an odd number of pixels, so there is
  // always a median index (if there where an even number of pixels
  // in the neighborhood we have to average the middle two values).

  ZeroFluxNeumannBoundaryCondition<InputImageType> nbc;
  std::vector<double> pixels;
  double norm;
  // Process each of the boundary faces.  These are N-d regions which border
  // the edge of the buffer.
  for (typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<
      InputImageType>::FaceListType::iterator fit = faceList.begin(); fit
      != faceList.end(); ++fit) {

    ImageRegionIterator<OutputImageType> it = ImageRegionIterator<
        OutputImageType> (output, *fit);

    ShapedIteratorType bit(this->GetRadius(), input, *fit);

    idx = 0;
    neighborhoodSize = 0;
    for (nit = structuringElement.Begin(); nit != structuringElement.End(); ++nit, ++idx) {
      if (*nit) {
        bit.ActivateOffset(bit.GetOffset(idx));
        neighborhoodSize++;
      } else {
        bit.DeactivateOffset(bit.GetOffset(idx));
      }
    }

    bit.OverrideBoundaryCondition(&nbc);
    bit.GoToBegin();

    while (!bit.IsAtEnd()) {
      const InputPixelType& p = bit.GetPixel(neighborhoodSize/2);
      if (p[0] <= 0) {
        it.Set(NumericTraits<OutputPixelType>::min());
      } else {
        pixels.resize(neighborhoodSize * InputImageType::ImageDimension);
        for (unsigned int i = 0; i < neighborhoodSize; ++i) {
          // collect all the pixels in the neighborhood, note that we use
          // GetPixel on the NeighborhoodIterator to honor the boundary conditions
          const InputPixelType& p = bit.GetPixel(i);
          for (unsigned int j = 0; j < InputImageType::ImageDimension; ++j) {
            const double pv = double(p[j]);
            pixels[i * InputImageType::ImageDimension + j] = pv;
          }
        }
        it.Set(classifier->classify(pixels));// < 0 ? NumericTraits<OutputPixelType>::One : NumericTraits<OutputPixelType>::Zero);
      }

      ++bit;
      ++it;
      progress.CompletedPixel();
    }
  }
}
} // end namespace itk

#endif
