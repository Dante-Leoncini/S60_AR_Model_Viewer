#include <AknViewAppUi.h>
#include <ARDemo.rsg>
#include <bautils.h>
#include <PathInfo.h>
#include <sysutil.h> // SysUtil

#include "CameraAppController.h"
#include "CameraAppBaseContainer.h"
#include "CameraAppView.h"
#include "CameraAppAppUi.h"
#include "CameraCaptureEngine.h"
#include "CGLEngine.h"
#include "vtracker.h"

// Constants
const TInt KFileNameIndexMaxLength = 5;

// For 2nd Edition emulator only
#ifdef __WINS__
_LIT(KEmulatorPath, "\\system\\Apps\\ARDemo\\");
#endif

/*
-------------------------------------------------------------------------------
C++ default constructor
-------------------------------------------------------------------------------
*/
CCameraAppController::CCameraAppController(CAknViewAppUi& aAppUi)
    :iAppUi( aAppUi ),
    iContainer( 0 )
    {
    }

/*
-------------------------------------------------------------------------------
Destructor. Frees allocated resources.
-------------------------------------------------------------------------------
*/
CCameraAppController::~CCameraAppController(){
    // Power off the camera
    PowerOff();
	iLight->ReleaseLight(CHWRMLight::EPrimaryDisplay);
	delete iLight;

    delete iCameraCaptureEngine;
    delete iBitmapSnap;
    delete iImagePath;
    delete iPath;
}

/*
-------------------------------------------------------------------------------
Symbian OS 2nd phase constructor
-------------------------------------------------------------------------------
*/
void CCameraAppController::ConstructL()
    {
    iEikEnv = CEikonEnv::Static();  // CSI: 27 # (Store a pointer to CEikonEnv)
    iPath = new(ELeave) TFileName;

    TFileName path = PathInfo::PhoneMemoryRootPath(); //Default
    path.Append( PathInfo::ImagesPath() );
    iImagePath = HBufC::NewL( path.Length() );
    iImagePath->Des().Copy( path );

	iLight = CHWRMLight::NewL(this);
// aRestoreState = ETrue means that any previously frozen state will be restored.
// aForceNoCCoeEnv = EFalse means that the CCoeEnv background/foreground status
// is always used to control further reservations.
	iLight->ReserveLightL(CHWRMLight::EPrimaryDisplay , ETrue, EFalse );
	iLight->LightOnL(CHWRMLight::EPrimaryDisplay);

    }

/*
-------------------------------------------------------------------------------
Powers off the camera
-------------------------------------------------------------------------------
*/
void CCameraAppController::PowerOff()
    {
    iCameraCaptureEngine->PowerOff();
    }

/*
-------------------------------------------------------------------------------
Gets the camera orientation
-------------------------------------------------------------------------------
*/
TCameraOrientation CCameraAppController::CameraOrientation() const
    {
    return iCameraOrientation;
    }

/*
-------------------------------------------------------------------------------
Sets the camera orientation
-------------------------------------------------------------------------------
*/
void CCameraAppController::SetCameraOrientation(
                            TCameraOrientation aCameraOrientation)
    {
    iCameraOrientation = aCameraOrientation;
    }


/*
-------------------------------------------------------------------------------
Sets the capture mode, used for displaying information about image size/format
-------------------------------------------------------------------------------
*/
void CCameraAppController::SetCaptureModeL(const TSize& aSize,
                                           TInt aFormat,
                                           TBool aAFSupported){
    iCaptureFormat = aFormat;
    const TInt KBufferSize = 32;
    TBuf<KBufferSize> text;
    _LIT(KFormatText, "[%1.1fM]");
    const TInt KMega = 1000000;
    text.Format(KFormatText, TReal(aSize.iWidth * aSize.iHeight) / KMega );

    if( iCaptureFormat == CCamera::EFormatExif ){
        _LIT(KExifText, "EXIF");
        text.Append(KExifText);
	}
}

/*
-------------------------------------------------------------------------------
Returns the default display mode
-------------------------------------------------------------------------------
*/
TDisplayMode CCameraAppController::DisplayMode() const
    {
    return iCameraCaptureEngine->DisplayMode();
    }

/*
-------------------------------------------------------------------------------
Initializes still image capture engine
-------------------------------------------------------------------------------
*/
void CCameraAppController::InitializeCameraL(const TRect& aRect)
    {
    if (!iCameraCaptureEngine)
        {
        iCameraCaptureEngine = CCameraCaptureEngine::NewL( this, aRect );

		iLog = &(iCameraCaptureEngine->iLog);
        //Create snap image
        iBitmapSnap = new (ELeave) CWsBitmap(iEikEnv->WsSession());
        User::LeaveIfError( iBitmapSnap->Create(aRect.Size(), DisplayMode()));
        }
    }

void CCameraAppController::IncContrast()
{
	iCameraCaptureEngine->IncContrast();
}

void CCameraAppController::DecContrast()
{
	iCameraCaptureEngine->DecContrast();
}



/*
-------------------------------------------------------------------------------
Starts the view finding operation
-------------------------------------------------------------------------------
*/
void CCameraAppController::StartViewFinderL()
    {
    if (iCameraCaptureEngine)
        {
        if(iCameraCaptureEngine->GetEngineState() == EEngineNoHardware)
            {
            HandleError( KErrHardwareNotAvailable );
            return;
            }

        if(iCameraCaptureEngine->IsViewFinderActive())
            return;

        iCameraCaptureEngine->SetOrientation(iCameraOrientation);
        iCameraCaptureEngine->StartViewFinderL();
        }
    }


/*
-------------------------------------------------------------------------------
Gives to Controller the reference to Active view container
-------------------------------------------------------------------------------
*/
void CCameraAppController::SetAppContainer( CCameraAppBaseContainer* aAppContainer)
    {
    iContainer = aAppContainer;
    }

/*
-------------------------------------------------------------------------------
Returns the complete path of the saved image
-------------------------------------------------------------------------------
*/
const TDesC& CCameraAppController::ImagePath() const
    {
    return *iImagePath;
    }

/*
-------------------------------------------------------------------------------
Set the actual bitmap size
-------------------------------------------------------------------------------
*/
void CCameraAppController::SetActualVFSize( const TSize& /*aSize*/ )
    {
    if ( iContainer )
        {
        iContainer->ReDrawOffScreenBitmap( );
        }
    }

/*
-------------------------------------------------------------------------------
Gets the image from image capture engine and displays it on the screen.
Called by the capture engine to display the bitmap.
-------------------------------------------------------------------------------
*/

void CCameraAppController::ViewFinding( CFbsBitmap& aFrame )
    {
    // skip frames if it was requested
    if ( iSkipFrames )
      {
      --iSkipFrames;
      return;
      }
    if ( !iContainer )
        {
        return;
        }
	if (iContainer && iContainer->iGLEngine)
        {
			iContainer->DrawImageNow( aFrame );
        }
   }

void CCameraAppController::ProcessFrame(CFbsBitmap* pFrame)
{
	if (iContainer && iContainer->iGLEngine)
    {
		iContainer->ProcessFrame(pFrame);
    }
};


/*
-------------------------------------------------------------------------------
Stops view finding
-------------------------------------------------------------------------------
*/
void CCameraAppController::StopViewFinder()
    {
    if (iCameraCaptureEngine)
        iCameraCaptureEngine->StopViewFinder();
    }

/*
-------------------------------------------------------------------------------
Shows the status of image conversion process
-------------------------------------------------------------------------------
*/
void CCameraAppController::ShowConversionStatusL( const TDesC& /*aStatus*/,
                                                  TBool /*aShowFileName*/ )
    {
    if( !iContainer )
        return;
    }

/*
-------------------------------------------------------------------------------
Redraw navi tabs
-------------------------------------------------------------------------------
*/
void CCameraAppController::RedrawNaviPaneL()
    {
    if ( iCameraCaptureEngine )
        {
        iCameraCaptureEngine->SetEngineState( EEngineIdle );
        }
    }

/*
-------------------------------------------------------------------------------
Returns the engine state
-------------------------------------------------------------------------------
*/
TEngineState CCameraAppController::GetEngineState()
    {
    return iCameraCaptureEngine->GetEngineState();
    }

/*
-------------------------------------------------------------------------------
Sets the engine state
-------------------------------------------------------------------------------
*/
void CCameraAppController::SetEngineState( TEngineState aState )
    {
    iCameraCaptureEngine->SetEngineState( aState );
    }

/*
-------------------------------------------------------------------------------
Gets the next usable file name
-------------------------------------------------------------------------------
*/
void CCameraAppController::GetNextUsableFileName()
    {
    TInt index = 0;

    do {
        iNewFileName.Copy( iImagePath->Des() );
        iNewFileName.Append( KImageFileName );

        TBuf<KFileNameIndexMaxLength> num;
        num.Num( index );
        iNewFileName.Append( num );

        iNewFileName.Append( KJpgFileExtension );
        if ( !BaflUtils::FileExists( iEikEnv->FsSession(),
            iNewFileName ) )
            break;

        index ++;
        } while ( 1 );
    }

/*
-------------------------------------------------------------------------------
Handles the error messages from the capture engine
-------------------------------------------------------------------------------
*/
void CCameraAppController::HandleError(TInt aError)
    {
    switch( aError )
        {
        case KErrNone:
            break;

        case KErrExtensionNotSupported:  // used when AF is not supported
            break;
        }
    }

/*
-------------------------------------------------------------------------------
Returns whether the camera is being used by another application
-------------------------------------------------------------------------------
*/
TBool CCameraAppController::IsCameraUsedByAnotherApp()
    {
    // return true if camera engine is not yet constructed
    if( !iCameraCaptureEngine )
      return ETrue;

    return iCameraCaptureEngine->IsCameraUsedByAnotherApp();
    }

/*
-------------------------------------------------------------------------------
Notifies the controller if the client rect size changes
-------------------------------------------------------------------------------
*/
void CCameraAppController::ClientRectChangedL(const TRect& aRect)
    {
    if( !iCameraCaptureEngine )
        return;

    iCameraCaptureEngine->ClientRectChangedL(aRect);
    }
