#include <Whisk3D.h>

GLfloat ListaColores[11][4] = {
	{ MATERIALCOLOR(0.22, 0.28, 0.25, 1.0)  },   //fondo
	{ MATERIALCOLOR(1.0, 1.0, 1.0, 1.0)     },   //blanco
	{ MATERIALCOLOR(0.91, 0.50, 0.98, 1.0)  },   //acento (violeta) 
	{ MATERIALCOLOR(0.48, 0.30, 1.0, 1.0)  },   //acento oscuro	
	{ MATERIALCOLOR(0.0, 0.0, 0.0, 1.0)     },   //negro
	{ MATERIALCOLOR(0.12, 0.12, 0.12, 1.0)  },   //gris
	{ MATERIALCOLOR(0.94, 0.59, 0.17, 0.25f)},   //naranja transparente
	{ MATERIALCOLOR(0.22, 0.22, 0.22, 1.0)  },    //cabezera de la barra de herramientas
	{ MATERIALCOLOR(0.0, 0.0, 0.0, 0.25f)     },   //negroTransparente
	{ MATERIALCOLOR(0.757, 0.757, 0.757, 1.0)     },   //azul de los iconos seleccionados
};

GLfloat objAmbient[4] = { MATERIALCOLOR(0.4, 0.4, 0.4, 1.0) };

RArray<TTexture*> Textures;
RArray<Material*> Materials;
RArray<Mesh*> Meshes;

GLfloat escala = 0.2f;

// -----------------------------------------------------------------------------
// CWhisk3D::CWhisk3D
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CWhisk3D::CWhisk3D() : CFiniteStateMachine()
	{
    iScreenWidth  = 320;
    iScreenHeight = 240;	
	}

void CWhisk3D::ConstructL( void ){}

// -----------------------------------------------------------------------------
// CWhisk3D::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWhisk3D* CWhisk3D::NewL(){
    /* Symbian 2-phase constructor. Calls both the default
       C++ constructor and Symbian ConstructL methods */
    CWhisk3D* self = new (ELeave) CWhisk3D();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();

    return self;
}

// Destructor.
CWhisk3D::~CWhisk3D(){
    //delete iContainer;
}

// -------------------------------------------------------------------------------------------------------
// CWhisk3D::OnStartLoadingTextures()
// Called for a MTextureLoadingListener by the texture manager when texture loading operation starts
// -------------------------------------------------------------------------------------------------------

void CWhisk3D::OnStartLoadingTexturesL(){
    SetStateL( ELoadingTextures );
}

// ------------------------------------------------------------------------------------------------------------
// CWhisk3D::OnEndLoadingTextures()
// Called for a MTextureLoadingListener by the texture manager when texture loading operation is completed
// ------------------------------------------------------------------------------------------------------------
void CWhisk3D::OnEndLoadingTexturesL(){
	if ( GetState() == ELoadingTextures ){
		SetStateL( ERunning );
	}
}

CWhisk3D* gWhisk3D = NULL;
