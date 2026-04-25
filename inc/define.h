#ifndef GLOB_DEFINES_HH
#define GLOB_DEFINES_HH

#define COLOR64K

#define UNITY 4096
#define  INV_UNITY_F 0.00024414f
#define  INV_UNITY_G (1.f/GLUNITY)
#define UNITY_MASK 4095
#define UNITY_LOG 12
#define HALF_UNITY 64
#define HALF_UNITY_LOG 6
#define QT_UNITY 8
#define QT_UNITY_LOG 3

#define INFINITY 67108864

#define SMALL_UNITY 16
#define SMALL_UNITY_LOG 4

#define MAX_VERTS 4
#define MAX_FACES 48
#define MAX_POLY_VERTS (MAX_FACES*3)
#define MAX_SCAN_LINE 208

#define GLUNITY (1<<16)
#define GLUNITY_MASK (UNITY-1)
#define GLUNITY_LOG 16

#define GLINVSQRT5 29309

#define GL_HALF_LOG	8
#define GL2U	(GLUNITY_LOG-UNITY_LOG)

#define TEXTURE_LINEAR_SIZE_LOG 6
#define TEXTURE_LINEAR_SIZE (1<<TEXTURE_LINEAR_SIZE_LOG)
#define TEXTURE_SIZE (TEXTURE_LINEAR_SIZE*TEXTURE_LINEAR_SIZE*4)
#define TEXURE_AREA_SIZE (TEXTURE_SIZE*(MAX_SITES+4))

#define MAX_SITES 3
#define MAX_MARKERS 16
#define MAX_ONBOARD_MARKERS 7

#define MAX_EMITTERS MAX_SITES

#define MAX_PHYS_OBJ 176
#define MAX_AGENTS 96
#define MAX_TOWERS 48

#define MAX_LIFES 15
#define MAX_EATEN 8
#define MAX_STACK 32

#if (MAX_EATEN > MAX_STACK)
#define MAX_EATEN MAX_STACK
#endif

#if (MAX_LIFES > MAX_STACK)
#define MAX_LIFES MAX_STACK
#endif

#define MAX_MSGES 10
#define MAX_MSG_LEN 4

//time dependant
#define MAX_ANGLE_BUFF 16

//geometrical
#define INV_SQRT_2 2896
#define INV_5 819
#define INV_6 683

#define INV_SQRT_2F 0.70710678
#define SQRT_2F 1.41421356

//#ifndef _WIN_EMULATOR_
//#define memset( st , c , memneeded) Mem::Fill((st), (memneeded), (c))
//#define memcpy(dst, src, size) Mem::Copy((dst), (src), (size))
//#endif

#define MASS2COLOR(m) ((((m)-UNITY/4)*2*INV_3)>>UNITY_LOG)
#define HARDNESS2COLOR(h) (Min(Max((h)-UNITY/8, 0), UNITY))
#define SAT2MASS(sat) (Min(2*(sat)*UNITY>>8, 4*UNITY) + UNITY/4)
#define SHAPE2HARDNESS(sh) ((sh) + UNITY/8)

#define VFWIDTH 320
#define VFHEIGHT 240

#define BKG_TXT_SIZEY0 256
#define BKG_TXT_SIZEY1 64

#define TEXT_SIZE 64
#define TEXT_ENTRIES 64

#define SCALE_SHIFT 3

#define MAX_BOX_COLORS 4
#define BOX_COLORS 4

#define STRONGBOT 1
#define FASTBOT 2
#define HEAVYBOT 3
#define SUPERBOT 4
#define SPINBOT 5

#define MAX_SIZE_X (VFWIDTH/2)
#define MAX_SIZE_Y (VFHEIGHT/2)

#define V2RX (VFWIDTH/MAX_SIZE_X)
#define V2RY (VFHEIGHT/MAX_SIZE_Y)

//#define NOKIAE51


#define MAX_FRAMES 200
//#define _DEB_VERSION_

#ifdef _DEB_VERSION_
#define _DEB_FRAME_BUFF_ 
#define _DEB_FULL_BUFF_ 
#endif

//#define _AUTOFOCUS_ON_


#ifndef __WINS__
#define _FINAL_
#endif

#endif
