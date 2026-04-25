#ifndef VTRACKER_H
#define VTRACKER_H

#include <e32std.h>		// User
#include <e32math.h>  
#include <flogger.h>
#include <VGLEnv.h>

#define ASEG_SIZE 32

class CTracker;
class GLEnv;

struct TRectData
{
	TUint16* iData;
	TRect iRect; 
	TInt iPitch;

	TRectData(){};

	TRectData(TUint16* aData, TRect aRect, TInt aPitch)
	{
		iData = aData;
		iRect = aRect;
		iPitch = aPitch;
	};
};

struct TExportMarkerData
{
	TRPoint iPoint[4];
	TInt iMarkerID;
	TFloat iMinX;
	TFloat iMaxX;
	TFloat iMinY;
	TFloat iMaxY;
	TRPoint iC;
	TFloat iRad;
};

class VTracker
{
friend class  VGLEnv;
public:
	VTracker(TRectData& aRectData, RFileLogger* aLog, TBool aLogEnabled);
	~VTracker();
	void IncTreshold();
	void DecTreshold();
	void IncFocus();
	void DecFocus();
	void ToggDebugBW();
	TBool isDebugBW();
	void FillGreyBuffer(CFbsBitmap* aFrame);
	TBool Capture();
	TInt GetNumberOfFoundMarkers();
	TInt GetFixIntFocus();
	TInt* GetFixIntView();
	void GetFoundMarker(TExportMarkerData* data, int num);
	void ResetFilter();
	void SetLog(TBool aLogEnabled);
//  void UnlockMarker(TInt aMarkerID);
//  void DeleteMarker(TInt aMarkerID);

//fast fixed point division
	static TInt m_fastDiv[ASEG_SIZE];
//auxillary memory manager
	TInt GetMemAreaSize();
	TInt SetMemAreaSize(TInt aSize);
	void SetSaveSize(int aSaveSize);
	void* areaAlloc(int aSize);
	void* areaAlloc();
	void* areaGetNoSave();
	TInt GetSaveSize();
	void* areaNextAlloc();
	TBool addAreaSize(TInt aSize);
	void* areaAllocZero(int aSize);
	void areaReset();
	void areaUndo();
	void areaRestore(TInt aSize);
private:
	CTracker* GetTracker();
	static CTracker* iTracker;
};
#endif
