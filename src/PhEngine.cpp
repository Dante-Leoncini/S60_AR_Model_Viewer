
#define _PLATFORM_FILE
#include "pltPh.h"


//Platform dependent staff
CPhEngine::CPhEngine(CGLengine* aEngine)
{
	m_glEngine = aEngine;
	m_numObj = 0;

	m_minX = -INFINITY;
	m_maxX = INFINITY;
	m_minY = -INFINITY;
	m_maxY = INFINITY;

	m_center =TVector3(0, 0, 0);
	m_areaRad = INFINITY;
	m_phTime = 0;

	m_boxesColorInd = 0;

}

CPhEngine::~CPhEngine()
{
};

int CPhEngine::GetAreaSize()
{
	return iTracker->GetMemAreaSize();
};

int CPhEngine::GetSaveSize()
{
	return iTracker->GetSaveSize();
};

void CPhEngine::SetAreaSize(int aAreaSize)
{
	iTracker->SetMemAreaSize(aAreaSize);
};

void CPhEngine::SetSaveSize(int aSaveSize)
{
	iTracker->SetSaveSize(aSaveSize);
};

void* CPhEngine::areaAlloc(int aSize)
{
	return iTracker->areaAlloc(aSize);
};

void* CPhEngine::areaAlloc()
{
	return iTracker->areaAlloc();
};

void* CPhEngine::areaGetNoSave()
{
	return iTracker->areaGetNoSave();
};


void* CPhEngine::areaNextAlloc()
{
	return iTracker->areaNextAlloc();
};

TBool CPhEngine::addAreaSize(TInt aSize)
{
	return iTracker->addAreaSize(aSize);
}

void* CPhEngine::areaAllocZero(int aSize)
{
	return iTracker->areaAllocZero(aSize);
};

void CPhEngine::areaReset()
{
	iTracker->areaReset();
};

void CPhEngine::areaUndo()
{
	iTracker->areaUndo();
};

void CPhEngine::areaRestore(TInt aSize)
{ 
	iTracker->areaRestore(aSize);
};


void CPhEngine::Save()
{


};


void CPhEngine::SaveRecord()
{

	iView->StartSaveStreamL(1);

	iView->ToStream((TUint8*)(&m_ctrlCount), sizeof(TInt));

	TInt count;
	for(count = 0; count < m_ctrlCount; count++)
	{
		iView->ToStream((TUint8*)(&m_ctrlBuffer[count]), sizeof(TCtrlData));
	}

	iView->EndSaveStream();

};

void CPhEngine::LoadRecord()
{

	iView->StartLoadStreamL(1);

	iView->FromStream((TUint8*)(&m_ctrlSize), sizeof(TInt));
	TInt count;
	for(count = 0; count < m_ctrlSize; count++)
	{
		iView->FromStream((TUint8*)(&m_ctrlBuffer[count]), sizeof(TCtrlData));
	}

	iView->EndLoadStream();
	m_ctrlCount = 0;

};

void CPhEngine::GLLine2Color(TVector3 vV0, TVector3 vV1, TInt* aColor)
{

	glDisable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glShadeModel(GL_SMOOTH);


	TVector3 aP[2];

	aP[0] = vV0<<GL2U;
	aP[1] = vV1<<GL2U;
	glVertexPointer(3, GL_FIXED, 0, aP);
	glColorPointer(4, GL_FIXED, 0, aColor);
	glDrawArrays(GL_LINES, 0, 2);

	glShadeModel(GL_FLAT);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	glDisableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

void CPhEngine::GLLine2ColorFlat(TVector3 vV0, TVector3 vV1, TInt* aColor)
{

	glDisable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glShadeModel(GL_FLAT);


	TVector3 aP[2];

	aP[0] = vV0<<GL2U;
	aP[1] = vV1<<GL2U;
	glVertexPointer(3, GL_FIXED, 0, aP);
	glColor4x(aColor[0], aColor[1], aColor[2], aColor[3]);
	glDrawArrays(GL_LINES, 0, 2);

	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

void CPhEngine::GLLine2ColorFlat(TVector3 vV0, TVector3 vV1)
{
	TVector3 aP[2];
	aP[0] = vV0<<GL2U;
	aP[1] = vV1<<GL2U;
	glVertexPointer(3, GL_FIXED, 0, aP);
	glDrawArrays(GL_LINES, 0, 2);
}



void CPhEngine::GLLine(TVector3 vV0, TVector3 vV1, TInt* aColor)
{

	glDisable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	TVector3 aP[2];

	aP[0] = vV0<<GL2U;
	aP[1] = vV1<<GL2U;
	glColor4x(aColor[0], aColor[1], aColor[2], GLUNITY);
	glVertexPointer(3, GL_FIXED, 0, aP);
	glDrawArrays(GL_LINES, 0, 2);

	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	glDisableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

void CPhEngine::GLLineTransp(TVector3 vV0, TVector3 vV1, TInt* aColor)
{

	glDisable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	TVector3 aP[2];

	aP[0] = vV0<<GL2U;
	aP[1] = vV1<<GL2U;
	glColor4x(aColor[0], aColor[1], aColor[2], aColor[3]);
	glVertexPointer(3, GL_FIXED, 0, aP);
	glDrawArrays(GL_LINES, 0, 2);

	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);


}


void CPhEngine::GLLineDeb(TVector3 vV0, TVector3 vV1, TInt* aColor)
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	TVector3 aP[2];

	aP[0] = vV0<<GL2U;
	aP[1] = vV1<<GL2U;
	glColor4x(aColor[0], aColor[1], aColor[2], GLUNITY);
	glVertexPointer(3, GL_FIXED, 0, aP);
	glDrawArrays(GL_LINES, 0, 2);

	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glColor4x(GLUNITY, GLUNITY, GLUNITY, GLUNITY);

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

void CPhEngine::Load(){

};
