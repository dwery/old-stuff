/*

		MuiWheel.mcp (c) Copyright 1999-2002 by Alessandro Zummo

		MuiWheelp.c

*/

#include <string.h>

#include <clib/alib_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>
#include <clib/muimaster_protos.h>
#include <pragmas/muimaster_pragmas.h>
#include <libraries/mui.h>
#include "mui/MuiWheel_mcc.h"
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>
#include <stdio.h>

#include "prefssemaphore.h"

#define CLASS       "MuiWheel.mcp"
#define SUPERCLASSP MUIC_Mccprefs

#define MUIM_MuiWheel_Active		(TAG_USER | 0x0001)
#define MUIM_MuiWheel_UpdateSkip	(TAG_USER | 0x0002)
#define MUIM_MuiWheel_UpdateQSkip	(TAG_USER | 0x0003)
#define MUIM_MuiWheel_UpdateStatus	(TAG_USER | 0x0004)
#define MUIM_MuiWheel_UpdateClick	(TAG_USER | 0x0005)

#define MWID_Numeric 		0x00
#define MWID_Cycle 			0x01
#define MWID_Prop 			0x02
#define MWID_Scrollgroup 	0x03
#define MWID_List 			0x04
#define MWID_Virtgroup 		0x05
#define MWID_MAX	 		0x05

struct EntryPrefs
{
	BOOL	ep_Qualifier,
			ep_Skip,
			ep_QSkip;

	struct	ClassEntry *ep_ce;
};

struct ClassEntry
{
	STRPTR	ce_ClassName;
	BOOL	ce_Enabled;

	LONG	ce_Skip,
			ce_QualSkip;

	char	ce_SSkip[6],
			ce_SQualSkip[6];

	UWORD	ce_Id;

	ULONG	ce_pSkip,
			ce_pQSkip,
			ce_pEnabled;

//	STRPTR	ce_Qualifier;
	
};

struct MUIP_MuiWheel_Active {ULONG MethodID; LONG active;};
struct MUIP_MuiWheel_UpdateSkip {ULONG MethodID; LONG skip;};


struct DataP
{
	Object	*d_NList,
			*d_Image;

	Object	*d_QualStr,
			*d_Skip,
			*d_QSkip;

	Object	*d_Enable,
			*d_Disable;

	struct EntryPrefs	d_ep[MWID_MAX + 1];
};


#define VERSION 		19
#define REVISION 		4
#define MASTERVERSION 	19

#define __VERSCR     "Copyright 1998-2002, Alessandro Zummo"
#define __VSTRING    "MuiWheel.mcp 19.4 " __AMIGADATE__
#define __VSTRINGN   "MuiWheel.mcc 19.4 " __AMIGADATE__

#define UserLibID 	 "$VER: MuiWheel.mcp 19.4 "__AMIGADATE__" "__VERSCR


#define PREFSIMAGEOBJECT prefs_image_object()
Object *prefs_image_object(void);

#define USE_PREFS_IMAGE_BODY
#define USE_PREFS_IMAGE_COLORS

#include "prefs_image.c"

#include "mui/mccheader.c"

Object * __inline prefs_image_object(void)
{
	Object *obj = BodychunkObject,
		MUIA_FixWidth             , PREFS_IMAGE_WIDTH,
		MUIA_FixHeight            , PREFS_IMAGE_HEIGHT,
		MUIA_Bitmap_Width         , PREFS_IMAGE_WIDTH,
		MUIA_Bitmap_Height        , PREFS_IMAGE_HEIGHT,
		MUIA_Bodychunk_Depth      , PREFS_IMAGE_DEPTH,
		MUIA_Bodychunk_Body       , (UBYTE *)prefs_image_body,
		MUIA_Bodychunk_Compression, PREFS_IMAGE_COMPRESSION,
		MUIA_Bodychunk_Masking    , PREFS_IMAGE_MASKING,
		MUIA_Bitmap_SourceColors  , (ULONG *)prefs_image_colors,
		MUIA_Bitmap_Transparent   , 0,
		End;


	return(obj);
}

LONG xget(Object *obj,ULONG attribute)
{
	LONG x;
	get(obj,attribute,&x);
	return(x);
}


ULONG __stdargs __inline DoSuperNew(struct IClass *cl,Object *obj,ULONG tag1,...)
{
        return(DoSuperMethod(cl,obj,OM_NEW,&tag1,NULL));
}
/*

			MUIA_ShortHelp, "Enable", 
			MUIA_ShortHelp, "The amount of movements made on the gadget\nfor each wheel movement.", 
			MUIA_ShortHelp, "The amount of movements made on the gadget\nfor each qualified wheel movement.", 
			MUIA_ShortHelp, "The qualifier for the qualified skip.\n i.e. shift, control, middlebutton...\nIt's currently forced to middlebutton.\nWil be unlocked in the next release.",
*/


APTR __asm __saveds ConstructFunc(register __a2 APTR pool, register __a1 struct ClassEntry *e, register __a0 struct Hook *h)
{
	struct ClassEntry *ne;

	if(ne = AllocPooled(pool, sizeof(struct ClassEntry)))
		CopyMem(e,ne,sizeof(struct ClassEntry));
	else
		return(0);

	return(ne);
}


LONG __asm __saveds DestructFunc(register __a2 APTR pool, register __a1 struct ClassEntry *e, register __a0 struct Hook *h)
{
	FreePooled(pool,e,sizeof(struct ClassEntry));

	return(0);
}

LONG __asm __saveds DisplayFunc(register __a2 char **array, register __a1 struct ClassEntry *ce, register __a0 struct Hook *h)
{ 
	struct DataP *Data = (struct DataP *)h->h_Data;


	if(ce)
	{
		struct EntryPrefs *ep = &Data->d_ep[ce->ce_Id];
	
		sprintf(ce->ce_SSkip, 		"%ld", ce->ce_Skip		);
		sprintf(ce->ce_SQualSkip, 	"%ld", ce->ce_QualSkip	);

//		*array++ = ce->ce_Enabled ? "\033o[1]" : "";	// Wheel image

		*array++ = ce->ce_Enabled ? "x" : "";
		*array++ = ce->ce_ClassName;
		*array++ = ep->ep_Skip  ? 	ce->ce_SSkip 		: "n/a";
		*array++ = ep->ep_QSkip ?	ce->ce_SQualSkip	: "n/a";
	    *array   = "middlebutton";

		ep->ep_ce = ce;
	}
	else
	{
		// Title

		*array++ = "Enabled";
		*array++ = "Class";
		*array++ = "Standard Skip";
		*array++ = "Qualified Skip";
		*array   = "Qualifier";
	}
	
	return(0);
}


LONG __asm __saveds CompareFunc(register __a1 struct ClassEntry *ce1, register __a2 struct ClassEntry *ce2, register __a0 struct Hook *h) 
{
		return(stricmp(ce1->ce_ClassName,ce2->ce_ClassName));
}

// Hook definitions

struct Hook ConstructHook 	= { {NULL, NULL}, (void *)ConstructFunc, NULL,NULL };
struct Hook DestructHook 	= { {NULL, NULL}, (void *)DestructFunc,  NULL,NULL };
struct Hook CompareHook 	= { {NULL, NULL}, (void *)CompareFunc, 	 NULL,NULL };
struct Hook DisplayHook 	= { {NULL, NULL}, (void *)DisplayFunc,   NULL,NULL };


ULONG ASM __inline _DisposeP(REG(a0)struct IClass *cl, REG(a2)Object *obj, REG(a1)Msg msg)
{
//	struct DataP *Data = INST_DATA(cl,obj);

//	MUI_DisposeObject(Data->d_Image);

	return(DoSuperMethodA(cl,obj,(Msg)msg));
}

ULONG ASM __inline _NewP(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	struct DataP *Data;

	struct MsgPort *newmouse_port;

	Object *grp;


	if(!(obj = (Object *)DoSuperMethodA(cl, obj, msg)))
		return(0);

	Forbid();
	newmouse_port = FindPort("NewMouse");
	Permit();

	Data = INST_DATA(cl, obj);

	Data->d_ep[MWID_Numeric		].ep_Qualifier 	= FALSE;
	Data->d_ep[MWID_Numeric		].ep_Skip 	  	= TRUE;
	Data->d_ep[MWID_Numeric		].ep_QSkip     	= TRUE;

	Data->d_ep[MWID_Cycle		].ep_Qualifier	= FALSE;
	Data->d_ep[MWID_Cycle		].ep_Skip 	  	= FALSE;
	Data->d_ep[MWID_Cycle		].ep_QSkip     	= FALSE;

	Data->d_ep[MWID_Prop		].ep_Qualifier 	= FALSE;
	Data->d_ep[MWID_Prop		].ep_Skip 	  	= TRUE;
	Data->d_ep[MWID_Prop		].ep_QSkip     	= TRUE;

	Data->d_ep[MWID_List		].ep_Qualifier 	= FALSE;
	Data->d_ep[MWID_List		].ep_Skip 	  	= TRUE;
	Data->d_ep[MWID_List		].ep_QSkip     	= TRUE;

	Data->d_ep[MWID_Virtgroup	].ep_Qualifier 	= FALSE;
	Data->d_ep[MWID_Virtgroup	].ep_Skip 	  	= FALSE;
	Data->d_ep[MWID_Virtgroup	].ep_QSkip     	= FALSE;

	DisplayHook.h_Data	= (APTR) Data;

	grp = VGroup,
		Child, MUI_MakeObject(MUIO_BarTitle, "Preferences"),
		Child, VGroup,
			Child, NListviewObject,
				MUIA_NListview_NList, Data->d_NList = NListObject,
					MUIA_NList_Format, "BAR PREPARSE="MUIX_C", BAR, BAR PREPARSE="MUIX_C", BAR PREPARSE="MUIX_C",",	
					MUIA_NList_Title, TRUE, 
					MUIA_NList_ConstructHook, 	&ConstructHook,	
					MUIA_NList_DestructHook, 	&DestructHook,
					MUIA_NList_DisplayHook, 	&DisplayHook,
					MUIA_NList_CompareHook, 	&CompareHook,
					MUIA_NList_PoolPuddleSize, sizeof(struct ClassEntry) * 6,
					MUIA_NList_PoolThreshSize, sizeof(struct ClassEntry),
				End,		
			End,
			Child, HGroup,
				GroupFrame,
				MUIA_Background, MUII_GroupBack,
				MUIA_Group_Columns, 5,
				Child, Data->d_Enable 	= SimpleButton("_Enable"),
				Child, HVSpace, 
				Child, HVSpace, 
				Child, LLabel("Standard skip:"),
				Child, Data->d_Skip = SliderObject, MUIA_Numeric_Min, -20, MUIA_Numeric_Max, 20, End,
				Child, Data->d_Disable = SimpleButton("_Disable"),
				Child, LLabel("Qualifier:"),
				Child, Data->d_QualStr = StringObject, StringFrame,  End,
				Child, LLabel("Qualified skip:"),
				Child, Data->d_QSkip = SliderObject, MUIA_Numeric_Min, -20, MUIA_Numeric_Max, 20, End,
			End,
		End,
//		Child, HVSpace,
		Child, CrawlingObject,
    		MUIA_FixHeightTxt, "\n",
			TextFrame,
			MUIA_Background, MUII_TextBack,
			Child, HGroup,
				Child, prefs_image_object(),
				Child, TextObject,
					MUIA_Text_Contents, __VSTRING "\n" __VERSCR,
					MUIA_Text_PreParse, MUIX_C,
				End,
				Child, prefs_image_object(),
			End,
			Child, TextObject,
				MUIA_Text_Contents, "\nFor request and reports\nwrite an email to:\n\nazummo-mw@towertech.it\n",
				MUIA_Text_PreParse, MUIX_C,
			End,
			Child, TextObject,
				MUIA_Text_Contents, newmouse_port ? "\nNewMouse detected! Great choice!\n\n" : "\nYou need a MouseWheel compatible\nmouse driver (i.e. NewMouse)\n\n",
				MUIA_Text_PreParse, MUIX_C,
			End,
			Child, HGroup,
				Child, prefs_image_object(),
				Child, TextObject,
					MUIA_Text_Contents, __VSTRING "\n" __VERSCR,
					MUIA_Text_PreParse, MUIX_C,
				End,
				Child, prefs_image_object(),
			End,
		End,
	End;

	set(Data->d_Enable, 	MUIA_Disabled, TRUE);
	set(Data->d_Disable, 	MUIA_Disabled, TRUE);
	set(Data->d_Skip,	 	MUIA_Disabled, TRUE);
	set(Data->d_QSkip,	 	MUIA_Disabled, TRUE);
	set(Data->d_QualStr, 	MUIA_Disabled, TRUE);

	if(!grp)
	{
		CoerceMethod(cl, obj, OM_DISPOSE);
		return(0);
	}


//	DoMethod(Data->d_NList, MUIM_NList_UseImage, Data->d_Image = prefs_image_object(), 1, 0); 

	DoMethod(obj, OM_ADDMEMBER, grp);

	DoMethod(Data->d_NList, 	MUIM_Notify, MUIA_NList_Active,  MUIV_EveryTime, obj, 2, MUIM_MuiWheel_Active, 		MUIV_TriggerValue);
	DoMethod(Data->d_QSkip,  	MUIM_Notify, MUIA_Numeric_Value, MUIV_EveryTime, obj, 2, MUIM_MuiWheel_UpdateQSkip, MUIV_TriggerValue);
	DoMethod(Data->d_Skip,  	MUIM_Notify, MUIA_Numeric_Value, MUIV_EveryTime, obj, 2, MUIM_MuiWheel_UpdateSkip, 	MUIV_TriggerValue);

	DoMethod(Data->d_Enable,  	MUIM_Notify, MUIA_Pressed,  FALSE, obj, 1, MUIM_MuiWheel_UpdateStatus);
	DoMethod(Data->d_Disable,  	MUIM_Notify, MUIA_Pressed,  FALSE, obj, 1, MUIM_MuiWheel_UpdateStatus);

	DoMethod(Data->d_NList,  	MUIM_Notify, MUIA_NList_DoubleClick, TRUE, obj, 1, MUIM_MuiWheel_UpdateStatus);


	return((ULONG)obj);

}

ULONG ASM __inline _UpdateP(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) struct MUIP_MuiWheel_Active *mwa)
{
	struct DataP *Data = INST_DATA(cl, obj);

	if(mwa->active != MUIV_List_Active_Off)
	{
		struct ClassEntry *ce; 
	
		DoMethod(Data->d_NList, MUIM_NList_GetEntry, mwa->active, &ce);

		if(ce)
		{
			struct EntryPrefs *ep = &Data->d_ep[ce->ce_Id];

			set(Data->d_QualStr, 	MUIA_Disabled, !ep->ep_Qualifier);
			set(Data->d_Skip, 		MUIA_Disabled, !ep->ep_Skip		);
			set(Data->d_QSkip, 		MUIA_Disabled, !ep->ep_QSkip	);

			set(Data->d_Enable, 	MUIA_Disabled,  ce->ce_Enabled	);
			set(Data->d_Disable, 	MUIA_Disabled, !ce->ce_Enabled	);

			DoMethod(Data->d_Skip,  MUIM_NoNotifySet, MUIA_Numeric_Value, ep->ep_Skip 	? ce->ce_Skip 		: 0);
			DoMethod(Data->d_QSkip, MUIM_NoNotifySet, MUIA_Numeric_Value, ep->ep_QSkip 	? ce->ce_QualSkip 	: 0);
		}
	}

	return(0);
}

ULONG ASM __inline _UpdateSkipP(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) struct MUIP_MuiWheel_UpdateSkip *mwu)
{
	struct DataP *Data = INST_DATA(cl, obj);

	struct ClassEntry *ce; 
	
	DoMethod(Data->d_NList, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &ce);

	if(ce)	ce->ce_Skip = mwu->skip;

	DoMethod(Data->d_NList, MUIM_NList_Redraw, MUIV_NList_Redraw_Active);

	return(0);
}

ULONG ASM __inline _UpdateQSkipP(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) struct MUIP_MuiWheel_UpdateSkip *mwu)
{
	struct DataP *Data = INST_DATA(cl, obj);

	struct ClassEntry *ce; 
	
	DoMethod(Data->d_NList, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &ce);

	if(ce)	ce->ce_QualSkip = mwu->skip;

	DoMethod(Data->d_NList, MUIM_NList_Redraw, MUIV_NList_Redraw_Active);

	return(0);
}

ULONG ASM __inline _UpdateStatus(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	struct DataP *Data = INST_DATA(cl, obj);

	struct ClassEntry *ce; 

	DoMethod(Data->d_NList, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &ce);

	if(ce)
	{
		ce->ce_Enabled = !ce->ce_Enabled;	// Switch status

		set(Data->d_Enable, 	MUIA_Disabled, ce->ce_Enabled	);
		set(Data->d_Disable, 	MUIA_Disabled, !ce->ce_Enabled	);

		DoMethod(Data->d_NList, MUIM_NList_Redraw, MUIV_NList_Redraw_Active);
	}

	return(0);
}

ULONG ASM __inline _UpdateClick(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	struct DataP *Data = INST_DATA(cl, obj);

	if(xget(Data->d_NList, MUIA_NList_ClickColumn) == 0)
		return(_UpdateStatus(cl, obj, msg));
	else
		return(0);
}

void FillListEntry(STRPTR classname, struct ClassEntry *ce, struct MUIP_Settingsgroup_ConfigToGadgets *msg, ULONG v, ULONG q, ULONG e, UWORD Id)
{
	LONG *ptrd;                                                              

	ce->ce_ClassName 	= classname;
	ce->ce_Id 			= Id;

	if (ptrd = (LONG *)DoMethod(msg->configdata, MUIM_Dataspace_Find, v))  
		ce->ce_Skip = *ptrd;
	else
		ce->ce_Skip = 1;

	if (ptrd = (LONG *)DoMethod(msg->configdata, MUIM_Dataspace_Find, q))  
		ce->ce_QualSkip = *ptrd;
	else
		ce->ce_QualSkip = 2;

	if (ptrd = (LONG *)DoMethod(msg->configdata, MUIM_Dataspace_Find, e))  
		ce->ce_Enabled = *ptrd ? TRUE : FALSE;
	else
		ce->ce_Enabled = TRUE;

	ce->ce_pSkip 	= v;
	ce->ce_pQSkip	= q;
	ce->ce_pEnabled = e;
}


ULONG ASM __inline _ConfigToGadgets(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) struct MUIP_Settingsgroup_ConfigToGadgets *msg)
{
	struct DataP *Data = INST_DATA(cl, obj);

	struct ClassEntry ce;

	// Clear entries

	DoMethod(Data->d_NList, MUIM_NList_Clear);
	

	// Numeric.mui

	FillListEntry("Numeric.mui", &ce, msg, MUICFG_MuiWheel_Numeric_Value, MUICFG_MuiWheel_Numeric_QualValue, MUICFG_MuiWheel_Numeric_Enabled, MWID_Numeric);
	DoMethod(Data->d_NList, MUIM_NList_InsertSingle, &ce, MUIV_NList_Insert_Sorted);


	// Cycle.mui

	FillListEntry("Cycle.mui", &ce, msg, MUICFG_MuiWheel_Cycle_Value, MUICFG_MuiWheel_Cycle_QualValue, MUICFG_MuiWheel_Cycle_Enabled, MWID_Cycle);
	DoMethod(Data->d_NList, MUIM_NList_InsertSingle, &ce, MUIV_NList_Insert_Sorted);


	// Prop.mui

	FillListEntry("Prop.mui", &ce, msg, MUICFG_MuiWheel_Prop_Value, MUICFG_MuiWheel_Prop_QualValue, MUICFG_MuiWheel_Prop_Enabled, MWID_Prop);
	DoMethod(Data->d_NList, MUIM_NList_InsertSingle, &ce, MUIV_NList_Insert_Sorted);


	// List.mui

	FillListEntry("List.mui", &ce, msg, MUICFG_MuiWheel_List_VValue, MUICFG_MuiWheel_List_VQualValue, MUICFG_MuiWheel_List_Enabled, MWID_List);
	DoMethod(Data->d_NList, MUIM_NList_InsertSingle, &ce, MUIV_NList_Insert_Sorted);


	// Virtgroup.mui

	FillListEntry("Virtgroup.mui", &ce, msg, MUICFG_MuiWheel_Virtgroup_Value, MUICFG_MuiWheel_Virtgroup_QualValue, MUICFG_MuiWheel_Virtgroup_Enabled, MWID_Virtgroup);
	DoMethod(Data->d_NList, MUIM_NList_InsertSingle, &ce, MUIV_NList_Insert_Sorted);


	return(0);
}


#define UpdateConfig(class) \
	ce = Data->d_ep[MWID_ ## class ## ].ep_ce;\
	if(ce)\
	{\
		ps->ps_ ## class ## Enabled 	= ce->ce_Enabled;\
		ps->ps_ ## class ## Skip 		= ce->ce_Skip;\
		ps->ps_ ## class ## QualSkip 	= ce->ce_QualSkip;\
	}\


ULONG ASM __inline _GadgetsToConfig(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) struct MUIP_Settingsgroup_GadgetsToConfig *msg)
{
	struct DataP *Data = INST_DATA(cl, obj);

	struct PrefsSemaphore *ps;
	
	UWORD i;

	for(i = 0;;i++)	
	{
		struct ClassEntry *ce;

		DoMethod(Data->d_NList, MUIM_NList_GetEntry, i, &ce);

		if(!ce				)	break;
		if(!ce->ce_pEnabled	) 	break;

		DoMethod(msg->configdata, MUIM_Dataspace_Add, &ce->ce_Skip, 	4, ce->ce_pSkip		); 
		DoMethod(msg->configdata, MUIM_Dataspace_Add, &ce->ce_QualSkip, 4, ce->ce_pQSkip	); 
		DoMethod(msg->configdata, MUIM_Dataspace_Add, &ce->ce_Enabled, 	4, ce->ce_pEnabled	); 
	}

	// Update the configuration in memory

	Forbid();
	ps = (struct PrefsSemaphore *)FindSemaphore("MuiWheel");

	if(ps)
	{
		struct ClassEntry *ce;

		UpdateConfig(Numeric	);
		UpdateConfig(Cycle		);
		UpdateConfig(Prop		);
		UpdateConfig(List		);
		UpdateConfig(Virtgroup	);
	}
	Permit();

	return(0);
}


ULONG ASM SAVEDS _DispatcherP(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch(msg->MethodID)
	{
		case OM_NEW                 	      	: return(_NewP           	(cl,obj,(APTR)msg));
		case MUIM_Settingsgroup_ConfigToGadgets	: return(_ConfigToGadgets	(cl,obj,(APTR)msg));
		case MUIM_Settingsgroup_GadgetsToConfig	: return(_GadgetsToConfig	(cl,obj,(APTR)msg));
		case MUIM_MuiWheel_Active      	      	: return(_UpdateP          	(cl,obj,(APTR)msg));
		case MUIM_MuiWheel_UpdateSkip      	    : return(_UpdateSkipP      	(cl,obj,(APTR)msg));
		case MUIM_MuiWheel_UpdateQSkip     	    : return(_UpdateQSkipP     	(cl,obj,(APTR)msg));
		case MUIM_MuiWheel_UpdateStatus    	    : return(_UpdateStatus     	(cl,obj,(APTR)msg));
//		case MUIM_MuiWheel_UpdateClick    	    : return(_UpdateClick     	(cl,obj,(APTR)msg));
	}

	return(DoSuperMethodA(cl,obj,msg));
}
