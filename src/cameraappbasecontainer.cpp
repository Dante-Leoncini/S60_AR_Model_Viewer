/*
 * Copyright © 2008 Nokia Corporation.
 */

#include <w32std.h>
#include <coemain.h>
#include <aknsutils.h>
#include <aknnavide.h>
#include <barsread.h>
#include <akntabgrp.h>
#include <ARDemo.rsg>

#include "CameraAppBaseContainer.h"
#include "CameraAppController.h"
#include "cameraappapp.h"
#include "cameraappappui.h"
#include "CGLengine.h"
#include "PhEngine.h"
#include "vtracker.h"


// Constants
const TInt KErrorMsgXPos = 10;
const TInt KErrorMsgYPos = 50;
const TInt KErrorMaxWidth = 160;
const TInt KRedColor = 35;


/*
-------------------------------------------------------------------------------
Constructor. Initializes member variables.
-------------------------------------------------------------------------------
*/
CCameraAppBaseContainer::CCameraAppBaseContainer(
  CCameraAppController*& aController )
: iController( aController ),
  iBitmap( 0 ),
  iOffScreenBitmap( 0 ),
  iFbsBitGc( 0 ),
  iBmpDevice( 0 ),
  iGLEngine( 0 ),
  iTracker( 0 )
  {
	iDoOneTimeIni = ETrue;
	iLastKey = -1;
	iFreeze = EFalse;
	iKeyPressed = EFalse;

	iController->SetAppContainer( this );
  }

void CCameraAppBaseContainer::SetController(CCameraAppController*& aController)
    {
    iController = aController;
    if (iController)
        {
        iController->SetAppContainer( this );
        }
    }


/*
-------------------------------------------------------------------------------
Symbian OS 2nd phase constructor
-------------------------------------------------------------------------------
*/

void CCameraAppBaseContainer::ConstructL(const TRect& aRect)
    {
    CreateWindowL();

	//SetRect(aRect);

	SetExtentToWholeScreen();
    ActivateL();

	iGLEngine = new(ELeave) CGLengine(Window(), CCoeEnv::Static()->ScreenDevice()->DisplayMode(), aRect);
	iGLEngine->ConstructL();

	iStat = new TDisplayStat();

	iGLEngine->iStat = iStat;

    // This should be called after the windows has been activated
    CreateOffScreenBitmapL();
    }

/*
-------------------------------------------------------------------------------
Destructor. Frees reserved resources.
-------------------------------------------------------------------------------
*/
CCameraAppBaseContainer::~CCameraAppBaseContainer()
    {

	if(iPhEngine)
		delete iPhEngine;
	if(iGLEngine)
		delete iGLEngine;
	if(iTracker)
		delete iTracker;
    delete iOffScreenBitmap;
    delete iFbsBitGc;
    delete iBmpDevice;
	delete iStat;
    }

/*
-------------------------------------------------------------------------------
Renders a bitmap image onto a real screen
-------------------------------------------------------------------------------
*/
void CCameraAppBaseContainer::DrawImage(CWindowGc& aGc,
    const TRect& aRect) const
    {
    if( iController && iController->IsSkippingFrames() )
      {
      return;
      }

    TSize bmpSizeInPixels( iBitmap->SizeInPixels() );
    TInt xDelta = (aRect.Width() - bmpSizeInPixels.iWidth) / 2;
    TInt yDelta = (aRect.Height() - bmpSizeInPixels.iHeight) / 2;
    TPoint pos( xDelta, yDelta ); // displacement vector
    pos += aRect.iTl; // bitmap top left corner position

    // Drawing viewfinder image to bitmap
    iFbsBitGc->BitBlt( pos, iBitmap, TRect( TPoint( 0, 0 ), bmpSizeInPixels ));

    // Draws bitmap with indicators on the screen
    TSize size( iOffScreenBitmap->SizeInPixels() );
    aGc.BitBlt( TPoint(0,0), iOffScreenBitmap, TRect( TPoint(0,0), size ) );
    }

/*
-------------------------------------------------------------------------------
Creates the data member offscreen bitmap
-------------------------------------------------------------------------------
*/
void CCameraAppBaseContainer::CreateOffScreenBitmapL()
    {
    iOffScreenBitmap = new (ELeave) CWsBitmap( iCoeEnv->WsSession() );

    TSize size = Rect().Size();

    TInt createError = iOffScreenBitmap->Create( size,
        iController->DisplayMode());
    User::LeaveIfError( createError );

    iFbsBitGc = CFbsBitGc::NewL(); //graphic context
    iBmpDevice = CFbsBitmapDevice::NewL(iOffScreenBitmap);
    iFbsBitGc->Activate(iBmpDevice);
    iFbsBitGc->SetBrushColor( KRgbBlack );
    iFbsBitGc->Clear();

    MAknsSkinInstance* skin = AknsUtils::SkinInstance();
    CFbsBitmap* bitmap = AknsUtils::GetCachedBitmap( skin,
        KAknsIIDQsnBgAreaMain);
    if ( bitmap )
        {
        iFbsBitGc->BitBlt( TPoint(0,0), bitmap );
        }

    iOffScreenBitmapCreated = ETrue;
    }

/*
-------------------------------------------------------------------------------
Gets from Controller the image to draw and calls DrawNow()
-------------------------------------------------------------------------------
*/
void CCameraAppBaseContainer::DrawImageNow(CFbsBitmap& aBitmap)
    {
    if (iOffScreenBitmapCreated && IsActivated() && IsReadyToDraw())
        {
        iBitmap = &aBitmap;
        DrawNow();
        }
    }


/*
-------------------------------------------------------------------------------
Redraws offscreen bitmap with landscape focus indicators
-------------------------------------------------------------------------------
*/
void CCameraAppBaseContainer::ReDrawOffScreenBitmap()
    {
    iOffScreenBitmapCreated = ETrue;

    iFbsBitGc->SetBrushColor( KRgbBlack );
    iFbsBitGc->Clear();

    MAknsSkinInstance* skin = AknsUtils::SkinInstance();
    CFbsBitmap* bitmap = AknsUtils::GetCachedBitmap( skin,
        KAknsIIDQsnBgAreaMain );
    if ( bitmap )
        {
        iFbsBitGc->BitBlt( TPoint(0,0), bitmap );
        }

    DrawNow();
    }

/*
-------------------------------------------------------------------------------
Shows an error message on the screen
-------------------------------------------------------------------------------
*/
void CCameraAppBaseContainer::ShowErrorMessage(const TDesC &aMsg)
    {
    const CFont* font = LatinBold16();
    iFbsBitGc->UseFont( font );
    TInt ascent = font->AscentInPixels();
    TInt height = font->HeightInPixels();
    iFbsBitGc->SetPenColor( AKN_LAF_COLOR( KRedColor ) );
    TPoint point( KErrorMsgXPos, KErrorMsgYPos );
    iFbsBitGc->DrawText(
                  aMsg,
                  TRect( point, TSize( KErrorMaxWidth, height )),
                  ascent,
                  CGraphicsContext::ERight,
                  0 );

    // Draw the offscreen bitmap now to show the message
    DrawNow();
    }

/*
-------------------------------------------------------------------------------
Called when this control's rect changes
-------------------------------------------------------------------------------
*/
void CCameraAppBaseContainer::SizeChanged()
    {
    TRect rect = Rect();
    if( iOffScreenBitmapCreated )
      {
      iOffScreenBitmap->Resize( rect.Size() );
      iBmpDevice->Resize( rect.Size() );
      iFbsBitGc->Resized();
      }
    if (iController)
        {
        TRAP_IGNORE( iController->ClientRectChangedL( rect ) );
        }
    }

/*
-------------------------------------------------------------------------------
Handles a change to the control's resources. Specifically, handles a layout
(orientation) change event.
-------------------------------------------------------------------------------
*/
void CCameraAppBaseContainer::HandleResourceChange( TInt aType )
    {
  if( aType == KEikDynamicLayoutVariantSwitch )
      {
      if( iOffScreenBitmapCreated )
          {
          iFbsBitGc->Clear();
          }
      // skip a couple of vf frames, otherwise display can get messed up
      // during layout switch
      const TInt KNumOfFrames = 5;
      if (iController)
          {
          iController->SkipFrames(KNumOfFrames);
          }
      }

    // call base class implementation
    CCoeControl::HandleResourceChange( aType );
    }


/*
-------------------------------------------------------------------------------
Called by the framework to draw the screen
-------------------------------------------------------------------------------
*/
void CCameraAppBaseContainer::Draw(const TRect& /*aRect*/ ) const 
    {
    TRect drawingRect = Rect();
    CWindowGc& gc = SystemGc();

    if ( iOffScreenBitmapCreated )
        {
        if ( iBitmap ) //Viewfinding ongoing
            {
            DrawImage(gc, drawingRect); 
            }
        else
            {
            MAknsSkinInstance* skin = AknsUtils::SkinInstance();
            CFbsBitmap* bitmap = AknsUtils::GetCachedBitmap( skin, 
                KAknsIIDQsnBgAreaMain );
            if ( bitmap ) 
                {
                gc.BitBlt( TPoint(0,0), bitmap);
                }
            else
                {
                // Draws bitmap with indicators on the screen
                TSize size( iOffScreenBitmap->SizeInPixels() );
                gc.BitBlt( TPoint(0,0), iOffScreenBitmap, 
                    TRect( TPoint(0,0), size ) );
                }
            }
        }
    }


TKeyResponse CCameraAppBaseContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
	{
	TKeyResponse returnKey = EKeyWasNotConsumed;


	if(iDoOneTimeIni)
		return EKeyWasConsumed;

	// If the app switch key is pressed we need to stop drawing later put menu here if DSA frame implemented
	if(aType == EEventKey && aKeyEvent.iScanCode == EStdKeyApplication0)
	{


		// we still want the key event to be passed on so don't consume the key
	}

	if(aType == EEventKeyDown)
	{
		iLastKey = aKeyEvent.iScanCode;
		iKeyPressed = ETrue;

		iFreeze = EFalse;

		switch(aKeyEvent.iScanCode)
			{
			case EStdKeyLeftArrow:
				returnKey = EKeyWasConsumed;
				break;
			case EStdKeyRightArrow:
				returnKey = EKeyWasConsumed;
				break;
			case EStdKeyUpArrow:
				iPhEngine->m_ctrlData.m_ctrlAction = 5;
				returnKey = EKeyWasConsumed;
				break;
			case EStdKeyDownArrow:
				iPhEngine->m_ctrlData.m_ctrlAction = 6;
				returnKey = EKeyWasConsumed;
				break;
			case EStdKeyDevice3:
				iPhEngine->m_ctrlData.m_ctrlAction = 10;
				returnKey = EKeyWasConsumed;
				break;

			case 48: //0
				returnKey = EKeyWasConsumed;
			break;

			case 49:
				iController->DecContrast();; //1
				returnKey = EKeyWasConsumed;
				//iTracker->newFrame = 1;
				iKeyPressed = EFalse;
			break;

			case 50://2
				iController->IncContrast();//2
				returnKey = EKeyWasConsumed;
				iKeyPressed = EFalse;
			break;

			case 51://3
				iGLEngine->m_bPortrait = !iGLEngine->m_bPortrait;
				returnKey = EKeyWasConsumed;
			break;

			case 52://4
				iTracker->IncTreshold();
				returnKey = EKeyWasConsumed;
			break;

			case 53://5
				iTracker->DecTreshold();
				returnKey = EKeyWasConsumed;
			break;

			case 54: //6
//#ifdef  _DEB_VERSION_		
				iTracker->ToggDebugBW();
				returnKey = EKeyWasConsumed;
			break;

			case 55: //7
				iTracker->DecFocus();
				returnKey = EKeyWasConsumed;
			break;

			case 56: //8
				iTracker->IncFocus();
				returnKey = EKeyWasConsumed;
			break;

			case 0x2A://left from zero
				returnKey = EKeyWasConsumed;
			break;

			case 127: //right from zero
				returnKey = EKeyWasConsumed;
			break;


			case 57: //9
		//		iPhEngine->Save();

				returnKey = EKeyWasConsumed;
			break;

			case 5:

				returnKey = EKeyWasConsumed;
			break;

		default:
			returnKey = EKeyWasConsumed;
			break;
			}

		}
	else if(aType == EEventKeyUp)
		{
		}


	return returnKey;

	}


void CCameraAppBaseContainer::StartSaveStreamL(TInt aInd)
{
    m_fileSession = new RFs;
    User::LeaveIfError(m_fileSession->Connect());
	TPtrC16 aName;

    m_file = new RFile;

	if(aInd==0)
	{
		_LIT (KSaveFilename,"E:\\Others\\save_p.bin");
		aName.Set(KSaveFilename);
	}
	else
	{
		_LIT (KSaveFilename,"E:\\Others\\save_r.bin");
		aName.Set(KSaveFilename);
	}



	m_file->Replace(*m_fileSession, aName, EFileWrite|EFileStream);

    m_pOutputFileStream = new(ELeave) RFileWriteStream(*m_file) ;

};


void CCameraAppBaseContainer::StartLoadStreamL(TInt aInd)
{
    m_fileSession = new RFs;
    User::LeaveIfError(m_fileSession->Connect());
	TPtrC16 aName;

    m_file = new RFile;

	if(aInd==0)
	{
		_LIT (KSaveFilename,"E:\\Others\\save_p.bin");
		aName.Set(KSaveFilename);
	}
	else
	{
		_LIT (KSaveFilename,"E:\\Others\\save_r.bin");
		aName.Set(KSaveFilename);
	}

	m_file->Open(*m_fileSession, aName, EFileRead|EFileStream);

    m_pReadFileStream = new(ELeave) RFileReadStream(*m_file) ;

};


void CCameraAppBaseContainer::ToStream(TUint8* aPtr, TInt aSize)
{
	m_pOutputFileStream->WriteL(aPtr, aSize);
	m_pOutputFileStream->CommitL();
};

void CCameraAppBaseContainer::FromStream(TUint8* aPtr, TInt aSize)
{
	m_pReadFileStream->ReadL(aPtr, aSize);
};

void CCameraAppBaseContainer::EndSaveStream()
{
	m_pOutputFileStream->Close();
	delete m_pOutputFileStream;
	m_file->Close();
	delete m_file; 
	m_fileSession->Close();
	delete m_fileSession;
};

void CCameraAppBaseContainer::EndLoadStream()
{
	m_pReadFileStream->Close();
	delete m_pReadFileStream;
	m_file->Close();
	delete m_file; 
	m_fileSession->Close();
	delete m_fileSession;
};

void CCameraAppBaseContainer::ProcessFrame(CFbsBitmap* pFrame)
{

	if(iPhEngine->m_restart && iKeyPressed)
	{
		iPhEngine->Restart();
		iPhEngine->m_restart = EFalse;
		iPhEngine->m_ctrlData.m_ctrlAction = 0;
	}

	iGLEngine->FillRGBBuffer(pFrame);
	iTracker->FillGreyBuffer(pFrame);

	TBool aCaptureSuccess = EFalse;
	aCaptureSuccess = iTracker->Capture();

	//insert markers
	if(aCaptureSuccess)
	{
		int numFoundMarkers = iTracker->GetNumberOfFoundMarkers();
		int fmarkers;
		for(fmarkers = 0; fmarkers < numFoundMarkers; fmarkers++)
		{
			TExportMarkerData expData;
			iTracker->GetFoundMarker(&expData, fmarkers);
			iPhEngine->MarkerStructure(expData.iMarkerID, expData.iPoint,
						expData.iMinX, expData.iMaxX, expData.iMinY, expData.iMaxY, expData.iC, expData.iRad);
		}
	}

	if(!aCaptureSuccess)
	{
		iTracker->ResetFilter();
		TInt aLostMsg[MAX_MSG_LEN];
		aLostMsg[0] = 0;
		aLostMsg[1] = 0;
		iGLEngine->iStat->ResetMsg();
		iGLEngine->iStat->AddMsg(aLostMsg);
	}

	if(aCaptureSuccess)
	{
		iPhEngine->Tick();
	}

	iGLEngine->StartRender();

	iPhEngine->Render();

	iGLEngine->FinishRender();

	iKeyPressed = EFalse;

};

void CCameraAppBaseContainer::SaveFramesL()
{
//#ifdef _DEB_FRAME_BUFF_
//
//	if(iTracker->iLogEnabled)
//		iLog->WriteFormat(_L("SaveFramesL"));
//
//    RFs fileSession;
//    User::LeaveIfError(fileSession.Connect());
//    CleanupClosePushL(fileSession);
//
//    RFile file;
//
//	_LIT (KSaveFilename,"E:\\Others\\frames.deb");
//
//	file.Replace(fileSession, KSaveFilename, EFileWrite);
//    CleanupClosePushL(file);
//
//    RFileWriteStream outputFileStream(file);
//    CleanupClosePushL(outputFileStream);
//	
//	outputFileStream.WriteInt32L(iTracker->numFrames) ;
//
//#ifndef _DEB_FULL_BUFF_
//	outputFileStream.WriteL((TUint16*)iTracker->iDebFrames, iTracker->numFrames*MAX_SIZE_X*MAX_SIZE_Y);
//#else
//	outputFileStream.WriteL((TUint16*)iTracker->iDebFrames, iTracker->numFrames*VFWIDTH*VFHEIGHT);
//#endif
//
//	outputFileStream.WriteL((TUint8 *)iTracker->iDebX, iTracker->numFrames*sizeof(TVectorR));
//	outputFileStream.WriteL((TUint8 *)iTracker->iDebY, iTracker->numFrames*sizeof(TVectorR));
//	outputFileStream.WriteL((TUint8 *)iTracker->iDebZero, iTracker->numFrames*sizeof(TVectorR));
//
//
//	//outputFileStream.WriteInt32L(iTracker->iNumbFeature);
//	//outputFileStream.WriteL((TUint8*)iTracker->iFeature, iTracker->iNumbFeature*sizeof(TFeature));
//
//	//outputFileStream.WriteInt32L(iTracker->iNumbStoredMarkers);
//	//outputFileStream.WriteL((TUint8*)iTracker->iStoredMarker, iTracker->iNumbStoredMarkers*sizeof(TMarker));
//
//
//  // Clean up
//    CleanupStack::PopAndDestroy(); // Close outputFileStream
//    CleanupStack::PopAndDestroy(); // Close file 
//    CleanupStack::PopAndDestroy(); // Close fileSession
//#endif
}


