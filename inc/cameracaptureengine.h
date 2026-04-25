/*
 * Copyright � 2008 Nokia Corporation.
 */
 
#ifndef __CAMERA_CAPTURE_ENGINE_H__
#define __CAMERA_CAPTURE_ENGINE_H__

#include <f32file.h>
#include <e32std.h>
#include <gdi.h>
#include <ECam.h>

#include "define.h"

#ifdef _AUTOFOCUS_ON_
#include <ccamautofocus.h>
#endif

#include <flogger.h>

#include "CameraApp.hrh"
#include "CameraAppController.h"

//Color sampling used with the JPEG encoder
const TJpegImageData::TColorSampling KCameraColorSampling = 
    TJpegImageData::EColor420;

class CWsBitmap;

/**
 * Provides all still image releated methods.
 */
class CCameraCaptureEngine : public CActive, 
                             public MCameraObserver
#ifdef _AUTOFOCUS_ON_
                           ,public MCamAutoFocusObserver
#endif
    {
    public: // Constructors and destructor
        /**
         * Two-phased constructor.
         */
        static CCameraCaptureEngine* NewL( CCameraAppController* aController,
            const TRect& aRect );
        
        /**
         * Destructor.
         */
        virtual ~CCameraCaptureEngine();

    private:
        /**
         * Symbian OS default constructor.
         */
        CCameraCaptureEngine( CCameraAppController* aController );

        /**
         * Symbian OS constructor.
         */
       void ConstructL(const TRect& aRect);

    public: // New Functions

        /**
         * Checks whether the camera is being used by another application.
         * 
         * @return true if the camera is used by another app.
         */
        TBool IsCameraUsedByAnotherApp();
        
        /**
         * Checks whether the viewfinder is currently active.
         */
        TBool IsViewFinderActive();
        
        /**
         * Checks whether AF is supported.
         */
        TBool IsAutoFocusSupported();
        
        /**
         * Sets the engine state.
         * 
         * @param aState the new state
         */
        void SetEngineState( TEngineState aState );

        /**
         * Returns the engine state.
         */
        TEngineState GetEngineState();

        /**
         * Starts view finding.
         */
        void StartViewFinderL();

        /**
         * Stops view finding.
         */
        void StopViewFinder(); 

        /**
         * Destroys the JPEG encoder.
         */
        void DeleteEncoder();

        /**
         * Returns the default display mode.
         */
        TDisplayMode DisplayMode() const;

        /**
         * Sets the camera orientation.
         */
        void SetOrientation( TCameraOrientation anOrientation );
        
        /**
         * Reserves the camera.
         */
        void ReserveCameraL();

        /**
         * Releases the camera.
         */
        void ReleaseCamera();

        /**
         * Switches off the camera power.
         */
        void PowerOff();

        /**
         * Displays an error message.
         */
        void HandleError( TInt aError ); 
        
        /**
         * Notifies the engine if the client rect size changes.
         * 
         * @param the new client rect
         */
        void ClientRectChangedL(const TRect& aRect);

		void IncContrast();
		void DecContrast();

		TBool iLogEnabled;
		RFileLogger iLog;

    private:
    
        /**
         * Calculates portrait image size from bigger snapped image 
         * keeping the aspect ratio unchanged.
         * 
         * @return the portrait image size
         */
        TRect Portrait( const CFbsBitmap* aBitmap);
    
        /**
         * Returns camera image format to be used with current display mode.
         */
        CCamera::TFormat ImageFormat() const;

        /**
         * Returns highest color mode supported by the HW.
         */
        CCamera::TFormat ImageFormatMax() const;

        /**
         * Clips the viewfinding images according to portrait mode size.
         * 
         * @param A bitmap to be clipped
         */
        void ClipL(const CFbsBitmap& aFrame);
 

        /**
         * Starts view finding and prepares image capturing.
         */
        void DoViewFinderL(); 
        
        /**
         * Returns the index for the requested image size (if supported).
         */
         
        TInt GetImageSizeIndexL(const TSize& aRequestedSize, 
                                const CCamera::TFormat& aFormat);

        /**
         * From MCameraObserver.
         * Gets called when CCamera::Reserve() is completed.
         */
        virtual void ReserveComplete(TInt aError);

        /**
         * From MCameraObserver.
         * Gets called when CCamera::PowerOn() is completed.
         */
        virtual void PowerOnComplete(TInt aError);

        /**
         * From MCameraObserver.
         * Gets called when CCamera::StartViewFinderBitmapsL() is completed.
         */
        virtual void ViewFinderFrameReady(CFbsBitmap& aFrame);

        /**
         * From MCameraObserver.
         * Gets called when CCamera::CaptureImage() is completed.
         */
        virtual void ImageReady(CFbsBitmap* aBitmap,HBufC8* aData,TInt aError);

        /**
         * From MCameraObserver.
         * Gets called when CCamera::StartVideoCapture() is completed.
         */
        virtual void FrameBufferReady(MFrameBuffer* aFrameBuffer,TInt aError);
        
        /**
         * From MCamAutoFocusObserver.
         * Gets called when CCamAutoFocus::InitL() is completed.
         */
         virtual void InitComplete( TInt aError );
         
         /**
          * Changes the engine state and completes an asynchronous request
          * immediately.
          */
        void AsyncStateChange(const TEngineState& aNextState);

    private: // Functions from base CActive classes
        /**
         * From CActive.
         * Cancels the Active object. Empty.
         */
        void DoCancel();

        /**
         * From CActive.
         * Called when an asynchronous request has completed.
         */
        void RunL();

    private: //data
     
        CEikonEnv*                iEikEnv;            // Not owned
        CCamera*                  iCamera;
		TInt					  iContrast;
        TDisplayMode              iDisplayMode;
        CCamera::TFormat          iColorDepth;
        CCamera::TFormat          iColorDepthHW;
        CCameraAppController*     iController;
        TCameraInfo               iInfo;
        TInt                      iZoomFactor;
        TBool                     iCapturePrepared; 
        CWsBitmap*                iBitmapPortraitVF; 
        CFbsBitmap*               iBitmapSave;
        HBufC8*                   iImageExif;
        RFile                     iFile;
        CImageEncoder*            iEncoder;
        TBool                     iStart;
        TBool                     iCameraReserved;
        TSize                     iLandscapeSize;
        TSize                     iPortraitSize;
        TSize                     iCaptureSize;
        TCameraOrientation        iOrientation;
        TBool                     iPowering;
        TEngineState              iState;
		CCamera::TFormat            iFormat;
#ifdef _AUTOFOCUS_ON_        
       CCamAutoFocus*                  iAutoFocus;       
       CCamAutoFocus::TAutoFocusRange  iRange;
#endif
    };

#endif // __CAMERA_CAPTURE_ENGINE_H__
