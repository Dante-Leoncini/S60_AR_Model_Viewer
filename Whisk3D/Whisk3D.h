#ifndef WHISK3D_H
#define WHISK3D_H

#include <GLES/gl.h> // OpenGL ES header file
#include <e32base.h>

#define FRUSTUM_LEFT   -1.f     //left vertical clipping plane
#define FRUSTUM_RIGHT   1.f     //right vertical clipping plane
#define FRUSTUM_BOTTOM -1.f     //bottom horizontal clipping plane
#define FRUSTUM_TOP     1.f     //top horizontal clipping plane
#define FRUSTUM_NEAR    3.f     //near depth clipping plane
#define FRUSTUM_FAR  1000.f     //far depth clipping plane

#define MATERIAL_MAX 1

#define MATERIALCOLOR(r, g, b, a)     \
       (GLfloat)(r * MATERIAL_MAX),   \
       (GLfloat)(g * MATERIAL_MAX),   \
       (GLfloat)(b * MATERIAL_MAX),   \
       (GLfloat)(a * MATERIAL_MAX)

extern GLfloat objAmbient[4];

#include "../Utils/Utils3d.h" // Utilities (texmanager, textures etc.)

enum {lineal, closest};

enum{
	background,
	blanco,
	accent,
	accentDark,
	negro,
	gris,
	naranjaFace,
	headerColor,
	negroTransparente,
	grisUI
};

extern GLfloat ListaColores[11][4];

class Material { 
	public:
		TBool textura;
	    TInt textureIndex;   // índice en Textures
		TBool transparent;
		TBool vertexColor;
		TBool lighting;
		TBool repeat;
		TBool culling;
		TInt interpolacion;
		GLuint textureID;
		GLfloat diffuse[4];		
		GLfloat specular[4];	
		GLfloat emission[4];
		HBufC* name;
};

extern RArray<TTexture*> Textures;
extern RArray<Material*> Materials;

class MaterialGroup { 
	public:
        TInt start; //donde esta el primer triangulo real
        TInt count; //cuantos triangulos son reales

        TInt startDrawn; //indice del primer triangulo para dibujar
		TInt indicesDrawnCount; //cuantos vertices son
		Material* material; //que material
};

class Mesh { 
	public:
		TInt vertexSize;
		GLfloat* vertex;
		GLubyte* vertexColor;
		GLbyte* normals;
		GLfloat* uv;

		//caras
   		TInt facesSize;
   		GLushort* faces;
        RArray<MaterialGroup> materialsGroup;
        
		HBufC* name;

		//libera la memoria
		void Mesh::LiberarMemoria(){
			delete[] vertex;
			delete[] vertexColor;
			delete[] normals;
			delete[] uv;
			delete[] faces;
			materialsGroup.Close();
		}
};

extern RArray<TTexture*> Textures;
extern RArray<Mesh*> Meshes;

extern GLfloat escala;

class CWhisk3D : public CFiniteStateMachine, public MTextureLoadingListener
    {
    public:  // Constructors and destructor

        /**
         * Factory method for creating a new CSimpleCube object.
         * @param iInputHandler Input handler that maps keycodes to inputs and stores the current state for each key.
         */
        static CWhisk3D* NewL();

        /**
         * Destructor. Does nothing.
         */
        virtual ~CWhisk3D();
            
    public:  // New functions
        
        /**
		 * Called when texture manager starts loading the textures.
		 * Sets the current state to "loading textures".
		 */
		void OnStartLoadingTexturesL();

		/**
		 * Called when texture manager has completed texture loading.
		 * Changes the current state to "running".
		 */
		void OnEndLoadingTexturesL();

    protected: // New functions

        /**
         * Standard constructor that must never Leave.
         * Stores the given screen width and height.
         * @param aWidth Width of the screen.
         * @param aHeight Height of the screen.
         */
        CWhisk3D();

        /**
         * Second phase contructor. Does nothing.
         */
        void ConstructL( void );

    public:  // Data

		/**
		 * Application states:
		 * ELoadingTextures - indicates that the app. is loading textures.
		 * ERunning - indicates that the app. is running.
		 */
		enum {
			ELoadingTextures,
			ERunning
		};
        
		/** Texture manager that is used to load the used textures. */
		CTextureManager * iTextureManager;
		/** Width of the screen */
		TUint iScreenWidth;
		/** Height of the screen */
		TUint iScreenHeight;
};
	
extern CWhisk3D* gWhisk3D;

#endif
