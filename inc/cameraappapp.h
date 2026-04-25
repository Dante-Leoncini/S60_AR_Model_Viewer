/*
 * Copyright © 2008 Nokia Corporation.
 */

#ifndef __CAMERAAPP__APP_H__
#define __CAMERAAPP__APP_H__

#include <aknapp.h>

// UID of the application
const TUid KUidCameraApp = { 0xA000FFDF };
const TUid KUidHelpFile = {0x2000E191};  // From help rtf file

/**
 * CCameraAppApp application class.
 * Provides factory to create concrete document object.
 */
class CCameraAppApp : public CAknApplication
    {
    private:

        /**
         * From CApaApplication. Creates CCameraAppDocument document object.
         * @return A pointer to the created document object.
         */
        CApaDocument* CreateDocumentL();

        /**
         * From CApaApplication. Returns application's UID (KUidCameraApp).
         * @return The value of KUidCameraApp.
         */
        TUid AppDllUid() const;
    };

#endif // __CAMERAAPP__APP_H__
