#include "bitmapmethods.h"
#include "Geometry3d.h"
#include "3dtypes.h"
#include "CGLEngine.h"
#include "CameraAppBaseContainer.h"

#include "box.h"

#include "vtracker.h"
#include "EikonEnvironment.h"
#include <s32file.h>


void CGLengine::CreateBox(){
	if(m_numbGraphObj >= MAX_GRAPH_OBJ)
		return;

	TMeshData aBase;

	TInt numInds = KNumBoxFaces*3;
	TInt aSize = numInds*sizeof(TUint16);
	if(m_numAllocMeshData >= MAX_GLMESHES){
		return;
	}
	m_allocMeshData[m_numAllocMeshData] = new(ELeave)TUint8[aSize];
	TUint16* aPtrInd = (TUint16*)m_allocMeshData[m_numAllocMeshData];
	m_numAllocMeshData++;
	memcpy(aPtrInd, KBoxFaceData, aSize);

	TInt numVerts = KNumBoxVertices;
	aSize = 3*numVerts*sizeof(TInt);
	if(m_numAllocMeshData >= MAX_GLMESHES){
		return;
	}
	m_allocMeshData[m_numAllocMeshData] = new(ELeave)TUint8[aSize];
	TInt* aPtrVert = (TInt*)m_allocMeshData[m_numAllocMeshData];
	m_numAllocMeshData++;
	memcpy(aPtrVert, KBoxVertexData, aSize);

	aSize = 2*numVerts*sizeof(TInt);
	if(m_numAllocMeshData >= MAX_GLMESHES){
		return;
	}
	m_allocMeshData[m_numAllocMeshData] = new(ELeave)TUint8[aSize];
	TInt* aPtrTxt = (TInt*)m_allocMeshData[m_numAllocMeshData];
	m_numAllocMeshData++;
	memcpy(aPtrTxt, KBoxTxtData, aSize);

	aBase.FillData(aPtrVert,
					NULL,
					aPtrTxt, 
					aPtrInd,
					KNumBoxFaces*3,
					KNumBoxVertices);


	TGLMesh* aBaseMesh = AddMesh(aBase, NULL);

	TMatrixGL aIdentity;
	TransformStatObj((TInt*)aBaseMesh->m_Verts, aBaseMesh->m_numVerts, aIdentity);
	aBaseMesh->GetBox(iMath);
	TGLFrame* aBaseFrame = AddFrame(aBaseMesh, NULL, NULL);

	iBoxModel = m_numbGraphObj;
	m_graphObj[m_numbGraphObj].m_frame = aBaseFrame;
	m_numbGraphObj++;
};


void CGLengine::RenderMeshImg(TInt aInd, TInt aXoff, TInt aYoff, TInt aRnd){
	GLfloat aVal =.08f;
	int nS = ICON_SIZE;
	glViewport(aXoff, aYoff, nS, nS);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrthof(-aVal, aVal, 0, 2*aVal, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glRotatef(30, 1,0,0);
	glRotatef(-90, 0,1,0);
	glTranslatef(0,.03,0);
	glColor4x(GLUNITY, GLUNITY, GLUNITY, GLUNITY);
	TGraphObj& aObj = m_graphObj[aInd];
	renderMesh(aObj.m_frame->m_mesh);
	TGLFrame* aChild = aObj.m_frame->m_frameChild;
	while(aChild)
	{
		renderFrame(aChild, TVector3(UNITY,0,0));
		aChild = aChild->m_frameSibling;
	}
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}


void  CGLengine::CreateObjects(){
	m_numbGraphObj = 0;
	m_numTxt = 0;
	m_numFrame = 0;
	m_numMesh = 0;

	CreateBox();
}

#include "PhEngine.h"

void CPhEngine::MarkerStructure(TInt aMarkerID, TRPoint* aPoint, TFloat aMinX, TFloat aMaxX, TFloat aMinY, TFloat aMaxY, TRPoint& aC, TFloat aRad){
	TInt aScale = UNITY>>SCALE_SHIFT;

	TPoint aX;
	aX.iX = (TInt)(aScale*(aPoint[3].iX - aPoint[0].iX));
	aX.iY = (TInt)(aScale*(aPoint[3].iY - aPoint[0].iY));

	TPoint aY;
	aY.iX = (TInt)(aScale*(aPoint[1].iX - aPoint[0].iX));
	aY.iY = (TInt)(aScale*(aPoint[1].iY - aPoint[0].iY));

	TPoint aPX;
	aPX = 2*aX;

	TPoint aPY;
	aPY = 2*aY;

	TPoint aPos;
	aPos.iX = (TInt)(2*aScale*aPoint[0].iX);
	aPos.iY = (TInt)(2*aScale*aPoint[0].iY);

	aPos = aPos - ((aPX + aPY)>>3);
	aPX = aPX + ((aPX)>>2);
	aPY = aPY + ((aPY)>>2);

	TPoint aPosL[4];

	aPosL[0] = aPos;
	aPosL[1] = aPos + aPY;
	aPosL[2] = aPos + aPY+aPX;
	aPosL[3] = aPos + aPX;
	TPoint aPosC = aPos + ((aPX + aPY)>>1);
	//--------------------------------------
	//bounding box and circle

	m_minX = (TInt)(2*aScale*aMinX);
	m_minY = (TInt)(2*aScale*aMinY);
	m_maxX = (TInt)(2*aScale*aMaxX);
	m_maxY = (TInt)(2*aScale*aMaxY);

	//rewrite center into real center
	m_center = TVector3((TInt)(2*aScale*aC.iX), 0, (TInt)(2*aScale*aC.iY));

	if(m_areaRad != INFINITY)
		m_areaRad = Max( m_areaRad, (TInt)(2*aScale*aRad + UNITY/3));
	else
		m_areaRad = (TInt)(2*aScale*aRad + UNITY/3);

//-----------------------------------------------
	PutBoxes(aPosL, aMarkerID, aPX, aPY);
};


void TBox::Init(TPoint& aZero, TPoint& aX, TPoint& aY, TMathLookup& aMath){
	memset(this, 0, sizeof(TBox));

	TVector3 aVx(aX.iX, 0, aX.iY);
	TVector3 aVz(aY.iX, 0, aY.iY);

	m_boxZero = TVector3(aZero.iX, 0, aZero.iY)<<GL2U;
	m_boxX = aVx<<GL2U;
	m_boxZ = aVz<<GL2U;

	TInt xx = (aX.iX*aX.iX + aX.iY*aX.iY)>>UNITY_LOG;
	TInt yy = 0;
	TInt zz = (aY.iX*aY.iX + aY.iY*aY.iY)>>UNITY_LOG;

	aMath.GetSqrt(xx, lx);
	aMath.GetSqrt(zz, lz);
	ly = 0;

	aMath.GetReciprocalShort(lx, inv_x, downshift_x);
	aMath.GetReciprocalShort(ly, inv_y, downshift_y);
	aMath.GetReciprocalShort(lz, inv_z, downshift_z);

	downshift_x -= UNITY_LOG;
	downshift_y -= UNITY_LOG;
	downshift_z -= UNITY_LOG;

	aMath.GetReciprocalShort(xx, inv_xx, downshift_xx);
	aMath.GetReciprocalShort(yy, inv_yy, downshift_yy);
	aMath.GetReciprocalShort(zz, inv_zz, downshift_zz);

	downshift_xx -= UNITY_LOG;
	downshift_yy -= UNITY_LOG;
	downshift_zz -= UNITY_LOG;

	CenterUnity(aMath);

	TVector3 aR = (m_boxZero>>GL2U )- vUC;
	rad = aR*aR>>UNITY_LOG;
	aMath.GetSqrt(rad, rad);

};

void TBox::CenterUnity(TMathLookup& aMath){
	TVector3 aVx = m_boxX>>GL2U;
	TVector3 aVz = m_boxZ>>GL2U;
	TVector3 aZero = m_boxZero>>GL2U;
	TVector3 aD = (aVx + aVz)>>1; 
	vUC = aZero + aD;
	rad = aD.GetLen(aMath);
};




