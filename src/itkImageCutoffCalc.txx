/*=========================================================================

 Program:   Insight Segmentation & Registration Toolkit
 Module:    $RCSfile: itkImageCutoffCalc.txx,v $
 Language:  C++
 Date:      $Date: 2009-05-02 05:43:55 $
 Version:   $Revision: 1.1 $

 Copyright (c) Insight Software Consortium. All rights reserved.
 See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/
#ifndef __itkImageCutoffCalc_txx
#define __itkImageCutoffCalc_txx

#include "itkHistogram.h"
#include "itkNumericTraits.h"
#include "itkImageRegionConstIterator.h"
#include "itkMinimumMaximumImageCalculator.h"

#include "itkImageToHistogramFilter.h"



namespace itk {
namespace Statistics {

template<class TImage>
typename TImage::PixelType RobustMax(const TImage*image, double CutOff) {

  typedef TImage ImageType;
  typedef typename ImageType::PixelType PixelType;
  typedef typename NumericTraits<PixelType>::ValueType ValueType;
  typedef typename NumericTraits<ValueType>::RealType ValueRealType;

  typedef MinimumMaximumImageCalculator<ImageType> MinMaxFilterType;

  typedef Histogram<ValueRealType> HistogramType;
  typedef typename HistogramType::Pointer HistogramPointer;
  typedef typename HistogramType::ConstPointer HistogramConstPointer;
  typedef typename HistogramType::SizeType HistogramSizeType;
  typedef typename HistogramType::MeasurementType HistogramMeasurementType;
  typedef typename HistogramType::MeasurementVectorType
      HistogramMeasurementVectorType;

  typename HistogramType::Pointer histogram = HistogramType::New();

  typename MinMaxFilterType::Pointer minMax = MinMaxFilterType::New();

  minMax->SetImage(image);
  minMax->Compute();
  {

    // allocate memory for the histogram
    HistogramSizeType size(1);

    HistogramMeasurementVectorType lowerBound(1);
    HistogramMeasurementVectorType upperBound(1);

    size[0] = 1024;

    lowerBound.Fill(minMax->GetMinimum());
    upperBound.Fill(minMax->GetMaximum());

    //Initialize with equally spaced bins.
    histogram->SetMeasurementVectorSize(1);
    histogram->Initialize(size, lowerBound, upperBound);
    histogram->SetToZero();
  }

  HistogramMeasurementVectorType measurement(1);

  {
    // put each image pixel into the histogram
    typedef ImageRegionConstIterator<ImageType> ConstIterator;
    ConstIterator iter(image, image->GetBufferedRegion());

    iter.GoToBegin();
    while (!iter.IsAtEnd()) {
      PixelType value = iter.Get();

      if (static_cast<double> (value) >= minMax->GetMinimum()
          && static_cast<double> (value) <= minMax->GetMaximum()) {
        // add sample to histogram
        measurement[0] = value;
        histogram->IncreaseFrequencyOfMeasurement(measurement, 1);
      }
      ++iter;
    }
  }

  // calculate histogram cut
  {
    typename HistogramType::Iterator histogramIterator = histogram->Begin();
    double f = 0;
    CutOff *= histogram->GetTotalFrequency();
    while (histogramIterator != histogram->End() && f < CutOff) {

      f = f + histogramIterator.GetFrequency();
      ++histogramIterator;
    }
    if (histogramIterator != histogram->End()) {
      return (PixelType)(histogramIterator.GetMeasurementVector()[0]);
    }
  }
  return minMax->GetMaximum();
}

} // end of namespace Statistics 
} // end of namespace itk

#endif
