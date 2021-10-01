#include <stdio.h>
#include <conio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "../Include/3d_types.h"
#include "../Include/ModelObject.h"



//private
void ModelObject::MyMaterial(GLenum mode, GLfloat *f, GLfloat alpha) {
	GLfloat d[4];
	d[0]=f[0];
	d[1]=f[1];
	d[2]=f[2];
	d[3]=alpha;
	glMaterialfv(GL_FRONT_AND_BACK,mode,d);
}


void ModelObject::SelectMaterial(GLint i) {
  //
  // Define the reflective properties of the 3D Object faces.
  //
  GLfloat alpha;
  
  //glEnd();

  alpha=materials[i].alpha;
  MyMaterial(GL_AMBIENT, materials[i].ambient,alpha);
  MyMaterial(GL_DIFFUSE, materials[i].diffuse,alpha);
  MyMaterial(GL_SPECULAR, materials[i].specular,alpha);
  MyMaterial(GL_EMISSION, materials[i].emission,alpha);
  glMaterialf (GL_FRONT_AND_BACK,GL_SHININESS,materials[i].phExp);

//	glEnd();
	if(show_tex) {
		if(materials[i].texture>-1)	{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,texture_maps[materials[i].texture].id);
		}
		else glDisable(GL_TEXTURE_2D);
	}
//	glBegin(GL_TRIANGLES);

 // glBegin(GL_TRIANGLES);
}


BOOL ModelObject::LoadDIB(char*file,DIB2D*dib) {
  BOOL result=FALSE;
  HANDLE hfile=CreateFile(file,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING ,FILE_ATTRIBUTE_NORMAL,0);
  if(hfile!=INVALID_HANDLE_VALUE) {
		DWORD readed;
		int size=GetFileSize(hfile,0);
		if(size>sizeof(BITMAPFILEHEADER)) {
			BITMAPFILEHEADER bmfh;
			ReadFile(hfile,&bmfh,sizeof(BITMAPFILEHEADER),&readed,0);
			if((readed==sizeof(BITMAPFILEHEADER)) && (bmfh.bfType==0x4d42)) {
				dib->Info=(BITMAPINFOHEADER*)(new BYTE[size-sizeof(BITMAPFILEHEADER)]);
				ReadFile(hfile,dib->Info,size-sizeof(BITMAPFILEHEADER),&readed,0);
				dib->bits=(BYTE*)(dib->Info+1);
				if(dib->Info->biBitCount==8) {
					dib->palette=(RGBQUAD*)dib->bits;
					if(dib->Info->biClrUsed)dib->bits+=dib->Info->biClrUsed*4;else dib->bits+=1024;
				}
				else {
					dib->palette=NULL;
				}
				result=TRUE;
			}
		}
		CloseHandle(hfile);
	}
	return result;
};


long ModelObject::ScanBytes(int pixWidth, int bitsPixel) {
  return (((long)pixWidth*bitsPixel+31) / 32) * 4;
}


BOOL ModelObject::ScaleImage(DIB2D&dib,GLTXTLOAD&p) {
   GLint glMaxTexDim;     // OpenGL maximum texture dimension
   GLint XDMaxTexDim=512; // user maximum texture dimension
   GLint minsize =2;
   double xPow2, yPow2;
   int ixPow2, iyPow2;
   int xSize2, ySize2;
   GLint m_iWidth=dib.Info->biWidth;
   GLint m_iHeight=dib.Info->biHeight;
   glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glMaxTexDim);
   glMaxTexDim = min(XDMaxTexDim, glMaxTexDim);
   if (m_iWidth <= glMaxTexDim)
      xPow2 = log((double)m_iWidth) / log(2.0);
   else
      xPow2 = log((double)glMaxTexDim) / log(2.0);
   if (m_iHeight <= glMaxTexDim)
      yPow2 = log((double)m_iHeight) / log(2.0);
   else
      yPow2 = log((double)glMaxTexDim) / log(2.0);
   ixPow2 = (int)xPow2;
   iyPow2 = (int)yPow2;
   if (xPow2 != (double)ixPow2)
      ixPow2++;
   if (yPow2 != (double)iyPow2)
      iyPow2++;
   xSize2 = 1 << ixPow2;
   ySize2 = 1 << iyPow2;
   if(xSize2<minsize)xSize2=minsize;
   if(ySize2<minsize)ySize2=minsize;
   if(((xSize2==m_iWidth) && (ySize2==m_iHeight))) {
		 if(dib.Info->biBitCount==24) {
			 p.format=GL_BGR_EXT;
			 p.perpixel=3;
       return FALSE;
		 }
     if(dib.Info->biBitCount==32) {
       p.format=GL_BGRA_EXT;
       p.perpixel=4;
       return FALSE;
		 }
   }
		BYTE *bits=(BYTE *)dib.bits;
		if(dib.Info->biBitCount==8) {
		 // convert to TRUECOLOR
			int _perline=ScanBytes(8,m_iWidth);
			int perline=ScanBytes(24,m_iWidth);
			bits= new BYTE[perline*m_iHeight * sizeof(BYTE)];
			for(int y=0;y<m_iHeight;y++) {
				BYTE *_b=((BYTE *)dib.bits)+y*_perline;
				BYTE *b=bits+y*perline;
				for(int x=0;x<m_iWidth;x++) {
					RGBQUAD _p=dib.palette[*_b];
					_b++;
					*b=_p.rgbBlue;b++;
					*b=_p.rgbGreen;b++;
					*b=_p.rgbRed;b++;
				}
			}
		}
		BOOL isAlpha=(dib.Info->biBitCount==32);
		int _mem_size=xSize2 * ySize2 *  sizeof(BYTE);
		if(isAlpha){
			_mem_size*=4;
      p.perpixel=4;
      p.format=GL_BGRA_EXT;
		}
		else {
			_mem_size*=3;
      p.perpixel=3;
      p.format=GL_BGR_EXT;
		}
		BYTE *pData = (BYTE*)new BYTE[_mem_size];
		if (!pData) return FALSE;

		if(isAlpha){
			gluScaleImage(GL_BGRA_EXT, m_iWidth, m_iHeight,
                 GL_UNSIGNED_BYTE, bits,
                 xSize2, ySize2, GL_UNSIGNED_BYTE, pData);
		}
		else
			gluScaleImage(GL_RGB, m_iWidth, m_iHeight,
                 GL_UNSIGNED_BYTE, bits,
                 xSize2, ySize2, GL_UNSIGNED_BYTE, pData);
   if(bits!=dib.bits) delete bits;
   m_iWidth = xSize2 ;
   m_iHeight = ySize2 ;
   p.Width=m_iWidth;
   p.Height=m_iHeight;
   p.bits=pData;
   return TRUE ;
}


void ModelObject::LoadTexture() {
  DIB2D dib;
  GLTXTLOAD load;
  BmpImage img;
  //modificadon +
  
	glGenTextures(1,&texture_name);
 texture_maps[0].id=texture_name;
  glBindTexture(GL_TEXTURE_2D,texture_name);
	//modificado -
  char fileName[50]="faces/";
  strcat (fileName,texture_maps[0].name);

  if(LoadDIB(fileName,&dib)) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		if(ScaleImage(dib,load)) {

			glTexImage2D(GL_TEXTURE_2D,0,load.perpixel,
                load.Width,load.Height,0,
                load.format,GL_UNSIGNED_BYTE,
                load.bits);
      delete load.bits;
    }
		else {

			glTexImage2D(GL_TEXTURE_2D,0,load.perpixel,
                dib.Info->biWidth,dib.Info->biHeight,
                0,load.format,GL_UNSIGNED_BYTE,dib.bits);
		}
		delete dib.Info;
  }
 /* else 
  {
	  img.readBmpFile (texture_maps[0].name);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width, img.height, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data );

  }
*/
}



//public
ModelObject::ModelObject() {
	num_faces=num_points=num_materials=num_refs=0;
	faces=NULL;
	vertices=normals=textures=NULL;
	materials=NULL;
	material_ref=NULL;
	texture_maps=NULL;
	norm_temp=NULL;
	show_tex=GL_TRUE;
	num_tex_maps = 0;
	texture_name = 0;
}


ModelObject::~ModelObject() {
	if(faces)			delete [] faces,	faces    = NULL;
	if(vertices)		delete [] vertices,	vertices = NULL;
	if(normals)			delete [] normals,		normals  = NULL;
	if(textures)		delete [] textures,	textures = NULL;
	if(materials)		delete [] materials,	materials = NULL;
	if(material_ref)	delete [] material_ref, material_ref = NULL;
	if(texture_maps)	delete [] texture_maps, texture_maps = NULL;
	if (norm_temp)		delete [] norm_temp, norm_temp = NULL;

	if (texture_name)	
		glDeleteTextures (1, &texture_name);
}


int ModelObject::LoadOGL(char *in) {
	FILE *fin;
	char palabra[256];
	int i;
	GLmaterial mat;
	//nuevo +
	GLtexture tex;
	GLboolean tex_pres=GL_FALSE;
	//nuevo -
	GLint face[9];
	GLpoint pto;
	GLint ref[2];
	fin=fopen(in,"rt");

	
	if(fin==NULL) {
		printf("Unable to open OGL file: %s\n",in);
		return FILE_NOT_FOUND;
	}

	//Busqueda de materiales
	palabra[0]='\0';
	while(strcmp(palabra,"materials")!=0) 
		fscanf(fin,"%s",palabra);
	if(fscanf(fin," [ %d ] = {",&num_materials)!=1) {
		printf("Problem while reading materials in OGL file: %s\n",in);
		fclose(fin);
		return MATERIAL_PROBLEMS;
	}
	
	materials=new(GLmaterial[num_materials]);
	for(i=0;i<num_materials;i++) {
		palabra[0]=palabra[1]='\0';
		while(palabra[0]!='{') palabra[0]=fgetc(fin);
		fscanf(fin," {%ff,%ff,%ff} , {%ff,%ff,%ff} , {%ff,%ff,%ff} , {%ff,%ff,%ff} , %ff , %ff , %d }",
			&mat.ambient[0],&mat.ambient[1],&mat.ambient[2],&mat.diffuse[0],&mat.diffuse[1],&mat.diffuse[2],
			&mat.specular[0],&mat.specular[1],&mat.specular[2],&mat.emission[0],&mat.emission[1],&mat.emission[2],
			&mat.alpha,&mat.phExp,&mat.texture
		);
		materials[i]=mat;
		if(mat.texture!=-1) tex_pres=GL_TRUE;
	}

	//Busqueda de mapas de textura
	if(tex_pres==GL_TRUE) {
		palabra[0]='\0';
		while(strcmp(palabra,"texture_maps")!=0) 
			fscanf(fin,"%s",palabra);
		if(fscanf(fin," [ %d ] = {",&num_tex_maps)!=1) {
			printf("Problem while reading texture maps in OGL file: %s\n",in);
			fclose(fin);
			return TEXTURE_MAP_PROBLEM;
		}
		texture_maps=new(GLtexture[num_tex_maps]);
		for(i=0;i<num_tex_maps;i++) {
//			palabra[0]=palabra[1]='\0';
//			while(palabra[0]!='{') palabra[0]=fgetc(fin);
			fscanf(fin," { \"%[^\"]\" , %d } ",tex.name,&tex.id);
			texture_maps[i]=tex;
		}
	}


	//Busqueda de caras
	palabra[0]='\0';
	
	while(strncmp(palabra,"face_indicies", 13)!=0) 		
		fscanf(fin,"%s",palabra);
	
	if(sscanf(palabra,"face_indicies[ %d ][ 9 ] = {",&num_faces)!=1) {
		printf("Problem while reading OGL file: %s\n",in);
		fclose(fin);
		return READING_PROBLEM;
	}

	faces=new(GLint[num_faces][9]);
	palabra[0]=palabra[1]='\0';
	while(palabra[0]!='{') palabra[0]=fgetc(fin);

	for(i=0;i<num_faces;i++) {
		palabra[0]=palabra[1]='\0';
		while(palabra[0]!='{') palabra[0]=fgetc(fin);
		fscanf(fin," %d , %d , %d , %d , %d , %d , %d , %d , %d }",
			&face[0],&face[1],&face[2],&face[3],&face[4],&face[5],&face[6],&face[7],&face[8]
		);
		faces[i][0]=face[0];
		faces[i][1]=face[1];
		faces[i][2]=face[2];
		faces[i][3]=face[3]; 
		faces[i][4]=face[4];
		faces[i][5]=face[5];
		faces[i][6]=face[6];
		faces[i][7]=face[7];
		faces[i][8]=face[8];
	}

	//Busqueda de vertices
	palabra[0]='\0';
	while(strcmp(palabra,"vertices")!=0) 
		fscanf(fin,"%s",palabra);
	if(fscanf(fin," [ %d ] [ 3 ] = {",&num_points)!=1) {
		printf("Problem while reading OGL file: %s\n",in);
		fclose(fin);
		return READING_PROBLEM;
	}
	vertices=new(GLpoint[num_points]);
	for(i=0;i<num_points;i++) {
		palabra[0]=palabra[1]='\0';
		while(palabra[0]!='{') palabra[0]=fgetc(fin);
		fscanf(fin," %ff , %ff , %ff }",&pto.x,&pto.y,&pto.z);
		vertices[i]=pto;
	}

	//Busqueda de normales
	palabra[0]='\0';
	while(strcmp(palabra,"normals")!=0) 
		fscanf(fin,"%s",palabra);
	if(fscanf(fin," [ %d ] [ 3 ] = {",&num_norm)!=1) {
		printf("Problem while reading OGL file: %s\n",in);
		fclose(fin);
		return  READING_PROBLEM;
	}
	normals=new(GLpoint[num_norm]);
	for(i=0;i<num_norm;i++) {
		palabra[0]=palabra[1]='\0';
		while(palabra[0]!='{') palabra[0]=fgetc(fin);
		fscanf(fin," %ff , %ff , %ff }",&pto.x,&pto.y,&pto.z);
		normals[i]=pto;
	}

	//Busqueda de coords. de textura
	palabra[0]='\0';
	while(strcmp(palabra,"textures")!=0) 
		fscanf(fin,"%s",palabra);
	if(fscanf(fin," [ %d ] [ 2 ] = {",&num_tex)!=1) {
		printf("Problem while reading OGL file: %s\n",in);
		fclose(fin);
		return READING_PROBLEM;
	}
	textures=new(GLpoint[num_tex]);
	pto.z=0;
	for(i=0;i<num_tex;i++) {
		palabra[0]=palabra[1]='\0';
		while(palabra[0]!='{') palabra[0]=fgetc(fin);
		fscanf(fin," %ff , %ff }",&pto.x,&pto.y);
		textures[i]=pto;
	}

	//Busqueda de referencias de material
	palabra[0]='\0';
	while(strcmp(palabra,"material_ref")!=0 && !feof(fin)) 
		fscanf(fin,"%s",palabra);
	if(fscanf(fin," [ %d ] [ 2 ] = {",&num_refs)!=1) {
		printf("Problem while reading OGL file: %s\n",in);
		fclose(fin);
		return READING_PROBLEM;
	}
	material_ref=new(GLint[num_refs][2]);
	for(i=0;i<num_refs;i++) {
		palabra[0]=palabra[1]='\0';
		while(palabra[0]!='{') palabra[0]=fgetc(fin);
		fscanf(fin," %d , %d }",&ref[0],&ref[1]);
		material_ref[i][0]=ref[0];
		material_ref[i][1]=ref[1];
	}

	fclose(fin);

	if(tex_pres) LoadTexture();

	return OK;

}


void ModelObject::Load3DS(char *in) {}


void ModelObject::LoadOBJ(char *in) {}


void ModelObject::ShowTextures(GLboolean show) {
	show_tex=show;
}


void ModelObject::Normalize() {
	GLint f,i;
	GLpoint v1,v2,n;
	if(num_norm!=num_points) {
		delete normals;
		normals=new(GLpoint[num_points]);
		num_norm=num_points;
		for(f=0;f<num_faces;f++) {
			faces[f][3]=faces[f][0];
			faces[f][4]=faces[f][1];
			faces[f][5]=faces[f][2];
		}
	}
	if(norm_temp==NULL) norm_temp=new(GLshort[num_points]);
	for(i=0;i<num_points;i++) {
		normals[i].x=normals[i].y=normals[i].z=0.0;
		norm_temp[i]=0;
	}
	for(f=0;f<num_faces;f++) {
		v1=sub(vertices[faces[f][1]],vertices[faces[f][0]]);
		v2=sub(vertices[faces[f][1]],vertices[faces[f][2]]);
		n=crossprod(v1,v2);
		n=normalize(n);
		for(i=0;i<3;i++) {
			normals[faces[f][i]]=add(normals[faces[f][i]],n);
			norm_temp[faces[f][i]]++;
		}
	}
	for(i=0;i<num_points;i++) {
		normals[i].x/=-norm_temp[i];
		normals[i].y/=-norm_temp[i];
		normals[i].z/=-norm_temp[i];
	}


}


void ModelObject::Display(int drawTextures) {
	GLint mcount=0;
	GLint mindex=0;
	GLint i,j,vi,ni,ti;
	
	if (drawTextures == DISPLAY_GI_X) {
		glBegin (GL_TRIANGLES);
		for (i=0;i<num_faces;i++){
			for (j=0;j<3;j++) {
				
				vi=faces[i][j];
				//ni=faces[i][j+3];
				ti=faces[i][j+6];

				glColor3f (vertices[vi].x,vertices[vi].y,vertices[vi].z);
				glVertex3f (textures[ti].x,textures[ti].y,-vertices[vi].x);
			}
		}
		glEnd ();
	}
	else if (drawTextures == DISPLAY_GI_Y) {
		glBegin (GL_TRIANGLES);
		for (i=0;i<num_faces;i++){
			for (j=0;j<3;j++) {
				
				vi=faces[i][j];
				//ni=faces[i][j+3];
				ti=faces[i][j+6];

				glColor3f (vertices[vi].x,vertices[vi].y,vertices[vi].z);
				glVertex3f (textures[ti].x,textures[ti].y,-vertices[vi].y);
			}
		}
		glEnd ();
	}

	else if (drawTextures == DISPLAY_GI_Z) {
		glBegin (GL_TRIANGLES);
		for (i=0;i<num_faces;i++){
			for (j=0;j<3;j++) {
				
				vi=faces[i][j];
				//ni=faces[i][j+3];
				ti=faces[i][j+6];

				glColor3f (vertices[vi].x,vertices[vi].y,vertices[vi].z);
				glVertex3f (textures[ti].x,textures[ti].y,-vertices[vi].z);
			}
		}
		glEnd ();
	}

	else if (drawTextures == DISPLAY_GI) {
		glBegin (GL_TRIANGLES);
		for (i=0;i<num_faces;i++){
			for (j=0;j<3;j++) {
				
				vi=faces[i][j];
				//ni=faces[i][j+3];
				ti=faces[i][j+6];

				
				glColor3f (vertices[vi].x,vertices[vi].y,vertices[vi].z);
				glVertex2f (textures[ti].x,textures[ti].y);
				}
		}
		glEnd ();
	}

	else if (drawTextures == DISPLAY_NORMALS_X) {
		glBegin (GL_TRIANGLES);
		for (i=0;i<num_faces;i++){
			for (j=0;j<3;j++) {
				
				//vi=faces[i][j];
				ni=faces[i][j+3];
				ti=faces[i][j+6];

//				glColor3f (normals[ni].x,0,0);
				glVertex3f (textures[ti].x,textures[ti].y,-normals[ni].x);
				
			}
		}
		glEnd ();
	}

	else if (drawTextures == DISPLAY_NORMALS_Y) {
		glBegin (GL_TRIANGLES);
		for (i=0;i<num_faces;i++){
			for (j=0;j<3;j++) {
				
				//vi=faces[i][j];
				ni=faces[i][j+3];
				ti=faces[i][j+6];

//				glColor3f (0,normals[ni].y,0);
				glVertex3f (textures[ti].x,textures[ti].y,-normals[ni].y);
			}
		}
		glEnd ();
	}

	else if (drawTextures == DISPLAY_NORMALS_Z) {
		glBegin (GL_TRIANGLES);
		for (i=0;i<num_faces;i++){
			for (j=0;j<3;j++) {
				
				//vi=faces[i][j];
				ni=faces[i][j+3];
				ti=faces[i][j+6];

//				glColor3f (0,0,normals[ni].z);
				glVertex3f (textures[ti].x,textures[ti].y,-normals[ni].z);
			}
		}
		glEnd ();
	}


	else if (drawTextures == DISPLAY_NORMALS) {
		glBegin (GL_TRIANGLES);
		for (i=0;i<num_faces;i++){
			for (j=0;j<3;j++) {
				
				//vi=faces[i][j];
				ni=faces[i][j+3];
				ti=faces[i][j+6];

				glColor3f (normals[ni].x,normals[ni].y,normals[ni].z);
				glVertex2f (textures[ti].x,textures[ti].y);
			}
		}
		glEnd ();
	}

	else {
	
		for(i=0;i<num_faces;i++) {

			if(!mcount) {
				SelectMaterial(material_ref[mindex][0]);
				mcount=material_ref[mindex][1];
				mindex++;
			}
			mcount--;
			
			glBegin (GL_TRIANGLES);
			for(j=0;j<3;j++) {
				if (!drawTextures){
					vi=faces[i][j];
					ni=faces[i][j+3];
					ti=faces[i][j+6];
					glNormal3f (normals[ni].x,normals[ni].y,normals[ni].z);

					//glColor3f (vertices[vi].x,vertices[vi].y,vertices[vi].z);
					//glVertex2f(textures[ti].x,textures[ti].y);
					glTexCoord2f(textures[ti].x,textures[ti].y);
					
					glVertex3f (vertices[vi].x,vertices[vi].y,vertices[vi].z);
				}
				else{
					ti=faces[i][j+6];
					glVertex2f(textures[ti].x,textures[ti].y);
					//glTexCoord2f(textures[ti].x,textures[ti].y);
				}
			}
			glEnd ();
		}
			
	}
}



