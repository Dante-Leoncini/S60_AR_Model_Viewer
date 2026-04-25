#include <ImportOBJ.h>
#include <ARDemo.rsg>

#include <utf.h>

void ImportOBJ(){	
	_LIT(KTitle, "Import Wavefront (.obj)");
	TFileName file(KNullDesC);
	if (AknCommonDialogs::RunSelectDlgLD(file, R_SELECT_DIALOG, KTitle)){		
		RFs fsSession;	
		User::LeaveIfError(fsSession.Connect());
		CleanupClosePushL(fsSession);

		// Revisar la extension del archivo
		TPtrC extension = file.Right(4);  // Obtiene las últimas 4 letras del nombre del archivo
		if (extension.CompareF(_L(".obj")) != 0) {
			_LIT(KExtensionError, "Error: El archivo seleccionado no tiene la extension .obj");
			CAknMessageQueryDialog* dlg = new (ELeave) CAknMessageQueryDialog();
			dlg->PrepareLC(R_AVKON_MESSAGE_QUERY_DIALOG);

			dlg->SetHeaderTextL(_L("Error"));
			dlg->SetMessageTextL(KExtensionError);

			dlg->RunLD(); // BLOQUEA hasta que el usuario cierre
			
			fsSession.Close();
			return;
		}

		RFile rFile;
		TInt err;	

		TRAP(err,rFile.Open(fsSession, file, EFileRead));
		if (err != KErrNone){
			_LIT(KFormatString, "Error al abrir: %S");
			CAknMessageQueryDialog* dlg = new (ELeave) CAknMessageQueryDialog();
			dlg->PrepareLC(R_AVKON_MESSAGE_QUERY_DIALOG);

			dlg->SetHeaderTextL(_L("Error"));
			dlg->SetMessageTextL(KFormatString);

			dlg->RunLD(); // BLOQUEA hasta que el usuario cierre
			return;
		}	

		TInt64 startPos = 0; // Variable para mantener la posicion de lectura en el archivo
		//esto se hace para no cerrar el archivo y por cada nuevo obj encontrado simplemente volvemos a usar leerOBJ con el archivo donde quedo
		//TBool hayMasObjetos;
		TInt objetosCargados = 0;		
		TInt acumuladoVertices = 0;
		TInt acumuladoNormales = 0;
		TInt acumuladoUVs = 0;
		while (LeerOBJ(&fsSession, &rFile, &file, &startPos, &acumuladoVertices, &acumuladoNormales, &acumuladoUVs)) { // && objetosCargados < 1
			objetosCargados++;
		}

		// Cerrar el archivo
		rFile.Close();
		fsSession.Close();

		TFileName mtlFile = file;
		mtlFile.Replace(file.Length() - 4, 4, _L(".mtl"));

		RFs fs;
		User::LeaveIfError(fs.Connect()); // Asegurarse de que fs se conecta correctamente
		CleanupClosePushL(fs);
		
		TEntry entry;
		err = fs.Entry(mtlFile, entry);

		//si el archivo existe. no tendria que marcar error
		if (err == KErrNone) {
			TRAP(err, LeerMTL(mtlFile, objetosCargados));
			//si ocurrio algun error al leerlo
			if (err != KErrNone) {
				//_LIT(KFormatString, "Error al leer el archivo .mtl");
			}
		} else {
			// El archivo no existe, manejar el error
			//_LIT(KFileNotFound, "El archivo .mtl no existe");
		}
		fs.Close();
		
		_LIT(KFormatString, "Vertices: %d\nTextures array: %d\nTextures GL: %d");

		TInt texturesArray = Textures.Count();
		TInt texturesGL = 0;

		// contar cuantas tienen ID valido
		for (TInt i = 0; i < Textures.Count(); i++)
		{
		    if (Textures[i] && Textures[i]->iID != 0)
		        texturesGL++;
		}

		TBuf<128> msg;
		msg.Format(KFormatString, acumuladoVertices, texturesArray, texturesGL);

		CAknMessageQueryDialog* dlg = new (ELeave) CAknMessageQueryDialog();
		dlg->PrepareLC(R_AVKON_MESSAGE_QUERY_DIALOG);

		dlg->SetHeaderTextL(_L("Debug OBJ"));
		dlg->SetMessageTextL(msg);

		dlg->RunLD();
	}
}
		
TBool LeerOBJ(
    RFs* fsSession,
    RFile* rFile,
    TFileName* file,
    TInt64* startPos,
    TInt* acumuladoVertices,
    TInt* acumuladoNormales,
    TInt* acumuladoUVs
){
	TInt err;
	TBool NombreEncontrado = false;
	TBool hayMasObjetos = false;

	Wavefront Wobj;
	Wobj.Reset();
	Face NewFace;
	FaceCorners NewFaceCorners;
	TBool TieneVertexColor = false;
	TInt acumuladoVerticesProximo = 0;
	TInt acumuladoNormalesProximo = 0;
	TInt acumuladoUVsProximo = 0;

	TBool continuarLeyendo = true; // Variable para controlar la lectura del archivo
	TBuf8<2048> buffer;
	TInt pos = 0;
	TInt fileSize;
	rFile->Size(fileSize);
	
	Mesh* mesh = new Mesh();

	while (continuarLeyendo && *startPos < fileSize ) {
		// Leer una linea del archivo desde la posicion actual
		err = rFile->Read(*startPos, buffer, buffer.MaxLength());
		if (err != KErrNone) {
			_LIT(KFormatString, "Error al leer el archivo .mtl");
							
			CAknMessageQueryDialog* dlg = new (ELeave) CAknMessageQueryDialog();
			dlg->PrepareLC(R_AVKON_MESSAGE_QUERY_DIALOG);

			dlg->SetHeaderTextL(_L("Error"));
			dlg->SetMessageTextL(KFormatString);

			dlg->RunLD(); // BLOQUEA hasta que el usuario cierre
			continuarLeyendo = false; // Salir del bucle
			break;
		}
		// Procesar la linea hasta que no haya mas caracteres en buffer
		while (continuarLeyendo && (pos = buffer.Locate('\n')) != KErrNotFound || (pos = buffer.Locate('\r')) != KErrNotFound) {
			TPtrC8 line = buffer.Left(pos);
		
			// Contador para almacenar la cantidad de "strings" separados por espacios
			TInt contador = 0;
			if (line.Length() > 0) {
				if (!NombreEncontrado && line.Left(2) == _L8("o ")) {
					//evita el crasheo en caso de que no tenga materiales
					//se le asigna el material por defecto

					TLex8 lex(line.Mid(2));
					if (!lex.Eos()){
						TPtrC8 currentString = lex.NextToken();							
						mesh->name = HBufC::NewL(currentString.Length());
						mesh->name->Des().Copy(currentString);
					}

					NombreEncontrado = true;
				}
				//si encuentra otro objeto para
				else if (NombreEncontrado && line.Left(2) == _L8("o ")) {
					continuarLeyendo = false; // Salir del bucle
					hayMasObjetos = true;
				}
				else {
					if (line.Left(2) == _L8("v ")) {
						contador = 0;
						Wobj.vertex.ReserveL(Wobj.vertex.Count()+3);
						Wobj.vertexColor.ReserveL(Wobj.vertexColor.Count()+3);
						acumuladoVerticesProximo++;

						TLex8 lex(line.Mid(2));  // Inicializa TLex con la subcadena a partir del tercer caracter
						// Iterar mientras no se llegue al final del descriptor y se haya alcanzado el limite de 8 strings
						while (!lex.Eos() && contador < 6) {		
							TPtrC8 currentString = lex.NextToken(); // Mostrar el mensaje con el valor actual del "string" y el contador					
							TLex8 testLex(currentString);// Crear un nuevo objeto TLex para la prueba
							
							// Convertir el string en un número TInt
							TReal number = 0.0;
							TInt err = testLex.Val(number, '.');
							if (err == KErrNone){
								if (contador < 3) {
									//number = number;//*2000;								
									GLfloat glNumber = static_cast<GLfloat>(number);
									Wobj.vertex.Append(glNumber);							
								}
								//que el vertice tenga el color es una mierda. se hara un nuevo formato y se guardara en el face corner...							
								else if (contador < 6){
									number = number*255.0;	
									if (number > 255.0){number = 255.0;}	
									GLshort glNumber = static_cast<GLubyte>(number); // Conversion a GLbyte
									Wobj.vertexColor.Append(glNumber);	
									TieneVertexColor = true;
								}
							}

							// Avanzar al siguiente "string" que no sea espacio en blanco
							lex.SkipSpace();

							// Incrementar el contador para llevar la cuenta de los strings procesados
							contador++;
						}
						//en caso de no tener color
						while (contador < 6) {		
							if (contador > 2){
								Wobj.vertexColor.Append(255);	
							}
							contador++;
						}
					}
					else if (line.Left(3) == _L8("vn ")) {
						acumuladoNormalesProximo++;
						contador = 0;
						Wobj.normals.ReserveL(Wobj.normals.Count()+3);
						TLex8 lex(line.Mid(3));  // Inicializa TLex con la subcadena a partir del tercer caracter
						// Iterar mientras no se llegue al final del descriptor y se haya alcanzado el limite de 8 strings
						while (!lex.Eos() && contador < 3) {							
							TPtrC8 currentString = lex.NextToken(); // Mostrar el mensaje con el valor actual del "string" y el contador
							TLex8 testLex(currentString); // Crear un nuevo objeto TLex para la prueba
							
							TReal number = 0.0;
							TInt err = testLex.Val(number, '.');
							if (err == KErrNone && contador < 3) {
								number = ((number +1)/2)* 255.0 - 128.0;
								if (number > 127.0){number = 127.0;}
								else if (number < -128.0){number = -128.0;}
								GLbyte glNumber = static_cast<GLbyte>(number); // Conversion a GLbyte
								Wobj.normals.Append(glNumber);
							}
							lex.SkipSpace();
							contador++;
						}			
					}
					else if (line.Left(3) == _L8("vt ")) {	
						acumuladoUVsProximo++;				
						contador = 0;
						Wobj.uv.ReserveL(Wobj.uv.Count()+2);

						TLex8 lex(line.Mid(3));  // Inicializa TLex con la subcadena a partir del tercer caracter
						// Iterar mientras no se llegue al final del descriptor y se haya alcanzado el limite de 8 strings
						while (!lex.Eos() && contador < 2) {							
							TPtrC8 currentString = lex.NextToken(); // Mostrar el mensaje con el valor actual del "string" y el contador
							TLex8 testLex(currentString); // Crear un nuevo objeto TLex para la prueba
							
							TReal number = 0.0;
							TInt err = testLex.Val(number, '.');
							if (err == KErrNone) {	
								GLfloat glNumber;
								switch (contador) {
									case 0:
										//number = number * 255.0 - 128.0;
										glNumber = static_cast<GLfloat>(number); // Conversion a GLbyte
										//newVertexUV.u = glNumber;
										Wobj.uv.Append(glNumber);
										break;
									case 1:
										number = -number;//+1;
										//number = number * 255.0 - 128.0;
										glNumber = static_cast<GLfloat>(number); // Conversion a GLbyte
										//number = number * 255.0 - 128.0;
										//newVertexUV.v = glNumber;
										Wobj.uv.Append(glNumber);
										break;
									default:
										break;
								}		
							}
							contador++;
							lex.SkipSpace();
						}	
					}
					else if (line.Left(2) == _L8("f ")) {
						contador = 0;
						TInt conTBarras = 0;

						TLex8 lex(line.Mid(2));  // Inicializa TLex con la subcadena a partir del tercer caracter

						Wobj.faces.Append(NewFace);
						Face& TempFace = Wobj.faces[Wobj.faces.Count()-1];
						MaterialGroup& UltimoMG = Wobj.materialsGroup[Wobj.materialsGroup.Count()-1];
													
						while (!lex.Eos()) {		
							TPtrC8 currentString = lex.NextToken();

							TInt startPos2 = 0; // Posicion inicial
							TInt tokenLength = 0; // Longitud del token
							conTBarras = 0;

							// Recorre la cadena hasta encontrar el final
							while (startPos2 < currentString.Length()) {
								// Busca la posicion de la siguiente barra diagonal
								TInt nextSlashPos = currentString.Mid(startPos2).Locate('/');

								// Si no se encuentra una barra diagonal, asume que es el último token
								if (nextSlashPos == KErrNotFound) {
									tokenLength = currentString.Length() - startPos2;
								} else {
									tokenLength = nextSlashPos; // Longitud del token hasta la barra diagonal
								}

								// Extrae el token utilizando la posicion y longitud calculadas
								TPtrC8 token = currentString.Mid(startPos2, tokenLength);
								TLex8 testLex(token); // Crear un nuevo objeto TLex para la prueba

								// Convertir el string en un número TInt
								TInt number = 0;
								TInt err = testLex.Val(number);
								//v1/vt1/vn1
								if (err == KErrNone) {
									//resetea el contador de barras si se pasa
									if (conTBarras > 2){
										conTBarras = 0;
									};
									switch (conTBarras) {
										case 0:
											NewFaceCorners.vertex = number-1;
											break;
										case 1:
											NewFaceCorners.uv = number-1;
											break;
										case 2:
											//se da por hecho que aca se termino un triangulo
											NewFaceCorners.normal = number-1;
											TempFace.corner.Append(NewFaceCorners);
											break;
										default:
											break;
									}	
									//cada face corner extra es un triangulo
									if (contador > 1 && conTBarras == 2){
										Wobj.facesCount++;
										Wobj.facesSize += 3;	
										UltimoMG.count++;
										UltimoMG.indicesDrawnCount += 3;
									}
								}

								// Actualiza la posicion inicial para el proximo token
								startPos2 += tokenLength + 1; // Suma 1 para omitir la barra diagonal
								conTBarras++;
							}	

							lex.SkipSpace();
							contador++;				
						}
					}
					else if (line.Left(7) == _L8("usemtl ")) {	
						MaterialGroup tempFaceGroup;
						tempFaceGroup.count = 0;
						tempFaceGroup.indicesDrawnCount = 0;
						tempFaceGroup.start = Wobj.facesSize/3;
						tempFaceGroup.startDrawn = Wobj.facesSize;
						tempFaceGroup.material = 0;

						Wobj.materialsGroup.Append(tempFaceGroup);
						tempFaceGroup.count = 0;
						Material* mat = new Material();	
						mat->specular[0] = mat->specular[1] = mat->specular[2] = mat->specular[3] = 0.3;
						mat->diffuse[0] = mat->diffuse[1] = mat->diffuse[2] = mat->diffuse[3] = 1.0;
						mat->emission[0] = mat->emission[1] = mat->emission[2] = mat->emission[3] = 0.0;
						mat->textura = false;
						mat->vertexColor = TieneVertexColor;
						mat->repeat = true;
						mat->lighting = true;
						mat->culling = true;
						mat->transparent = false;
						mat->interpolacion = lineal;
						mat->textureIndex = 0;
						mat->textureID = 0;
						HBufC* materialName16 = HBufC::NewLC(180);

						TInt encontrado = -1;	
						TLex8 lex(line.Mid(7));
						if (!lex.Eos()){
							TPtrC8 currentString = lex.NextToken();							
							mat->name = HBufC::NewL(currentString.Length());
							mat->name->Des().Copy(currentString);

							//busca si existe el material
							materialName16 = CnvUtfConverter::ConvertToUnicodeFromUtf8L(currentString);
							for(int f=0; f < Materials.Count(); f++){
								if (Materials[f]->name->Compare(*materialName16) == 0 && f != 0){
									encontrado = f;
									break;
								}
							}
						}
						else {	
							mat->name = HBufC::NewL(100);						
							_LIT(KMatName, "Material.%03d");
							mat->name->Des().Format(KMatName, Materials.Count()+1);
						}

						//si se encontro no crea el nuevo material
						if (encontrado > -1){
							Wobj.materialsGroup[Wobj.materialsGroup.Count()-1].material = Materials[encontrado];
						}
						else {
							Materials.Append(mat);
							Wobj.materialsGroup[Wobj.materialsGroup.Count()-1].material = Materials[Materials.Count()-1];
						}
						CleanupStack::PopAndDestroy(materialName16);
					}
				}
			}
			if (continuarLeyendo){
				// Actualizar la posicion de inicio para la proxima lectura
				*startPos += pos + 1;

				// Eliminar la parte de la linea ya procesada y el caracter de salto de linea
				buffer.Delete(0, pos + 1);
				buffer.TrimLeft(); // Eliminar espacios en blanco iniciales
			}
		}
	}
	
	Meshes.Append(mesh);
	
	Wobj.ConvertToES1(*mesh, acumuladoVertices, acumuladoNormales, acumuladoUVs);
	*acumuladoVertices += acumuladoVerticesProximo;
	*acumuladoNormales += acumuladoNormalesProximo;
	*acumuladoUVs += acumuladoUVsProximo;

	return false;
	//return hayMasObjetos;
}

TBool LeerMTL(const TFileName& aFile, TInt objetosCargado){
	RFs fsSession2;	
	User::LeaveIfError(fsSession2.Connect());

	RFile rFile;
	TInt err;
	
	TRAP(err,rFile.Open(fsSession2, aFile, EFileRead));
	if (err != KErrNone){
		//_LIT(KFormatString, "Error al abrir: %S");
        return false;
    }

	TBuf8<2048> buffer;
	TInt pos = 0;
	TInt64 startPos = 0; // Variable para mantener la posicion de lectura en el archivo
	TInt fileSize;
	rFile.Size(fileSize);

	HBufC* materialName16 = HBufC::NewLC(180);
	//HBufC* materialName16 = NULL;
	Material* mat = NULL; 
	TBool encontrado = false;

	// Cargar la textura desde la ruta absoluta
	TBool HaytexturasQueCargar = false;

	/*if (!gWhisk3D){		
		return true;		
	}*/
	
	gWhisk3D->iTextureManager = CTextureManager::NewL(gWhisk3D->iScreenWidth, gWhisk3D->iScreenHeight,
										FRUSTUM_TOP, FRUSTUM_BOTTOM, FRUSTUM_RIGHT, FRUSTUM_LEFT, FRUSTUM_NEAR,
										gWhisk3D); 

	while (startPos < fileSize) {
		// Leer una linea del archivo desde la posicion actual
		err = rFile.Read(startPos, buffer, buffer.MaxLength());
		if (err != KErrNone) {
			//_LIT(KFormatString, "Error al leer linea");
    		rFile.Close();	
    		fsSession2.Close();	
            //CleanupStack::PopAndDestroy(&fsSession);
			break;
		}      

        while ((pos = buffer.Locate('\n')) != KErrNotFound || (pos = buffer.Locate('\r')) != KErrNotFound) {
			//TInt indice = 0;
            TPtrC8 line = buffer.Left(pos);

            if (line.Length() > 0) {
                if (line.Left(7) == _L8("newmtl ")) {
                    TLex8 lex(line.Mid(7));
                    TPtrC8 materialName = lex.NextToken();

					// Convertir materialName de TPtrC8 (8 bits) a HBufC (16 bits)
					//delete materialName16;
					materialName16 = CnvUtfConverter::ConvertToUnicodeFromUtf8L(materialName);

					//buscar el material con el mismo nombre
					encontrado = false;
					
					for (TInt m = 0; m < Materials.Count(); m++) {
					    // Compara el nombre del material con el proporcionado
					    if (Materials[m]->name->Compare(*materialName16) == 0) {
					        mat = Materials[m];
					        encontrado = true;
					        break;
					    }
					}
                } 
				else if (encontrado){
					//specular
					if (line.Left(3) == _L8("Ns ")) {
						TLex8 lex(line.Mid(3));
						TReal nsValue;
						lex.Val(nsValue, '.');
						GLfloat resultado = nsValue/1000.0f;
						mat->specular[0] = resultado;
						mat->specular[1] = resultado;
						mat->specular[2] = resultado;
						mat->specular[3] = resultado;
					} 
					//difusso, Aqui manejas el color ambiental Ka (kaR, kaG, kaB)			
					else if (line.Left(3) == _L8("Kd ")) {
						TLex8 lex(line.Mid(3));
						TReal kdR, kdG, kdB;
						lex.Val(kdR, '.');
						lex.SkipSpace();
						lex.Val(kdG, '.');
						lex.SkipSpace();
						lex.Val(kdB, '.');

						mat->diffuse[0] = (GLfloat)kdR;
						mat->diffuse[1] = (GLfloat)kdG;
						mat->diffuse[2] = (GLfloat)kdB;
					} 
					/*else if (line.Left(3) == _L8("Ks ")) {
						TLex8 lex(line.Mid(3));
						TReal ksR, ksG, ksB;
						lex.Val(ksR, '.');
						lex.SkipSpace();
						lex.Val(ksG, '.');
						lex.SkipSpace();
						lex.Val(ksB, '.');
						// Aqui manejas el color especular Ks (ksR, ksG, ksB)
					}*/
					// Aqui manejas el color de emision Ke (keR, keG, keB)
					else if (line.Left(3) == _L8("Ke ")) {
						TLex8 lex(line.Mid(3));
						TReal keR, keG, keB;
						lex.Val(keR, '.');
						lex.SkipSpace();
						lex.Val(keG, '.');
						lex.SkipSpace();
						lex.Val(keB, '.');	
						mat->emission[0] = (GLfloat)keR;
						mat->emission[1] = (GLfloat)keG;
						mat->emission[2] = (GLfloat)keB;
					} 
					/*else if (line.Left(3) == _L8("Ni ")) {
						TLex8 lex(line.Mid(3));
						TReal niValue;
						lex.Val(niValue, '.');
						// Aqui manejas el indice de refraccion Ni
					}*/
					//opacidad 
					else if (line.Left(2) == _L8("d ")) {
						TLex8 lex(line.Mid(2));
						TReal dValue;
						lex.Val(dValue, '.');					
						mat->diffuse[3] = (GLfloat)dValue;
						//setea la transparencia deacuerdo al alpha
						if (dValue < 1.f){mat->transparent = true;}
						else {mat->transparent = false;}
					} 
					/*else if (line.Left(6) == _L8("illum ")) {
						TLex8 lex(line.Mid(6));
						TInt illumValue;
						lex.Val(illumValue);
						// Aqui manejas el modelo de iluminacion illum
					}*/
					// Aqui manejas la textura difusa map_Kd
					else if (line.Left(18) == _L8("BackfaceCullingOff")){
						mat->culling = false;
					}
					else if (line.Left(7) == _L8("NoLight")){
						mat->lighting = false;
					}
					else if (line.Left(6) == _L8("map_d ")){
						mat->transparent = true;
					}
					else if (line.Left(7) == _L8("map_Kd ")) {
						HaytexturasQueCargar = true;
						TLex8 lex(line.Mid(7));
						TPtrC8 texturePath = lex.NextToken();

						// Convertir la ruta relativa a una ruta absoluta
						HBufC* texturePath16 = HBufC::NewLC(180);
						texturePath16 = CnvUtfConverter::ConvertToUnicodeFromUtf8L(texturePath);
						TParse fileParser;
						fileParser.Set(aFile, NULL, NULL);
						TFileName absolutePath = fileParser.DriveAndPath();
						absolutePath.Append(*texturePath16);

						// Comprobar si la textura existe
						RFs fs;
						fs.Connect();
						//User::LeaveIfError(fs.Connect());
						TEntry entry;
						TInt err = fs.Entry(absolutePath, entry);
						if (err == KErrNone) {						
							TTexture* newTexture = new TTexture();
							newTexture->iTextureName = *texturePath16;
							Textures.Append(newTexture);
							mat->textura = true;
							mat->textureIndex = Textures.Count()-1;

							gWhisk3D->iTextureManager->RequestToLoad(newTexture->iTextureName, fileParser.DriveAndPath(), Textures[mat->textureIndex], false);
						} else {
							// El archivo no existe, manejar el error
							//_LIT(KFileNotFound, "No existe la textura '%S'");
						}
						fs.Close();
						CleanupStack::PopAndDestroy(texturePath16);
					}				
				}
            }

            startPos += pos + 1;
            buffer.Delete(0, pos + 1);
            buffer.TrimLeft();
        }
    }
	CleanupStack::PopAndDestroy(materialName16);
    rFile.Close();	
    fsSession2.Close();	

	//si hay texturas. las lee
	if (HaytexturasQueCargar){		
		gWhisk3D->iTextureManager->DoLoadL();
	}
	return true;
    //CleanupStack::PopAndDestroy(&fsSession);
}

