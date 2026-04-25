#include "pltPh.h"
#include "vtracker.h"

//time dependant end -----------------------
void MarkCrossGlob(TPoint aPoint, TInt* aColor);

TInt Signum(TInt aVal){
	if(aVal==0)
		return 0;
	else if(aVal > 0)
		return 1;
	else
		return -1;
}

TPoint Signum(const TPoint& aP){
	return TPoint(Signum(aP.iX), Signum(aP.iY)); 
}

TInt MaxAbs(const TPoint& aP){
	return Max(Abs(aP.iX), Abs(aP.iY));
}

////////////////////////////////////////////////////////////////////////
TInt UpSizePoint(TPoint& aP, TInt aUnity){
	TInt absX = aP.iX > 0 ? aP.iX : -aP.iX;
	TInt absY = aP.iY > 0 ? aP.iY : -aP.iY;

	// Get "Manhattan magnitude" of vector:
	TInt sumAbs = absX + absY;

	if(sumAbs == 0)
		return 0;

	// Is vector too large?
	// Do the components have trailing zeroes we can discard?
	TInt aRet = 0;
	while ( sumAbs < aUnity )
		{
		aP.iX <<= 1;
		aP.iY <<= 1;
		sumAbs <<= 1;
		aRet++;
		}

	return aRet;
}

void Transform(TBox& box, TMatrixGL& aMatrix){
	box.m_boxZero = aMatrix.TransformVertix(box.m_boxZero);
	box.m_boxX = aMatrix.TransformNormal(box.m_boxX);
	box.m_boxZ = aMatrix.TransformNormal(box.m_boxZ);
	box.m_boxY = aMatrix.TransformNormal(box.m_boxY);
};

void CPhEngine::ConstructL (TDisplayStat* aStat){
	iStat = aStat;
	aStat->ResetMsg();

	CreateObjects();

	m_interfaceState = eIWait;
	m_bFirstTick = ETrue;
}

void CPhEngine::Restart(){
	iStat->ResetMsg();
	m_bFirstTick = ETrue;
	ResetState();
}

void CPhEngine::Tick(){
	if(m_bFirstTick){
		m_bFirstTick = EFalse;
		iStat->ResetMsg();
	}

	TInt count;
	for(count = 0; count < m_numObj; count++){
		TPhObj& aObj = m_phObj[count];

		aObj.m_objTime++;
	}

	m_phTime++;

	iStat->PipeMsg();

	DeleteMarked();
}

void CPhEngine::DeleteMarked(){
	TInt count;
	TInt aCurrentNumb = m_numObj;

	for(count = 0; count < aCurrentNumb; count++)
	{
		TPhObj& aObj = m_phObj[count];

		if(count == 0)
			continue;

		if(aObj.m_type == eTypeEmpty)
			continue;
		if(aObj.m_toDelete)
			aObj.DeletePhObj(m_glEngine, this);
	}

};


void TPhObj::DeletePhObj(CGLengine* aGLengine, CPhEngine* aPhEngine){
	//dealloc frame
	if(m_frame)
		aGLengine->deallocFrame(m_frame);


	ResetPhObj();
}

void TPhObj::ResetPhObj(){
	TInt aSaveID = m_objID;
	memset(this, 0, sizeof(TPhObj));

	m_type = eTypeEmpty;
	m_objID = aSaveID;

	m_Dir = TVector3(UNITY, 0, 0);

	m_mass = UNITY;
}

void TPhObj::MarkDelete(){
	m_toDelete = ETrue;
}

TPhObj::TPhObj(){
	ResetPhObj();
};

void TPhObj::SetColorInd(TInt aInd){
	if(aInd >= 0)
	{
		m_isColored = ETrue;
		memcpy(m_color, KBoxColors + 3*(aInd%BOX_COLORS), 3*sizeof(TInt));
	}
	else
		m_isColored = EFalse;
};

void TPhObj::SetVel(TInt aVel, TInt aAngVel, TInt aAngVelCos, TInt aAngVelSin, TInt aAngVelHalfCos, TInt aAngVelHalfSin){
	m_Vel = aVel;
	m_angVelMax = aAngVel;
	m_angVelCos = aAngVelCos;
	m_angVelSin = aAngVelSin;
	m_angVelCosSave = aAngVelCos;
	m_angVelSinSave = aAngVelSin;
	m_angVelHalfCos = aAngVelHalfCos;
	m_angVelHalfSin = aAngVelHalfSin;

};

void TPhObj::UndoPos(){
	m_Pos = m_PrevPos;

	TVector3 aPos = m_Pos;
	aPos.Upshift(GL2U);

	m_frame->m_Self2Parent.iElement[12] = aPos.iX;
	m_frame->m_Self2Parent.iElement[13] = aPos.iY;
	m_frame->m_Self2Parent.iElement[14] = aPos.iZ;
};

void TPhObj::UndoPos(TInt aWeight){
	m_Pos = ((UNITY-aWeight)*m_PrevPos + aWeight*m_Pos)>>UNITY_LOG;

	TVector3 aPos = m_Pos;
	aPos.Upshift(GL2U);

	m_frame->m_Self2Parent.iElement[12] = aPos.iX;
	m_frame->m_Self2Parent.iElement[13] = aPos.iY;
	m_frame->m_Self2Parent.iElement[14] = aPos.iZ;
};

void TPhObj::SetPos(TVector3 aPos){
	m_PrevPos = m_Pos;
	SetPosNoPrev(aPos);
};

void TPhObj::SetPosNoPrev(TVector3 aPos){
	if(m_signalState & SIGNAL_STUCK)
		return;

	m_Pos = aPos;

	aPos.Upshift(GL2U);

	m_frame->m_Self2Parent.iElement[12] = aPos.iX;
	m_frame->m_Self2Parent.iElement[13] = aPos.iY;
	m_frame->m_Self2Parent.iElement[14] = aPos.iZ;
};

void TPhObj::SetOffsetPos(TVector3 aPos){
	m_PrevPos = m_Pos;
	SetOffsetPosNoPrev(aPos);
};

void TPhObj::SetOffsetPosNoPrev(TVector3 aPos){
	m_Pos = aPos;

	if(m_signalState & SIGNAL_STUCK){
		m_OffsetPos.Zero();
		m_signalState &= ~SIGNAL_OVERRIDE;
		m_signalState &= ~SIGNAL_MOVE;
		m_signalState &= ~SIGNAL_STUCK;
	}


	m_signalState &= ~SIGNAL_NOT_MOVED_YET;//clear this flag
	if(m_signalState & SIGNAL_MOVE){
		m_Pos = m_Pos + m_OffsetPos;
		aPos = aPos + m_OffsetPos;
		m_OffsetPos.Zero();
		m_signalState &= ~SIGNAL_MOVE;
	}

	aPos.Upshift(GL2U);

	m_frame->m_Self2Parent.iElement[12] = aPos.iX;
	m_frame->m_Self2Parent.iElement[13] = aPos.iY;
	m_frame->m_Self2Parent.iElement[14] = aPos.iZ;
};

void TPhObj::SetPosSimple(const TVector3& aPos){
	m_Pos = aPos;
	m_PrevPos = m_Pos;
};

void TPhObj::GetPos(TVector3& aPos){
	aPos = m_Pos;
};

TVector3 TPhObj::GetPos(){
	return m_Pos;
};

TInt TPhObj::GetX(){
	return m_Pos.iX;
};

TInt TPhObj::GetY(){
	return m_Pos.iY;
};

TInt TPhObj::GetZ(){
	return m_Pos.iZ;
};

void TPhObj::SetDirXZ(TVector3 aDir){
	m_Dir = aDir;
	m_ctrlDir = m_Dir;
	m_frameDir = m_Dir;
	SetFrameDirXZ();
};

void TPhObj::SetFrameDirXZ(){
	TVector3 aDir = m_frameDir;
	aDir.Upshift(GL2U);

	m_frame->m_Self2Parent.iElement[0] = aDir.iX;
	m_frame->m_Self2Parent.iElement[8] = -aDir.iZ;

	m_frame->m_Self2Parent.iElement[2] =  aDir.iZ;
	m_frame->m_Self2Parent.iElement[10] = aDir.iX;
};


void TPhObj::SetDir(TVector3 aDir, TMathLookup& aMath){
	m_Dir = aDir;
	m_ctrlDir = m_Dir;
	m_frameDir = m_Dir;
	SetFrameDir(aMath);
};

void TPhObj::SetFrameDir(TMathLookup& aMath){
	TVector3 aDir = m_frameDir;

	//	pitch
	TVector3 aXZ(aDir.iX, 0, aDir.iZ);
	aXZ.FastNormaliseFrom(aMath, aXZ);
	TInt aCosP = (aDir*aXZ)>>(UNITY_LOG-GL2U);
	TInt aSinP = aDir.iY<<GL2U;
	TMatrixGL aRotP;

	aRotP.iElement[0] = aCosP;
	aRotP.iElement[4] = -aSinP;
	aRotP.iElement[1] = aSinP;
	aRotP.iElement[5] = aCosP;

	aDir.Upshift(GL2U);

	aXZ = aXZ<<GL2U;

	m_frame->m_Self2Parent.MakeIdentity();

	m_frame->m_Self2Parent.iElement[0] = aXZ.iX;
	m_frame->m_Self2Parent.iElement[8] = -aXZ.iZ;

	m_frame->m_Self2Parent.iElement[2] =  aXZ.iZ;
	m_frame->m_Self2Parent.iElement[10] = aXZ.iX;

	m_frame->m_Self2Parent.MakeCompound(m_frame->m_Self2Parent, aRotP);

};

TInt CPhEngine::GetEmptyObj(){
	TInt aInd = -1;
	TInt count;
	for(count = 0; count < m_numObj; count++){
		if(m_phObj[count].m_type == eTypeEmpty)
		{
			aInd = count;
			break;
		}
	}

	if(aInd < 0){
		if(m_numObj >= MAX_PHYS_OBJ)
			return -1;
		aInd = m_numObj;
		m_phObj[aInd].m_type = eTypeEmpty;
		m_numObj++;
	}

	m_phObj[aInd].ResetPhObj();

	return aInd;
};

void CPhEngine::ProcessMarkDelete(TInt* aDeleteStack, TInt& aDelStackSize){
	TInt count;
	for(count = 0; count < aDelStackSize; count++){
		m_phObj[aDeleteStack[count]].MarkDelete();//ProcessMarkDelete
	}
};

void CPhEngine::Render(){
	TInt count;

	for(count = 0; count < m_numObj; count++){
		TPhObj& aObj = m_phObj[count];

		m_glEngine->ResetColor();

		if(aObj.m_isColored){
			m_glEngine->SetColor(aObj.m_color);
		}
		else
			m_glEngine->ResetColor();

		/*if(aObj.m_type == eTypeBox){
			glEnable(GL_BLEND);
			m_glEngine->SetColorTransp(aObj.m_color, 3*GLUNITY/4);
		}*/

		m_glEngine->renderFrame(aObj.m_frame, aObj.m_Dir);
	}
}

void CPhEngine::CreateObjects(){
	TInt count;
	for(count = 0; count < MAX_PHYS_OBJ; count++){
		TPhObj& aObj = m_phObj[count];
		aObj.DeletePhObj(m_glEngine, this);
		aObj.m_objID = count;
	}
	m_numObj  = 0;

	ResetState();

//-----------------------------------------------------------

	for(count = 0; count < m_numObj; count++){
		TBox currBox = m_phObj[count].m_frame->m_mesh->m_box;
		Transform(currBox, m_phObj[count].m_frame->m_Self2Parent);
		m_phObj[count].m_frame->m_frameBox = currBox;
	}

	m_glEngine->SaveStaticObjNumb();
}

void CPhEngine::PutBoxes(const TPoint* aPosL, TInt aMarkerID, const TPoint& aPX, const TPoint& aPY){
	TInt aBit0 = aMarkerID & 1;

	TPoint aX = aPX>>1;
	TPoint aY = aPY>>1;

	TPoint aDir = aX;
	TInt oldObjNumb = m_numObj;

	TInt aColorInd = m_boxesColorInd;

	//--------------------------------------
	if(m_numObj >= MAX_PHYS_OBJ)
		return;

	aDir = -aY;
	if(aBit0)
		aDir = -aX;

	// calcular centro del marker
    TPoint center;
    center.iX = (aPosL[0].iX + aPosL[1].iX + aPosL[2].iX + aPosL[3].iX) >> 2;
    center.iY = (aPosL[0].iY + aPosL[1].iY + aPosL[2].iY + aPosL[3].iY) >> 2;

	PutBox(center, aDir, aColorInd);

	aColorInd = (aColorInd + 1)%4;

	if(m_numObj >= MAX_PHYS_OBJ)
		return;

	TInt count;
	for(count = oldObjNumb; count < m_numObj; count++){
		TPhObj& aObj = m_phObj[count];
		TBox currBox = aObj.m_frame->m_mesh->m_box;
		Transform(currBox, aObj.m_frame->m_Self2Parent);
		aObj.m_frame->m_frameBox = currBox;
		aObj.m_frame->m_frameBox.CenterUnity(iMath);
		UpdateBox(iMath, aObj.m_frame->m_frameBox);
	}

	m_boxesColorInd = aColorInd;
};

void CPhEngine::ResetState(){
	TInt count;

	for(count = 0; count < m_numObj; count++){
		TPhObj& aObj = m_phObj[count];
		if(aObj.m_type == eTypeBox)
			continue;
		aObj.DeletePhObj(m_glEngine, this);
		aObj.m_objID = count;
	}

	m_minX = -INFINITY;
	m_maxX = INFINITY;
	m_minY = -INFINITY;
	m_maxY = INFINITY;

	m_phTime = 0;

	m_restart = EFalse;
	m_ctrlData.m_ctrlMove = 0;

	m_ctrlCount = 0;
	m_ctrlSize = 0;
	m_bRecording = EFalse;
}

void CPhEngine::PutBox(const TPoint aPosL, TPoint aDir, TInt aColorInd){
	if(m_numObj >= MAX_PHYS_OBJ)
		return;

	TPhObj& aObj = m_phObj[m_numObj];

	aObj.m_type = eTypeBox;
	aObj.m_movable = EFalse;
	aObj.m_frame = m_glEngine->addCloneFrame( m_glEngine->m_graphObj[m_glEngine->iBoxModel].m_frame);
	if(aObj.m_frame == NULL)
		return;

	NormPoint(iMath, aDir);
	//twice for prevPos
	aObj.SetPos(TVector3(aPosL.iX, 0, aPosL.iY)); //PutBox
	aObj.SetPos(TVector3(aPosL.iX, 0, aPosL.iY)); //PutBox

	aObj.SetDirXZ(TVector3(aDir.iX, 0, aDir.iY));

	TInt aAngl = 10;
	TInt aCos, aSin;
	iMath.GetTrigRatios(aAngl, aCos, aSin);
	TInt aHalfCos, aHalfSin;
	iMath.GetTrigRatios(aAngl/2, aHalfCos, aHalfSin);
	aObj.SetVel(0, aAngl, aCos, aSin, aHalfCos, aHalfSin);

	aObj.m_isColored = ETrue;
	memcpy(&aObj.m_color, KBoxColors + 3*aColorInd, 3*sizeof(TInt));

	m_numObj++;
};
