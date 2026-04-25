#ifndef __CAMERAAPP_APPUI_H__
#define __CAMERAAPP_APPUI_H__

#include <eikapp.h>
#include <eikdoc.h>
#include <e32std.h>
#include <coeccntx.h>
#include <aknviewappui.h>

#include "Cameraapp.hrh"

class CCameraAppContainer;
class CCameraAppController;
class CCameraAppView;

const TInt KNumOfFrames = 5;

/**
 * Application UI class.
 * Provides support for the following features:
 * - EIKON control architecture
 * - view architecture
 * - status pane
 */
class CCameraAppAppUi : public CAknViewAppUi,
                        public MCoeForegroundObserver
    {
    public: // Constructors and destructor

        /**
         * Constructor.
         */
        CCameraAppAppUi();

        /**
         * Symbian OS 2nd phase constructor.
         */      
        void ConstructL();

        /**
         * Destructor.
         */      
        ~CCameraAppAppUi();

    public: // Public methods

        void ResetEngine();
        
        void DoNewImageL();
    
        /**
         * Returns the camera mode.
         */
        TCameraMode CameraMode();

    private:
        /**
         * From CEikAppUi.
         * Takes care of command handling.
         * 
         * @param aCommand command to be handled
         */
        void HandleCommandL(TInt aCommand);
        void HandleResourceChangeL( TInt aType );
        /**
         * From CEikAppUi.
         * Handles key events.
         * 
         * @param aKeyEvent Event to be handled.
         * @param aType Type of the key event. 
         * @return Response code (EKeyWasConsumed, EKeyWasNotConsumed). 
         */
        virtual TKeyResponse HandleKeyEventL(
            const TKeyEvent& aKeyEvent,TEventCode aType);            

    private:
        TUint32 ResolveCameraOrientation();
        
        
    private: // From MCoeForegroundObserver
        void HandleGainingForeground();
        void HandleLosingForeground();

    public:  // Data
        CCameraAppController*           iController;
    
    private: // Data
        CCameraAppView*                 iView;
        // Whether the camera is in viewfinding mode or in the 
        // image snapped mode
        TCameraMode iCameraMode;
        
        TInt32 iCameraKeyHandle;
    };

#endif // __CAMERAAPP_APPUI_H__
