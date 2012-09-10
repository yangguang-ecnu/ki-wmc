/*=========================================================================

 Program:   Insight Segmentation & Registration Toolkit
 Module:    $RCSfile: itkRobustOneScale.txx,v $
 Language:  C++
 Date:      $Date: 2008-10-16 19:33:41 $
 Version:   $Revision: 1.20 $

 Copyright (c) Insight Software Consortium. All rights reserved.
 See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/
#ifndef __itkRobustOneScale_txx
#define __itkRobustOneScale_txx

#include "itkRobustOneScale.h"
#include "itkCastImageFilter.h"

#include "itkImageCutoffCalc.h"

namespace itk {

template<class TInputImageType, class TOutputImageType>
RobustOneScale<TInputImageType, TOutputImageType>::RobustOneScale() {
  medianFilter = MedianFilterType::New();
  scaleFilter = ScaleFilterType::New();
  this->m_Radius.Fill(2);
  this->TriggerOn();
}

template<class TInputImageType, class TOutputImageType>
void RobustOneScale<TInputImageType, TOutputImageType>::GenerateData() {

  if (!this->m_Trigger) {
    typedef CastImageFilter<TInputImageType, TOutputImageType>
        CastFilterType;
    typename CastFilterType::Pointer castFilter = CastFilterType::New();
    castFilter->SetInput(this->GetInput());
    castFilter->Update();
    this->GraftOutput(castFilter->GetOutput());
    return;
  }

  const float co = 0.99;
  OutPixelType max = Statistics::RobustMax<TInputImageType>(this->GetInput(),
      co);
  typename ScaleFilterType::RealType scale = NumericTraits<OutPixelType>::One / max;

  std::cout << "Max "<<co<<" is: " << max << " scale is: " << scale << std::endl;
  scaleFilter->SetInput(this->GetInput());
  scaleFilter->SetScale(scale);
  scaleFilter->SetShift(NumericTraits<OutPixelType>::Zero);
  scaleFilter->Update();

  std::cout << "Done scaling..\n";
  this->GraftOutput(scaleFilter->GetOutput());
}

/**
 * Standard "PrintSelf" method
 */
template<class TInputImageType, class TOutputImageType>
void RobustOneScale<TInputImageType, TOutputImageType>::PrintSelf(
    std::ostream& os, Indent indent) const {
  Superclass::PrintSelf(os, indent);
  os << indent << "Radius: " << this->m_Radius << std::endl;

}

} // end namespace itk

#endif
