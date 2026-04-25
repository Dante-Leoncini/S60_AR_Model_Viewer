/*
 * Copyright © 2008 Nokia Corporation.
 */

#include <aknviewappui.h>
#include <avkon.hrh>
#include <eikmenup.h>
#include <ARDemo.rsg>
#include "CameraAppView.h"
#include "CameraAppBaseContainer.h" 
#include "CameraApp.hrh"
#include "CameraAppController.h"
#include "CameraAppAppUi.h"
#include <aknappui.h>
#include "CGLengine.h"


CCameraAppView::CCameraAppView()
    {
    }

/*
-------------------------------------------------------------------------------
Symbian OS 2nd phase constructor
-------------------------------------------------------------------------------
*/
void CCameraAppView::ConstructL()
    {
    BaseConstructL( R_CAMERAAPP_VIEW1 );
    }


/*
-------------------------------------------------------------------------------
Two-phased constructor
-------------------------------------------------------------------------------
*/
CCameraAppView* CCameraAppView::NewLC()
    {
    CCameraAppView* self = new( ELeave ) CCameraAppView();
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;  
    }

/*
-------------------------------------------------------------------------------
Destructor. Frees allocated resources.
-------------------------------------------------------------------------------
*/
CCameraAppView::~CCameraAppView()
    {
    if ( iContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        }
		if(iContainer)
			delete iContainer;
    }

/*
-------------------------------------------------------------------------------
Returns the view ID
-------------------------------------------------------------------------------
*/
TUid CCameraAppView::Id() const
    {
    return KViewIdStandard;
    }


void CCameraAppView::HandleSizeChange( TInt aType )
    {
    if( iContainer )
        {
        iContainer->HandleResourceChange( aType );
        if ( aType==KEikDynamicLayoutVariantSwitch )
            {        
            TRect rect;
            AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, rect);
            iContainer->SetRect(rect);
            }
        }         
    }


/*
-------------------------------------------------------------------------------
Takes care of command handling
-------------------------------------------------------------------------------
*/
void CCameraAppView::HandleCommandL(TInt aCommand)
    {   
    switch ( aCommand )
        {
        case EAknSoftkeyOk:
            {
            break;
            }

        case EAknSoftkeyBack:
            {
            AppUi()->HandleCommandL(EEikCmdExit);
            break;
            }

        default:
            {
            AppUi()->HandleCommandL( aCommand );
            break;
            }
        }
    }

/*
-------------------------------------------------------------------------------
Called by the framework when view is activated
-------------------------------------------------------------------------------
*/
void CCameraAppView::DoActivateL(
    const TVwsViewId& /*aPrevViewId*/,TUid /*aCustomMessageId*/,
    const TDesC8& /*aCustomMessage*/)
    {
    CCameraAppController* controller = ((CCameraAppAppUi*)AppUi())->iController;
    controller->InitializeCameraL(TRect(0, 0, VFWIDTH,  VFHEIGHT));
    
    if (!iContainer)
        {
        iContainer = new (ELeave) CCameraAppBaseContainer(((CCameraAppAppUi*)AppUi())->iController);
        iContainer->SetMopParent( this );
        iContainer->ConstructL( TRect(0, 0, VFWIDTH,  VFHEIGHT)); 
        AppUi()->AddToStackL( *this, iContainer );
        }   
    iContainer->SetFocus( ETrue );

    controller->SetAppContainer(iContainer);
    controller->StartViewFinderL();
   }


/*
-------------------------------------------------------------------------------
Called by the framework when view is deactivated
-------------------------------------------------------------------------------
*/
void CCameraAppView::DoDeactivate()
    {
    if ( iContainer )
        {
        iContainer->SetFocus(EFalse);
        AppUi()->RemoveFromViewStack( *this, iContainer );
		if(iContainer)
			delete iContainer;
		iContainer = NULL;
        }    
    }

/*
-------------------------------------------------------------------------------
Gets called by the EIKON framework just before it displays a menu pane. Its
default implementation is empty, and by overriding it, the application can set
the state of menu items dynamically according to the state of application data.
-------------------------------------------------------------------------------
*/
void CCameraAppView::DynInitMenuPaneL(
    TInt aResourceId, CEikMenuPane* aMenuPane )
    {
/*
    CCameraAppController* controller = ((CCameraAppAppUi*)AppUi())->iController;
    
    if ( aResourceId == R_CAMERAAPP_APP_MENU )
        {
        // If image has been converted, then hide the "Snap" menu item
        TEngineState state = controller->GetEngineState();

        // If the engine is not in the idle state, bar the "Snap" function
        // from being accessed.
        if ( state != EEngineIdle || controller->IsCameraUsedByAnotherApp())
            aMenuPane->SetItemDimmed( ECameraAppCmdSnap, ETrue );

        // Only when the engine is in the converted state, show the "
        // "New image" menu item
        if ( state == EConverted )
            aMenuPane->SetItemDimmed( ECameraAppCmdNewImage, EFalse );
        else
            aMenuPane->SetItemDimmed( ECameraAppCmdNewImage, ETrue );
            
 //       if( !controller->IsAutoFocusSupported() )
 //           aMenuPane->SetItemDimmed( ECameraAppCmdFocusRange, ETrue );
        }  
*/
    }


/*
-------------------------------------------------------------------------------
Called by the framework when resource is changed
-------------------------------------------------------------------------------
*/
void CCameraAppView::HandleStatusPaneSizeChange()
    {
    CAknView::HandleStatusPaneSizeChange(); //call to upper class
    if (iContainer)
        iContainer->SetRect(TRect(0, 0, VFWIDTH,  VFHEIGHT));
    }

// End of file
