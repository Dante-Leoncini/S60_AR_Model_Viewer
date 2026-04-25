/*
 * Copyright © 2008 Nokia Corporation.
 */

#ifndef __CAMERAAPP_VIEW_H__
#define __CAMERAAPP_VIEW_H__

// INCLUDES
#include <aknview.h>


// CONSTANTS
// UID of view
const TUid KViewIdStandard = {1};

// FORWARD DECLARATIONS
class CCameraAppBaseContainer;
class CCameraAppController;

// CLASS DECLARATION

/**
*  CCameraAppView view class.
* 
*/
class CCameraAppView : public CAknView
    {
    public: // Constructors and destructor

        /**
         * Symbian OS two-phased constructor.	
         */
        static CCameraAppView* NewLC();

        /**
         * Destructor.
         */
        ~CCameraAppView();

    private:
        /**
         * Symbian OS default constructor.
         */
        void ConstructL();

        /**
         * Symbian OS default constructor.
         */
        CCameraAppView();

    public: // Functions from base classes
	
        /**
         * From CAknView.
         * Returns the uid of the View.
         * 
         * @return uid of the view
         */
        TUid Id() const;

        /**
         * From MEikMenuObserver.
         * Handles commands from the menu.
         * 
         * @param aCommand a command emitted by the menu 
         */
        void HandleCommandL(TInt aCommand);

        void HandleSizeChange( TInt aType );
        
        /**
         * From MEikMenuObserver.
         */
        void DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane);

        /**
         * Called by the framework when status pane size changes.
         */
        void HandleStatusPaneSizeChange();

    private:

        /**
         * From CAknView.
         * Activates the view.
         * 
         * @param aPrevViewId 
         * @param aCustomMessageId 
         * @param aCustomMessage 
         */
        void DoActivateL(const TVwsViewId& aPrevViewId,TUid aCustomMessageId,
            const TDesC8& aCustomMessage);

        /**
         * From CAknView.
         * Deactivates the view (frees resources).
         */
        void DoDeactivate();

    public: // Data
        CCameraAppBaseContainer*    iContainer;

    };

#endif // __CAMERAAPP_VIEW_H__
