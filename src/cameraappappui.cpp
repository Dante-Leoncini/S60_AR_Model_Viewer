#include <avkon.hrh>
#include <eikmenup.h>

#include <ARDemo.rsg>

#include <apgcli.h>
#include <hal.h>
#include <hlplch.h>  // HlpLauncher
#include <aknnotewrappers.h> 

#include <centralrepository.h>
#include <ProfileEngineSDKCRKeys.h>


#include <stringloader.h>

#include "CameraAppAppUi.h"
#include "CameraAppView.h"
#include "Cameraapp.hrh"
#include "CameraAppController.h"
#include "CameraAppBaseContainer.h"
#include "CameraApp.pan"
#include "cameraappapp.h"

#include "../Whisk3D/ImportOBJ.h"


/*
-------------------------------------------------------------------------------
Constructor. Initializes member variables.
-------------------------------------------------------------------------------
*/
CCameraAppAppUi::CCameraAppAppUi()
: CAknViewAppUi(),
  iController( 0 )
  {
  }

/*
-------------------------------------------------------------------------------
Symbian OS 2nd phase constructor
-------------------------------------------------------------------------------
*/
void CCameraAppAppUi::ConstructL(){
    // Try to resolve the best orientation for the app
    BaseConstructL(EAknEnableSkin|ResolveCameraOrientation());

    // Create a camera controller
    iController = new (ELeave) CCameraAppController(*this);
    iController->ConstructL();
	iController->SetCameraOrientation(ECameraLandscape);
    
    iView = CCameraAppView::NewLC();
    AddViewL( iView );      // transfer ownership to CAknViewAppUi
    CleanupStack::Pop(iView);
    SetDefaultViewL(*iView);

    // Make this class observe changes in foreground events
    iEikonEnv->AddForegroundObserverL( *this );
    
    // Default mode is viewfinding
    iCameraMode = EViewFinding;
}

void CCameraAppAppUi::HandleGainingForeground()
    {
    // Application gets focused so reserve the camera
    // if it was disabled (ResetEngine() called)
    if (!iController)
        {
        ResetEngine();
        }
    }

void CCameraAppAppUi::HandleLosingForeground(){
    // Application loses focus so release the camera
    if (iController){
        // Delete old controller and capture engine
        delete iController;
        iController = NULL;
	}
}

/*
-------------------------------------------------------------------------------
Reset controller and capture engine
-------------------------------------------------------------------------------
*/
void CCameraAppAppUi::ResetEngine()
    {

    // Delete old controller and capture engine
    if (iController)
        {
        delete iController;
        iController = NULL;
        }

    // Create new controller and capture engine
    iController = new (ELeave) CCameraAppController(*this);
    iController->ConstructL();

     // Set new controller pointers
    iView->iContainer->SetController(iController);
    iController->SetAppContainer(iView->iContainer);
   
    // Check orientation
    CAknAppUiBase::TAppUiOrientation orientation = Orientation();
    if (orientation == CAknAppUiBase::EAppUiOrientationPortrait)
        {
        iController->SetCameraOrientation(ECameraPortrait);
        }
    else
        {
        iController->SetCameraOrientation(ECameraLandscape);
        }

    iController->SkipFrames(KNumOfFrames);
    iController->InitializeCameraL( ClientRect() );
    iController->StartViewFinderL();

    iCameraMode = EViewFinding;

    }


/*
-------------------------------------------------------------------------------
Destructor. Frees reserved resources.
-------------------------------------------------------------------------------
*/
CCameraAppAppUi::~CCameraAppAppUi()
    {
    if (iController)
        delete iController;
    }

void CCameraAppAppUi::HandleResourceChangeL( TInt aType )
    {
    CAknAppUi::HandleResourceChangeL( aType );

    if ( aType==KEikDynamicLayoutVariantSwitch )
        {
        static_cast<CCameraAppView*>( View( TUid::Uid( EViewIdStandard ) ) )->HandleSizeChange( aType );

        // Reset engine if app is in foreground. If not ResetEngine() will be
        // called when app is coming to foreground later.
        if (IsForeground())
            {
            ResetEngine();
            }
        }  
    }

/*
-------------------------------------------------------------------------------
Takes care of key event handling
-------------------------------------------------------------------------------
*/
TKeyResponse CCameraAppAppUi::HandleKeyEventL(
    const TKeyEvent& aKeyEvent,TEventCode aType)
    {
    if (!iController)
      {
      return EKeyWasNotConsumed;
      }

    if (iController->IsCameraUsedByAnotherApp())
      {
      return EKeyWasNotConsumed;
      }

	/*case EKeyOK:
		{
		//DoSnapL();
		return EKeyWasConsumed;
		}
	default:
		break;
	}
*/
    return EKeyWasNotConsumed;
}

/*
-------------------------------------------------------------------------------
Takes care of command handling
-------------------------------------------------------------------------------
*/
void CCameraAppAppUi::HandleCommandL(TInt aCommand){
    switch ( aCommand )
        {
    	case EImportOBJ:
    		ImportOBJ();
    		break;
        case EAknSoftkeyExit:
        case EAknSoftkeyBack:
        case EEikCmdExit: {
            Exit();
            break;
		}
        default:
            break;
        }
}

/*
-------------------------------------------------------------------------------
Returns the camera mode
-------------------------------------------------------------------------------
*/
TCameraMode CCameraAppAppUi::CameraMode()
    {
    return iCameraMode;
    }

/*
-------------------------------------------------------------------------------
Starts the viewfinder after taking a picture. Also updates the navi tabs.
-------------------------------------------------------------------------------
*/
void CCameraAppAppUi::DoNewImageL()
    {
    // Redisplay the default navi pane
    iController->RedrawNaviPaneL();

    //Start the viewfinder
    iController->StartViewFinderL();

    iCameraMode = EViewFinding;
    }

/*
-------------------------------------------------------------------------------
Returns KAppOrientationLandscape for devices where the camera is designed to
work in landscape orientation, 0 for others.
-------------------------------------------------------------------------------
*/
TUint32 CCameraAppAppUi::ResolveCameraOrientation()
    {
    TUint32 ret = 0;
    TInt mUid;
    HAL::Get(HAL::EMachineUid, mUid);

    // for S60 3rd Edition devices (MachineUID == 0x20??????), assume landscape
    if( mUid > 0x20000000 )
        {
        ret = EAppOrientationLandscape;
        }

    // ...with these exceptions:
    switch( mUid )
        {
        case 0x20000602: // 5500
        case 0x20000606: // 6290
        case 0x20002495: // E50
        case 0x20002D7F: // E61i
        case 0x20000604: // E65
        case 0x20001857: // E70 (orientation follows cover position)
        case 0x200005FF: // N71
        case 0x200005FC: // N91
            {
            ret = 0;
            break;
            }
        case 0x10200F98: // N90 (2nd Ed, FP3)
            {
            ret = EAppOrientationLandscape;
            break;
            }
        default:
            {
            break;
            }
        }

    return ret;
    }

// end of file

