#ifndef __CAMERAAPP_BASE_CONTAINER_H__
#define __CAMERAAPP_BASE_CONTAINER_H__

#include <coecntrl.h>
#include <akntabobserver.h>
#include <aknutils.h>
#include <flogger.h>
#include <e32base.h>
#include <s32file.h>

class CFbsBitmap;
class CWsBitmap;
class CFbsBitGc;
class CFbsBitmapDevice;
class CCameraAppController;
class CAknNavigationDecorator;
class CGLengine;
class VTracker;
class CPhEngine;
class TDisplayStat;


/**
 * Container control class.
 */
class CCameraAppBaseContainer : public CCoeControl
    {
    public: // Constructors and destructor
        
        /**
         * Symbian OS second phase constructor.
         * 
         * @param aRect Frame rectangle for container.
         */
        void ConstructL(const TRect& aRect);

        /**
         * Constructor.
         * 
         * @param aController Camera controller
         */
        CCameraAppBaseContainer(CCameraAppController*& aController);

        /**
         * Destructor.
         */
        virtual ~CCameraAppBaseContainer();
		TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);

    public: // New functions
        /**
         * Draws the bitmap image immediately.
         * 
         * @param aBitmap bitmap to be displayed
         */
        void DrawImageNow(CFbsBitmap& aBitmap);

        /**
         * Redraws the offscreen bitmap.
         */
        void ReDrawOffScreenBitmap();


    public:  // Functions from base classes
        void SetController(CCameraAppController*& aController);

        /**
         * Shows an error message on the screen.
         * 
         * @param aMsg the message to be displayed
         */
        void ShowErrorMessage( const TDesC &aMsg );

        /**
         * From CCoeControl.
         * Handles changes to the control's resources.
         *
         * @param aType A message UID value        
         */
        void HandleResourceChange( TInt aType );

		void ProcessFrame(CFbsBitmap* pFrame);
        
    protected: // New protected functions
        /** 
         * Draws a bitmap onto the real screen.
         */
        void DrawImage(CWindowGc& aGc, const TRect& aRect) const;

        
    private: // New function
        /**
         *  Creates the offscreen bitmap.
         */
        void CreateOffScreenBitmapL();
        
    private: // Functions from base classes         

        /**
         * From CoeControl.
         * Called when this control's rect changes.
         */
        void SizeChanged();

        /**
         * From CCoeControl.
         */
        void Draw(const TRect& aRect) const;                

    public:
		RFileLogger* iLog;
        TBool iOffScreenBitmapCreated;
		CGLengine*	iGLEngine;
		VTracker*	iTracker;
		CPhEngine*	iPhEngine;
		TDisplayStat* iStat;
		TBool		iDoOneTimeIni;
		TBool		iFreeze;
		TBool		iKeyPressed;
		TInt		iLastKey;

		RFs* m_fileSession;
		RFile* m_file;
		RFileWriteStream* m_pOutputFileStream;
		RFileReadStream* m_pReadFileStream;

			//physics dump
		void StartSaveStreamL(TInt aInd);
		void EndSaveStream();
		void ToStream(TUint8* aPtr, TInt aSize);

		void StartLoadStreamL(TInt aInd);
		void EndLoadStream();
		void FromStream(TUint8* aPtr, TInt aSize);

		void SaveFramesL();


    protected: //data
        CCameraAppController*&      iController;
        CFbsBitmap*                 iBitmap;
        CWsBitmap*                  iOffScreenBitmap;
        CFbsBitGc*                  iFbsBitGc;
        CFbsBitmapDevice*           iBmpDevice;
   };

#endif // __CAMERAAPP_BASE_CONTAINER_H__
