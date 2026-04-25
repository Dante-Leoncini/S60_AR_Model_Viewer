#ifndef __CAMERA_CONTROLLER_H__
#define __CAMERA_CONTROLLER_H__

#include <e32base.h>
#include <eikapp.h>

#include <avkon.hrh>
#include <avkon.rsg>
#include <StringLoader.h>
#include <flogger.h>

#include <AknViewAppUi.h>
#include <HWRMLight.h> //light

#define __WITHIN_MCI_LIBRARY 
#include <ImageConversion.h> 

#include "define.h"

#include <f32file.h>
#include "CameraApp.hrh"

class TVolumeInfo;
class TRequestStatus;
class TSysAgentEvent;
class CCoeControl;

class CCameraCaptureEngine;
class CPAlbImageData;
class CFbsBitmap;
class CAknKeySoundSystem;
class CCameraAppBaseContainer;

const TInt KWhiteColor = 0;
_LIT(KJpgFileExtension, ".jpg");
_LIT8(KMimeType,"image/jpeg");

// The name for the image file.
_LIT(KImageFileName, "CamApp");

#ifndef __SERIES60_3X__
const TInt KErrExtensionNotSupported = -47;
#endif

/**
 * Interface between the UI and the external modules.
 */
class CCameraAppController : public CBase, public MHWRMLightObserver    
    {
    public: // Constructor and Destructor

        /**
         * Symbian OS C++ default constructor.
         */
        CCameraAppController(CAknViewAppUi& aAppUi); 

        /**
         * Destructor.
         */
        virtual ~CCameraAppController();

        /**
         * Symbian OS second phase constructor.
         */
        void ConstructL();  

    public:
        /**
         * Sets the zoom factor.
         * 
         * @param aEnable enable the zooming
         *        true  == increase the zoom factor
         *        false == decrease the zoom factor
         * @return zoom factor used
         */
        TInt SetZoomL(TBool aEnable);

        /**
         * Checks whether the camera is being used by another application.
         * 
         * @return, true if the camera is used by another application.
         */
        TBool IsCameraUsedByAnotherApp();

        /**
         * Handles the error message from the engine.
         * 
         * @param aError an error message to be handled
         */
        void HandleError(TInt aError);

        /**
         * Sets the engine state.
         * 
         * @param aState new state of the engine
         */
        void SetEngineState( TEngineState aState );

        /**
         * Returns the engine state.
         * 
         * @return the current engine state
         */
        TEngineState GetEngineState();

        /**
         * Redraws the navi pane.
         */
        void RedrawNaviPaneL();

        /**
         * Updates the conversion status.
         * 
         * @param aStatus the status to be shown
         */
        void ShowConversionStatusL(const TDesC &aStatus, 
            TBool aShowFileName = EFalse);

        /**
         * Sets the camera orientation.
         * 
         * @param aCameraOrientation the new camera orientation
         */
        void SetCameraOrientation(TCameraOrientation aCameraOrientation);
        
        /**
         * Sets the capture mode used for displaying information about
         * capture mode/capabilities.
         * 
         * @param aSize image size in pixels
         * @param aFormat capture format used (CCamera::TFormat)
         * @param aAFSupported is autofocus supported?
         */
        void SetCaptureModeL(const TSize& aSize, TInt aFormat, TBool aAFSupported);
        
        /**
         * Checks if AF is supported.
         */
#ifdef _AUTOFOCUS_ON_
        TBool IsAutoFocusSupported();
        /**
         * Sets the AF range.
         * 
         * @param aRange the new range
         */

        void SetFocusRangeL(CCamAutoFocus::TAutoFocusRange aRange);
#endif
        /**
         * Returns the snapped image.
         * 
         * @return the snapped image
         */
        CFbsBitmap& GetSnappedImage();
        
        /**
         * Checks if there is not enough free space left on drive C.
         * 
         * @return ETrue if there is not enough free space
         */
        TBool DiskSpaceBelowCriticalLevel();

        /**
         * Returns camera orientation setting.
         * 
         * @return current camera orientation
         */
        TCameraOrientation CameraOrientation() const;

        /**
         * Gives to Controller a reference to Active view container
         * 
         * @param aAppContainer reference to view container
         * @param aViewId active view Id 
         */
        void SetAppContainer(CCameraAppBaseContainer* aAppContainer);

        /**
         * Initializes the camera.
         * 
         * @param aRect
         */
        void InitializeCameraL(const TRect& aRect);       

        /**
         * Stops view finding.
         */
        void StopViewFinder();

        /**
         * Starts the Active Objet that calls ViewFinding operation regularly
         */        
        void StartViewFinderL();

        /**
         * Gets an image from image capture engine, and displays it on screen
         * 
         * @param aFrame
         */
        void ViewFinding(CFbsBitmap& aFrame);

		void ProcessFrame(CFbsBitmap* aFrame);
        
        /**
         * Returns the complete path of the saved image.
         * 
         * @return the complete path and name of the last saved image
         */
        const TDesC& ImagePath() const;
   
        /**
         * Returns the default display mode.
         * 
         * @return the default display mode
         */
        TDisplayMode DisplayMode() const;

        /**
         * Bitmaps will be as close as possible to requested size but may not
         * be exactly if only certain sizes are supported. The actual size
         * used will be set in aSize.
         * 
         * @param aSize the size of the image
         */
        void SetActualVFSize(const TSize& aSize);
        
        /**
         * Notifies the controller if the client rect size changes.
         * 
         * @param aRect the new client rect
         */
        void ClientRectChangedL(const TRect& aRect);
        
        /**
         * Skips a specified number of viewfinder frames.
         * (Used for avoiding display mess when changing layout.)
         * 
         * @param aNumber the number of frames to skip
         */
        void SkipFrames( TUint aNumber ) { iSkipFrames = aNumber; }
        
        /**
         * Returns true if frames are being currently skipped
         */
        TBool IsSkippingFrames() const { return (TBool)iSkipFrames; }

		void IncContrast();
		void DecContrast();
//debug move ?
        CCameraAppBaseContainer*              iContainer; // Not owned
		RFileLogger* iLog;
	public:
         // from MHWRMLightObserver
        virtual void LightStatusChanged(TInt aTarget, 
			CHWRMLight::TLightStatus aStatus){};

    private:
        CHWRMLight* iLight;
     
    private: 
        // From MMdaAudioPlayerCallback
        void MapcInitComplete(TInt aError, const TTimeIntervalMicroSeconds &aDuration);
        void MapcPlayComplete(TInt aError);
    
        /**
         * Powers off the camera.
         */
        void PowerOff();

        /**
         * Returns the next usable file name.
         */
        void GetNextUsableFileName();

    private: // data
        CAknViewAppUi&            iAppUi; // Not owned
        TCameraOrientation        iCameraOrientation;
        CCameraCaptureEngine*     iCameraCaptureEngine;
        CFbsBitmap*               iBitmapSnap;
        HBufC*                    iImagePath;
        TFileName*                iPath;


        CEikonEnv*                iEikEnv; // Not owned
        TFileName                 iNewFileName;
        TInt                      iCaptureFormat;
        TUint                     iSkipFrames;
    };

#endif // __CAMERA_CONTROLLER_H__
