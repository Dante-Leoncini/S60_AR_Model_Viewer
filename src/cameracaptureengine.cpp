/*
 * Copyright � 2008 Nokia Corporation.
 */

#include "CameraCaptureEngine.h"
#include "Cameraapp.hrh"

#include <AknViewAppUi.h>
#include <ARDemo.rsg>
#include <centralrepository.h>
#include <ProfileEngineSDKCRKeys.h>
#include <eikenv.h>
#include <barsread.h>
#include <stringloader.h>
#include <hal.h>

#include <aknnotewrappers.h> 
#include "define.h"
#include "CameraAppBaseContainer.h"
#include "CGLengine.h"
#include "vtracker.h"
#include "PhEngine.h"


// GetImageSizeIndexL() could be used for selecting a specific image
// size. Here, we simply select 2nd largest size (index 1)
const TUint KImageSizeIndex = 1;

/*
-------------------------------------------------------------------------------
C++ default constructor
-------------------------------------------------------------------------------
*/
CCameraCaptureEngine::CCameraCaptureEngine( CCameraAppController* aController)
: CActive( EPriorityStandard ),
  iController( aController ),
  iZoomFactor( 0 ),
  iCapturePrepared( EFalse ),
  iBitmapSave( 0 ),
  iImageExif( 0 ),
  iEncoder( 0 ),
  iCameraReserved( EFalse )
#ifdef _AUTOFOCUS_ON_
  ,iAutoFocus( 0 )
#endif
    {
    CActiveScheduler::Add( this );
    iState = EEngineNotReady;
    }

/*
-------------------------------------------------------------------------------
Two-phased constructor
-------------------------------------------------------------------------------
*/
CCameraCaptureEngine* CCameraCaptureEngine::NewL(
    CCameraAppController* aController, const TRect& aRect )
    {
    CCameraCaptureEngine* self =
        new (ELeave) CCameraCaptureEngine( aController );

    CleanupStack::PushL( self );
    self->ConstructL(aRect);
    CleanupStack::Pop(self);

    return self;
    }


/*
-------------------------------------------------------------------------------
Destructor. Frees allocated resources.
-------------------------------------------------------------------------------
*/
CCameraCaptureEngine::~CCameraCaptureEngine()
    {
#ifdef _AUTOFOCUS_ON_
	delete iAutoFocus;
#endif
    delete iEncoder;
    delete iCamera;
    delete iBitmapPortraitVF;
    delete iBitmapSave;
    delete iImageExif;

	if(iLogEnabled)
	{
		iLog.WriteFormat(_L("~CCameraCaptureEngine"));
		iLog.CloseLog();
		iLog.Close();
	}


    // Cancel any outstanding request
    Cancel();
    }

/*
-------------------------------------------------------------------------------
Symbian OS 2nd phase constructor
-------------------------------------------------------------------------------
*/

void CCameraCaptureEngine::ConstructL(const TRect& aRect)
    {
    iEikEnv = CEikonEnv::Static();  // CSI: 27 # (Store a pointer to CEikonEnv)
    if ( !CCamera::CamerasAvailable() )
        {
        HandleError( KErrHardwareNotAvailable );
        return;
        }
	iContrast = 20;
    // camera index 0 (the main camera)
    iCamera = CCamera::NewL( *this, 0 );
#ifdef _AUTOFOCUS_ON_
    // try to construct autofocus object
    TRAPD(afErr, iAutoFocus = CCamAutoFocus::NewL( iCamera ));
    if(afErr)
        {
        iAutoFocus = 0;
        HandleError( KErrExtensionNotSupported );
        }
#endif

    iBitmapSave = new (ELeave) CFbsBitmap;

    // Gets information about the camera device. refer to SDK for more info.
    iCamera->CameraInfo(iInfo);
    iDisplayMode = DisplayMode();
    iColorDepth = ImageFormat();
    iColorDepthHW = ImageFormatMax();
    ClientRectChangedL(aRect);

	iLogEnabled = EFalse;//

	if(iLogEnabled)
	{
		iLog.Connect();
		iLog.CreateLog(_L("ARDemo"),_L("LogFile.txt"),EFileLoggingModeOverwrite);
		iLog.SetDateAndTime(EFalse, EFalse);
		iLog.Write(_L("Log started"));
	}

    }

/*
-------------------------------------------------------------------------------
Returns the default display mode
-------------------------------------------------------------------------------
*/
TDisplayMode CCameraCaptureEngine::DisplayMode() const
    {
    TInt color;
    TInt gray;
    TDisplayMode displayMode =
    iEikEnv->WsSession().GetDefModeMaxNumColors( color, gray );
    return displayMode;
    }

/*
-------------------------------------------------------------------------------
Returns camera image format to be used with current display mode
-------------------------------------------------------------------------------
*/
CCamera::TFormat CCameraCaptureEngine::ImageFormat() const
    {
    switch ( iDisplayMode )
        {
        case EColor16M:
            return CCamera::EFormatFbsBitmapColor16M;
        case EColor64K:
            return CCamera::EFormatFbsBitmapColor64K;
        case EColor4K:
            return CCamera::EFormatFbsBitmapColor4K;
        default:
            return CCamera::EFormatFbsBitmapColor4K;
        }
    }

/*
-------------------------------------------------------------------------------
Returns the highest color mode supported by HW
-------------------------------------------------------------------------------
*/
CCamera::TFormat CCameraCaptureEngine::ImageFormatMax() const
    {
    if ( iInfo.iImageFormatsSupported & CCamera::EFormatFbsBitmapColor16M )
        {
        return CCamera::EFormatFbsBitmapColor16M;
        }
    else if ( iInfo.iImageFormatsSupported & CCamera::EFormatFbsBitmapColor64K)
        {
        return CCamera::EFormatFbsBitmapColor64K;
        }
    else
        {
        return CCamera::EFormatFbsBitmapColor4K;
        }
    }

/*
-------------------------------------------------------------------------------
Stops view finding
-------------------------------------------------------------------------------
*/
void CCameraCaptureEngine::StopViewFinder()
    {
    if ( iCameraReserved )
        {
//        __LOGSTR_TOFILE("stop vf");
        iCamera->StopViewFinder();
        }
    }

void CCameraCaptureEngine::IncContrast()
{
	iContrast += 10;
#ifndef NOKIAE51
	TRAP_IGNORE(iCamera->SetContrastL(iCamera->Contrast() + iContrast));
#endif

}
void CCameraCaptureEngine::DecContrast()
{
	iContrast -= 10;
#ifndef NOKIAE51
	TRAP_IGNORE(iCamera->SetContrastL(iCamera->Contrast() + iContrast));
#endif
}



/*
-------------------------------------------------------------------------------
Starts view finding and prepares image capturing
-------------------------------------------------------------------------------
*/


void CCameraCaptureEngine::DoViewFinderL()
    {

#ifndef NOKIAE51
    //Default, always supported by API
    TRAP_IGNORE(iCamera->SetExposureL());
    TRAP_IGNORE(iCamera->SetDigitalZoomFactorL( iZoomFactor ));
	TRAP_IGNORE(iCamera->SetContrastL(iCamera->Contrast() + iContrast));
#endif

	//if(iLogEnabled) //debug log
	//		iLog.WriteFormat(_L("after SetExposureL  "));

    if ( iInfo.iOptionsSupported & TCameraInfo::EViewFinderBitmapsSupported )
        {
        if ( iInfo.iOptionsSupported & TCameraInfo::EImageCaptureSupported
            && !iCapturePrepared ) // first try capturing in EXIF JPEG format
            {
            iFormat = CCamera::EFormatExif;
			//image capture removed
			/*
            TRAPD(exifErr, iCamera->PrepareImageCaptureL(iFormat, KImageSizeIndex));
            if(exifErr) // capturing in EXIF format not supported,
                        // fall back to bitmap format
                {
                iFormat = iColorDepthHW;
                iCamera->PrepareImageCaptureL(iFormat, 1);
                }
            else    // notify controller that we're using EXIF capture mode
                {
                iCamera->EnumerateCaptureSizes( iCaptureSize, KImageSizeIndex,
                                               iFormat );

                iController->SetCaptureModeL( iCaptureSize,
                                             (TInt)iFormat,
                                             (TBool)iAutoFocus );
                }
				*/

            iCapturePrepared = ETrue;
            }

		iController->iContainer->iLog = &iLog;

		TRectData aRectData;
		aRectData.iRect = TRect(TPoint(0,0), TPoint(MAX_SIZE_X, MAX_SIZE_Y));
		aRectData.iPitch = MAX_SIZE_X;
		aRectData.iData = new(ELeave) TUint16[MAX_SIZE_X*MAX_SIZE_Y];

		if(iLogEnabled) //debug log
			iLog.WriteFormat(_L("before create tarcker  "));


		if(iController->iContainer->iTracker == 0)
		{
			VTracker* aTracker = NULL;
			
			aTracker = new(ELeave) VTracker(aRectData, &iLog, iLogEnabled);	

			CGLengine* aGLEngine = iController->iContainer->iGLEngine;
			aGLEngine->SetTracker(aTracker);

			iController->iContainer->iTracker = aTracker;
			iController->iContainer->iTracker->SetLog(iLogEnabled);
			iController->iContainer->iGLEngine->iLog = &iLog;
			iController->iContainer->iGLEngine->iLogEnabled = iLogEnabled;

			iController->iContainer->iPhEngine = new(ELeave) CPhEngine(iController->iContainer->iGLEngine);
			iController->iContainer->iPhEngine->iView = iController->iContainer;
			iController->iContainer->iGLEngine->m_phEngine = iController->iContainer->iPhEngine;
			iController->iContainer->iPhEngine->iTracker = aTracker;
			iController->iContainer->iPhEngine->ConstructL(iController->iContainer->iStat);
		}


		iPortraitSize.iWidth = VFWIDTH;
		iPortraitSize.iHeight = VFHEIGHT;


		TRect aRect = iController->iContainer->iGLEngine->m_clientRect;

		TCameraInfo aInfo;
		iCamera->CameraInfo(aInfo);

		TInt nanokernel_tick_period;
		HAL::Get(HAL::ENanoTickPeriod, nanokernel_tick_period);
		if(iLogEnabled) 
			iLog.WriteFormat(_L("nanokernel_tick_period %i  "), nanokernel_tick_period);

        // Start the view finding, and transfer the view finder data.
        // Bitmaps are returned by MCameraObserver::ViewFinderFrameReady().
#ifdef _AUTOFOCUS_ON_ 
		SetFocusRangeL(CCamAutoFocus::ERangeInfinite );
#endif

		iController->iContainer->iDoOneTimeIni = EFalse;
        iCamera->StartViewFinderBitmapsL( iPortraitSize );

		if(iLogEnabled) //debug log
			iLog.WriteFormat(_L("after StartViewFinderBitmapsL"));

        }
    else if (iInfo.iOptionsSupported & TCameraInfo::EViewFinderDirectSupported)
        {
        User::Leave(KErrNotSupported);
        }
    else
        {
        //Images could be taken even without viewfinder, if following is true:
        //( iInfo.iOptionsSupported & TCameraInfo::EImageCaptureSupported )
        User::Leave(KErrNotSupported);
        }
    }

/*
-------------------------------------------------------------------------------
Reserves camera, switches power on, and eventually starts view finding
-------------------------------------------------------------------------------
*/
void CCameraCaptureEngine::StartViewFinderL()
    {
//    __LOGSTR_TOFILE("start vf");
    ReserveCameraL();
    }


/*
-------------------------------------------------------------------------------
Returns the index for the requested image size, if supported by camera.
May leave with KErrNotSupported.
-------------------------------------------------------------------------------
*/
TInt CCameraCaptureEngine::GetImageSizeIndexL(const TSize& aRequestedSize,  // CSI: 40 # (Image size index is not an error code)
                                              const CCamera::TFormat& aFormat)
    {
    TCameraInfo camInfo;
    iCamera->CameraInfo(camInfo);

    for (TInt i=0; i< camInfo.iNumImageSizesSupported; i++)
        {
        TSize supportedSize;
        iCamera->EnumerateCaptureSizes(supportedSize, i, aFormat);
        if (supportedSize == aRequestedSize)
            {
            // Found the index
            return(i);
            }
        }
    // Index not found
    User::Leave(KErrNotSupported);
    return(0);
    }

/*
-------------------------------------------------------------------------------
Sets engine's camera orientation
-------------------------------------------------------------------------------
*/
void CCameraCaptureEngine::SetOrientation(TCameraOrientation anOrientation)
    {
    iOrientation = anOrientation;
    }

/*
-------------------------------------------------------------------------------
Destroys the JPEG encoder
-------------------------------------------------------------------------------
*/
void CCameraCaptureEngine::DeleteEncoder()
    {
    if (iEncoder)
        {
        delete iEncoder;
        iEncoder = NULL;
        }

    if (iBitmapSave)
        {
        iBitmapSave->Reset();
        }
    }

/*
-------------------------------------------------------------------------------
Reserves the camera.
"ReserveComplete" will be called after completion.
-------------------------------------------------------------------------------
*/
void CCameraCaptureEngine::ReserveCameraL()
    {
    iStart = ETrue;
    
    // Async. Will set iCameraReserved to ETrue on success.
    if (!iCameraReserved && iCamera)
        {
        iCamera->Reserve();
        }
    else if (iCameraReserved && iCamera)
        {
        if( iCapturePrepared && !iCamera->ViewFinderActive() )
            {
		if(iLogEnabled) 
			iLog.WriteFormat(_L("ReserveCameraL "));

            iCamera->StartViewFinderBitmapsL( iPortraitSize );
            iState = EEngineIdle;
            }
        }
    }

/*
-------------------------------------------------------------------------------
Releases the camera
-------------------------------------------------------------------------------
*/
void CCameraCaptureEngine::ReleaseCamera()
    {
    iStart = EFalse;
    if ( iCameraReserved )
        {
#ifdef _AUTOFOCUS_ON_
        if(iAutoFocus)
            {
            // bring AF subsystem to idle
            TRAP_IGNORE(iAutoFocus->ResetToIdleL());
            iAutoFocus->Close();
            }
#endif
        iCamera->Release();
        iCameraReserved = EFalse;
        }

    iCapturePrepared = EFalse;
    iState = EEngineNotReady;
    }

/*
-------------------------------------------------------------------------------
Switches off camera power
-------------------------------------------------------------------------------
*/
void CCameraCaptureEngine::PowerOff()
    {
    if ( !iPowering && iCameraReserved )
        {
        iCamera->PowerOff();
        ReleaseCamera();
        }
    }

/*
-------------------------------------------------------------------------------
Symbian Onboard Camera API observer. Gets called after CCamera::Reserve() is
called.
-------------------------------------------------------------------------------
*/
void CCameraCaptureEngine::ReserveComplete(TInt aError)
    {
    if ( aError )
        {
        iCameraReserved = EFalse;
        HandleError( aError );
        }
    else
        {
        iCameraReserved = ETrue;
        }

    if ( iStart )
        {
        iPowering = ETrue;
        iCamera->PowerOn();
        }
    else
        {
        ReleaseCamera();
        }
    }

/*
-------------------------------------------------------------------------------
Powering operation completes.
Symbian Onboard Camera API observer. Gets called after CCamera::PowerOn() is
called.
-------------------------------------------------------------------------------
*/
void CCameraCaptureEngine::PowerOnComplete(TInt aError)
    {
    HandleError( aError );
    iPowering = EFalse;
    if ( iStart ) //The Operation is not cancelled
        {
#ifdef _AUTOFOCUS_ON_
        // try to init the AF control - not fatal if fails
        if( iAutoFocus )
            {
            TRAPD( afErr, iAutoFocus->InitL( *this ) );
            if(afErr)
                {
                HandleError( KErrExtensionNotSupported );
                delete iAutoFocus;
                iAutoFocus = 0;
                }
            }

#endif
        // start viewfinder
        AsyncStateChange(EStartingViewFinder);
        }
    else
        {
        ReleaseCamera();
        }
    }

/*
-------------------------------------------------------------------------------
Switches off camera power.
Symbian Onboard Camera API observer. Gets called after StartViewFinderBitmapsL
is called.
-------------------------------------------------------------------------------
*/
//debug fps
TReal32 gTime = -1;
TReal32 gFNumb = 0;

void CCameraCaptureEngine::ViewFinderFrameReady(CFbsBitmap& aFrame)
    {
    if (!iController)
        {
        return;
        }

	gFNumb++;

	if(gTime < 0)
	{
		gTime =  User::NTickCount()*1.e-3;;
	}
	
	//debug fps


	if(gFNumb == 60 && iLogEnabled)
	{
		User::ResetInactivityTime();//shouldn't be commented!

		TReal32 currTime =  User::NTickCount()*1.e-3;
		TReal32 fps = gFNumb/(currTime - gTime);
		iLog.WriteFormat(_L("fps %f "), fps);

		TDisplayMode aMode = aFrame.DisplayMode();
		TSize aSize = aFrame.SizeInPixels();

		iLog.WriteFormat(_L(" mode %i w%i h %i "), aMode, aSize.iWidth, aSize.iHeight);

		gTime = User::NTickCount()*1.e-3;;
		gFNumb = 0;
	}


	iController->ProcessFrame(&aFrame);


 
    }

/*
-------------------------------------------------------------------------------
called when an image is ready
Symbian Onboard Camera API observer. Gets called after CCamera::CaptureImage()
is called.
-------------------------------------------------------------------------------
*/

void CCameraCaptureEngine::ImageReady(CFbsBitmap* aBitmap, HBufC8* aData,
                    TInt aError)
    {
//    __LOGSTR_TOFILE1("ImageReady: %d", aError);
    TInt err(KErrNone);

    if ( aError == KErrNone )
        {
        if( iFormat == CCamera::EFormatExif )
            {
            if ( iImageExif )
                {
                delete iImageExif;
                iImageExif = 0;
                }
            iImageExif = aData;
            // TODO: Post-exposure drawing for images
            // captured directly in EXIF JPEG format
            AsyncStateChange(EStartToSaveImage);
            }
 /*       else
            {

            iBitmapSave->Reset();
            if( aBitmap )
                {
                err = iBitmapSave->Duplicate( aBitmap->Handle() );
                if( err == KErrNone )
                    {
                    TRAP(err, DrawL()); // draw post-exposure image
                    }
                HandleError( err );
                }
            }
*/
        }

    else
        {
        HandleError( aError );
        }
    }


/*
-------------------------------------------------------------------------------
Called when initalising autofocus is ready
-------------------------------------------------------------------------------
*/
void CCameraCaptureEngine::InitComplete( TInt aError )
    {
//    __LOGSTR_TOFILE1("InitComplete: %d", aError);
    // TODO: Error handling
    }

/*
-------------------------------------------------------------------------------
Called when a framebuffer is ready.
Symbian Onboard Camera API observer. Gets called once
CCamera::StartVideoCapture() is called.
-------------------------------------------------------------------------------
*/
void CCameraCaptureEngine::FrameBufferReady(MFrameBuffer*  /*aFrameBuffer*/,
                      TInt /*aError*/)
    {
    // We are not using video capture
    }

/*
-------------------------------------------------------------------------------
Clips the viewfinding images according to portrait mode size
-------------------------------------------------------------------------------
*/
void CCameraCaptureEngine::ClipL(const CFbsBitmap& aFrame)
    {
    TSize size = aFrame.SizeInPixels();
    TInt x1 = (size.iWidth-iPortraitSize.iWidth)/2;
    TInt x2 = x1+iPortraitSize.iWidth;
    TInt y1 = (size.iHeight-iPortraitSize.iHeight)/2;
    TInt y2 = y1+iPortraitSize.iHeight;

    CFbsBitGc* fbsBitGc = CFbsBitGc::NewL(); //graphic context
    CleanupStack::PushL( fbsBitGc );
    CFbsBitmapDevice* portraitImageDevice =
        CFbsBitmapDevice::NewL( iBitmapPortraitVF );
    fbsBitGc->Activate( portraitImageDevice );
    fbsBitGc->SetBrushColor( KRgbBlack );
    fbsBitGc->Clear();

    fbsBitGc->BitBlt( TPoint(0,0), &aFrame, TRect(x1,y1,x2,y2) );

    delete portraitImageDevice;
    CleanupStack::PopAndDestroy(fbsBitGc);

    }

/*
-------------------------------------------------------------------------------
Calculates a portrait image size from a bigger snapped image while preserving
the aspect ratio
-------------------------------------------------------------------------------
*/
TRect CCameraCaptureEngine::Portrait( const CFbsBitmap* aBitmap)
    {
    TRect portrait = TRect();
    if ( aBitmap )
        {
        TSize size = aBitmap->SizeInPixels();
        TInt portx =
            iPortraitSize.iWidth * size.iWidth / iLandscapeSize.iWidth;
        TInt porty =
            iPortraitSize.iHeight * size.iHeight / iLandscapeSize.iHeight;
        TInt x1 = (size.iWidth-portx)/2;
        TInt x2 = x1+portx;
        TInt y1 = (size.iHeight-porty)/2;
        TInt y2 = y1+porty;
        portrait.SetRect(x1,y1,x2,y2);
        }
    return portrait;
    }

/*
-------------------------------------------------------------------------------
Handles error situations
-------------------------------------------------------------------------------
*/
void CCameraCaptureEngine::HandleError(TInt aError )
    {
    TInt reason(KErrNone);
    switch( aError )
        {
        case KErrNone:
            reason = KErrNone;
            break;
        case KErrNoMemory:
            iEikEnv->HandleError( aError );
            reason = ECameraOverflow;
            break;
        case KErrInUse:
            reason = ECameraInUse;
            iController->HandleError( aError );
            break;
        case KErrHardwareNotAvailable:
            iState = EEngineNoHardware;
            reason = ECameraHwFailure;
            break;
        case KErrExtensionNotSupported:     // AF not supported
            reason = KErrNone;
            iController->HandleError( aError );
            break;
        case KErrTimedOut:
            reason = ECameraOverflow;
            break;
        default:
            iEikEnv->HandleError( aError );
            reason = ECameraOverflow;
        }

    if ( reason )
        {
        if(iBitmapSave)
            iBitmapSave->Reset();
        delete iImageExif;
        iImageExif = 0;
        }
    }

/*
-------------------------------------------------------------------------------
Cancels the Active Object
-------------------------------------------------------------------------------
*/
void CCameraCaptureEngine::DoCancel()
    {
    if ( iState == EConvertingImage )
        {
        iEncoder->Cancel();
        DeleteEncoder();
        iFile.Close();  // Close file if open
        }
    }

/*
-------------------------------------------------------------------------------
Called when an asynchronous request is completed
-------------------------------------------------------------------------------
*/
void CCameraCaptureEngine::RunL(){
  if ( iStatus == KErrNone ){
    switch ( iState ){
      case EStartingViewFinder: {
          TRAPD( err, DoViewFinderL() );
                HandleError( err );
                if( err == KErrNone ){
                    iState = EEngineIdle;
				}
                break;
	  }

      // Conversion process ends
      case EConvertingImage:
        {
        iFile.Close(); // Close file if open
        if( iFormat == CCamera::EFormatExif )     // EXIF file has been saved
            {                                     // - ok to discard image data
            delete iImageExif;
            iImageExif = 0;
            }
        else
            DeleteEncoder();                      // release captured image file

        AsyncStateChange( EConverted );
        break;
        }

      default:
        break;
      }
    }
  else if (iStatus == KErrDiskFull)
    {
    // TODO: Disk full. Reset the engine (cf. selecting New Image).
    }
  else
    {
    // RunL called with status iStatus.Int()
    }

  }

/*
-------------------------------------------------------------------------------
Changes the engine state and completes an asynchronous request immediately
-------------------------------------------------------------------------------
*/
void CCameraCaptureEngine::AsyncStateChange( const TEngineState& aNextState )
    {
    TRequestStatus* status=(&iStatus);
    iState = aNextState;
    User::RequestComplete(status, KErrNone);
    SetActive();
    }

/*
-------------------------------------------------------------------------------
Returns the engine state
-------------------------------------------------------------------------------
*/
TEngineState CCameraCaptureEngine::GetEngineState()
  {
  return iState;
  }

/*
-------------------------------------------------------------------------------
Sets the engine state
-------------------------------------------------------------------------------
*/
void CCameraCaptureEngine::SetEngineState( TEngineState aState )
  {
  iState = aState;
  }

/*
-------------------------------------------------------------------------------
Returns whether the camera is being used by another application
-------------------------------------------------------------------------------
*/
TBool CCameraCaptureEngine::IsCameraUsedByAnotherApp()
  {
  return (!iCameraReserved);
  }


/*
-------------------------------------------------------------------------------
Returns whether viewfinder is currently active
-------------------------------------------------------------------------------
*/
TBool CCameraCaptureEngine::IsViewFinderActive()
  {
  return iCamera->ViewFinderActive();
  }


#ifdef _AUTOFOCUS_ON_
/*
-------------------------------------------------------------------------------
Returns whether AF is supported
-------------------------------------------------------------------------------
*/
TBool CCameraCaptureEngine::IsAutoFocusSupported()
  {
  return ( iAutoFocus != 0 );
  }

#endif
/*
-------------------------------------------------------------------------------
Notifies the engine if the client rect size changes
-------------------------------------------------------------------------------
*/
void CCameraCaptureEngine::ClientRectChangedL(const TRect& aRect)
    {
    if( iLandscapeSize == aRect.Size() )
      {
      return;
      }
    // The given client rect size is the same as the landscape picture size
    iLandscapeSize = aRect.Size();

    // In portrait mode the height is the same, but the width needs to be
    // calculated according to the aspect ratio
    iPortraitSize = TSize(
        (aRect.Size().iHeight * aRect.Size().iHeight / aRect.Size().iWidth),
        aRect.Size().iHeight);

    delete iBitmapPortraitVF;
    iBitmapPortraitVF = 0;
    iBitmapPortraitVF = new (ELeave) CWsBitmap( iEikEnv->WsSession() );
    User::LeaveIfError( iBitmapPortraitVF->Create( iPortraitSize,
                                                   iDisplayMode ) );

    }
// end of file
