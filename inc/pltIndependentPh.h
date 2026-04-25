#ifndef __pltIndependentPh_H__
#define __pltIndependentPh_H__

class TGLFrame;
struct TRPoint;
class TDisplayStat;

#include "CameraAppBaseContainer.h"

struct TBox {
	TVector3 m_boxZero;
	TVector3 m_boxX;
	TVector3 m_boxZ;
	TVector3 m_boxY;

	TInt lx, ly, lz;
	
	TInt inv_xx;
	TInt downshift_xx;
	TInt inv_yy;
	TInt downshift_yy;
	TInt inv_zz;
	TInt downshift_zz;

	TInt inv_x;
	TInt downshift_x;
	TInt inv_y;
	TInt downshift_y;
	TInt inv_z;
	TInt downshift_z;

	void Init(TPoint& aZero, TPoint& aX, TPoint& aY, TMathLookup& aMath);
	void CenterUnity(TMathLookup& aMath);

	TVector3 vUC;
	TInt rad;
};

enum EnumType {
	eTypeEmpty,//0
	eTypeBox,//1
};

enum EIntefaceState {
	eINormal,
	eIWait,
};

struct TCtrlData {
	TInt m_ctrlAction;
	TInt m_ctrlMove;
	TVector3 m_ctrlPos;
};


struct TBoxData {
	TInt m_colorInd;
	TInt m_state;
};

struct TObjData {
	int m_Val0;
	int m_Val1;
	int m_Val2;
	int m_Val3;
	int m_Val4;
	int m_Val5;
	int m_Val6;
	int m_Val7;
	int m_Val8;
	int m_Val9;
};

class TPhObj {
	public:
		TPhObj();
		void ResetPhObj();
		void MarkDelete();
		void DeletePhObj(CGLengine* aGLengine, CPhEngine* aPhEngine);
		void SetVel(TInt aVel, TInt aAngVel, TInt aAngVelCos, TInt aAngVelSin, TInt aAngVelHalfCos, TInt aAngVelHalfSin);
		void SetDirXZ(TVector3 aDir);
		void SetFrameDirXZ();
		void SetDir(TVector3 aDir, TMathLookup& aMath);
		void SetFrameDir(TMathLookup& aMath);
		void SetColorInd(TInt aInd);

		void GetPos(TVector3& aPos);
		TVector3 GetPos();
		TInt GetX();
		TInt GetY();
		TInt GetZ();

		void SetPos(TVector3 aPos);
		void SetPosNoPrev(TVector3 aPos);
		void SetOffsetPos(TVector3 aPos);
		void SetOffsetPosNoPrev(TVector3 aPos);
		void SetPosSimple(const TVector3& aPos);
		void UndoPos();
		void UndoPos(TInt aWeight);

		TInt m_objID;

		TInt m_rad;
		TInt m_mass;

		TInt m_objTime;

		EnumType m_type;
		TObjData m_data;

		TBool m_round;
		TBool m_movable;
		TBool m_visible;
		TBool m_noRender;
		TBool m_nonPhysical;
		TBool m_toDelete;

		TInt m_color[3];
		TBool m_isColored;

		//pos and orient
		TVector3 m_OffsetPos;
		TVector3 m_Pos;
		TVector3 m_PrevPos;
		TVector3 m_Dir;
		TVector3 m_ctrlDir;
		TVector3 m_frameDir;
		TGLFrame* m_frame;

		//speed
		TInt m_Vel;
		TInt m_VelMax;
		TInt m_VelMaxSave;
		TInt m_Accel;

		TInt m_angVelMax;
		TInt m_angVelCos;
		TInt m_angVelSin;
		TInt m_angVelCosSave;
		TInt m_angVelSinSave;
		TInt m_angVelHalfCos;
		TInt m_angVelHalfSin;

		TUint m_signalState;
};

#define SAVE_CTRL_SIZE 3072

class CPhEngine {
	public:
		CPhEngine (CGLengine* aEngine);
		~CPhEngine();
		void Tick();
		void Restart();
		void Render();

		void ConstructL(TDisplayStat* aStat);

		void ProcessMarkDelete(TInt* aDeleteStack, TInt& aDelStackSize);

		void CreateObjects();

		void DeleteMarked();

		void MarkerStructure(TInt aMarkerID, TRPoint* aPoint, TFloat aMinX, TFloat aMaxX, TFloat aMinY, TFloat aMaxY, TRPoint& aC, TFloat aRad);
		void PutBoxes(const TPoint* aPosL, TInt aMarkerID, const TPoint& aPX, const TPoint& aPY);
		void PutBox(const TPoint aPosL, TPoint aDir, TInt aColorInd);

		void ResetState();

		void GLLine(TVector3 vV0, TVector3 vV1, TInt* aColor);
		void GLLineDot(TVector3 vV0, TVector3 vV1, TInt* aColor, TInt aStipple);
		void GLLine2Color(TVector3 vV0, TVector3 vV1, TInt* aColor);
		void GLLine2ColorFlat(TVector3 vV0, TVector3 vV1, TInt* aColor);
		void GLLine2ColorFlat(TVector3 vV0, TVector3 vV1);
		void GLLineTransp(TVector3 vV0, TVector3 vV1, TInt* aColor);
		void GLLineDeb(TVector3 vV0, TVector3 vV1, TInt* aColor);


		CCameraAppBaseContainer* iView;

		VTracker* iTracker;
		TDisplayStat* iStat;

		TInt GetEmptyObj();

		TInt m_boxesColorInd;

		TPhObj	m_phObj[MAX_PHYS_OBJ];

		TUint	m_phTime;

		TBool m_restart;

		TCtrlData m_ctrlData;
	
		TCtrlData m_ctrlBuffer[SAVE_CTRL_SIZE];
		int m_ctrlCount;
		int m_ctrlSize;
		TBool m_bRecording;

		void Load();
		void Save();
		void SaveRecord();
		void LoadRecord();

		TInt m_areaRad; 

		EIntefaceState m_interfaceState;
		TBool m_bFirstTick;

	private:

		CGLengine* m_glEngine;
		TInt m_numObj;

		TInt m_minX, m_maxX, m_minY, m_maxY;
		TVector3 m_center;

		TMathLookup iMath;

		int GetAreaSize();
		int GetSaveSize();
		void SetAreaSize(int aAreaSize);
		void SetSaveSize(int aSaveSize);
		void* areaAlloc(int aSize);
		void* areaAlloc();
		void* areaGetNoSave();
		void* areaNextAlloc();
		TBool addAreaSize(TInt aSize);
		void* areaAllocZero(int aSize);
		void areaReset();
		void areaUndo();
		void areaRestore(TInt aSize);

		friend class TPhObj;
};

void Transform(TBox& box, TMatrixGL& aMatrix);

#define SIGNAL_MOVE 1
#define SIGNAL_OVERRIDE (1<<5)
#define SIGNAL_CLEAR (1<<6)
#define SIGNAL_NOT_MOVED_YET (1<<7)
#define SIGNAL_STUCK (1<<8)

//geometrical
#define INV_SQRT_2 2896
#define INV_3 1365
#define INV_4 1024
#define INV_5 819
#define INV_6 683
#define INV_7 585
#define INV_8 512
#define INV_9 455
#define INV_10 410
#define INV_15 273

//time dependant
#define MSG_DELAY 20

#endif
