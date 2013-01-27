/*

                LayGroup.mcc (c) Copyright 19976 by Alessandro Zummo

                LayGroup.c
*/

        #include <clib/alib_protos.h>
        #include <clib/graphics_protos.h>
        #include <clib/intuition_protos.h>
        #include <clib/utility_protos.h>
        #include <clib/muimaster_protos.h>
        #include <pragmas/muimaster_pragmas.h>
        #include <libraries/mui.h>
        #include <intuition/intuition.h>
		#include <intuition/classusr.h>
		#include <string.h>
		#include <math.h>

        #include "mui/LayGroup_mcc.h"
        #include "mui/LayGroup_mcc_private.h"

#define CLASS       MUIC_LayGroup
#define SUPERCLASS  MUIC_Virtgroup


// Our Instance Data

struct Data
{
	ULONG			lg_ChildNumber;

	LONG			lg_HSpace,	
					lg_VSpace;

	LONG			lg_LOff,	
					lg_TOff;

	WORD			lg_MaxHeight,
					lg_MaxWidth;

	ULONG			lg_Back;
	ULONG			lg_Frame;

	BOOL 			lg_PrefsReaded;
	BOOL			lg_BackgroundIsSupplied;

	ULONG			lg_RowNumber;
	ULONG			lg_ColNumber;

	BOOL			lg_LoadVSpace,
					lg_LoadHSpace,
					lg_LoadLOffset,
					lg_LoadTOffset,
					lg_LoadSleepWin;

	BOOL			lg_SleepWindow;

	struct Hook		*lg_LayHook;

	

};

#define UserLibID 		"$VER: LayGroup.mcc 19.2 "__AMIGADATE__" Copyright 1997-99 Alessandro Zummo"
#define VERSION 		19
#define REVISION 		2
#define MASTERVERSION 	19

#include "mui/mccheader.c"

void __inline LoadPrefs(Object *obj, struct Data *Data)
{
	if(!Data->lg_PrefsReaded)
	{
		LONG *kptrd,num;

		if(Data->lg_LoadHSpace)
		if(DoMethod(obj, MUIM_GetConfigItem, MUICFG_LayGroup_HSpace, &kptrd))
		{
			num = *kptrd;
			if(Data->lg_HSpace != 4) Data->lg_HSpace = num;
		}

		if(Data->lg_LoadVSpace)
		if(DoMethod(obj, MUIM_GetConfigItem, MUICFG_LayGroup_VSpace, &kptrd))
		{
			num = *kptrd;
			if(Data->lg_VSpace != 4) Data->lg_VSpace = num;
		}

		if(Data->lg_LoadLOffset)
		if(DoMethod(obj, MUIM_GetConfigItem, MUICFG_LayGroup_LOff, &kptrd))
		{
			num = *kptrd;
			if(Data->lg_LOff != 0) Data->lg_LOff = num;
		}

		if(Data->lg_LoadTOffset)
		if(DoMethod(obj, MUIM_GetConfigItem, MUICFG_LayGroup_TOff, &kptrd))
		{
			num = *kptrd;
			if(Data->lg_TOff != 0) Data->lg_TOff = num;
		}

		if(Data->lg_LoadSleepWin)
		if(DoMethod(obj, MUIM_GetConfigItem, MUICFG_LayGroup_SleepWindow, &kptrd))
		{
			num = *kptrd;
			Data->lg_SleepWindow = num;
		}

		Data->lg_PrefsReaded = TRUE;
	}
}


ULONG __stdargs __inline DoSuperNew(struct IClass *cl,Object *obj,ULONG tag1,...)
{
        return(DoSuperMethod(cl,obj,OM_NEW,&tag1,NULL));
}


/*** subs ***/


#include "layout.c"


SAVEDS ULONG ASM CommonLayoutFunc(REG(a0) struct Hook *h,REG(a2) Object *obj,REG(a1) struct MUI_LayoutMsg *lm)
{
   	/* get my private data from hook structure */
	// h_Data points to an istance data

	ULONG rc;

	ULONG ASM (*CommonLayoutFuncPtr)(REG(a0) struct Hook *h,REG(a2) Object *obj,REG(a1) struct MUI_LayoutMsg *lm);

	struct Data *Data 	= (struct Data *)h->h_Data;

	CommonLayoutFuncPtr = (ULONG ASM (* )(REG(a0) struct Hook *h,REG(a2) Object *obj,REG(a1) struct MUI_LayoutMsg *lm)) h->h_SubEntry;

	if(Data->lg_SleepWindow)
	{
		set(_win(obj), MUIA_Window_Sleep, TRUE);

		rc = (* CommonLayoutFuncPtr)(h,obj,lm);

		set(_win(obj), MUIA_Window_Sleep, FALSE);
	}
	else
		rc = (* CommonLayoutFuncPtr)(h,obj,lm);

	return(rc);
}


ULONG ASM __inline _Dispose(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	struct Data *Data  = INST_DATA(cl,obj);

	if(Data->lg_LayHook) FreeMem(Data->lg_LayHook, sizeof(struct Hook));

	return(DoSuperMethodA(cl,obj,msg));
}


ULONG ASM __inline _New(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) struct opSet *msg)
{
	struct Data *Data;

	struct Hook *LayHook;

	if(!(LayHook = AllocMem(sizeof(struct Hook), MEMF_ANY|MEMF_CLEAR)))
		return(0);

	LayHook->h_Entry = (void *)CommonLayoutFunc;

	if(GetTagData(MUIA_Group_SameSize, FALSE, msg->ops_AttrList))
	{
			LayHook->h_SubEntry = (void *)SameSizeGroupLayoutFunc;
	}
	else
	{
		if(GetTagData(MUIA_LayGroup_AskLayout, FALSE, msg->ops_AttrList))
			LayHook->h_SubEntry = (void *)AskLayoutGroupLayoutFunc;
		else
			LayHook->h_SubEntry = (void *)GroupLayoutFunc; 
	}

	if(obj = (Object *)DoSuperNew(cl,obj,
		MUIA_Frame, MUIV_Frame_Virtual, 
		MUIA_Group_LayoutHook, LayHook,
	TAG_MORE,msg->ops_AttrList))
	{
		struct TagItem *tags,*tag;

		Data = INST_DATA(cl,obj);

		LayHook->h_Data 		= Data;

		Data->lg_ChildNumber	= 0;
		Data->lg_HSpace			= 4;
		Data->lg_VSpace			= 4;
		Data->lg_LOff			= 0;
		Data->lg_TOff			= 0;
		Data->lg_SleepWindow	= FALSE;
		Data->lg_LayHook 		= LayHook;

		if(GetTagData(MUIA_Group_Child, FALSE, msg->ops_AttrList))				// Skip if there are no children
			for (tags = msg->ops_AttrList; tag = NextTagItem(&tags);)			// Count them...
				if(tag->ti_Tag == MUIA_Group_Child) Data->lg_ChildNumber++;




		// Check if a background is supplied, if we should inherit it or if we can use our own one.

		Data->lg_BackgroundIsSupplied = (BOOL) FindTagItem(MUIA_Background, msg->ops_AttrList) ? TRUE : (BOOL) GetTagData(MUIA_LayGroup_InheritBackground, FALSE, msg->ops_AttrList);

		Data->lg_LoadHSpace 	= (BOOL) FindTagItem(MUIA_LayGroup_HorizSpacing, msg->ops_AttrList) ? FALSE : TRUE;
		Data->lg_LoadVSpace 	= (BOOL) FindTagItem(MUIA_LayGroup_VertSpacing, msg->ops_AttrList) ? FALSE : TRUE;
		Data->lg_LoadLOffset	= (BOOL) FindTagItem(MUIA_LayGroup_LeftOffset, msg->ops_AttrList) ? FALSE : TRUE;
		Data->lg_LoadTOffset	= (BOOL) FindTagItem(MUIA_LayGroup_TopOffset, msg->ops_AttrList) ? FALSE : TRUE;
		Data->lg_LoadSleepWin	= (BOOL) FindTagItem(MUIA_LayGroup_SleepWindow, msg->ops_AttrList) ? FALSE : TRUE;

		Data->lg_MaxHeight 	= 0;
		Data->lg_MaxWidth 	= 0;

		Data->lg_PrefsReaded 		= FALSE;


		msg->MethodID = OM_SET; 	/* a little trick to parse taglist */ // Only for [IS?] aattributes
		DoMethodA(obj, (Msg) msg);  /* and set attributes with OM_SET. */
		msg->MethodID = OM_NEW; 	/* restore method id		       */

		return((ULONG)obj);
	}
	return(0);
}

ULONG ASM __inline _Set(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) struct opSet *msg)
{
	struct Data *Data = INST_DATA(cl,obj);

	struct TagItem *tags,*tag;

	BOOL change = FALSE;

	for (tags=msg->ops_AttrList;tag=NextTagItem(&tags);)
	{
		switch (tag->ti_Tag)
		{
			case MUIA_LayGroup_HorizSpacing :
			{
			 	Data->lg_HSpace		=	(LONG)tag->ti_Data;
				change = TRUE;				
				break;
			}
			case MUIA_LayGroup_VertSpacing:
			{
			 	Data->lg_VSpace		=	(LONG)tag->ti_Data;
				change = TRUE;				
				break;
			}
			case MUIA_LayGroup_LeftOffset:
			{
			 	Data->lg_LOff		=	(LONG)tag->ti_Data;
				change = TRUE;				
				break;
			}
			case MUIA_LayGroup_TopOffset:
			{
			 	Data->lg_TOff		=	(LONG)tag->ti_Data;
				change = TRUE;				
				break;
			}
			case MUIA_LayGroup_Spacing: 
			{
				Data->lg_HSpace = Data->lg_VSpace	= (LONG)tag->ti_Data;
				change = TRUE;				
				break;
			}
			case MUIA_LayGroup_SleepWindow: 
			{
				Data->lg_SleepWindow	= (BOOL)tag->ti_Data;
				break;
			}
			case MUIA_LayGroup_NumberOfRows:
			case MUIA_LayGroup_NumberOfColumns:
				break;
		}
	}
	
	if(change)
	{
		if(DoMethod(obj,MUIM_Group_InitChange))
			DoMethod(obj,MUIM_Group_ExitChange);
	}

	return(DoSuperMethodA(cl,obj,(Msg)msg));
}


ULONG ASM __inline _Get(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) struct opGet *msg)
{
	struct Data *Data = INST_DATA(cl,obj);

	switch (msg->opg_AttrID)
	{
		case MUIA_LayGroup_ChildNumber		: *(msg->opg_Storage) = Data->lg_ChildNumber; 	return(TRUE);
		case MUIA_LayGroup_NumberOfRows		: *(msg->opg_Storage) = Data->lg_RowNumber; 	return(TRUE);
		case MUIA_LayGroup_NumberOfColumns	: *(msg->opg_Storage) = Data->lg_ColNumber; 	return(TRUE);
		case MUIA_LayGroup_HorizSpacing 	: *(msg->opg_Storage) = Data->lg_HSpace; 		return(TRUE);
		case MUIA_LayGroup_VertSpacing  	: *(msg->opg_Storage) = Data->lg_VSpace; 		return(TRUE);
		case MUIA_LayGroup_LeftOffset 		: *(msg->opg_Storage) = Data->lg_LOff;	 		return(TRUE);
		case MUIA_LayGroup_TopOffset  		: *(msg->opg_Storage) = Data->lg_TOff; 			return(TRUE);
		case MUIA_LayGroup_SleepWindow 		: *(msg->opg_Storage) = Data->lg_SleepWindow;	return(TRUE);
	}
	return(DoSuperMethodA(cl,obj,(Msg)msg));
}


ULONG ASM __inline _AddMember(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	struct Data 	*Data   = INST_DATA(cl,obj);

	Data->lg_ChildNumber++;

	return(DoSuperMethodA(cl,obj,msg));
}

ULONG ASM __inline _RemMember(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	struct Data 	*Data   = INST_DATA(cl,obj);

	Data->lg_ChildNumber--;

	return(DoSuperMethodA(cl,obj,msg));
}

ULONG ASM __inline _Setup(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) struct MUIP_Setup *msg)
{                                                                                  
	struct Data *Data = INST_DATA(cl, obj);                                    

	if(!Data->lg_BackgroundIsSupplied)
		if(DoMethod(obj, MUIM_GetConfigItem, MUICFG_LayGroup_Back, &Data->lg_Back))
			set(obj,MUIA_Background, Data->lg_Back);
		else
			set(obj,MUIA_Background, MUII_GroupBack);

/*	if(MUIMasterBase->lib_Version >= 20)
	{
		if(DoMethod(obj, MUIM_GetConfigItem, MUICFG_LayGroup_Frame, &Data->lg_Frame))
			set(obj,MUIA_Frame, Data->lg_Frame);
		else
			set(obj,MUIA_Frame, MUIV_Frame_Virtual);
	}
*/
	if (!DoSuperMethodA(cl,obj,msg))
		return(FALSE);

	return(TRUE);
}

ULONG ASM __inline _AskLayout(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) struct MUIP_LayGroup_AskLayout *al)
{                                                                                  
	al->lgl->lgl_Width 	= _defwidth (al->lgl->lgl_Object);
	al->lgl->lgl_Height = _defheight(al->lgl->lgl_Object);

	return(0);
}

ULONG ASM SAVEDS _Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
        switch(msg->MethodID)
        {
            case OM_NEW        				: return(_New      		(cl,obj,(APTR)msg));
            case OM_DISPOSE    				: return(_Dispose  		(cl,obj,(APTR)msg));
            case OM_SET        				: return(_Set      		(cl,obj,(APTR)msg));
          	case OM_GET        				: return(_Get      		(cl,obj,(APTR)msg));
			case OM_ADDMEMBER  				: return(_AddMember		(cl,obj,(APTR)msg));
			case OM_REMMEMBER  				: return(_RemMember		(cl,obj,(APTR)msg));
          	case MUIM_Setup    				: return(_Setup    		(cl,obj,(APTR)msg));
         	case MUIM_LayGroup_AskLayout   	: return(_AskLayout   	(cl,obj,(APTR)msg));
        }
        return(DoSuperMethodA(cl,obj,msg));
}
