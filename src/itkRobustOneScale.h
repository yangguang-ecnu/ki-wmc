/*=========================================================================

 Program:   Insight Segmentation & Registration Toolkit
 Module:    $RCSfile: itkRobustOneScale.h,v $
 Language:  C++
 Date:      $Date: 2008-10-16 19:33:40 $
 Version:   $Revision: 1.7 $

 Copyright (c) Insight Software Consortium. All rights reserved.
 See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/
#ifndef ITKROBUSTONESCALE_H_
#define ITKROBUSTONESCALE_H_

#include "itkImageToImageFilter.h"
#include "itkMedianImageFilter.h"
#include "itkMinimumMaximumImageCalculator.h"
#include "itkNumericTraits.h"

#include "itkShiftScaleImageFilter.h"
#include "itkIntensityWindowingImageFilter.h"

namespace itk {
/** \class RobustOneScale
 * \brief Applies a median filter to an image
 *
 * Computes an image where a given pixel is the median value of the
 * the pixels in a neighborhood about the corresponding input pixel.
 *
 * A median filter is one of the family of nonlinear filters.  It is
 * used to smooth an image without being biased by outliers or shot noise.
 *
 * This filter requires that the input pixel type provides an operator<()
 * (LessThan Comparable).
 *
 * \sa Image
 *
 * \ingroup IntensityImageFilters
 */
template<class TInputImageType, class TOutputImageType>
class ITK_EXPORT RobustOneScale: public itk::ImageToImageFilter<TInputImageType, TOutputImageType> {
public:

  /** Extract dimension from input and output image. */
  itkStaticConstMacro(InputImageDimension, unsigned int,
      TInputImageType::ImageDimension);
  itkStaticConstMacro(OutputImageDimension, unsigned int,
      TInputImageType::ImageDimension);

  /** Convenient typedefs for simplifying declarations. */
  typedef TInputImageType InputImageType;
  typedef TOutputImageType OutputImageType;
  typedef typename OutputImageType::PixelType OutPixelType;

  /** Standard class typedefs. */
  typedef RobustOneScale Self;
  typedef ImageToImageFilter<InputImageType, OutputImageType> Superclass;
  typedef SmartPointer<Self> Pointer;
  typedef SmartPointer<const Self> ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self)
  ;

  /** Run-time type information (and related methods). */
  itkTypeMacro(RobustOneScale, ImageToImageFilter)
  ;

  /** Image typedef support. */
  typedef typename InputImageType::SizeType InputSizeType;
  typedef MedianImageFilter<InputImageType, InputImageType>
      MedianFilterType;
  typedef MinimumMaximumImageCalculator<InputImageType> MinMaxFilterType;

  typedef ShiftScaleImageFilter<InputImageType, OutputImageType>  ScaleFilterType;

  //typedef IntensityWindowingImageFilter<InputImageType, OutputImageType> ScaleFilterType;



  /** Set the radius of the neighborhood used to collect the neighbors. */
  itkSetMacro(Radius, InputSizeType)
  ;

  /** Get the radius of the neighborhood used to collect the neighbors */
  itkGetConstReferenceMacro(Radius, InputSizeType)
  ;
  itkSetMacro(Trigger, bool);
  itkGetMacro(Trigger, bool);
  itkBooleanMacro(Trigger);

#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
    itkConceptMacro(SameDimensionCheck,
        (Concept::SameDimension<InputImageDimension, OutputImageDimension>));
  /** End concept checking */
#endif

protected:
  RobustOneScale();
  virtual ~RobustOneScale() {
  }
  void PrintSelf(std::ostream& os, Indent indent) const;

  void GenerateData();

private:
  RobustOneScale(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  typename MedianFilterType::Pointer medianFilter;
  typename MinMaxFilterType::Pointer minMax;
  typename ScaleFilterType::Pointer scaleFilter;
  InputSizeType m_Radius;
  bool m_Trigger;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkRobustOneScale.txx"
#endif

///////////////////////////////////////////////////////////////////
#endif /* ITKROBUSTONESCALE_H_ */
