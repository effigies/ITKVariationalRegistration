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
#ifndef __itkVariationalRegistrationGaussianRegularizer_txx
#define __itkVariationalRegistrationGaussianRegularizer_txx
#include "itkVariationalRegistrationGaussianRegularizer.h"

#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIteratorWithIndex.h"

#include "itkGaussianOperator.h"
#include "itkVectorNeighborhoodOperatorImageFilter.h"

namespace itk
{

/**
 * Default constructor
 */
template< class TDisplacementField >
VariationalRegistrationGaussianRegularizer< TDisplacementField >
::VariationalRegistrationGaussianRegularizer()
{
  for( unsigned int j = 0; j < ImageDimension; j++ )
    {
    m_StandardDeviations[j] = 1.0;
    }

  m_MaximumError = 0.1;
  m_MaximumKernelWidth = 30;
}

/**
 * Set the standard deviations.
 */
template< class TDisplacementField >
void
VariationalRegistrationGaussianRegularizer< TDisplacementField >
::SetStandardDeviations( double value )
{
  unsigned int j;
  for( j = 0; j < ImageDimension; j++ )
    {
    if( value != m_StandardDeviations[j] )
      {
      break;
      }
    }
  if( j < ImageDimension )
    {
    this->Modified();
    for( j = 0; j < ImageDimension; j++ )
      {
      m_StandardDeviations[j] = value;
      }
    }
}

/**
 * TODO comment
 */
template< class TDisplacementField >
void
VariationalRegistrationGaussianRegularizer< TDisplacementField >
::GenerateData()
{
  // Allocate the output image
  this->AllocateOutputs();

  // Initialize and allocate data
  this->Initialize();

  DisplacementFieldConstPointer field = this->GetInput();

  typedef typename DisplacementFieldType::PixelType VectorType;
  typedef typename VectorType::ValueType ScalarType;
  typedef GaussianOperator< ScalarType, ImageDimension > OperatorType;
  typedef VectorNeighborhoodOperatorImageFilter<
      DisplacementFieldType,
      DisplacementFieldType > SmootherType;

  OperatorType opers[ImageDimension];
  typename SmootherType::Pointer smoothers[ImageDimension];

  for( unsigned int j = 0; j < ImageDimension; j++ )
    {
    // smooth along this dimension
    opers[j].SetDirection( j );
    double variance = vnl_math_sqr( this->GetStandardDeviations()[j] );
    opers[j].SetVariance( variance );
    opers[j].SetMaximumError( this->GetMaximumError() );
    opers[j].SetMaximumKernelWidth( this->GetMaximumKernelWidth() );
    opers[j].CreateDirectional();

    smoothers[j] = SmootherType::New();
    smoothers[j]->SetOperator( opers[j] );
    smoothers[j]->ReleaseDataFlagOn();

    if( j > 0 )
      {
      smoothers[j]->SetInput( smoothers[j - 1]->GetOutput() );
      }
    }
  smoothers[0]->SetInput( field );
  smoothers[ImageDimension - 1]->GetOutput()->SetRequestedRegion( field->GetBufferedRegion() );
  smoothers[ImageDimension - 1]->Update();

  this->GraftOutput( smoothers[ImageDimension - 1]->GetOutput() );
}

/*
 * Initialize flags
 */
template< class TDisplacementField >
void
VariationalRegistrationGaussianRegularizer< TDisplacementField >
::Initialize()
{
  // TODO warning use image spacing
  this->Superclass::Initialize();
}

/*
 * Print status information
 */
template< class TDisplacementField >
void
VariationalRegistrationGaussianRegularizer< TDisplacementField >
::PrintSelf( std::ostream& os, Indent indent ) const
{
  Superclass::PrintSelf( os, indent );

  os << indent << "Standard deviations: [" << m_StandardDeviations[0];
  for( unsigned int j = 1; j < ImageDimension; j++ )
    {
    os << ", " << m_StandardDeviations[j];
    }
  os << "]" << std::endl;

  os << indent << "MaximumError: ";
  os << m_MaximumError << std::endl;
  os << indent << "MaximumKernelWidth: ";
  os << m_MaximumKernelWidth << std::endl;
}

} // end namespace itk

#endif