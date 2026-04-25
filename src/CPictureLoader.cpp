   /*
============================================================================
    * Name : CPictureLoader.cpp
    * Part of : Example3D
    * Description : Definition of CPictureLoader
    * Copyright (c) 2007 Nokia Corporation
============================================================================
    */

// INCLUDES
#include "CPictureLoader.h"
#include <eikenv.h>
#include <eikappui.h>
#include <eikapp.h>

#include <imageConversion.h>

// MEMBER FUNCTIONS
CPictureLoader* CPictureLoader::NewL()
	{
	CPictureLoader* self = new( ELeave )CPictureLoader();
	CleanupStack::PushL( self );
	self->ConstructL();
	CleanupStack::Pop( self );
	return self;
	}

CPictureLoader::~CPictureLoader()
	{

	iFs.Close();
	if(iBitmap)
		delete iBitmap;
    Cancel();
	}

void CPictureLoader::ConstructL()
	{
 	CActiveScheduler::Add(this);
    iBitmap = new (ELeave) CFbsBitmap;
    
    User::LeaveIfError(iFs.Connect());
    iFs.PrivatePath(iPath);
 //   fs.Close();
#ifndef __WINS__
	TFileName appFullName = CEikonEnv::Static()->EikAppUi()->Application()->AppFullName();
	TParse parse;
    parse.Set( appFullName, NULL, NULL);
    iPath.Insert(0, parse.Drive());
#endif
	}

CPictureLoader::CPictureLoader()
: CActive(CActive::EPriorityStandard)
	{
	}

TBitmap CPictureLoader::LoadL( const TFileName& aFileName, TDisplayMode aDisplayMode)
	{
	TFileName filename( iPath );
	filename.Append( aFileName );
	
	iDisplayMode = aDisplayMode;	
	
	TBuf8<50> mimeType;
	CImageDecoder::GetMimeTypeFileL(iFs, filename, mimeType);
	CImageDecoder* imageDecoder = CImageDecoder::FileNewL(iFs,filename, mimeType );

	iBitmap->Create( imageDecoder->FrameInfo().iOverallSizeInPixels, iDisplayMode);

    iState = EConverting;
	imageDecoder->Convert(&iStatus, *iBitmap);
	SetActive();
    iWait.Start();
    delete imageDecoder;
    return SetPixels();
	}


TBitmap CPictureLoader::SetPixels(){
	TBitmap bm;

	bm.iSize = iBitmap->SizeInPixels();	

	TInt pixels = bm.iSize.iWidth * bm.iSize.iHeight;

	// this code assumed that pixels are always 16 bit
	if (iDisplayMode == EColor16MU)
		{
		// 32 bit pixels - convert to 24 bits, ignore alpha channel
		bm.iData = new( ELeave )TUint8[ pixels * 3 ];
		bm.iMode = EColor16M;

		iBitmap->LockHeap(EFalse);

		TUint8* source = (TUint8*)iBitmap->DataAddress();
		TUint8* destination = (TUint8*)bm.iData;

		TInt i;
		for (i=0; i<pixels; i++)
			{
			destination[0] = source[2];
			destination[1] = source[1];
			destination[2] = source[0];

			destination += 3;
			source += 4;
			}
		iBitmap->UnlockHeap(EFalse);
		}
	else
		{
		// 16 bit pixels
		bm.iData = new( ELeave )TUint16[ pixels ];
		bm.iMode = iDisplayMode;
		
		TInt y;
		for( y=0; y<bm.iSize.iHeight; y++ )
			{
			TPtr8 ydata( ((TUint8*)bm.iData) + y * bm.iSize.iWidth * 2, bm.iSize.iWidth * 2 );
			iBitmap->GetScanLine( ydata, TPoint( 0, y ), bm.iSize.iWidth, iDisplayMode );
			}
		}
	
	
	Cancel();
	
	return bm;
	}

void CPictureLoader::RunL()
	{
    if (iState == EConverting)
        {
    	iWait.AsyncStop();
        iState = EIdle;
        }
	}

void CPictureLoader::DoCancel()
	{	
	if(iBitmap)
		delete iBitmap;
	iBitmap = NULL;
	}
