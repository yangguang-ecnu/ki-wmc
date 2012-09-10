/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkInsideEllipsoidInteriorExteriorSpatialFunction.txx,v $
  Language:  C++
  Date:      $Date: 2009-11-24 02:27:27 $
  Version:   $Revision: 1.28 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkInsideInsideEllipsoidInteriorExteriorSpatialFunction_txx
#define __itkInsideInsideEllipsoidInteriorExteriorSpatialFunction_txx

#include "itkInsideEllipsoidInteriorExteriorSpatialFunction.h"
#include <iostream>
#include <math.h>

using namespace std;
namespace itk 
{

template <unsigned int VDimension, typename TInput>
InsideEllipsoidInteriorExteriorSpatialFunction<VDimension, TInput>
::InsideEllipsoidInteriorExteriorSpatialFunction()
{
  m_Orientations = NULL;
  m_Axes.Fill(1.0f);   // Lengths of ellipsoid axes.
  m_Center.Fill(0.0f); // Origin of ellipsoid
}

template <unsigned int VDimension, typename TInput >
InsideEllipsoidInteriorExteriorSpatialFunction<VDimension, TInput>
::~InsideEllipsoidInteriorExteriorSpatialFunction()
{
  unsigned int i;
  if (m_Orientations)
    {
    for(i = 0; i < VDimension; i++)
      {
      delete []m_Orientations[i];
      }
    delete []m_Orientations;
    }
}

template <unsigned int VDimension, typename TInput>
typename InsideEllipsoidInteriorExteriorSpatialFunction<VDimension, TInput>::OutputType
InsideEllipsoidInteriorExteriorSpatialFunction<VDimension, TInput>
::Evaluate(const InputType& position) const
{  
  double distanceSquared = 0; 

  Vector<double, VDimension> orientationVector;
  Vector<double, VDimension> pointVector;

  // Project the position onto each of the axes, normalize by axis length, 
  // and determine whether position is inside ellipsoid. The length of axis0,
  // m_Axis[0] is orientated in the direction of m_Orientations[0].
  for(unsigned int i = 0; i < VDimension; i++)
    {
    pointVector[i] = position[i] - m_Center[i];
    }
  
  for(unsigned int i = 0; i < VDimension; i++)
    {  
    for(unsigned int j = 0; j < VDimension; j++)
      {
      orientationVector[j] = m_Orientations[i][j];
      }
    distanceSquared += vcl_pow(static_cast<double>((orientationVector * pointVector)/(.5*(m_Axes[i]-0.5))),static_cast<double>(2));
    }
  if(distanceSquared <= 1)
    {
  return 1;
    }
  //Default return value assumes outside the ellipsoid  
  return 0; // Outside the ellipsoid.
}

template <unsigned int VDimension, typename TInput>
void InsideEllipsoidInteriorExteriorSpatialFunction<VDimension, TInput>
::SetOrientations(const OrientationType &orientations)
{
  unsigned int i, j;
  // Initialize orientation vectors.
  if (m_Orientations)
    {
    for(i = 0; i < VDimension; i++)
      {
      delete []m_Orientations[i];
      }
    delete []m_Orientations;
    }
  m_Orientations = new double * [VDimension];
  for(i = 0; i < VDimension; i++)
    {
    m_Orientations[i] = new double[VDimension];
    }

  // Set orientation vectors (must be orthogonal).
  for(i = 0; i < VDimension; i++)
    {
    for(j = 0; j < VDimension; j++)
      {
      m_Orientations[i][j] = orientations[i][j];
      }
    }
}

template <unsigned int VDimension, typename TInput>
void InsideEllipsoidInteriorExteriorSpatialFunction<VDimension, TInput>
::PrintSelf(std::ostream& os, Indent indent) const
{
  unsigned int i, j;

  Superclass::PrintSelf(os, indent);

  os << indent << "Lengths of Ellipsoid Axes: " << m_Axes << std::endl;
  os << indent << "Origin of Ellipsoid: " << m_Center << std::endl;
  if (m_Orientations)
    {
    os << indent << "Orientations: " << std::endl;
    for (i = 0; i < VDimension; i++)
      {
      for (j = 0; j < VDimension; j++)
        {
        os << indent << indent <<  m_Orientations[i][j] << " ";
        }
      os << std::endl;
      }
    }
}

} // end namespace itk

#endif
