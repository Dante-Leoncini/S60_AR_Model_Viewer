#ifndef _MATHLOOKUP_H
#define _MATHLOOKUP_H

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

#include <e32math.h>

////////////////////////////////////////////////////////////////////////
class TMathLookup;

class TTrigLookup
	{
	public:
		TTrigLookup();

		void GetTrigRatios(TInt aAngle, TInt &aCos, TInt &aSin);
		void GetAngle(TMathLookup* aMath, TInt aCos, TInt aSin, TInt& aAngle);

	private:
		void InitCosTable();

	private:
		
		enum
		{
			quadrantSizeLog	=  8,
			quadrantSize	= ( 1 << quadrantSizeLog ),
			quadrantMask	= ( quadrantSize - 1 ),
			//
			angleSizeLog	=  quadrantSizeLog + 2,
			angleSize		= ( 1 << angleSizeLog ),
			angleMask		= ( angleSize - 1 ),
			//
			cosTableSize	= ( quadrantSize + 1 ),
			//
			trigUnityLog	= 12,
			trigUnity		= ( 1 << trigUnityLog ),
			acosSizeLog = 12,
			acosSize	= ( 1 << acosSizeLog),
			acosMask = (acosSize - 1),
			acosResolution = 255
		};

		TInt16 iCosRatio[cosTableSize];
		TInt16 iACosRatio[acosSize];
	};

////////////////////////////////////////////////////////////////////////
#define RUNITY 16384
#define RUNITY_MASK 16383
#define RUNITY_LOG 14

union TFloatLong
{
    TFloat iF;
    TUint32 iL;
};

#define SQSIZE (3*(1<<12))

class TRecipLookup
	{
	public:
		TRecipLookup();

		void GetReciprocal(TInt aValue, TInt &aReciprocal, TInt &aDownshift);
		void GetReciprocalShort(TInt aValue, TInt &aReciprocal, TInt &aDownshift);
//		void GetReciprocalLong(TInt aValue, TInt &aReciprocal, TInt &aDownshift);
		TFloat GetReciprocalLongF(TFloat aValue);
		void GetReciprocalSqF(TFloat aValue, TFloat &aRSq);
		void GetSqF(TFloat aValue, TFloat &aSq);

	private:
		void InitRecipTable();

	private:
		
		enum
		{
			recipTableSize		= UNITY,
			//
			baseRecipUnityLog	=   30,
			baseRecipUnity		= ( 1 << baseRecipUnityLog),
			maxRecip			= 0x7fff,

			recipTableSizeLong		= RUNITY,
		};

		TInt16	iRecip[recipTableSize];
		TInt16	iShift[recipTableSize];

		TUint16	iRecipLong[recipTableSizeLong];
		TUint16	iSqrtLong[SQSIZE];
		TUint16	iRSqrtLong[SQSIZE];

		TInt16	iRecipShort[recipTableSize];
		TInt16	iShiftShort[recipTableSize];
	};

////////////////////////////////////////////////////////////////////////

class TRecipSqLookup
	{
	public:
		TRecipSqLookup();

		void GetReciprocalSq(TUint aValue, TInt &aReciprocalSq, TInt &aDownshift);
	    void Normalize16bitSq(TInt &aX, TInt &aY);

	private:
		void InitRecipSqTable();

	private:
		
		enum
		{
			recipSqTableSize		= UNITY,
			//
			baseRecipSqUnityLog	=   30,
			baseRecipSqUnity		= ( 1 << baseRecipSqUnityLog),
			maxRecipSq			= 0x7fff,
		};

		TUint	iRecipSq[recipSqTableSize];
		TInt16	iShift[recipSqTableSize];
	};

////////////////////////////////////////////////////////////////////////

class TSqrtLookup
	{
	public:
		TSqrtLookup();

		void GetSqrt(TUint aValue, TInt &aSqrt);

	private:
		void InitSqrtTable();

	private:
		
		enum
		{
			sqrtTableSize		= UNITY,
			//
			baseSqrtUnityLog	=   12,
			baseSqrtUnity		= ( 1 << baseSqrtUnityLog),
			maxSqrt			= (1<<baseSqrtUnityLog)-1,
		};

		TInt16	iSqrt[sqrtTableSize];
		TInt16	iShift[sqrtTableSize];
	};

////////////////////////////////////////////////////////////////////////


class TMathLookup
	{
	public:

	  inline void GetTrigRatios(TInt aAngle, TInt &aCos, TInt &aSin)
			{ 
			iTrig.GetTrigRatios(aAngle,aCos,aSin); 
			}

	  inline void GetAngle(TInt aCos, TInt aSin, TInt& aAngle)
			{ 
			iTrig.GetAngle(this, aCos, aSin, aAngle);
			}
 
	  inline void GetReciprocal(TInt aValue, TInt &aReciprocal, TInt &aDownshift)
			{ 
			iRecip.GetReciprocal(aValue,aReciprocal,aDownshift); 
			}
	  inline void GetReciprocalShort(TInt aValue, TInt &aReciprocal, TInt &aDownshift)
			{ 
			iRecip.GetReciprocalShort(aValue,aReciprocal,aDownshift); 
			}

//	  void GetReciprocalLong(TInt aValue, TInt &aReciprocal, TInt &aDownshift)
//			{ 
//			iRecip.GetReciprocalLong(aValue,aReciprocal,aDownshift); 
//			}

	  inline TFloat GetReciprocalLongF(TFloat aValue)
			{ 
			return iRecip.GetReciprocalLongF(aValue); 
			}

	inline void GetReciprocalSq(TUint aValue, TInt &aReciprocalSq, TInt &aDownshift)
			{ 
			iRecipSq.GetReciprocalSq(aValue,aReciprocalSq,aDownshift); 
			}

	inline void Normalize16bitSq(TInt &aX, TInt &aY)
			{ 
			iRecipSq.Normalize16bitSq(aX, aY); 
			}


	inline void GetSqrt(TUint aValue, TInt &aSqrt)
			{ 
			iSqrt.GetSqrt(aValue,aSqrt); 
			}


	inline void GetReciprocalSqF(TFloat aValue, TFloat &aRSq)
	{
		iRecip.GetReciprocalSqF(aValue, aRSq); 
	};

	inline void GetSqF(TFloat aValue, TFloat &aRSq)
	{
		iRecip.GetSqF(aValue, aRSq); 
	};



	private:
		TTrigLookup iTrig;
		TSqrtLookup iSqrt;
		TRecipLookup iRecip;
		TRecipSqLookup iRecipSq;
	};
#define M_PI 3.14159265
#define MAX_SHORT_LOG 14
#define MAX_SHORT ((1<<MAX_SHORT_LOG)-1)
#define DIFF_MOD512(A,B) (((A) - (B) + 256 + 512)%512 - 256) 
#define DIFF_MOD64(A,B) (((A) - (B) + 32 + 64)%64 - 32) 
#define DIFF_MOD1024(A,B) (((A) - (B) + 512 + 1024)%1024 - 512)
#define MOD512(A) ((A) + 512)%512 
#define MOD1024(A) ((A) + 1024)%1024 
#define FIX_2PI 25735

#define DIFF_MOD256x8(A,B) (((A) - (B) + 1024 + 2048)%2048 - 1024)


#define PRE_ADD_MOD256x8(A,B) if((A) - (B) > 256*4)(A) -= 256*8;\
							  if((A) - (B) < - 256*4)(A) += 256*8;



////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

#endif

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
