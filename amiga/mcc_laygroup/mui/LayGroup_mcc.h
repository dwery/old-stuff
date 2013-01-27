/*
 *		MCC_LayGroup (c) by Alessandro Zummo
 *
 *		Registered class of the Magic User Interface.
 *
 *		LayGroup_mcc.h
*/

/*** Include stuff ***/

#ifndef LAYGROUP_MCC_H
#define LAYGROUP_MCC_H

#ifndef LIBRARIES_MUI_H
#include "libraries/mui.h"
#endif


/*** MUI Defines ***/

#define MUIC_LayGroup  "LayGroup.mcc"
#define MUIC_LayGroupP "LayGroup.mcp"
#define LayGroupObject MUI_NewObject(MUIC_LayGroup

#define TAGBASE_AZUMMO (TAG_USER | (0x2553 << 16))

// Class ids: 0xa5530001 - 0xa553000F

#define MUIA_LayGroup_ChildNumber  		(TAGBASE_AZUMMO | 0x0001) // [..G] ULONG
#define MUIA_LayGroup_MaxHeight  		(TAGBASE_AZUMMO | 0x0002) // [I..] WORD
#define MUIA_LayGroup_MaxWidth  		(TAGBASE_AZUMMO | 0x0003) // [I..] WORD
#define MUIA_LayGroup_HorizSpacing 		(TAGBASE_AZUMMO | 0x0004) // [ISG] WORD
#define MUIA_LayGroup_VertSpacing  		(TAGBASE_AZUMMO | 0x0005) // [ISG] WORD
#define MUIA_LayGroup_Spacing  			(TAGBASE_AZUMMO | 0x0006) // [IS.] WORD
#define MUIA_LayGroup_LeftOffset 		(TAGBASE_AZUMMO | 0x0007) // [ISG] WORD
#define MUIA_LayGroup_TopOffset  		(TAGBASE_AZUMMO | 0x0008) // [ISG] WORD
#define MUIA_LayGroup_AskLayout			(TAGBASE_AZUMMO | 0x0009) // [I..] BOOL 
#define MUIA_LayGroup_NumberOfColumns  	(TAGBASE_AZUMMO | 0x000A) // [..G] ULONG
#define MUIA_LayGroup_NumberOfRows  	(TAGBASE_AZUMMO | 0x000B) // [..G] ULONG
#define MUIA_LayGroup_InheritBackground	(TAGBASE_AZUMMO | 0x000C) // [I..] BOOL 
#define MUIA_LayGroup_SleepWindow		(TAGBASE_AZUMMO | 0x000D) // [ISG] BOOL 

#define MUIM_LayGroup_AskLayout		(TAGBASE_AZUMMO | 0x0101) 

struct MUIP_LayGroup_AskLayout		{ULONG MethodID; struct MUI_LayGroup_Layout *lgl;};

#define MUIV_LayGroup_MaxHeight_Auto -1
#define MUIV_LayGroup_MaxWidth_Auto  -1


#define MUIV_LayGroup_LeftOffset_Center -1
#define MUIV_LayGroup_TopOffset_Center  -1


struct MUI_LayGroup_Layout
{
	Object	*lgl_Object;

	UWORD	lgl_Height,
			lgl_Width;
};

#endif /* LAYGROUP_MCC_H */
