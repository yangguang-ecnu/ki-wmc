/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkImageCutoffCalc.h,v $
  Language:  C++
  Date:      $Date: 2009-08-11 12:41:16 $
  Version:   $Revision: 1.3 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkImageCutoffCalc_h
#define __itkImageCutoffCalc_h

namespace itk {
namespace Statistics {

template<class TImage>
typename TImage::PixelType RobustMax(const TImage*image, double CutOff);

} // end of namespace Statistics 
} // end of namespace itk 

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkImageCutoffCalc.txx"
#endif

#endif
