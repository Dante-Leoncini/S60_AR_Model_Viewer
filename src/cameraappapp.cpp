/*
 * Copyright © 2008 Nokia Corporation.
 */

#include "CameraAppApp.h"
#include "CameraAppDocument.h"
#include <eikstart.h>

/*
-------------------------------------------------------------------------------
Returns application UID
-------------------------------------------------------------------------------
*/
TUid CCameraAppApp::AppDllUid() const
    {
    return KUidCameraApp;
    }


/*
-------------------------------------------------------------------------------
Creates CCameraAppDocument object
    -------------------------------------------------------------------------------
*/
CApaDocument* CCameraAppApp::CreateDocumentL()
    {
    return CCameraAppDocument::NewL( *this );
    }


/*
-------------------------------------------------------------------------------
Constructs CCameraAppApp
    -------------------------------------------------------------------------------
*/
EXPORT_C CApaApplication* NewApplication()
    {
    return new CCameraAppApp;
    }

/*
-------------------------------------------------------------------------------
Entry point function for Symbian Apps
-------------------------------------------------------------------------------
*/
GLDEF_C TInt E32Main()
    {
    return EikStart::RunApplication( NewApplication );
    }
// End of File  
