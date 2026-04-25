/*
 * Copyright © 2008 Nokia Corporation.
 */

#include "CameraAppDocument.h"
#include "CameraAppAppUi.h"

/*
-------------------------------------------------------------------------------
Constructor
-------------------------------------------------------------------------------
*/
CCameraAppDocument::CCameraAppDocument(CEikApplication& aApp)
: CAknDocument(aApp)    
    {
    }

/*
-------------------------------------------------------------------------------
Destructor
-------------------------------------------------------------------------------
*/
CCameraAppDocument::~CCameraAppDocument()
    {
    }

/*
-------------------------------------------------------------------------------
Symbian OS 2nd phase constructor
	-------------------------------------------------------------------------------
*/
void CCameraAppDocument::ConstructL()
    {
    }

/*
-------------------------------------------------------------------------------
Two-phased constructor
	-------------------------------------------------------------------------------
*/
CCameraAppDocument* CCameraAppDocument::NewL( CEikApplication& aApp )
    {
    CCameraAppDocument* self = new (ELeave) CCameraAppDocument( aApp );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/*
-------------------------------------------------------------------------------
Constructs CCameraAppAppUi
-------------------------------------------------------------------------------
*/
CEikAppUi* CCameraAppDocument::CreateAppUiL()
    {
    return new (ELeave) CCameraAppAppUi;
    }
