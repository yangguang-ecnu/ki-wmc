/*=========================================================================

 Program:   Insight Segmentation & Registration Toolkit
 Module:    $RCSfile: itkImageNeighborLinearAccess.h,v $
 Language:  C++
 Date:      $Date: 2009-08-11 12:41:16 $
 Version:   $Revision: 1.3 $

 Copyright (c) Insight Software Consortium. All rights reserved.
 See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/
#ifndef __itkImageNeighborLinearAccess_h
#define __itkImageNeighborLinearAccess_h

#include "itkImage.h"
#include "itkShapedNeighborhoodIterator.h"
#include "itkInsideBinaryBallStructuringElement.h"

#include <vector>

namespace itk
{
  template<class TInputImage, class TMaskImage>
    class ImageNeighborLinearAccess
    {
    public:
      typedef ImageNeighborLinearAccess Self;itkTypeMacro(ImageNeighborLinearAccess, None)
      ;

      typedef TInputImage InputImageType;
      typedef typename InputImageType::PixelType InputImagePixelType;
      typedef typename InputImageType::Pointer InputImagePointerType;
      typedef typename InputImageType::RegionType InputImageRegionType;

      typedef itk::ShapedNeighborhoodIterator<InputImageType> IteratorType;

      void
      addImage(InputImagePointerType inputImage)
      {
        this->imgs.push_back(inputImage);
      }

      void
      makeReady(unsigned int rad)
      {
        typedef itk::InsideBinaryBallStructuringElement<char, 3> StructuringElementType;
        typename StructuringElementType::RadiusType elementRadius;
        elementRadius.Fill(rad);

        StructuringElementType structuringElement;
        structuringElement.SetRadius(elementRadius);
        structuringElement.CreateStructuringElement();
        this->radius.Fill(rad);
        numberOfInputs = this->imgs.size();
        it.reserve(numberOfInputs);
        it1.reserve(numberOfInputs);
        InputImageRegionType region;
        for (unsigned int i = 0; i < numberOfInputs; i++)
          {
            InputImagePointerType input = this->imgs[i];
            if (i == 0)
              {
                region = input->GetLargestPossibleRegion();
              }
            else if (input->GetLargestPossibleRegion() != region)
              {
                itkExceptionMacro(
                    << "All Inputs must have the same dimensions.");
              }
            it[i] = new IteratorType(this->radius, input,
                input->GetLargestPossibleRegion());
            it1[i] = new IteratorType(this->radius, input,
                input->GetLargestPossibleRegion());

              {
                typename StructuringElementType::ConstIterator nit;
                typename StructuringElementType::SizeValueType idx = 0;
                this->l = 0;
                for (nit = structuringElement.Begin();
                    nit != structuringElement.End(); ++nit, ++idx)
                  {
                    if (*nit)
                      {
                        it[i]->ActivateOffset(it[i]->GetOffset(idx));
                        it1[i]->ActivateOffset(it1[i]->GetOffset(idx));
                        this->l++;
                      }
                    else
                      {
                        it[i]->DeactivateOffset(it[i]->GetOffset(idx));
                        it1[i]->DeactivateOffset(it1[i]->GetOffset(idx));
                      }
                  }
              }

          }
        imgSize = region.GetSize();
        this->image_pix = 1;
        for (unsigned int i = 0; i < InputImageType::ImageDimension; i++)
          {
            this->image_pix *= imgSize[i];
          }
        this->total_pix = this->numberOfInputs * this->image_pix;

        std::ostringstream logMessage;
        logMessage << "Accessor is ready (" << this->numberOfInputs
            << " images) " << this->total_pix << " " << this->l << " DIM samples.\n";
        itk::OutputWindowDisplayText(logMessage.str().c_str());
      }

      void
      calc_index(size_t p_index, typename InputImageType::IndexType& index)
      {
        size_t mul = 1;
        for (unsigned int i = 0; i < InputImageType::ImageDimension; i++)
          {
            mul = p_index / imgSize[i];
            index[i] = p_index - mul * imgSize[i];
            p_index = mul;
          }
      }

      const double
      dot(size_t i1, size_t i2)
      {
        double d = 0;

        img_index = i1 / this->image_pix;
        calc_index(i1 - img_index * this->image_pix, indx);
        IteratorType* tit = this->it[img_index];
        tit->SetLocation(indx);

        img_index1 = i2 / this->image_pix;
        calc_index(i2 - img_index1 * this->image_pix, indx1);
        IteratorType* tit1 = this->it1[img_index1];
        tit1->SetLocation(indx1);

        for (unsigned int i = 0; i < this->l; ++i)
          {
            // collect all the pixels in the neighborhood, note that we use
            // GetPixel on the NeighborhoodIterator to honor the boundary conditions
            const InputImagePixelType& p = tit->GetPixel(i);
            const InputImagePixelType& p1 = tit1->GetPixel(i);
            for (unsigned int j = 0; j < InputImageType::ImageDimension; ++j)
              {
                const double pv = double(p[j]);
                const double pv1 = double(p1[j]);
                d += pv * pv1;
              }
          }
        return d;
      }

      const double
      norm2(size_t index)
      {
        double n = 0;
        img_index = index / this->image_pix;
        calc_index(index - img_index * this->image_pix, indx);
        IteratorType* tit = this->it[img_index];
        tit->SetLocation(indx);
        for (unsigned int i = 0; i < this->l; ++i)
          {
            // collect all the pixels in the neighborhood, note that we use
            // GetPixel on the NeighborhoodIterator to honor the boundary conditions
            const InputImagePixelType& p = tit->GetPixel(i);
            for (unsigned int j = 0; j < InputImageType::ImageDimension; ++j)
              {
                const double pv = double(p[j]);
                n += pv * pv;
              }
          }
        return n;
      }

      const std::vector<double>
      get(size_t index)
      {
        std::vector<double> pixels;
        img_index = index / this->image_pix;
        calc_index(index - img_index * this->image_pix, indx);
        IteratorType* tit = this->it[img_index];
        tit->SetLocation(indx);
        pixels.resize(this->l * InputImageType::ImageDimension);
        for (unsigned int i = 0; i < this->l; ++i)
          {
            // collect all the pixels in the neighborhood, note that we use
            // GetPixel on the NeighborhoodIterator to honor the boundary conditions
            const InputImagePixelType& p = tit->GetPixel(i);
            for (unsigned int j = 0; j < InputImageType::ImageDimension; ++j)
              {
                const double pv = double(p[j]);
                pixels[i * InputImageType::ImageDimension + j] = pv;
              }
          }

        return pixels;
      }

      const void
      printLoc(size_t index)
      {
        img_index = index / this->image_pix;
        calc_index(index - img_index * this->image_pix, indx);
        std::cout << "LinearIndex: " << index << std::endl;
        std::cout << "ImageIndex: " << img_index << std::endl;
        std::cout << InputImageType::ImageDimension << "DIndex: " << indx
            << std::endl;
      }

      ImageNeighborLinearAccess()
      {
        this->image_pix = 0;
        this->total_pix = 0;
        this->numberOfInputs = 0;
        this->img_index = 0;
        this->img_index1 = 0;
        this->l = 0;
      }
      virtual
      ~ImageNeighborLinearAccess()
      {
        for (size_t i = 0; i < this->numberOfInputs; i++)
          {
            if (it[i])
              {
                delete it[i];
              }
            if (it1[i])
              {
                delete it1[i];
              }
          }
      }
      size_t
      getTotal_pix()
      {
        return this->total_pix;
      }
      size_t
      getDim()
      {
        return this->l;
      }
      size_t
      getImage_pix()
      {
        return this->image_pix;
      }
      size_t
      getnumberOfInputs()
      {
        return this->numberOfInputs;
      }
    private:
      std::vector<InputImagePointerType> imgs;
      size_t l;
      size_t image_pix;
      size_t total_pix;
      size_t numberOfInputs;
      size_t img_index;
      size_t img_index1;
      typename InputImageType::IndexType indx, indx1;
      itk::Size<InputImageType::ImageDimension> radius;
      itk::Size<InputImageType::ImageDimension> imgSize;
      std::vector<IteratorType *> it;
      std::vector<IteratorType *> it1;
    };

} // end of namespace itk 

#ifndef ITK_MANUAL_INSTANTIATION
//#include "itkImageCutoffCalc.txx"
#endif

#endif
