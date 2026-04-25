   /*
============================================================================
    * Name : CPictureLoader.h
    * Part of : Example3D
    * Description : Definition of CPictureLoader
    * Copyright (c) 2007 Nokia Corporation
============================================================================
    */

#ifndef __CPICTURELOADER_H__
#define __CPICTURELOADER_H__


// INCLUDES
#include <e32base.h>
//#include <MdaImageConverter.h>
#include "TBitmap.h"

class CFbsBitmap;

// CLASS DECLARATION

/**
*  CPictureLoader can load picture types
*  supported by Symbian's CMdaImageFileToBitmapUtility
*  for example .BMP, .GIF and .JPG
*  Pictures are saved to TBitmap type bitmaps
*/

class CPictureLoader
	: public CActive
	//, public MMdaImageUtilObserver
	{
	public:

		/// Two-phased constructor
		static CPictureLoader* NewL();

		/// Destructor
		~CPictureLoader();

	private:

		/// Second-phase constructor
		void ConstructL();

		/// Default constructor
		CPictureLoader();

	public:	// New methods:

		/// Loads picture with given filename
		/// This method can leave if any error occures in load
		/// Application path is automatically added to filename
		/// @param aFileName file name without path
		/// @param aMode color mode to load to ( EColor4K, EColor64K )
		/// @return TBitmap type bitmap
		TBitmap LoadL( const TFileName& aFileName, TDisplayMode aMode);
		TBitmap SetPixels();

		void RunL();
		void DoCancel();

	private: // MMdaImageUtilObserver
		
//		void MiuoConvertComplete( TInt aError );
//		void MiuoCreateComplete( TInt aError );
//		void MiuoOpenComplete( TInt aError );
        enum TConvState
            {
            EIdle = 0,
            EConverting
            };
	private:
        
        TConvState iState;
        CActiveSchedulerWait iWait;
		TInt		iErrStatus;	// contains loading and conversion error status
		TFileName	iPath;		// contains application's path
		RFs			iFs;
		TDisplayMode iDisplayMode;
		CFbsBitmap* iBitmap;
	};


#endif
	
