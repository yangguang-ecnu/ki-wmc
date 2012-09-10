/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkInsideEllipsoidInteriorExteriorSpatialFunction.h,v $
  Language:  C++
  Date:      $Date: 2009-04-25 12:24:05 $
  Version:   $Revision: 1.19 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkInsideInsideEllipsoidInteriorExteriorSpatialFunction_h
#define __itkInsideInsideEllipsoidInteriorExteriorSpatialFunction_h

#include "itkInteriorExteriorSpatialFunction.h"
#include "vnl/vnl_matrix_fixed.h"

namespace itk
{

/**
 * \class EllipsoidSpatialFunction
 * \brief Function implementation of an ellipsoid
 *
 * Implements a function that returns 1 for points inside or on the
 * surface of a ellipsoid and 0 for points outside the ellipsoid. The
 * orientation of the  n-dimensional ellipsoid axes are defined by n
 * orthogonal vectors. See
 * Examples/InsideEllipsoidInteriorExteriorSpatialFunction/README for an
 * example of creating an Ellipsoid in an image.
 */
template <unsigned int VDimension = 3,
          typename TInput = Point<double, VDimension> >
class ITK_EXPORT InsideEllipsoidInteriorExteriorSpatialFunction
: public InteriorExteriorSpatialFunction<VDimension, TInput>
{
public:
  /** Standard class typedefs. */
  typedef InsideEllipsoidInteriorExteriorSpatialFunction            Self;
  typedef InteriorExteriorSpatialFunction<VDimension, TInput> Superclass;
  typedef SmartPointer<Self>                                  Pointer;
  typedef SmartPointer<const Self>                            ConstPointer; 
      
  /** Run-time type information (and related methods). */
  itkTypeMacro(InsideEllipsoidInteriorExteriorSpatialFunction,InteriorExteriorSpatialFunction);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);
  
  /** Input type for the function */
  typedef typename Superclass::InputType InputType;

  /** Output type for the function */
  typedef typename Superclass::OutputType OutputType;

  /** Typedef for the orientation matrix */
  typedef vnl_matrix_fixed<double, VDimension, VDimension> OrientationType;
   
  /** Set/Get and set the center of the ellipsoid. */
  itkGetConstMacro(Center, InputType);
  itkSetMacro(Center, InputType);
  
  /** Get and set the axes lengths of the ellipsoid. */
  itkGetConstMacro(Axes, InputType);
  itkSetMacro(Axes, InputType);
  
  /** Set the orientation vectors (must be orthogonal) of the ellipsoid axes.
   * Must be normalized!!!!! */
  void SetOrientations(const OrientationType &);

  /** Evaluates the function at a given position. */
  OutputType Evaluate(const InputType& position) const;
     
protected:
  InsideEllipsoidInteriorExteriorSpatialFunction();
  virtual ~InsideEllipsoidInteriorExteriorSpatialFunction();

  void PrintSelf(std::ostream& os, Indent indent) const;

private:
  InsideEllipsoidInteriorExteriorSpatialFunction(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  /** The center of the ellipsoid. */
  InputType m_Center;

  /** The axes lenths of the ellipsoid. */
  InputType m_Axes;

  /** The orientation vectors (must be orthogonal) of the ellipsoid axes. */  
  double ** m_Orientations;
};

} // end namespace itk

// Define instantiation macro for this template.
#define ITK_TEMPLATE_InsideEllipsoidInteriorExteriorSpatialFunction(_, EXPORT, x, y) namespace itk { \
  _(2(class EXPORT InsideEllipsoidInteriorExteriorSpatialFunction< ITK_TEMPLATE_2 x >)) \
  namespace Templates { typedef InsideEllipsoidInteriorExteriorSpatialFunction< ITK_TEMPLATE_2 x > \
                                                  InsideEllipsoidInteriorExteriorSpatialFunction##y; } \
  }

#if ITK_TEMPLATE_EXPLICIT
# include "Templates/itkInsideEllipsoidInteriorExteriorSpatialFunction+-.h"
#endif

#if ITK_TEMPLATE_TXX
# include "itkInsideEllipsoidInteriorExteriorSpatialFunction.txx"
#endif

#endif
