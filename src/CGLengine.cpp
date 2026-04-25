#include "bitmapmethods.h"
#include "Geometry3d.h"
#include "3dtypes.h"

#include "CGLEngine.h"
#include "pltPh.h"

#include "../Whisk3D/Whisk3D.h"

#define GL_BGRA (0x80E1)
//------------------------------------------------------------------------

CGLengine::CGLengine (RWindow& aWindow, TDisplayMode aDisplayMode, const TRect& aRect) {
	m_clientRect = aRect;

	iDisplayMode = aDisplayMode;

	m_numTxt = 0;
	// Initialize EGL

	m_polygonMode = FILL_MODE;

	m_bPortrait = false;
//-----------------------------------
	InitGL(aWindow, aDisplayMode,aRect);
//--------------------------------------
	TInt count;
	for(count = 0; count < MAX_SHAPES; count++){
		m_statVerts[count] = NULL;
		m_statNorms[count] = NULL;
		m_stattxtCoords[count] = NULL;
		m_Inds[count] = NULL;
		m_statnumInd[count] = 0;
		m_statnumVerts[count] = 0;
	}

	memset(m_storedTxt, 0, sizeof(m_storedTxt));
	m_numbStoredTxt = 0;

	memset(m_allocMeshData, 0, sizeof(m_allocMeshData));
	m_numAllocMeshData = 0;

	memset(m_graphObj, 0, sizeof(m_graphObj));
	m_numbGraphObj = 0;

	m_numFrame = 0;

	memset(m_glMesh, 0, sizeof(m_glMesh));
	m_numMesh = 0;

	m_numShapes = 0;
}

void CGLengine::ConstructL (void){
	gWhisk3D = CWhisk3D::NewL();
	CreateObjects();
	updateState();
}

void CGLengine::TransformStatObj(TInt* aVerts, TInt numVerts, TMatrixGL& aM){
	TInt count;
	for(count = 0; count < numVerts; count++)
	{
		TVector3 aV(aVerts[3*count + 0], aVerts[3*count + 1], aVerts[3*count + 2]);

		aV = aM.TransformVertix(aV);

		aV = aV >> SCALE_SHIFT;

		aVerts[3*count + 0] = aV.iX;
		aVerts[3*count + 1] = aV.iY;
		aVerts[3*count + 2] = aV.iZ;

	}
};

void CGLengine::SaveStaticObjNumb(void)
{
	m_numStaticTxt = m_numTxt;
	m_numStaticMesh = m_numMesh;
	m_numStaticFrame = m_numFrame;
}

void CGLengine::ResetGL(void){
	TInt count;
	for(count = m_numStaticTxt; count < m_numTxt; count++)
		glDeleteTextures(1, m_txtID + count);

	m_numTxt = m_numStaticTxt;
	m_numMesh = m_numStaticMesh;
	m_numFrame = m_numStaticFrame;

}

CGLengine::~CGLengine (void){
	TInt count;

	for(count = 0; count < m_numTxt; count++)
		glDeleteTextures(1, m_txtID + count);

	DeleteGL();

	for(count = 0; count < MAX_SHAPES; count++)
	{
		if(m_statVerts[count])
			delete [] m_statVerts[count];
		if(m_statNorms[count])
			delete [] m_statNorms[count];
		if(m_stattxtCoords[count])
			delete [] m_stattxtCoords[count];
		if(m_Inds[count])
			delete m_Inds[count];
	}


	for(count = 0; count < MAX_STORED_TXTS; count++)
		if(m_storedTxt[count])
			delete [] m_storedTxt[count];

	for(count = 0; count < MAX_GLMESHES; count++)
		if(m_allocMeshData[count])
			delete [] m_allocMeshData[count];

}
 
TGLMesh*  CGLengine::AddMesh(TMeshData& aBase, TUint8* aPtr){
	TGLMesh& aMesh = m_glMesh[m_numMesh];
	aMesh.FillMesh(aBase);
	aMesh.GetBox(iMath);
	aMesh.m_ptrTxtArea = aPtr; // only for run-time generated txt

	m_numMesh++;

	return m_glMesh + m_numMesh-1;
};

TGLFrame* CGLengine::AllocFrame(){
	TInt count;
	for(count = 0; count < m_numFrame; count++)
	{
		TGLFrame& aFrameC = m_frameStorage[count];

		if(aFrameC.m_mesh == NULL)
		{
			aFrameC.m_Self2Parent.MakeIdentity();
			aFrameC.m_frameChild = NULL;
			aFrameC.m_frameSibling = NULL;
			aFrameC.m_clonedFrom = NULL;
			return m_frameStorage + count;
		}
	}

	if(m_numFrame >= MAX_GLFRAMES)
		return NULL;


	TGLFrame& aFrame = m_frameStorage[m_numFrame];

	m_numFrame++;

	aFrame.m_Self2Parent.MakeIdentity();
	aFrame.m_mesh = NULL;
	aFrame.m_frameChild = NULL;
	aFrame.m_frameSibling = NULL;
	aFrame.m_clonedFrom = NULL;
	return m_frameStorage+m_numFrame-1;
};

TGLFrame* CGLengine::AddFrame(TGLMesh* aMesh, TGLFrame* aParent, TGLFrame* aPrev){
	if(m_numFrame >= MAX_GLFRAMES)
		return NULL;

	TGLFrame& aFrame = m_frameStorage[m_numFrame];

	aFrame.m_mesh = aMesh;
	aFrame.m_meshNumb = 1;
	aFrame.m_frameBox = aMesh->m_box;
	aFrame.m_frameChild = NULL;
	aFrame.m_frameSibling = NULL;
	aFrame.m_clonedFrom = NULL;

	TGLFrame* aFramePtr = m_frameStorage+m_numFrame;

	if(aParent)
		aParent->m_frameChild = aFramePtr;
	if(aPrev)
		aPrev->m_frameSibling = aFramePtr;

	m_numFrame++;
	return m_frameStorage+m_numFrame-1;
};

void CGLengine::updateState(void){
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_LIGHTING);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glShadeModel(GL_FLAT);
	glDisable(GL_DITHER);
	glClearColorx( 134<<8, 70<<8, 160<<8, GLUNITY);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);


	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_TEXTURE_2D);

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	

	//glEnable(GL_ALPHA_TEST);
	//glAlphaFunc(GL_GREATER, .005);
	glPointSizex(3*GLUNITY);

	const GLint param = GL_MODULATE;
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, (GLfloat)param);


	TUint color[] = {GLUNITY, GLUNITY, GLUNITY };
	glColor4x(color[0], color[1], color[2], GLUNITY);

   	glClearColorx( 134<<8, 70<<8, 160<<8, GLUNITY);
	glClear( GL_DEPTH_BUFFER_BIT);
}

void CGLengine::renderFrame(TGLFrame* aFrame, const TVector3& aDir){
	glEnable(GL_CULL_FACE);
	//scaling
	glPushMatrix();
	glMultMatrixx(aFrame->m_Self2Parent.iElement);

	renderMesh(aFrame->m_mesh);

	TGLFrame* aChild = aFrame->m_frameChild;

	while(aChild){
		renderFrame(aChild, aDir);
		aChild = aChild->m_frameSibling;
	}

	glPopMatrix();
};

TBool CGLengine::FrameCheck(TGLFrame* aFrame){
	if(aFrame < m_frameStorage || aFrame >= m_frameStorage + MAX_GLFRAMES){
		if(iLogEnabled) //Failed FrameCheck
			iLog->WriteFormat(_L("Failed FrameCheck %i"), aFrame - m_frameStorage);
		return EFalse;
	}
	else
		return ETrue;
};

TGLFrame* CGLengine::allocCloneFrame(TGLFrame* aFrame){
	TGLFrame* newFrame = AllocFrame();

	if(newFrame == NULL)
		return NULL;

	return cloneFrameWithAlloc(aFrame, newFrame);
};

TGLFrame* CGLengine::cloneFrameWithAlloc(TGLFrame* aFrame, TGLFrame* newFrame){
 	newFrame->m_frameBox = aFrame->m_frameBox;
	newFrame->m_rad = aFrame->m_rad;

	newFrame->m_mesh = aFrame->m_mesh;
	newFrame->m_meshNumb = aFrame->m_meshNumb;
	newFrame->m_Self2Parent = aFrame->m_Self2Parent;
	newFrame->m_clonedFrom = aFrame;

	TGLFrame* aChild = aFrame->m_frameChild;

	if(aChild){
		TGLFrame* newChild = allocCloneFrame(aChild);
		if(newChild == NULL)
			return NULL;
		newFrame->m_frameChild = newChild;
	}

	TGLFrame* aSibling = aFrame->m_frameSibling;

	if(aSibling){
		TGLFrame* newSibling = allocCloneFrame(aSibling);
		if(newSibling == NULL)
			return NULL;

		newFrame->m_frameSibling = newSibling;
	}

	return newFrame;
}

TGLFrame* CGLengine::addCloneFrame(TGLFrame* aFrame){
	TGLFrame* newFrame = m_frameStorage+m_numFrame;

	if(m_numFrame >= MAX_GLFRAMES)
		return NULL;

	m_numFrame++;

	return cloneFrame(aFrame, newFrame);
};

TGLFrame* CGLengine::clonePureFrame(TGLFrame* aFrame, TGLFrame* newFrame){
	newFrame->m_frameBox = aFrame->m_frameBox;
	newFrame->m_rad = aFrame->m_rad;

	newFrame->m_mesh = aFrame->m_mesh;
	newFrame->m_meshNumb = aFrame->m_meshNumb;
	newFrame->m_Self2Parent = aFrame->m_Self2Parent;
	newFrame->m_clonedFrom = aFrame;

	TGLFrame* aChild = aFrame->m_frameChild;

	if(aChild)
	{
		clonePureFrame(newFrame->m_frameChild, aFrame);

	}

	TGLFrame* aSibling = aFrame->m_frameSibling;

	if(aSibling)
	{
		clonePureFrame(newFrame->m_frameSibling, aFrame->m_frameSibling);
	}

	return newFrame;
}


TGLFrame* CGLengine::cloneFrame(TGLFrame* aFrame, TGLFrame* newFrame){
	newFrame->m_frameBox = aFrame->m_frameBox;
	newFrame->m_rad = aFrame->m_rad;

	newFrame->m_mesh = aFrame->m_mesh;
	newFrame->m_meshNumb = aFrame->m_meshNumb;
	newFrame->m_Self2Parent = aFrame->m_Self2Parent;
	newFrame->m_clonedFrom = aFrame;

	TGLFrame* aChild = aFrame->m_frameChild;

	if(aChild)
	{
		TGLFrame* newChild = addCloneFrame(aChild);
		if(newChild == NULL)
			return NULL;
		newFrame->m_frameChild = newChild;
	}

	TGLFrame* aSibling = aFrame->m_frameSibling;

	if(aSibling)
	{
		TGLFrame* newSibling = addCloneFrame(aSibling);
		if(newSibling == NULL)
			return NULL;

		newFrame->m_frameSibling = newSibling;
	}

	return newFrame;
}

void CGLengine::deallocFrame(TGLFrame* aFrame){
	if(!FrameCheck(aFrame))
		return;

	aFrame->m_mesh = NULL;
	if(aFrame->m_frameChild)
	{
		deallocFrame(aFrame->m_frameChild);
	}

	if( aFrame->m_frameSibling)
	{
		deallocFrame(aFrame->m_frameSibling);
	}

	aFrame->m_frameChild = NULL;
	aFrame->m_frameSibling = NULL;
	aFrame->m_clonedFrom = NULL;
};

void CGLengine::SetColor(TInt* color){
	glColor4x(color[0], color[1], color[2], GLUNITY);
};

void CGLengine::SetColorTransp(TInt* color, TInt aAlpha){
	glColor4x(color[0], color[1], color[2], aAlpha);
};

void CGLengine::ResetColor(){
	glDisableClientState(GL_COLOR_ARRAY);
	glColor4x(GLUNITY, GLUNITY, GLUNITY, GLUNITY);
};

void CGLengine::renderMesh(TGLMesh* aMesh){
	if (Meshes.Count() > 0){
		glPushMatrix();
		glScalef(escala, escala, escala);
		Mesh* pMesh = Meshes[0];		

		//glEnableClientState(GL_NORMAL_ARRAY);
		
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);	
		glMaterialfv(   GL_FRONT_AND_BACK, GL_AMBIENT,  objAmbient  );
		glMaterialx( GL_FRONT_AND_BACK, GL_SHININESS,   12 << 16     );
		glMaterialfv(   GL_FRONT_AND_BACK, GL_EMISSION, ListaColores[negro] );

		// Set array pointers from mesh.
		glVertexPointer( 3, GL_FLOAT, 0, pMesh->vertex );
		glColorPointer( 4, GL_UNSIGNED_BYTE, 0, pMesh->vertexColor );	
		glNormalPointer( GL_BYTE, 0, pMesh->normals );
		glTexCoordPointer( 2, GL_FLOAT, 0, pMesh->uv );
		
		glShadeModel( GL_SMOOTH );
		
		for(int f=0; f < pMesh->materialsGroup.Count(); f++){
			Material* mat = pMesh->materialsGroup[f].material;	
			glMaterialfv(   GL_FRONT_AND_BACK, GL_DIFFUSE, mat->diffuse ); 
			glMaterialfv(   GL_FRONT_AND_BACK, GL_SPECULAR, mat->specular );

			//vertex color
			if (mat->vertexColor){
				glColor4f(ListaColores[negro][0],ListaColores[negro][1],ListaColores[negro][2],ListaColores[negro][3]);
				glEnableClientState( GL_COLOR_ARRAY );
				glEnable(GL_COLOR_MATERIAL);
			}
			else {
				glColor4f(mat->diffuse[0], mat->diffuse[1], mat->diffuse[2], mat->diffuse[3]);
				glDisable(GL_COLOR_MATERIAL);
				glDisableClientState( GL_COLOR_ARRAY );
			}		

			//si usa culling
			if (mat->culling){glEnable( GL_CULL_FACE );}
			else {glDisable( GL_CULL_FACE );}		 

			//si tiene iluminacion	
			/*if (mat->lighting){glEnable( GL_LIGHTING );}
			else {glDisable( GL_LIGHTING );}*/
			glDisable( GL_LIGHTING );
			
			//transparent
			if (mat->transparent){
				glEnable(GL_BLEND);
				glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			}
			else {glDisable(GL_BLEND);}
			
			//si tiene texturas
			if (mat->textura){
				glEnable( GL_TEXTURE_2D );

			    TTexture* tex = Textures[mat->textureIndex];

			    glBindTexture( GL_TEXTURE_2D, tex->iID);			
				//glBindTexture(  GL_TEXTURE_2D, mat->textureID ); //selecciona la textura				
			
				//textura pixelada o suave
				if (mat->interpolacion == lineal){
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				}
				else if (mat->interpolacion == closest){
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				}	
				//si la textura se repite
				if (mat->repeat){
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				}
				else {
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				}	
			}
			else {glDisable( GL_TEXTURE_2D );}

			glMaterialfv(   GL_FRONT_AND_BACK, GL_EMISSION, mat->emission );

			glDrawElements(GL_TRIANGLES,
			               pMesh->materialsGroup[f].indicesDrawnCount,
			               GL_UNSIGNED_SHORT,
			               &pMesh->faces[pMesh->materialsGroup[f].startDrawn]);
		};
		glPopMatrix();
		/*glColor4f(1.0f,1.0f,1.0f,1.0f);
		glEnableClientState( GL_COLOR_ARRAY );
		glEnable(GL_COLOR_MATERIAL);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisable( GL_LIGHTING );*/
		return;
	} 

	glVertexPointer(3, GL_FIXED, 0, aMesh->m_Verts);

	glDisable(GL_TEXTURE_2D);

	if(m_polygonMode == FILL_MODE)
		glDrawElements(GL_TRIANGLES, aMesh->m_numInd,
							   GL_UNSIGNED_SHORT, aMesh->m_Inds);
	else
		glDrawElements(GL_LINES, aMesh->m_numInd,
							   GL_UNSIGNED_SHORT, aMesh->m_Inds);
};

void TGLMesh::GetBox(TMathLookup& aMath){
	TInt count;
	m_box.m_boxZero = TVector3(1<<30, 1<<30, 1<<30);
	m_box.m_boxZ = TVector3(0, 0, 0);
	m_box.m_boxX = TVector3(0, 0, 0);
	m_box.m_boxY = TVector3(0, 0, 0);

	for(count =0; count < m_numVerts; count++){
		m_box.m_boxZero.iX = Min(m_box.m_boxZero.iX, m_Verts[3*count+0]);
		m_box.m_boxZero.iY = Min(m_box.m_boxZero.iY, m_Verts[3*count+1]);
		m_box.m_boxZero.iZ = Min(m_box.m_boxZero.iZ, m_Verts[3*count+2]);
	}

	for(count =0; count < m_numVerts; count++){
		m_box.m_boxX.iX = Max(m_box.m_boxX.iX, m_Verts[3*count+0] - m_box.m_boxZero.iX);
		m_box.m_boxZ.iZ = Max(m_box.m_boxZ.iZ, m_Verts[3*count+2] - m_box.m_boxZero.iZ);
		m_box.m_boxY.iY = Max(m_box.m_boxY.iY, m_Verts[3*count+1] - m_box.m_boxZero.iY);
	}

	UpdateBox(aMath, m_box);
}

void UpdateBox(TMathLookup& aMath, TBox& aBox){
	TVector3 aVx = aBox.m_boxX>>GL2U;
	TVector3 aVy = aBox.m_boxY>>GL2U;
	TVector3 aVz = aBox.m_boxZ>>GL2U;

	aBox.lx = aVx.GetLen(aMath);
	aBox.ly = aVy.GetLen(aMath);
	aBox.lz = aVz.GetLen(aMath);

	TInt xx = (aBox.lx*aBox.lx)>>UNITY_LOG;
	TInt yy = (aBox.ly*aBox.ly)>>UNITY_LOG;
	TInt zz = (aBox.lz*aBox.lz)>>UNITY_LOG;

	aMath.GetReciprocalShort(aBox.lx, aBox.inv_x, aBox.downshift_x);
	aMath.GetReciprocalShort(aBox.ly, aBox.inv_y, aBox.downshift_y);
	aMath.GetReciprocalShort(aBox.lz, aBox.inv_z, aBox.downshift_z);

	aBox.downshift_x -= UNITY_LOG;
	aBox.downshift_y -= UNITY_LOG;
	aBox.downshift_z -= UNITY_LOG;

	aMath.GetReciprocalShort(xx, aBox.inv_xx, aBox.downshift_xx);
	aMath.GetReciprocalShort(yy, aBox.inv_yy, aBox.downshift_yy);
	aMath.GetReciprocalShort(zz, aBox.inv_zz, aBox.downshift_zz);

	aBox.downshift_xx -= UNITY_LOG;
	aBox.downshift_yy -= UNITY_LOG;
	aBox.downshift_zz -= UNITY_LOG;
}


void MarkCrossGlob(TPoint aPoint, TInt* aColor){
	GLubyte crossLines[4] = {0,1,2,3};
	TVector3 aVert[4];
	TVector3 aV(GLUNITY*aPoint.iX, GLUNITY*aPoint.iY, 0); 
	TVector3 aX(GLUNITY*10, 0, 0);
	TVector3 aY(0, GLUNITY*10, 0);
	aVert[0] = aV - aX;
	aVert[1] = aV + aX;
	aVert[2] = aV - aY;
	aVert[3] = aV + aY;
	glDisable(GL_TEXTURE_2D);
	glColor4x(aColor[0], aColor[1], aColor[2], aColor[3]);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer( 3, GL_FIXED, 0, aVert);
    glDrawElements(GL_LINES, 4, GL_UNSIGNED_BYTE, crossLines );
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_TEXTURE_2D);
	glColor4x(GLUNITY, GLUNITY, GLUNITY, GLUNITY);
}

void CGLengine::OutText(TInt* aMsg)
{

	if(aMsg[0] < 0 || aMsg[0] >= TEXT_ENTRIES)
		return;

	TBuf8<TEXT_SIZE> aFormatBuf;
	aFormatBuf.Format(iText[aMsg[0]]);

	SetText( aFormatBuf);	
}

void GLLine(TVector3 vV0, TVector3 vV1, TInt* aColor)
{
	glDisable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	TVector3 aP[2];

	aP[0] = vV0<<GL2U;
	aP[1] = vV1<<GL2U;
	glColor4x(aColor[0], aColor[1], aColor[2], GLUNITY);
	glVertexPointer(3, GL_FIXED, 0, aP);
	glDrawArrays(GL_LINES, 0, 2);
	glColor4x(GLUNITY, GLUNITY, GLUNITY, GLUNITY);

	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

void DrawPixLine(TVector3 vV0, TVector3 vV1, TInt* aColor)
{
//downsampl
	vV0.iX *= V2RX;
	vV0.iY *= V2RY;

	vV1.iX *= V2RX;
	vV1.iY *= V2RY;


	vV0.iY = VFHEIGHT - 1 - vV0.iY;
	vV1.iY = VFHEIGHT - 1 - vV1.iY;

	vV0 = UNITY*vV0;
	vV1 = UNITY*vV1;

	GLLine(vV0, vV1, aColor);
}

void DrawPixLine(TPoint aP0, TPoint aP1, TInt* aColor){
	TVector3 vV0(aP0.iX, aP0.iY, 0);
	TVector3 vV1(aP1.iX, aP1.iY, 0);

	DrawPixLine(vV0, vV1, aColor);
}

void MarkQPoint(TPoint &aPoint, TInt delta, TInt* aColor){
//temp
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrthof(0, VFWIDTH, 0, VFHEIGHT, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	glViewport(0, 0, VFWIDTH, VFHEIGHT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	TInt aX = aPoint.iX;
	TInt aY = aPoint.iY;

	TPoint aP0(aX+delta,aY+delta);
	TPoint aP1(aX-delta,aY+delta);
	TPoint aP2(aX-delta,aY-delta);
	TPoint aP3(aX+delta,aY-delta);

	DrawPixLine(aP0, aP1, aColor);
	DrawPixLine(aP1, aP2, aColor);
	DrawPixLine(aP2, aP3, aColor);
	DrawPixLine(aP3, aP0, aColor);

}

void MarkCPoint(TPoint aPoint, TInt delta, TInt* aColor){
//temp
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrthof(0, VFWIDTH, 0, VFHEIGHT, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	glViewport(0, 0, VFWIDTH, VFHEIGHT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	TInt aX = aPoint.iX;
	TInt aY = aPoint.iY;

	TPoint aP0(aX+delta,aY);
	TPoint aP1(aX-delta,aY);
	TPoint aP2(aX,aY+delta);
	TPoint aP3(aX,aY-delta);

	DrawPixLine(aP0, aP1, aColor);
	DrawPixLine(aP2, aP3, aColor);

}

TDisplayStat::TDisplayStat(){
	ResetMsg();

	m_msgBlock = -1;
};

void TDisplayStat::ResetMsg()
{
	m_numbMsg = 0;
	m_msgDelay = 0;
	m_msgStack[0] = -1;
	m_msgBlock = -1;
};

void TDisplayStat::PipeMsg(){
	if(m_numbMsg <= 0){
		return;
	}

	if(m_msgDelay >= MSG_DELAY/m_numbMsg)
	{
		TInt count;
		for(count  = 0; count < m_numbMsg-1; count++)
		{
			memcpy(m_msgStack + count*MAX_MSG_LEN, m_msgStack + (count+1)*MAX_MSG_LEN, MAX_MSG_LEN*sizeof(TInt));
		}

		m_numbMsg--;
		m_msgDelay = 0;
		
	}
	else
		m_msgDelay++;

};

void TDisplayStat::AddMsg(TInt* aMsg){
	if(aMsg[0] == m_msgBlock)
		return;

	while(m_numbMsg >= MAX_MSGES)
	{
		m_msgDelay = MSG_DELAY;
		PipeMsg();
	}
//eat the same msg
	if(m_numbMsg > 0 && aMsg[0] == m_msgStack[(m_numbMsg-1)*MAX_MSG_LEN])
		m_numbMsg--;

	if(m_numbMsg > 0 && (aMsg[0] == 20 || aMsg[0] == 21) &&  
		(m_msgStack[(m_numbMsg-1)*MAX_MSG_LEN] == 20 || m_msgStack[(m_numbMsg-1)*MAX_MSG_LEN] == 21) )
		m_numbMsg--;

	memcpy(m_msgStack + m_numbMsg*MAX_MSG_LEN, aMsg, MAX_MSG_LEN*sizeof(TInt));
	m_numbMsg++;
	m_msgDelay = 0;
};

void TDisplayStat::MsgBlock(TInt aMsgID){
	m_msgBlock = aMsgID;
};
