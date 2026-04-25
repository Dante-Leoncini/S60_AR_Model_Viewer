#ifndef __3DTYPES_H__
#define __3DTYPES_H__


// INCLUDES
#include <e32std.h>

// CONSTANTS
// 23:9 fixed point math used trough all math in 3D-example
const TInt KShift = 9;
const TInt KShift_U = 512;

// sin & cos table size
const TInt KSinTableSize = 4096;


// 4x3 matrix for world rotations

class TMatrix
	{
	public:
		inline TMatrix() {};
		inline TMatrix( TInt aM00, TInt aM10, TInt aM20, TInt aM30,
						TInt aM01, TInt aM11, TInt aM21, TInt aM31,
						TInt aM02, TInt aM12, TInt aM22, TInt aM32 )
			{
			iM[ 0 ] = aM00; iM[ 1 ] = aM10;  iM[ 2 ] = aM20; iM[ 3 ] = aM30;
			iM[ 4 ] = aM01; iM[ 5 ] = aM11;  iM[ 6 ] = aM21; iM[ 7 ] = aM31;
			iM[ 8 ] = aM02; iM[ 9 ] = aM12;  iM[ 10] = aM22; iM[ 11] = aM32;
			}


		inline TInt& operator[]( TInt aIndex )
			{
			return iM[ aIndex ];
			}

		inline void operator=( TMatrix aM )
			{
			TInt* m = aM.iM;
			for( TInt i=0; i<12; i++ ) iM[ i ] = m[ i ];
			}

		inline void operator*=( TMatrix aM )
			{
			TInt t[ 12 ];
			TInt* m = aM.iM;

			TInt v1 = 1 << KShift;
			
			t[ 0 ] = iM[ 0 ] * m[ 0 ] + iM[ 1 ] * m[ 4 ] + iM[ 2 ] * m[ 8 ];
			t[ 1 ] = iM[ 0 ] * m[ 1 ] + iM[ 1 ] * m[ 5 ] + iM[ 2 ] * m[ 9 ];
			t[ 2 ] = iM[ 0 ] * m[ 2 ] + iM[ 1 ] * m[ 6 ] + iM[ 2 ] * m[ 10];
			t[ 3 ] = iM[ 0 ] * m[ 3 ] + iM[ 1 ] * m[ 7 ] + iM[ 2 ] * m[ 11] + iM[ 3 ] * v1;

			t[ 4 ] = iM[ 4 ] * m[ 0 ] + iM[ 5 ] * m[ 4 ] + iM[ 6 ] * m[ 8 ];
			t[ 5 ] = iM[ 4 ] * m[ 1 ] + iM[ 5 ] * m[ 5 ] + iM[ 6 ] * m[ 9 ];
			t[ 6 ] = iM[ 4 ] * m[ 2 ] + iM[ 5 ] * m[ 6 ] + iM[ 6 ] * m[ 10];
			t[ 7 ] = iM[ 4 ] * m[ 3 ] + iM[ 5 ] * m[ 7 ] + iM[ 6 ] * m[ 11] + iM[ 7 ] * v1;

			t[ 8 ] = iM[ 8 ] * m[ 0 ] + iM[ 9 ] * m[ 4 ] + iM[ 10] * m[ 8 ];
			t[ 9 ] = iM[ 8 ] * m[ 1 ] + iM[ 9 ] * m[ 5 ] + iM[ 10] * m[ 9 ];
			t[ 10] = iM[ 8 ] * m[ 2 ] + iM[ 9 ] * m[ 6 ] + iM[ 10] * m[ 10];
			t[ 11] = iM[ 8 ] * m[ 3 ] + iM[ 9 ] * m[ 7 ] + iM[ 10] * m[ 11] + iM[ 11] * v1;

			for( TInt i=0; i<12; i++ ) iM[ i ] = t[ i ] >> KShift;
			
			}
		TInt iM[ 12 ];
	};


// type of 3D-vertex
// some basic operators supported

class TVertex
	{
	public:
		inline TVertex() {}
		inline TVertex( TInt aX, TInt aY, TInt aZ ) : iX( aX ), iY( aY ), iZ( aZ ) {}
		inline TVertex TVertex::operator-()
			{
			TVertex temp;
			temp.iX = -iX;
			temp.iY = -iY;
			temp.iZ = -iZ;
			return temp;
			}
		inline TVertex TVertex::operator-( const TVertex& aVertex )
			{
			TVertex temp = *this;
			temp.iX -= aVertex.iX;
			temp.iY -= aVertex.iY;
			temp.iZ -= aVertex.iZ;
			return temp;
			}
		inline TVertex TVertex::operator+( const TVertex& aVertex )
			{
			TVertex temp = *this;
			temp.iX += aVertex.iX;
			temp.iY += aVertex.iY;
			temp.iZ += aVertex.iZ;
			return temp;
			}
		inline void TVertex::operator+=( const TVertex& aVertex )
			{
			iX += aVertex.iX;
			iY += aVertex.iY;
			iZ += aVertex.iZ;
			}
		inline void TVertex::operator-=( const TVertex& aVertex )
			{
			iX -= aVertex.iX;
			iY -= aVertex.iY;
			iZ -= aVertex.iZ;
			}

		TBool TVertex::operator==( const TVertex& aVertex )
			{
			return ( ( iX == aVertex.iX ) && ( iY == aVertex.iY ) && ( iZ == aVertex.iZ ) );
			}

		inline void MulMatrix( TMatrix* aMatrix )
			{
			TInt* m = aMatrix->iM;

			TInt x = iX;
			TInt y = iY;
			TInt z = iZ;
			
			iX = ( ( x * m[ 0 ] + y * m[ 1 ] + z * m[ 2 ] ) >> KShift ) + m[ 3 ];
			iY = ( ( x * m[ 4 ] + y * m[ 5 ] + z * m[ 6 ] ) >> KShift ) + m[ 7 ];
			iZ = ( ( x * m[ 8 ] + y * m[ 9 ] + z * m[ 10 ] ) >> KShift ) + m[ 11 ];


			}

	public:
		TInt iX;
		TInt iY;
		TInt iZ;
	};






class TFace
	{
	public:
		inline TFace() {}
		inline TFace( TInt aV1, TInt aV2, TInt aV3 )
			: iV1( aV1 ), iV2( aV2 ), iV3( aV3 )
			{
			}
		inline TFace( TInt aV1, TInt aV2, TInt aV3, 
					  TInt aTx1, TInt aTy1,
					  TInt aTx2, TInt aTy2,
					  TInt aTx3, TInt aTy3
					)
			: iV1( aV1 ), iV2( aV2 ), iV3( aV3 ),
			  iTx1( aTx1 ), iTy1( aTy1 ),
			  iTx2( aTx2 ), iTy2( aTy2 ),
			  iTx3( aTx3 ), iTy3( aTy3 )
			{
			}

	public:
		TInt iV1;	// vertex indexes
		TInt iV2;
		TInt iV3;

		TInt iTx1;	// texture coordinates
		TInt iTy1;
		TInt iTx2;
		TInt iTy2;
		TInt iTx3;
		TInt iTy3;
	};


#endif
