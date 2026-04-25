#ifndef __VGLENV_H__
#define __VGLENV_H__

#include <e32std.h>		// User
#include <e32math.h>  
#include <w32std.h>	
#include <flogger.h>

#include "GLES/egl.h"
#include "GLES/gl.h"

class VTracker;
class GLEnv;

class VGLEnv
{
public:
	void InitGL(RWindow& aWindow, TDisplayMode aDisplayMode, const TRect& aRect);
	void DeleteGL();
	void FinishRender();
	void StartRender();
	void SetText(TBuf8<TEXT_SIZE>& aStr);
	void DrawText(TInt aOffX, TInt aOffY, TInt* aColor);
	void DrawTextOp(TInt aOffX, TInt aOffY, TInt* aColor);
	void FillRGBBuffer(CFbsBitmap* aFrame);
	void SetTracker(VTracker* aTracker);
private:
	GLEnv* iGLEnv;
};

#endif
