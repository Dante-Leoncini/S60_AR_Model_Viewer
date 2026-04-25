////////////////////////////////////////////////////////////////////////
//
// Geometry3D.h
//
////////////////////////////////////////////////////////////////////////

#ifndef _GEOMETRY3D_H
#define _GEOMETRY3D_H

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

#include <e32math.h>
#include "define.h"
#include "symbian_mathutil.h"


typedef TReal32 TFloat;

#define MAX_COLOR 255

////////////////////////////////////////////////////////////////////////

class TMathLookup;

////////////////////////////////////////////////////////////////////////

class Geometry3D
	{
	public:

		static TInt Reduce(TInt &aSubject, TInt aLimit);
		static TInt Reduce(TUint &aSubject, TInt aLimit);
		static TInt Reduce2(TUint &aSubject, TUint aLimit);
		static void Adjust(TInt& aVal, TInt shift0, TInt shift1, TInt& newShift);



	public:

		enum
		{
			leftBound	= 0,
			topBound	,
			rightBound	,
			bottomBound	,
			numBounds	,
			//
			leftOutcode		= ( 1 << leftBound		),
			topOutcode		= ( 1 << topBound		),
			rightOutcode 	= ( 1 << rightBound		),
			bottomOutcode	= ( 1 << bottomBound	),
			//
			screenCoordUnityLog		= 4,
			screenCoordUnity		= ( 1 << screenCoordUnityLog ),
			screenCoordBias			= ( 1 << ( screenCoordUnityLog - 1 ) ),
			screenCoordFractionMask	= ( screenCoordUnity - 1 ),
			//
		};
	};

////////////////////////////////////////////////////////////////////////

class TVector3
	{
	public:
		TVector3();
		TVector3(TInt aX, TInt aY, TInt aZ);
		
		void MakeSum(const TVector3 &aVectorA, const TVector3 &aVectorB);
		void MakeDifference(const TVector3 &aVectorA, const TVector3 &aVectorB);
		void MakeInverse(const TVector3 &aVector);
		void MakeCrossProduct(const TVector3 &aVectorA, const TVector3 &aVectorB);
		void ScaleFrom(const TVector3 &aVector, TInt aScale);
		void ScaleFrom(const TVector3 &aVector, TInt aScale, TInt aDownshift);
		TInt GetLen(TMathLookup &aMath);
		TInt GetMaxAbs();
		void RotateXZ(TVector3& aDir, TMathLookup& aMath);

		// Warning - slow! Use only for initialisation!
		void NormaliseFrom(const TVector3 &aVector, TInt aUnity);

		void FastNormaliseFrom(TMathLookup& aMath, const TVector3 &aVector);

		// Like normalisation, but doesn't guarantee any particular magnitude
		// for the result: instead it cuts large vectors down to a reasonable size
		// (always less than or equal to aUnity) and discards trailing zeroes.
		// Much faster than NormaliseFrom()!
		void ReduceFrom(const TVector3 &aVector, TInt aUnity);
		void ReduceSize(const TVector3 &aVector, TInt aUnity); // remove it ?
		void ReduceSize(TInt aUnity);
		TInt UpSize(TInt aUnity);
		void Downshift(const TInt aDownshift);
		void Upshift(const TInt aUpshift);

		static TInt DotProduct(const TVector3 &aVectorA, const TVector3 &aVectorB);

		void NormZ(TInt Unity )
		{
			iX = (iX*Unity)/iZ;
			iY = (iY*Unity)/iZ;
			iZ = Unity;
		};

		inline void cross(const TVector3 &A, const TVector3 &B)
		{
			iX = A.iY*B.iZ - A.iZ*B.iY;
			iY = A.iZ*B.iX - A.iX*B.iZ;
			iZ = A.iX*B.iY - A.iY*B.iX;
		}

		inline TInt abs()
		{
			return Abs(iX) + Abs(iY) + Abs(iZ);
		};

		inline TInt absXZ()
		{
			return Abs(iX) + Abs(iZ);
		};

		TPoint ToPoint()
		{
			return TPoint(iX, iZ);		
		};

		TVector3(const TPoint& aP)
		{
			iX = aP.iX;
			iY = 0;
			iZ = aP.iY;
		};

		void Zero()
		{
			iX = 0;
			iY = 0;
			iZ = 0;
		};

	public:
		TInt iX;
		TInt iY;
		TInt iZ;
	};


////////////////////////////////////////////////////////////////////////
class TMatrix3x3
	{
	public:

		TMatrix3x3();

		void MakeIdentity();

		void MakeRx(TInt aAngle, TMathLookup &aMath);
		void MakeRy(TInt aAngle, TMathLookup &aMath);
		void MakeRz(TInt aAngle, TMathLookup &aMath);

		void MakeCompound(const TMatrix3x3 &aMatrixA, const TMatrix3x3 &aMatrixB);

		void MakeInverse(const TMatrix3x3 &aSourceMatrix);


		void Transform(const TVector3 &aSourceVector, TVector3 &aDestVector) const;
		void InverseTransform(const TVector3 &aSourceVector, TVector3 &aDestVector) const;

	private:
		
		enum
		{
			matrixUnityLog	= UNITY_LOG,
			matrixUnity		= ( 1 << matrixUnityLog ),
		};
		TInt iElement[ 3 ][ 3 ];
	};

////////////////////////////////////////////////////////////////////////

class TMatrixGL
	{
	public:

		TMatrixGL();

		void MakeIdentity();

		void MakeCompound(const TMatrixGL &aMatrixA, const TMatrixGL &aMatrixB);
		TVector3 TransformVertix(const TVector3& aV);
		TVector3 TransformNormal(const TVector3& aV);

		TInt iElement[ 16 ];
	private:
		
		enum
		{
			matrixUnityLog	= GLUNITY_LOG,
			matrixUnity		= ( 1 << matrixUnityLog ),
		};
// GL convention - columns
	};

inline TInt GetDelta(const TMatrixGL &aMatrixA, const TMatrixGL &aMatrixB)
{
	TInt aMax = 0;
	TInt count;
	for(count =0; count < 16; count++)
	{
		if(count < 12)
			aMax = Max(aMax, 2*Abs(aMatrixA.iElement[count] - aMatrixB.iElement[count]));
		else
			aMax = Max(aMax, 2*Abs(aMatrixA.iElement[count] - aMatrixB.iElement[count]));
	}

	return aMax;

}
////////////////////////////////////////////////////////////////////////
class TVectorR;

inline TFloat FAbs(TFloat aR)
{
	return (aR >= 0)?aR:-aR;
};

inline TFloat FMax(TFloat aR0, TFloat aR1)
{
	return (aR0 >= aR1)?aR0:aR1;
};

class TMatrixR
	{
	public:

		TMatrixR();

		void MakeIdentity();
		void MakeCompound(const TMatrixR &aMatrixA, const TMatrixR &aMatrixB);

		void SetIntGL(TMatrixGL& aMatrix);

		TVectorR TransformVertix(const TVector3& aV);
		TVectorR TransformNormal(const TVector3& aV);

		TFloat iElement[ 16 ];
	private:
		

// GL convention - columns
	};

inline TFloat GetDelta(const TMatrixR &aMatrixA, const TMatrixR &aMatrixB)
{
	TFloat aMax = 0;
	TInt count;
	for(count =0; count < 16; count++)
	{
		if(count < 4*3)
			aMax = FMax(aMax, 2*FAbs(aMatrixA.iElement[count] - aMatrixB.iElement[count]));
		else
			aMax = FMax(aMax, FAbs(aMatrixA.iElement[count] - aMatrixB.iElement[count]));
	}

	return aMax;

}
////////////////////////////////////////////////////////////////////////


class TMatrix2x2
	{
	public:

		void MakeIdentity();
		void Transform(const TVector3 &aSourceVector, TVector3 &aDestVector) const;
		void MakeRz(TInt aAngle, TMathLookup &aMath);
		void MakeCompound(const TMatrix2x2 &aMatrixA, const TMatrix2x2 &aMatrixB);

//	private:
		
		enum
		{
			matrixUnityLog	= UNITY_LOG,
			matrixUnity		= ( 1 << matrixUnityLog ),
		};
		TInt iElement[ 2 ][ 2 ];
	};

////////////////////////////////////////////////////////////////////////


class TAffineTransform
	{
	public:

		TAffineTransform(){}
		TAffineTransform(const TVector3 &aVector);
		TAffineTransform(const TMatrix3x3 &aMatrix);

		void SetVector(const TVector3 &aVector);
		void SetMatrix(const TMatrix3x3 &aMatrix);

		void MakeCompound(const TAffineTransform &aAffineA, const TAffineTransform &aAffineB);
		void Transform(const TVector3 &aSourceVector, TVector3 &aDestVector) const;
		void GetInverseVector(TVector3 &aDestVector) const;

	protected:
		TMatrix3x3	iMatrix;
		TVector3	iVector;
	};

////////////////////////////////////////////////////////////////////////

class TEdgeBuffer
{
	public:
	TInt iStart, iEnd;
	TInt iEdge[MAX_SCAN_LINE];
	TInt iColor[MAX_SCAN_LINE][4];
	void ScanEdge(TMathLookup& aMath, const TVector3 &vA, const TVector3 &vB);
	void ScanEdgeColor(TMathLookup& aMath, const TVector3 &vA, const TVector3 &vB, TInt* aColorA, TInt* aColorB);
	void Reset();
};


class TScanConverter
{
public:
	TEdgeBuffer iLeftBuffer;
	TEdgeBuffer iRightBuffer;
	TInt iX0, iX1, iY0, iY1; 

	void SetClip(TInt aX0, TInt aX1, TInt aY0, TInt aY1);
	TBool ClipEdge(TMathLookup& aMath, TVector3& vA, TVector3& vB, TVector3& vV0, TVector3& vV1, TVector3& vV2, TBool& bScan);
	void Fill(TMathLookup& aMath, TUint16* aData, TInt aPitch, TUint16 aColor);
	void Fill(TMathLookup& aMath, TUint16* aData, TInt aPitch);
	void AlphaFill(TUint16* aData, TInt aPitch, TUint16 aColor, TInt alpha);
	void SetBuffers(TMathLookup& aMath, TVector3* aVertex, TInt aNumbVert, TPoint aOffPoint);
	void SetBuffers(TMathLookup& aMath, TVector3* aVertex, TInt aNumbVert, TInt* Colors, TPoint aOffPoint);
};

////////////////////////////////////////////////////////////////////////
class TObject;

class TIFace
	{
	public:
		TIFace(){ iNumVerts = 0;};
		TIFace(TInt* aInd, TInt aNum)
		{
			iNumVerts = aNum;
			memcpy(iVertIndex, aInd, aNum*sizeof(TInt));
		};

		TIFace(TInt aI0, TInt aI1, TInt aI2)
		{
			iNumVerts = 3;
			iVertIndex[0] = aI0;
			iVertIndex[1] = aI1;
			iVertIndex[2] = aI2;
		};

	void Render(TUint16* aData, 
			TScanConverter &aScanConverter, 
			TMathLookup &aMath,
			TInt aPitch, TPoint& aOff, TObject* aObj);

	void RenderFaceColor(TUint16* aData, 
			TScanConverter &aScanConverter, 
			TMathLookup &aMath,
			TInt aPitch, TPoint& aOff, TObject* aObj);

	void RenderAlphaFaceColor(TUint16* aData, 
			TScanConverter &aScanConverter, 
			TMathLookup &aMath,
			TInt aPitch, TPoint& aOff, TObject* aObj, TInt alpha);


	void RenderFaceColorSmooth(TUint16* aData, 
			TScanConverter &aScanConverter, 
			TMathLookup &aMath,
			TInt aPitch, TPoint& aOff, TObject* aObj);

	void GenerateNormal(TVector3* aVert);

    TInt		iFaceColor[3];
	TVector3	iNorm;
	TVector3	iNormBase;

	TInt		iNumVerts;
	TInt		iVertIndex[MAX_VERTS];

	};

class TObject
{
public:
	TObject(){iNumFaces = 0;};

	void AddFace(TInt aI0, TInt aI1, TInt aI2, TInt aI3, TInt aNumb);

	void Render(TUint16* aData, 
			TScanConverter &aScanConverter, 
			TMathLookup &aMath,
			TInt aPitch, TPoint& aOff);

	void RenderAlpha(TUint16* aData, 
			TScanConverter &aScanConverter, 
			TMathLookup &aMath,
			TInt aPitch, TPoint& aOff);

	void SetLight(TVector3& aLight);
	void SetColor(TInt* aColor);
	void ResetColor();
	void SetFlatColor(TInt* aColor0, TInt* aColor1, TInt aMult);
	void SetFlatColor(TInt* aColor, TInt aMult);
	void SetBaseColor(TInt* aColor);
	void SetBaseFlatColor();
	void GenerateNormals();

	TBool iFaceColorValid;
	TInt iNumFaces;
	TInt iNumVerts;
	TInt iDot[MAX_FACES];
	TIFace iFace[MAX_FACES];
	TVector3 iVert[3*MAX_FACES];
	TVector3 iNorm[3*MAX_FACES];
	TVector3 iVertBase[3*MAX_FACES];
	TVector3 iNormBase[3*MAX_FACES];
	TInt   iColor[3*3*MAX_FACES];
	TInt   iLightColor[3][3*MAX_FACES];
	TVector3 iLight;
};

inline TInt mult(TInt aA, TInt aB, TInt h_unity)
{
	return (aA>>h_unity)*(aB>>h_unity);
}

class TVector64
{
public:
	TInt64 iX;
	TInt64 iY;
	TInt64 iZ;
	TVector64(){};
	TVector64(TInt64 aX, TInt64 aY, TInt64 aZ)
	{
		iX =  aX; iY = aY; iZ= aZ;
	};
	TVector64(TInt aX, TInt aY, TInt aZ)
	{
		iX =  (TInt64)aX; iY = (TInt64)aY; iZ= (TInt64)aZ;
	};
	TVector64(const TVector3& aV)
	{
		iX =  (TInt64)aV.iX; iY = (TInt64)aV.iY; iZ= (TInt64)aV.iZ;
	};

	void Attach(const TVector3& aV)
	{
		iX =  (TInt64)aV.iX; iY = (TInt64)aV.iY; iZ= (TInt64)aV.iZ;
	};


	void ReduceSize(const TVector64 &aVector, TInt64 aUnity)
	{
		*this = aVector;

		TInt64 absX = iX > 0 ? iX : -iX;
		TInt64 absY = iY > 0 ? iY : -iY;
		TInt64 absZ = iZ > 0 ? iZ : -iZ;

		// Get "Manhattan magnitude" of vector:
		TInt64 sumAbs = absX + absY + absZ;

		// Is vector too large?
		// Do the components have trailing zeroes we can discard?
		while ( sumAbs > aUnity )
			{
			iX >>= 1;
			iY >>= 1;
			iZ >>= 1;

			sumAbs >>= 1;
			}
	};

	
	void Downshift(const TInt aDownshift)
 	{
 	iX >>= aDownshift;
 	iY >>= aDownshift;
 	iZ >>= aDownshift;
 	}



	TVector3 GetTInt()
	{
		return TVector3(I64INT(iX), I64INT(iY), I64INT(iZ));
	};

	void NormZ(TInt64 Unity )
	{
		iX = (iX*Unity)/iZ;
		iY = (iY*Unity)/iZ;
		iZ = Unity;
	};



}; 


class TVectorR
{
public:
	TFloat iX;
	TFloat iY;
	TFloat iZ;
	TVectorR(){};

	TVectorR(TFloat aX, TFloat aY, TFloat aZ)
	{
		iX =  aX; iY = aY; iZ= aZ;
	};

	TVectorR(TInt aX, TInt aY, TInt aZ)
	{
		iX =  (TFloat)aX; iY = (TFloat)aY; iZ= (TFloat)aZ;
	};

	TVectorR(TVector3& aV)
	{
		iX =  (TFloat)aV.iX; iY = (TFloat)aV.iY; iZ= (TFloat)aV.iZ;
	};

	inline void Fill(TVector3& aV)
	{
		iX =  (TFloat)aV.iX; iY = (TFloat)aV.iY; iZ= (TFloat)aV.iZ;
	};

	TVector3 GetTInt()
	{
		return TVector3((TInt)(iX*UNITY), (TInt)(iY*UNITY), (TInt)(iZ*UNITY));
	};

	TVector3 GetTIntGL()
	{
		return TVector3((TInt)(iX*GLUNITY), (TInt)(iY*GLUNITY), (TInt)(iZ*GLUNITY));
	};

	inline void operator+=(const TVectorR &A)
	{
		iX += A.iX;
		iY += A.iY;
		iZ += A.iZ;
	};

	inline void operator-=(const TVectorR &A)
	{
		iX -= A.iX;
		iY -= A.iY;
		iZ -= A.iZ;
	}

	inline void operator*=(const TFloat &A)
	{
		iX *= A;
		iY *= A;
		iZ *= A;
	};

	inline void cross(const TVectorR &A, const TVectorR &B)
	{
		iX = A.iY*B.iZ - A.iZ*B.iY;
		iY = A.iZ*B.iX - A.iX*B.iZ;
		iZ = A.iX*B.iY - A.iY*B.iX;
	};

	inline void Normalize()
	{
		TFloat aAbsV = (TFloat)sqrt(iX*iX + iY*iY + iZ*iZ);
		iX /= aAbsV;
		iY /= aAbsV;
		iZ /= aAbsV;
	};

	inline void NormZ()
	{
		iX /= iZ;
		iY /= iZ;
		iZ = 1.;
	};


	inline void FastScale()
	{
		TFloat aAbsV = Abs(iX) + Abs(iY) + Abs(iZ);
		iX /= aAbsV;
		iY /= aAbsV;
		iZ /= aAbsV;
	}

	inline TFloat GetMaxAbs()
	{
		return Max(Max(Abs(iX), Abs(iY)), Abs(iZ));
	}

	inline void zero()
	{
		iX = 0.;
		iY = 0.;
		iZ = 0.;
	};
};

inline TFloat FastScaleMax(const TVectorR &A)
{
	return Abs(A.iX) + Abs(A.iY) + Abs(A.iZ);
};

inline TVector64 operator+(const TVector64 &A, const TVector64 &B)
{
	return TVector64(A.iX + B.iX, A.iY + B.iY, A.iZ + B.iZ);
}

inline TVector64 operator-(const TVector64 &A, const TVector64 &B)
{
	return TVector64(A.iX - B.iX, A.iY - B.iY, A.iZ - B.iZ);
}

inline TVector64 operator*(const TInt64 &A, const TVector64 &B)
{
	return TVector64(A*B.iX, A*B.iY, A*B.iZ);
}

inline TInt64 operator*(const TVector64 &A, const TVector64 &B)
{
	return A.iX*B.iX + A.iY*B.iY + A.iZ*B.iZ;
}

inline TVector64 operator>>(const TVector64 &A, const TInt shift)
{
	return TVector64(A.iX>>shift, A.iY>>shift, A.iZ>>shift);
}


inline TVector64 operator%(const TVector64 &A, const TVector64 &B)
{
	return TVector64(A.iY*B.iZ - A.iZ*B.iY, A.iZ*B.iX - A.iX*B.iZ, A.iX*B.iY - A.iY*B.iX);
}

inline TVector3 operator^(const TVector3 &A, const TVector3 &B)
{
	return TVector3(A.iY*B.iZ - A.iZ*B.iY, A.iZ*B.iX - A.iX*B.iZ, A.iX*B.iY - A.iY*B.iX);
}

inline TVector3 operator+(const TVector3 &A, const TVector3 &B)
{
	return TVector3(A.iX + B.iX, A.iY + B.iY, A.iZ + B.iZ);
}

inline TVector3 operator-(const TVector3 &A, const TVector3 &B)
{
	return TVector3(A.iX - B.iX, A.iY - B.iY, A.iZ - B.iZ);
}

inline TVector3 operator*(const TInt &A, const TVector3 &B)
{
	return TVector3(A*B.iX, A*B.iY, A*B.iZ);
}

inline TInt operator*(const TVector3 &A, const TVector3 &B)
{
	return A.iX*B.iX + A.iY*B.iY + A.iZ*B.iZ;
}

inline TVector3 operator>>(const TVector3 &A, const TInt shift)
{
	return TVector3(A.iX>>shift, A.iY>>shift, A.iZ>>shift);
}

inline TVector3 operator<<(const TVector3 &A, const TInt shift)
{
	return TVector3(A.iX<<shift, A.iY<<shift, A.iZ<<shift);
}

inline int operator%(const TVector3 &A, TVector3 &B)
{
	return A.iX*B.iY - A.iY*B.iX;
}


inline int crossXZ(const TVector3 &A, const TVector3 &B)
{
	return A.iX*B.iZ - A.iZ*B.iX;
}

inline TInt abs(const TVector3 &A)
{
	return Abs(A.iX) + Abs(A.iY) + Abs(A.iZ);
}

TBool IntervalsIntersection(const TPoint& vV0, const TPoint& vD0, const TPoint& vV1, const TPoint& vD1, TInt& aAlpha,  TInt& aBeta, TMathLookup& aMath);


inline TVectorR operator+(const TVectorR &A, const TVectorR &B)
{
	return TVectorR(A.iX + B.iX, A.iY + B.iY, A.iZ + B.iZ);
}

inline TVectorR operator-(const TVectorR &A, const TVectorR &B)
{
	return TVectorR(A.iX - B.iX, A.iY - B.iY, A.iZ - B.iZ);
}

inline TVectorR operator*(const TFloat &A, const TVectorR &B)
{
	return TVectorR(A*B.iX, A*B.iY, A*B.iZ);
}

inline TFloat operator*(const TVectorR &A, const TVectorR &B)
{
	return A.iX*B.iX + A.iY*B.iY + A.iZ*B.iZ;
}

inline TVectorR operator-(const TVectorR &A)
{
	return TVectorR(-A.iX, -A.iY, -A.iZ);
}

inline TVectorR operator^(const TVectorR &A, const TVectorR &B)
{
	return TVectorR(A.iY*B.iZ - A.iZ*B.iY, A.iZ*B.iX - A.iX*B.iZ, A.iX*B.iY - A.iY*B.iX);
}

inline TFloat neg_dot(const TVectorR &A, const TVectorR &B)
{
	return -A.iX*B.iX - A.iY*B.iY - A.iZ*B.iZ;
}

inline TVectorR operator%(const TVectorR &A, const TVectorR &B)
{
	return TVectorR(A.iY*B.iZ - A.iZ*B.iY, A.iZ*B.iX - A.iX*B.iZ, A.iX*B.iY - A.iY*B.iX);
}

inline TFloat mix(const TVectorR &A, const TVectorR &B, const TVectorR &C)
{
	TVectorR aCross;
	aCross.cross(B, C);

	return A*aCross;
}

inline TFloat abs(const TVectorR &A)
{
	return Abs(A.iX) + Abs(A.iY) + Abs(A.iZ);
}

class TVectorR4
{
public:
	TVectorR iV[4];

	TVectorR4(){};

	inline void NormZ()
	{
		TInt count;
		for(count = 0; count < 4; count++)
			iV[count] = (1.f/iV[count].iZ)*iV[count];
	};

	inline void NormAZ()
	{
		TInt count;
		for(count = 0; count < 4; count++)
			iV[count] = (1.f/FAbs(iV[count].iZ))*iV[count];
	};

	inline void NormZ(TFloat aD)
	{
		TInt count;
		for(count = 0; count < 4; count++)
			iV[count] = (aD/iV[count].iZ)*iV[count];
	};

	inline void FastScale()
	{
		TInt count;
		for(count = 0; count < 4; count++)
			iV[count].FastScale();
	};


};

////////////////////////////////////////////////////////////////////////

inline TInt CondDownshift(const TInt& aA, TInt aShift)
{
	if(aShift > 0)
		return aA>>aShift;
	else
		return aA<<aShift;
}

inline void SelfDownshift(TInt& aA, TInt aShift)
{
	if(aShift > 0)
		aA>>=aShift;
	else
		aA<<=aShift;
}


////////////////////////////////////////////////////////////////////////

#define NORMZ_RECIEP(aV) \
{\
	TFloat aInv_reciep = iMath.GetReciprocalLongF((aV).iZ);\
	(aV).iX *= aInv_reciep;\
	(aV).iY *= aInv_reciep;\
	(aV).iZ = 1.f;\
};

#define NORMZ_RECIEP4(aV, aD) \
{\
	TInt count;\
	for(count = 0; count <4; count++)\
	{\
		TFloat aInv_reciep = (aD)*iMath.GetReciprocalLongF((aV).iV[count].iZ);\
		(aV).iV[count].iX *= aInv_reciep;\
		(aV).iV[count].iY *= aInv_reciep;\
		(aV).iV[count].iZ = 1.f;\
	}\
};


////////////////////////////////////////////////////////////////////////


struct TRPoint
{
	TFloat iX;
	TFloat iY;
	TRPoint( TFloat aX, TFloat aY)
	{
		iX = aX;
		iY = aY;
	}

	inline void operator+=(const TRPoint &A)
	{
		iX += A.iX;
		iY += A.iY;
	};

	inline void operator-=(const TRPoint &A)
	{
		iX -= A.iX;
		iY -= A.iY;
	}

	inline void operator*=(const TFloat &A)
	{
		iX *= A;
		iY *= A;
	};


	TRPoint(){};
};

inline TFloat operator*(const TRPoint &A, const TRPoint &B)
{
	return A.iX*B.iX + A.iY*B.iY;
};

inline TFloat  operator^(const TRPoint& aP0, const TRPoint& aP1)
{
	return aP0.iX*aP1.iY - aP0.iY*aP1.iX;
};

inline TRPoint  operator+(const TRPoint& aP0, const TRPoint& aP1)
{
	return TRPoint(aP0.iX + aP1.iX, aP0.iY + aP1.iY);
};

inline TRPoint  operator-(const TRPoint& aP0, const TRPoint& aP1)
{
	return TRPoint(aP0.iX - aP1.iX, aP0.iY - aP1.iY);
};

inline TRPoint  operator*(const TFloat& aM, const TRPoint& aP)
{
	return TRPoint(aM*aP.iX, aM*aP.iY);
};

inline TFloat abs(const TRPoint& aP)
{
	return Abs(aP.iX) + Abs(aP.iY);
};

inline TFloat absM(const TRPoint& aP)
{
	return Max(Abs(aP.iX),Abs(aP.iY));
};

////////////////////////////////////////////////////////////////////////
inline TInt PDist(TPoint& aP0, TPoint& aP1)
{
	TInt aX = Abs(aP1.iX - aP0.iX);
	TInt aY = Abs(aP1.iY - aP0.iY);

	return Max(aX, aY);
}
////////////////////////////////////////////////////////////////////////
//inline TBool operator==(const TPoint& aP0, const TPoint& aP1)
//{
//	return aP0.iX == aP1.iX && aP0.iY == aP1.iY;
//}

////////////////////////////////////////////////////////////////////////

inline TInt abs(const TPoint& aP)
{
	return Abs(aP.iX) + Abs(aP.iY);
};


inline void GetMin(TPoint& aDst, TPoint& aSrc)
{
	aDst.iX = Min(aDst.iX, aSrc.iX);
	aDst.iY = Min(aDst.iY, aSrc.iY);
};
inline void GetMax(TPoint& aDst, TPoint& aSrc)
{
	aDst.iX = Max(aDst.iX, aSrc.iX);
	aDst.iY = Max(aDst.iY, aSrc.iY);
}; 

inline TBool operator<=(TPoint& aP0, TPoint& aP1)
{
	return (aP0.iX <= aP1.iX && aP0.iY <= aP1.iY);
};

inline TPoint operator/(TPoint aP, TInt& aDiv)
{
	return TPoint(aP.iX/aDiv, aP.iY/aDiv);
};

inline TPoint operator*(TPoint& aP, TInt& aMul)
{
	return TPoint(aP.iX*aMul, aP.iY*aMul);
};


inline TInt  operator*(const TPoint& aP0, const TPoint& aP1)
{
	return aP0.iX*aP1.iX + aP0.iY*aP1.iY;
};

inline TInt  operator^(const TPoint& aP0, const TPoint& aP1)
{
	return aP0.iX*aP1.iY - aP0.iY*aP1.iX;
};

/*

inline TPoint  operator+(TPoint& aP0, TPoint& aP1)
{
	return TPoint(aP0.iX + aP1.iX, aP0.iY + aP1.iY);
};

inline TPoint  operator-(TPoint& aP0, TPoint& aP1)
{
	return TPoint(aP0.iX - aP1.iX, aP0.iY - aP1.iY);
};
*/
inline TPoint  operator*(TInt aM, const TPoint& aP)
{
	return TPoint(aM*aP.iX, aM*aP.iY);
};

inline TPoint  operator>>(const TPoint& aP, TInt aM)
{
	return TPoint(aP.iX>>aM, aP.iY>>aM);
};

inline TPoint  operator<<(const TPoint& aP, TInt aM)
{
	return TPoint(aP.iX<<aM, aP.iY<<aM);
};


void ReduceSize(TPoint &aP, TInt aUnity);
void NormPoint(TMathLookup& aMath, TPoint &aP);
TInt GetLenPoint(TMathLookup &aMath, const TPoint &aP);

struct TBeta
{
	TRPoint iP;
	TRPoint iProjBeta;
	TRPoint iBeta;
	TInt iBetaInd;
};

inline int sign(TInt A)
{
	if(A > 0)
		return 1;
	else if(A < 0)
		return -1;
	else return 0;

}

#endif

////////////////////////////////////////////////////////////////////////
