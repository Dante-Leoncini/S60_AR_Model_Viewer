/*
 * Copyright © 2008 Nokia Corporation.
 */

#ifndef CAMERAAPPDOCUMENT_H
#define CAMERAAPPDOCUMENT_H

#include <akndoc.h>
   
class  CEikAppUi;

class CCameraAppDocument : public CAknDocument
    {
    public: // Constructors and destructor
        /**
         * Symbian OS Two-phased constructor.
         */
        static CCameraAppDocument* NewL(CEikApplication& aApp);

        /**
         * Destructor.
         */
        virtual ~CCameraAppDocument();

    private:
        /**
         * Symbian OS C++ default constructor.
         */
        CCameraAppDocument(CEikApplication& aApp);

        /** 
         * Symbian OS second phase constructor
         */
        void ConstructL();

    private:

        /**
         * From CEikDocument.
         * Create CCameraAppAppUi "App UI" object.
         */
        CEikAppUi* CreateAppUiL();
    };

#endif
