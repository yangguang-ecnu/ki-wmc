/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkNeighborhoodCollectorImageFilter.txx,v $
  Language:  C++
  Date:      $Date: 2008-10-16 19:33:41 $
  Version:   $Revision: 1.20 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkNeighborhoodCollectorImageFilter_txx
#define __itkNeighborhoodCollectorImageFilter_txx


// First make sure that the configuration is available.
// This line can be removed once the optimized versions
// gets integrated into the main directories.
#include "itkConfigure.h"

#include "itkNeighborhoodCollectorImageFilter.h"

#include "itkConstNeighborhoodIterator.h"
#include "itkNeighborhoodInnerProduct.h"
#include "itkImageRegionIterator.h"
#include "itkNeighborhoodAlgorithm.h"
#include "itkZeroFluxNeumannBoundaryCondition.h"
#include "itkOffset.h"
#include "itkProgressReporter.h"

#include "itkInsideBinaryBallStructuringElement.h"

#include <vector>
#include <algorithm>

namespace itk
{

template <class TInputImage>
NeighborhoodCollectorImageFilter<TInputImage>
::NeighborhoodCollectorImageFilter()
{
  m_Radius.Fill(0);
}


template <class TInputImage>
void 
NeighborhoodCollectorImageFilter<TInputImage>
::GenerateInputRequestedRegion() throw (InvalidRequestedRegionError)
{
  // call the superclass' implementation of this method
  Superclass::GenerateInputRequestedRegion();
  
  // get pointers to the input and output
  typename Superclass::InputImagePointer inputPtr = 
    const_cast< TInputImage * >( this->GetInput() );
  typename Superclass::OutputImagePointer outputPtr = this->GetOutput();
  
  if ( !inputPtr || !outputPtr )
    {
    return;
    }

  // get a copy of the input requested region (should equal the output
  // requested region)
  typename TInputImage::RegionType inputRequestedRegion;
  inputRequestedRegion = inputPtr->GetRequestedRegion();

  // pad the input requested region by the operator radius
  inputRequestedRegion.PadByRadius( m_Radius );

  // crop the input requested region at the input's largest possible region
  if ( inputRequestedRegion.Crop(inputPtr->GetLargestPossibleRegion()) )
    {
    inputPtr->SetRequestedRegion( inputRequestedRegion );
    return;
    }
  else
    {
    // Couldn't crop the region (requested region is outside the largest
    // possible region).  Throw an exception.

    // store what we tried to request (prior to trying to crop)
    inputPtr->SetRequestedRegion( inputRequestedRegion );
    
    // build an exception
    InvalidRequestedRegionError e(__FILE__, __LINE__);
    e.SetLocation(ITK_LOCATION);
    e.SetDescription("Requested region is (at least partially) outside the largest possible region.");
    e.SetDataObject(inputPtr);
    throw e;
    }
}


template< class TInputImage>
void
NeighborhoodCollectorImageFilter< TInputImage>
::BeforeThreadedGenerateData()
{

}
template< class TInputImage>
void
NeighborhoodCollectorImageFilter< TInputImage>
::GenerateOutputInformation()
{
  typedef InsideBinaryBallStructuringElement<unsigned char, InputImageDimension> StructuringElementType;
  typedef typename StructuringElementType::ConstIterator StructIter;
   
  StructuringElementType structuringElement;
  structuringElement.SetRadius(m_Radius);
  structuringElement.CreateStructuringElement();
  
  int i=0;
  index.erase(index.begin(), index.end());
  m_CollectedSize = 0;
  for(StructIter it = structuringElement.Begin(); it != structuringElement.End(); ++it,i++)
  {
    if (float(*it) > 0)
    {
      index.push_back(i);
      m_CollectedSize ++;
    }
  }
  m_NumOutPuts = this->ProcessObject::GetNumberOfInputs();
  m_CollectedSize*= m_NumOutPuts;
  this->GetOutput()->SetVectorLength(m_CollectedSize);
}

template< class TInputImage>
void
NeighborhoodCollectorImageFilter< TInputImage>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
                       int threadId)
{
  // Allocate output
  typename OutputImageType::Pointer output = this->GetOutput();
    
  // Find the data-set boundary "faces"

  typedef typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType>::FaceListType FaceListType;
  
  vector<FaceListType> faceLists(m_NumOutPuts);
  vector<typename FaceListType::iterator> fits(m_NumOutPuts);
  vector<ConstNeighborhoodIterator<InputImageType> > neighbors(m_NumOutPuts);
  
  unsigned int i=0,ii=0;
  
  for(ii  = 0; ii < m_NumOutPuts; ii++){
    NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType> bC;
  faceLists[ii] = bC(this->GetInput(ii), outputRegionForThread, m_Radius);
  fits[ii] = faceLists[ii].begin();
  }
  
  // support progress methods/callbacks
  ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());

  // All of our neighborhoods have an odd number of pixels, so there is
  // always a median index (if there where an even number of pixels
  // in the neighborhood we have to average the middle two values).
  
  ZeroFluxNeumannBoundaryCondition<InputImageType> nbc;
  OutputPixelType pixels(index.size()*m_NumOutPuts);
  
  // Process each of the boundary faces.  These are N-d regions which border
  // the edge of the buffer.
  
    
  //for ( typename FaceListType::iterator fit=faceLists[0].begin(); fit != faceLists[0].end(); ++fit)
  bool flag = true;
  while(true)
    {
  for(ii  = 0; ii < m_NumOutPuts; ii++){  
    if(fits[ii] == faceLists[ii].end()){
      return;
    }
  }
  
  {
  ImageRegionIterator<OutputImageType> it = ImageRegionIterator<OutputImageType>(output, *(fits[0]));

  for(ii  = 0; ii < m_NumOutPuts; ii++){  
    ConstNeighborhoodIterator<InputImageType> bit =
      ConstNeighborhoodIterator<InputImageType>(m_Radius, this->GetInput(ii), *(fits[ii]));
    bit.OverrideBoundaryCondition(&nbc);
    bit.GoToBegin();
    neighbors[ii] = bit;
  }
  flag = true;
    while ( flag )
      {
      // collect all the pixels in the neighborhood, note that we use
      // GetPixel on the NeighborhoodIterator to honor the boundary conditions

    i = 0;
    for(unsigned int ii  = 0; ii < m_NumOutPuts; ii++){      
      for (std::vector<unsigned int>::iterator in_it = index.begin(); in_it!=index.end(); ++in_it)
      {
        pixels.SetElement(i,neighbors[ii].GetPixel(*in_it));
        i++;
      }
      }
  
    for(unsigned int ii  = 0; ii < m_NumOutPuts; ii++){
      ++neighbors[ii];
      if(neighbors[ii].IsAtEnd())
        flag = false;
    }
      
    it.Set( static_cast<typename OutputImageType::PixelType> (pixels) );
      ++it;
      progress.CompletedPixel();
       
      }
  
  }
    
    for(ii  = 0; ii < m_NumOutPuts; ii++){  
    ++(fits[ii]);  
  }

    }
}


/**
 * Standard "PrintSelf" method
 */
template <class TInputImage>
void
NeighborhoodCollectorImageFilter<TInputImage>
::PrintSelf(
  std::ostream& os, 
  Indent indent) const
{
  Superclass::PrintSelf( os, indent );
  os << indent << "Radius: " << m_Radius << std::endl;

}

} // end namespace itk

#endif
