#ifndef __GLENGINE_H__
#define __GLENGINE_H__

#include <e32std.h>		// User
#include <e32math.h>  // math
#include <coecntrl.h>	// CCoeControl
#include <aknappui.h>	// CAknAppUi
#include <eikdoc.h> 	// CEikDocument
#include <aknapp.h> 	// CAknApplication
#include <flogger.h>
 
#include "CPictureLoader.h"

#include "GLES/egl.h"
#include "GLES/gl.h"

#include "define.h"
#include "Geometry3D.h"
#include "PhEngine.h"
#include "MathLookup.h"
#include "pltIndependentPh.h"
#include "VGLEnv.h"

#define MAX_GEOM_STRUCT 64
#define MAX_GRAPHS (MAX_GEOM_STRUCT/2)
#define MAX_GEOM_STRUCT_SIZE 8

#define MAX_GRAPH_OBJ	64
#define MAX_GLMESHES	64
#define MAX_GLFRAMES	256
#define MAX_STORED_TXTS 64
#define MAX_SHAPES		64
#define MAX_TXT			64

#define ICON_SIZE 32
#define OBJ_TXT_LENGTH 32
#define FONTYSIZE 16

#define QUAD_MESH -1

#define FILL_MODE 0
#define LINES_MODE 1

class CPhEngine;
class CGLengine;
class CTracker;
struct TEffect;

class TDisplayStat {
public:

	TInt m_msgStack[MAX_MSGES*MAX_MSG_LEN];

	TInt m_numbMsg;
	TInt  m_msgDelay;

	TInt  m_msgBlock;

	void AddMsg(TInt* aMsg);
	void MsgBlock(TInt aMsgID);

	void ResetMsg();
	void PipeMsg();

	TDisplayStat();
};

struct TMeshData {
	TInt* m_Verts;
	TInt* m_Norms;
	TInt* m_txtCoords;
	TUint16* m_Inds;
	TInt m_numInd;
	TInt m_numVerts;

	void FillData(TInt* aVerts, TInt* aNorms, TInt* aTxtCoords,
		TUint16* aInds, TInt numInd,  TInt numVerts)
	{
		m_Verts = aVerts;
		m_Norms = aNorms;
		m_txtCoords = aTxtCoords;
		m_Inds = aInds;
		m_numInd = numInd;
		m_numVerts = numVerts;
	};

};

class TGLMesh {
	public:
		TGLMesh(){
			m_Verts = NULL;
			m_Norms = NULL;
			m_txtCoords = NULL;
			m_Inds = NULL;
			m_numInd = 0;
			m_ptrTxtArea = NULL;
			m_dataInd = 0;
		};

		void FillMesh(TInt* aVerts, TInt* aNorms, TInt* aTxtCoords,
			TUint16* aInds, TInt numInd,  TInt numVerts, GLuint aTxt )
		{
			m_Verts = aVerts;
			m_Norms = aNorms;
			m_txtCoords = aTxtCoords;
			m_Inds = aInds;
			m_numInd = numInd;
			m_numVerts = numVerts;
			m_ptrTxtArea = NULL;
		};

		void FillMesh(TMeshData& aData)
		{
			m_Verts = aData.m_Verts;
			m_Norms = aData.m_Norms;
			m_txtCoords = aData.m_txtCoords;
			m_Inds = aData.m_Inds;
			m_numInd = aData.m_numInd;
			m_numVerts = aData.m_numVerts;
		};

		void GetBox(TMathLookup& aMath);

		TInt* m_Verts;
		TInt* m_Norms;
		TInt* m_txtCoords;
		TUint16* m_Inds;
		TInt m_numInd;
		TInt m_numVerts;
		TUint8* m_ptrTxtArea;
		TInt m_dataInd;

		TBox m_box;
};

class TGLFrame
{
public:
	TGLFrame(){
		m_Self2Parent.MakeIdentity();
		m_mesh = NULL;
		m_meshNumb = 0;
		m_frameSibling = NULL;
		m_frameChild = NULL;
		m_clonedFrom = NULL;
	};

	TMatrixGL m_Self2Parent;
	TGLMesh* m_mesh;
	TInt	m_meshNumb;
	TGLFrame* m_frameSibling;
	TGLFrame* m_frameChild;
	TGLFrame* m_clonedFrom;

	TBox m_frameBox;
	TInt      m_rad;
};

class TGraphObj
{
public:
	TGLFrame* m_frame;
};

class CPhEngine;
class CGLengine;

class CGLengine : public VGLEnv
{
public:

	CGLengine		(RWindow& aWindow, TDisplayMode aDisplayMode, const TRect& aRect);
	virtual			~CGLengine		(void);

	void			ConstructL	(void);

	void			updateState	(void);

	void			CreateObjects();
	void			CreateImages();
	void			renderMesh(TGLMesh* aMesh);
	void			renderFrame(TGLFrame* aFrame, const TVector3& aDir);
	void			RenderMeshImg(TInt aInd, TInt aXoff, TInt aYoff, TInt aRnd);

	TGLFrame*		addCloneFrame(TGLFrame* aFrame);
	TGLFrame*		allocCloneFrame(TGLFrame* aFrame);
	TGLFrame*		cloneFrame(TGLFrame* aFrame, TGLFrame* newFrame);
	TGLFrame*		cloneFrameWithAlloc(TGLFrame* aFrame, TGLFrame* newFrame);
	TGLFrame*		clonePureFrame(TGLFrame* aFrame, TGLFrame* newFrame);

	void			deallocFrame(TGLFrame* aFrame);
	
	TGLMesh*  		AddMesh(TMeshData& aBase, TUint8* aPtr);
	TGLFrame*		AddFrame(TGLMesh* aMesh, TGLFrame* aParent, TGLFrame* aPrev);
	TGLFrame*		AllocFrame();
	void			SetColor(TInt* color);
	void			SetColorTransp(TInt* color, TInt aAlpha);
	void			ResetColor();
	void			MarkCross(TPoint aPoint);
	//void			InitText();
	void			OutText(TInt* aMsg);

	TGraphObj		m_graphObj[MAX_GRAPH_OBJ];

	TInt			m_numFrame;

	void			ResetGL();

	TInt			iBoxModel;

	TDisplayStat*	iStat;

	CPhEngine*		m_phEngine;

	TBool			FrameCheck(TGLFrame* aFrame);

	TInt			m_polygonMode;
	TDisplayMode	iDisplayMode;
	TRect			m_clientRect;

	RFileLogger* iLog;
	TBool		 iLogEnabled;

	TBuf8<TEXT_SIZE> iText[TEXT_ENTRIES];

	bool			m_bPortrait;

private:
	void			CreateBox();

	void SaveStaticObjNumb(void);

	TInt			m_numStaticTxt;
	TInt			m_numStaticMesh;
	TInt			m_numStaticFrame;

	TGLMesh			m_glMesh[MAX_GLMESHES];
	TInt			m_numMesh;

	GLuint			m_txtID[MAX_TXT];
	TInt			m_numTxt;

	TGLFrame		m_frameStorage[MAX_GLFRAMES];
	TInt			m_numbGraphObj;

//static data
	TInt* m_statVerts[MAX_SHAPES];
	TInt* m_statNorms[MAX_SHAPES];
	TInt* m_stattxtCoords[MAX_SHAPES];
	TUint16* m_Inds[MAX_SHAPES];
	TInt m_statnumInd[MAX_SHAPES];
	TInt m_statnumVerts[MAX_SHAPES];
	TInt m_numShapes;

	TUint8* m_storedTxt[MAX_STORED_TXTS];
	TInt m_numbStoredTxt;

	TUint8*			m_allocMeshData[MAX_GLMESHES];
	TInt			m_numAllocMeshData;

	void TransformStatObj(TInt* aVerts, TInt numVerts, TMatrixGL& aM);

	TMathLookup iMath;

	friend class CPhEngine;
};

void UpdateBox(TMathLookup& aMath, TBox& aBox);


#endif
