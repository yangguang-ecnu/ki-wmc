/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkNeighborhoodCollectorImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2008-10-16 19:33:40 $
  Version:   $Revision: 1.7 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkNeighborhoodCollectorImageFilter_h
#define __itkNeighborhoodCollectorImageFilter_h


// First make sure that the configuration is available.
// This line can be removed once the optimized versions
// gets integrated into the main directories.
#include "itkConfigure.h"

#include "itkImageToImageFilter.h"
#include "itkVariableLengthVector.h"
#include "itkVectorImage.h"

#include "itkImage.h"

#include <vector>
namespace itk
{
/** \class NeighborhoodCollectorImageFilter
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
 * \sa Neighborhood
 * \sa NeighborhoodOperator
 * \sa NeighborhoodIterator
 * 
 * \ingroup IntensityImageFilters
 */

template <class TInputImage>
class ITK_EXPORT NeighborhoodCollectorImageFilter :
  public ImageToImageFilter< TInputImage,VectorImage< typename TInputImage::InternalPixelType,::itk::GetImageDimension< TInputImage >::ImageDimension > >
{
public:
  /** Extract dimension from input and output image. */
  itkStaticConstMacro(InputImageDimension, unsigned int,
                      TInputImage::ImageDimension);
  itkStaticConstMacro(OutputImageDimension, unsigned int,
                      TInputImage::ImageDimension);

  /** Convenient typedefs for simplifying declarations. */
  typedef TInputImage  InputImageType;
  typedef VectorImage< typename TInputImage::InternalPixelType,::itk::GetImageDimension< TInputImage >::ImageDimension >  OutputImageType;

  /** Standard class typedefs. */
  typedef NeighborhoodCollectorImageFilter                                    Self;
  typedef ImageToImageFilter< InputImageType, OutputImageType> Superclass;
  typedef SmartPointer<Self>                                   Pointer;
  typedef SmartPointer<const Self>                             ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(NeighborhoodCollectorImageFilter, ImageToImageFilter);
  
  /** Image typedef support. */
  typedef typename InputImageType::PixelType  InputPixelType;
  typedef typename OutputImageType::PixelType OutputPixelType;

  typedef typename InputImageType::RegionType  InputImageRegionType;
  typedef typename OutputImageType::RegionType OutputImageRegionType;

  typedef typename InputImageType::SizeType InputSizeType;

  typedef typename Superclass::Superclass::Superclass::DataObjectPointerArraySizeType InputLengthSize;
  
  /** Set the radius of the neighborhood used to collect the neighbors. */
  itkSetMacro(Radius, InputSizeType);
  
  void FillRadius(typename InputSizeType::SizeValueType val){
    this->m_Radius.Fill(val);
  }

  /** Get the radius of the neighborhood used to collect the neighbors */
  itkGetConstReferenceMacro(Radius, InputSizeType);
  
  itkGetConstReferenceMacro(CollectedSize, unsigned int);
  /** NeighborhoodCollectorImageFilter needs a larger input requested region than
   * the output requested region.  As such, NeighborhoodCollectorImageFilter needs
   * to provide an implementation for GenerateInputRequestedRegion()
   * in order to inform the pipeline execution model.
   *
   * \sa ImageToImageFilter::GenerateInputRequestedRegion() */
  virtual void GenerateInputRequestedRegion() throw(InvalidRequestedRegionError);
  
#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro(SameDimensionCheck,
                  (Concept::SameDimension<InputImageDimension, OutputImageDimension>));
  /** End concept checking */
#endif

protected:
  NeighborhoodCollectorImageFilter();
  virtual ~NeighborhoodCollectorImageFilter() {}
  void PrintSelf(std::ostream& os, Indent indent) const;

  /** NeighborhoodCollectorImageFilter can be implemented as a multithreaded filter.
   * Therefore, this implementation provides a ThreadedGenerateData()
   * routine which is called for each processing thread. The output
   * image data is allocated automatically by the superclass prior to
   * calling ThreadedGenerateData().  ThreadedGenerateData can only
   * write to the portion of the output image specified by the
   * parameter "outputRegionForThread"
   *
   * \sa ImageToImageFilter::ThreadedGenerateData(),
   *     ImageToImageFilter::GenerateData() */
  virtual void ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
                            int threadId );
  
  
  virtual void BeforeThreadedGenerateData();
  virtual void GenerateOutputInformation(void);

private:
  NeighborhoodCollectorImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  InputLengthSize m_NumOutPuts;
  InputSizeType m_Radius;
  std::vector<unsigned int> index;
  unsigned int m_CollectedSize;
};
  
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkNeighborhoodCollectorImageFilter.txx"
#endif

#endif
