/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, version 1.0 beta 4      *
*                (c) 2006-2009 MGH, INRIA, USTL, UJF, CNRS                    *
*                                                                             *
* This library is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This library is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this library; if not, write to the Free Software Foundation,     *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.          *
*******************************************************************************
*                               SOFA :: Modules                               *
*                                                                             *
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#ifndef SOFA_GPU_CUDA_CUDATYPES_BASE_H
#define SOFA_GPU_CUDA_CUDATYPES_BASE_H

#include "CudaTypes.h"
#include <sofa/component/linearsolver/FullMatrix.h>

//#define DEBUG_BASE

extern "C"
{
    void matrix_vector_productf(int dim,const void * M,int mPitch,const void * r,void * z);
    void matrix_vector_productd(int dim,const void * M,int mPitch,const void * r,void * z);
}

template<typename real> class CudaBaseMatrixKernels;

template<> class CudaBaseMatrixKernels<float>
{
public:
    static void matrix_vector_product(int dim,const void * M,int mPitch,const void * r,void * z)
    {   matrix_vector_productf(dim,M,mPitch,r,z); }
};

#ifdef SOFA_GPU_CUDA_DOUBLE
template<> class CudaBaseMatrixKernels<double>
{
public:
    static void matrix_vector_product(int dim,const void * M,int mPitch,const void * r,void * z)
    {   matrix_vector_productd(dim,M,mPitch,r,z); }
};
#endif


namespace sofa
{
namespace gpu
{
namespace cuda
{

using namespace sofa::defaulttype;

template <class T>
class CudaBaseVector : public BaseVector
{
public :
    typedef T Real;

    CudaVector<T>& getCudaVector()
    {
        return v;
    }

    T& operator[](int i)
    {
        return v[i];
    }

    const T& operator[](int i) const
    {
        return v[i];
    }

    void resize(int nbRow)
    {
        v.resize(nbRow);
    }

    void resize(int nbRow,int warp_size)
    {
        v.resize(nbRow,warp_size);
    }

    unsigned int size() const
    {
        return v.size();
    }

    SReal element(int i) const
    {
        return v[i];
    }

    void clear()
    {
        //for (unsigned int i=0; i<size(); i++) v[i]=(T)(0.0);
        v.memsetHost();
    }

    void set(int i, SReal val)
    {
        v[i] = (T) val;
    }

    void add(int i, SReal val)
    {
        v[i] += (T)val;
    }

    static const char* Name(); /* {
			return "CudaBaseVector";
            }*/

private :
    CudaVector<T> v;
};

typedef CudaBaseVector<float> CudaBaseVectorf;
typedef CudaBaseVector<double> CudaBaseVectord;

template<> inline const char* CudaBaseVectorf::Name() { return "CudaBaseVectorf"; }
template<> inline const char* CudaBaseVectord::Name() { return "CudaBaseVectord"; }

template <class T>
class CudaBaseMatrix : public BaseMatrix
{
public :
    typedef T Real;

    CudaMatrix<T> & getCudaMatrix()
    {
        return m;
    }

    void resize(int nbRow, int nbCol)
    {
        m.resize(nbCol,nbRow,BSIZE);
    }

    void resize(int nbRow, int nbCol,int ws)
    {
        m.resize(nbCol,nbRow,ws);
    }

    unsigned int rowSize() const
    {
        return m.getSizeY();
    }

    unsigned int colSize() const
    {
        return m.getSizeX();
    }

    SReal element(int i, int j) const
    {
        return m[i][j];
    }

    const T* operator[] ( int i ) const
    {
        return m[i];
    }

    void clear()
    {
// 			for (unsigned j=0; j<m.getSizeX(); j++) {
// 				for (unsigned i=0; i<m.getSizeY(); i++) {
// 				  m[j][i] = (T)(0.0);
// 				}
// 			}
        m.memsetHost();
    }

    void set(int i, int j, double v)
    {
#ifdef DEBUG_BASE
        if ((j>=rowSize()) || (i>=colSize()))
        {
            printf("forbidden acces %d %d\n",j,i);
            exit(1);
        }
#endif
        m[i][j] = (T)v;
    }

    void add(int i, int j, double v)
    {
#ifdef DEBUG_BASE
        if ((j>=rowSize()) || (i>=colSize()))
        {
            printf("forbidden acces %d %d\n",j,i);
            exit(1);
        }
#endif
        m[i][j] += (T)v;
    }

    static const char* Name();

    CudaBaseVector<Real> operator*(const CudaBaseVector<Real> & v) const
    {
        CudaBaseVector<Real> res(rowSize());
        CudaBaseMatrixKernels<Real>::matrix_vector_product(rowSize(),
                m.deviceRead(),
                m.getPitch(),
                v.getCudaVector().deviceRead(),
                res.getCudaVector().deviceWrite());
        return res;
    }

    void mult(CudaBaseVector<Real>& v,CudaBaseVector<Real> & r)
    {
        CudaBaseMatrixKernels<Real>::matrix_vector_product(rowSize(),
                m.deviceRead(),
                m.getPitch(),
                r.getCudaVector().deviceRead(),
                v.getCudaVector().deviceWrite());
    }


private :
    CudaMatrix<T> m;
};

typedef CudaBaseMatrix<float> CudaBaseMatrixf;
typedef CudaBaseMatrix<double> CudaBaseMatrixd;

template<> inline const char* CudaBaseMatrixf::Name() { return "CudaBaseMatrixf"; }
template<> inline const char* CudaBaseMatrixd::Name() { return "CudaBaseMatrixd"; }

} // namespace cuda
} // namespace gpu
} // namespace sofa

#endif
