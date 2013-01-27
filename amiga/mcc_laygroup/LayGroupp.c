/*

		LayGroup.mcc (c) Copyright 1997 by Alessandro Zummo

		LayGroupp.c

*/
	#include <string.h>
	
	#include <clib/alib_protos.h>
	#include <clib/graphics_protos.h>
	#include <clib/utility_protos.h>
	#include <clib/muimaster_protos.h>
	#include <pragmas/muimaster_pragmas.h>
	#include <libraries/mui.h>

	#include "mui/LayGroup_mcc.h"
	#include "mui/LayGroup_mcc_private.h"

	#include "debug.h"

#define CLASS       MUIC_LayGroupP
#define SUPERCLASSP MUIC_Mccprefs

struct DataP
{
	Object *rd_sample;
	Object *rd_sample2;
	Object *rd_sample3;
	Object *rd_string;
	Object *gr_prefs;

	Object *dp_Back;
	Object *dp_Frame;

	Object *dp_Vo, *dp_Ho;
	Object *dp_Lo, *dp_To;

	struct MUI_ImageSpec dp_BackSpec;

	struct MUI_CustomClass *dp_ZCenterClass;
};

char help_string[] = "Every single word of this text is a \033iText.mui\0330 object. You can enter a new string in the text gadget below and a special routine will parse and convert it to Text.mui objects. There are a few exceptions: every word prefixed by the \"@\" character will be converted to a @button and if you use the \"#\" character, it will be converted to a #string gadget. To output the special character itself, simply write it twice. \033bAnyway\0330 it's better to set the left and top offset to a fixed value, otherwise strange things may happen. The background of this object is fixed to MUII_TextBack. Enjoy the \033bpower\0330 of LayGroup.mcc.";

LONG __inline xget(Object *obj,ULONG attribute)
{
	LONG x;
	get(obj,attribute,&x);
	return(x);
}

struct ZCenter_Data
{
	char buf[20];
};

#define VERSION 		19
#define REVISION 		2
#define MASTERVERSION 	19



#define __VERSCR     "Copyright 1997-99, Alessandro Zummo"
#define __VSTRING    "LayGroup.mcp 19.2 " __AMIGADATE__
#define __VSTRINGN   "LayGroup.mcc 19.2 " __AMIGADATE__

#define UserLibID 	 "$VER: LayGroup.mcp 19.2 "__AMIGADATE__" "__VERSCR


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
//		MUIA_Bitmap_Transparent   , 0,
		End;


	return(obj);
}

Object * __inline imagegroup(ULONG qty)
{
	ULONG i;

	Object *obj = HGroup, 	MUIA_Group_HorizSpacing, 0,End;

	for(i = 0; i < qty; i++)
		DoMethod(obj, OM_ADDMEMBER, prefs_image_object());

	return(obj);
}

void __inline addalotofimages(ULONG qty, Object *grp)
{
	ULONG i;

	for(i = 0; i < qty; i++)
		DoMethod(grp, OM_ADDMEMBER, prefs_image_object());
}

ULONG __inline wordsasobjects(Object *grp, char *instr)
{
	char *token;
	char *str;

	ULONG added = 0, len;

	len = strlen(instr);

	if(len)
	{
		if(str = AllocMem(len + 1, MEMF_ANY | MEMF_CLEAR))
		{
			CopyMem(instr, str, len + 1);

			token = strtok(str, " ");

			while(token)
			{
				if(token[0] == '@')
				{
					token++;
					if(token[0] != '@')
						DoMethod(grp, OM_ADDMEMBER, SimpleButton(token));
					else
						DoMethod(grp, OM_ADDMEMBER, TextObject, MUIA_Text_Contents, token, End);
				}
				else
				if(token[0] == '#')
				{
					token++;
					if(token[0] != '#')
						DoMethod(grp, OM_ADDMEMBER, StringObject, StringFrame, MUIA_String_Contents, token, End);
					else
						DoMethod(grp, OM_ADDMEMBER, TextObject, MUIA_Text_Contents, token, End);
				}
				else
					DoMethod(grp, OM_ADDMEMBER, TextObject, MUIA_Text_Contents, token, End);
		
				added++;
		
				token = strtok(NULL, " ");
			}
			FreeMem(str, len + 1);
		}
	}
	return(added);
}

#define ZCenterObject NewObject(Data->dp_ZCenterClass->mcc_Class, NULL

SAVEDS ULONG __inline ZCenter_New(struct IClass *cl,Object *obj,struct opSet *msg)
{
	char c[] = "centered";
 
	if (obj=(Object *)DoSuperMethodA(cl,obj,msg))
	{
		struct ZCenter_Data *Data = INST_DATA(cl,obj);

		strcpy(Data->buf, c);
		return((ULONG)obj);
	}
	return(0);
}

SAVEDS ULONG __inline ZCenter_Stringify(struct IClass *cl,Object *obj,struct MUIP_Numeric_Stringify *msg)
{
	struct ZCenter_Data *Data = INST_DATA(cl,obj);

	if(msg->value == -1)
		return((ULONG)Data->buf);
	else
		return(DoSuperMethodA(cl,obj,msg));
}

SAVEDS ASM LONG ParseFunc(REG(a2) APTR obj, REG(a1) ULONG *x)
{
	char *string = *x;
	ULONG added;

	if(DoMethod(obj, MUIM_Group_InitChange))
	{
		struct Node *node;                    
		struct List *list;

		// Delete all objects...
	
		if(list = (struct List *)xget(obj,MUIA_Group_ChildList))
		{
			Object *child;

		    node = list->lh_Head;                    

			while(child = NextObject(&node))
			{	
				DoMethod(obj,OM_REMMEMBER,child);
				MUI_DisposeObject(child);
			}
		}

		// Check if help tet has been requested

		if((stricmp(string, "help") == 0) || (stricmp(string, "?") == 0))
			wordsasobjects(obj, help_string);
		else
		{	
			// Do the conversion of the user entered text

			added = wordsasobjects(obj, string);
	
			// If a single space has been enteres, print the secret text

			if(added == 0)
			{
				DoMethod(obj, OM_ADDMEMBER, prefs_image_object());
				wordsasobjects(obj, "You've discovered the secret example!");
			}
		}
		DoMethod(obj, MUIM_Group_ExitChange);
	}
	return(0);
}

static const struct Hook ParseHook = {{NULL,NULL },(VOID *)ParseFunc,NULL,NULL };

ULONG ASM SAVEDS ZCenter_Dispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW					:	return(ZCenter_New			(cl,obj,(APTR)msg));
		case MUIM_Numeric_Stringify	: 	return(ZCenter_Stringify   	(cl,obj,(APTR)msg));
	}
	return(DoSuperMethodA(cl,obj,msg));
}
	



ULONG ASM __inline _NewP(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	struct DataP *Data;

static const char *titles[] = { "Sample", "Another sample", "A complex one", NULL };

	if(!(obj = (Object *)DoSuperMethodA(cl, obj, msg)))
		return(0);

	Data = INST_DATA(cl,obj);

	Data->dp_ZCenterClass = MUI_CreateCustomClass(NULL, MUIC_Slider, NULL, sizeof(struct ZCenter_Data), ZCenter_Dispatcher);

	if(!Data->dp_ZCenterClass)
	{
		CoerceMethod(cl, obj, OM_DISPOSE);
		return(0);
	}


	Data->gr_prefs = 
	VGroup,
		Child, RegisterGroup(titles),
		MUIA_ShortHelp, "The LayGroup object sample,\nreacts on preferences below.",
		Child, VGroup,
			Child,	ScrollgroupObject,                                                   
				MUIA_Weight,80,
	        	MUIA_Scrollgroup_FreeHoriz, FALSE,                           
				MUIA_Scrollgroup_Contents, 
				Data->rd_sample = LayGroupObject,
					MUIA_Group_SameSize, TRUE,
					MUIA_VertWeight, 10,
					// Children will be added later
				End,
			End,
		End,
		Child, VGroup,
			Child,	ScrollgroupObject,                                                   
				MUIA_Weight,80,
	        	MUIA_Scrollgroup_FreeHoriz, FALSE,                           
				MUIA_Scrollgroup_Contents, 
				Data->rd_sample2 = LayGroupObject,
					Child, imagegroup(3),
					Child, imagegroup(5),
					Child, imagegroup(2),
					Child, imagegroup(7),
					Child, imagegroup(2),
					Child, imagegroup(3),
					Child, imagegroup(2),
					Child, imagegroup(4),
					Child, imagegroup(6),
					Child, imagegroup(1),
					Child, imagegroup(2),
					Child, imagegroup(2),
					Child, imagegroup(4),
					Child, imagegroup(2),
					Child, imagegroup(2),
					Child, imagegroup(3),
					Child, imagegroup(2),
					Child, imagegroup(2),
					Child, imagegroup(3),
					Child, imagegroup(2),
					Child, imagegroup(4),
					Child, imagegroup(6),
					Child, imagegroup(1),
					Child, imagegroup(2),
					Child, imagegroup(3),
					Child, imagegroup(2),
					Child, imagegroup(2),
					Child, imagegroup(3),
					Child, imagegroup(2),
					Child, imagegroup(2),
				End,
			End,
		End,
		Child, VGroup,
			Child,	ScrollgroupObject,                                                   
				MUIA_Weight,80,
	        	MUIA_Scrollgroup_FreeHoriz, FALSE,                           
				MUIA_Scrollgroup_Contents, 
				Data->rd_sample3 = LayGroupObject,
					MUIA_Background, MUII_TextBack,
//					MUIA_Frame, MUIV_Frame_None,
//					MUIA_LayGroup_AskLayout, TRUE,
					// Children will be added later
				End,
			End,
			Child, Data->rd_string = StringObject, StringFrame, End,
		End,
		End,
		Child, GroupObject,
			MUIA_Weight, 60,
			GroupFrameT("Preferences"),
			MUIA_Group_Columns, 4,
			Child,TextObject,
				MUIA_Text_Contents, "Background",
				MUIA_Text_SetVMax,FALSE,
			End,
			Child, Data->dp_Back = MUI_NewObject(MUIC_Popimage,        
			  MUIA_CycleChain, 1,                                          
			  MUIA_VertWeight, 60,
			  MUIA_Draggable, TRUE,                                        
			  MUIA_ShortHelp, "Adjust LayGroup Background.",                   
			  MUIA_Window_Title, "LayGroup Background",                        
			  MUIA_Imageadjust_Type, MUIV_Imageadjust_Type_Background,     
			End,                                                           
			Child,TextObject,
				MUIA_Text_Contents, "Frame",
				MUIA_Text_SetVMax,FALSE,
			End,
			Child, Data->dp_Frame = MUI_NewObject(MUIC_Popframe,        
			  MUIA_Disabled, TRUE,
			  MUIA_VertWeight, 60,
			  MUIA_CycleChain, 1,                                          
			  MUIA_Draggable, TRUE,                                        
			  MUIA_ShortHelp, "Adjust LayGroup Frame",                   
			  MUIA_Window_Title, "LayGroup Frame",                        
			End,                                                           
			Child,TextObject,
				MUIA_Text_Contents, "Horizontal spacing",
				MUIA_Text_SetVMax,FALSE,
			End,
			Child, Data->dp_Ho = SliderObject,
				MUIA_Numeric_Min,0,
				MUIA_Numeric_Max,9,
				MUIA_Numeric_Default, 4,
				MUIA_ShortHelp, "Horizontal space between two children",
			End,
			Child,TextObject,
				MUIA_Text_Contents, "Vertical spacing",
				MUIA_Text_SetVMax,FALSE,
			End,
			Child, Data->dp_Vo = SliderObject,
				MUIA_Numeric_Min,0,
				MUIA_Numeric_Max,9,
				MUIA_Numeric_Default, 4,
				MUIA_ShortHelp, "Vertical space between two children",
			End,
			Child,TextObject,
				MUIA_Text_Contents, "Left offset",
				MUIA_Text_SetVMax,FALSE,
			End,
			Child, Data->dp_Lo = ZCenterObject,
				MUIA_Numeric_Min,-1,
				MUIA_Numeric_Max,9,
				MUIA_Numeric_Default, 0,
				MUIA_ShortHelp, "Left offset",
			End,
			Child,TextObject,
				MUIA_Text_Contents, "Top offset",
				MUIA_Text_SetVMax,FALSE,
			End,
			Child, Data->dp_To = ZCenterObject,
				MUIA_Numeric_Min,-1,
				MUIA_Numeric_Max,9,
				MUIA_Numeric_Default, 0,
				MUIA_ShortHelp, "Top offset",
			End,
		End,
		Child, CrawlingObject,
    		MUIA_FixHeightTxt, "\n",
			TextFrame,
			MUIA_Background, MUII_TextBack,
			Child,HGroup,
				Child, prefs_image_object(),
				Child, TextObject,
					MUIA_Text_Contents, __VSTRINGN "\n" __VERSCR,
					MUIA_Text_PreParse, MUIX_C,
				End,
				Child, prefs_image_object(),
			End,
			Child, TextObject,
				MUIA_Text_Contents, __VSTRING "\n" __VERSCR,
				MUIA_Text_PreParse, MUIX_C,
			End,
			Child, TextObject,
				MUIA_Text_Contents, "\nFor request and reports\nwrite an email to:\n\nazummo@ita.flashnet.it",
				MUIA_Text_PreParse, MUIX_C,
			End,
			Child, TextObject,
				MUIA_Text_Contents, "\nThanks to Holger Papajewski for his help\non the layout hook.\n",
				MUIA_Text_PreParse, MUIX_C,
			End,
			Child,HGroup,
				Child, prefs_image_object(),
				Child, TextObject,
					MUIA_Text_Contents, __VSTRINGN "\n" __VERSCR,
					MUIA_Text_PreParse, MUIX_C,
				End,
				Child, prefs_image_object(),
			End,
		End,
	End;


	if(!Data->gr_prefs)
	{
		CoerceMethod(cl, obj, OM_DISPOSE);
		return(0);
	}

	addalotofimages(63, Data->rd_sample);

	wordsasobjects(Data->rd_sample3, help_string);

	DoMethod(obj, OM_ADDMEMBER, Data->gr_prefs);

	DoMethod(Data->dp_Back, MUIM_Notify, MUIA_Imagedisplay_Spec, MUIV_EveryTime, Data->rd_sample, 7, MUIM_MultiSet, MUIA_Background, MUIV_TriggerValue, Data->rd_sample, Data->rd_sample2, NULL);

	DoMethod(Data->dp_Vo, MUIM_Notify, MUIA_Numeric_Value, MUIV_EveryTime, Data->rd_sample, 7, MUIM_MultiSet, MUIA_LayGroup_VertSpacing, MUIV_TriggerValue, Data->rd_sample, Data->rd_sample2, Data->rd_sample3, NULL);
	DoMethod(Data->dp_Ho, MUIM_Notify, MUIA_Numeric_Value, MUIV_EveryTime, Data->rd_sample, 7, MUIM_MultiSet, MUIA_LayGroup_HorizSpacing, MUIV_TriggerValue, Data->rd_sample, Data->rd_sample2, Data->rd_sample3, NULL);
	DoMethod(Data->dp_Lo, MUIM_Notify, MUIA_Numeric_Value, MUIV_EveryTime, Data->rd_sample, 7, MUIM_MultiSet, MUIA_LayGroup_LeftOffset, MUIV_TriggerValue, Data->rd_sample, Data->rd_sample2, Data->rd_sample3, NULL);
	DoMethod(Data->dp_To, MUIM_Notify, MUIA_Numeric_Value, MUIV_EveryTime, Data->rd_sample, 7, MUIM_MultiSet, MUIA_LayGroup_TopOffset, MUIV_TriggerValue, Data->rd_sample, Data->rd_sample2, Data->rd_sample3, NULL);

//	DoMethod(Data->rd_string, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, Data->dp_Lo, 3, MUIM_ZCenter_Job, MUIV_TriggerValue, Data->rd_sample3);
	DoMethod(Data->rd_string, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, Data->rd_sample3, 3, MUIM_CallHook, &ParseHook, MUIV_TriggerValue);

//	DoMethod(Data->dp_Frame,MUIM_Notify,MUIA_Framedisplay_Spec, MUIV_EveryTime, Data->rd_sample, 3, MUIM_Set, MUIA_Frame     , MUIV_TriggerValue);

	return((ULONG)obj);
}


ULONG ASM __inline _DisposeP(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	struct DataP *Data = INST_DATA(cl, obj);

	ULONG rc = DoSuperMethodA(cl,obj,msg);

	if(Data->dp_ZCenterClass) MUI_DeleteCustomClass(Data->dp_ZCenterClass);	

	return(rc);
}


ULONG ASM __inline _ConfigToGadgets(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) struct MUIP_Settingsgroup_ConfigToGadgets *msg)
{
	struct DataP *Data = INST_DATA(cl, obj);

	LONG *ptrd;                                                              

	if (ptrd = (LONG *)DoMethod(msg->configdata,MUIM_Dataspace_Find,MUICFG_LayGroup_Back))  
	  set(Data->dp_Back, MUIA_Imagedisplay_Spec, ptrd);                   
	else                                                                      
	  set(Data->dp_Back, MUIA_Imagedisplay_Spec, "6:35");        

	if (ptrd = (LONG *)DoMethod(msg->configdata,MUIM_Dataspace_Find,MUICFG_LayGroup_Frame))  
	  set(Data->dp_Frame, MUIA_Framedisplay_Spec, ptrd);                   
	else                                                                      
	  set(Data->dp_Frame, MUIA_Framedisplay_Spec, MUIV_Frame_Virtual);        


	if (ptrd = (LONG *)DoMethod(msg->configdata,MUIM_Dataspace_Find,MUICFG_LayGroup_HSpace))  
	  set(Data->dp_Ho, MUIA_Numeric_Value, *ptrd);
	else
	  set(Data->dp_Ho, MUIA_Numeric_Value, 4);        

	if (ptrd = (LONG *)DoMethod(msg->configdata,MUIM_Dataspace_Find,MUICFG_LayGroup_VSpace))  
	  set(Data->dp_Vo, MUIA_Numeric_Value, *ptrd);                   
	else                                                                      
	  set(Data->dp_Vo, MUIA_Numeric_Value, 4);        

	if (ptrd = (LONG *)DoMethod(msg->configdata,MUIM_Dataspace_Find,MUICFG_LayGroup_LOff))  
	  set(Data->dp_Lo, MUIA_Numeric_Value, *ptrd);
	else
	  set(Data->dp_Lo, MUIA_Numeric_Value, -1);        

	if (ptrd = (LONG *)DoMethod(msg->configdata,MUIM_Dataspace_Find,MUICFG_LayGroup_TOff))  
	  set(Data->dp_To, MUIA_Numeric_Value, *ptrd);
	else
	  set(Data->dp_To, MUIA_Numeric_Value, -1);        




//191 Botta

//Responsabile ISDN al negozio: sig.ra Pravella

	return(0);
}


ULONG ASM __inline _GadgetsToConfig(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) struct MUIP_Settingsgroup_GadgetsToConfig *msg)
{
	struct DataP *Data = INST_DATA(cl, obj);

	LONG ptrd;

// 64 bytes
	get(Data->dp_Back, MUIA_Imagedisplay_Spec, &ptrd);                                               
	if (ptrd)                                                                                            
	  DoMethod(msg->configdata, MUIM_Dataspace_Add, ptrd, sizeof(struct MUI_ImageSpec), MUICFG_LayGroup_Back); 


// 32 bytes
	get(Data->dp_Frame, MUIA_Framedisplay_Spec, &ptrd);                                               
	if (ptrd)                                                                                            
	  DoMethod(msg->configdata, MUIM_Dataspace_Add, ptrd, 32 /*sizeof(struct MUI_FrameSpec)*/, MUICFG_LayGroup_Frame); 

	get(Data->dp_Ho, MUIA_Numeric_Value, &ptrd);                                               
	if (ptrd)                                                                                            
	  DoMethod(msg->configdata, MUIM_Dataspace_Add, &ptrd, 4, MUICFG_LayGroup_HSpace); 

	get(Data->dp_Vo, MUIA_Numeric_Value, &ptrd);                                               
	if (ptrd)                                                                                            
	  DoMethod(msg->configdata, MUIM_Dataspace_Add, &ptrd, 4, MUICFG_LayGroup_VSpace); 

	get(Data->dp_Lo, MUIA_Numeric_Value, &ptrd);                                               
	if (ptrd)                                                                                            
	  DoMethod(msg->configdata, MUIM_Dataspace_Add, &ptrd, 4, MUICFG_LayGroup_LOff); 

	get(Data->dp_To, MUIA_Numeric_Value, &ptrd);                                               
	if (ptrd)                                                                                            
	  DoMethod(msg->configdata, MUIM_Dataspace_Add, &ptrd, 4, MUICFG_LayGroup_TOff); 

	return(0);
}

ULONG ASM SAVEDS _DispatcherP(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch(msg->MethodID)
	{
		case OM_NEW                       : return(_NewP           (cl,obj,(APTR)msg));
		case OM_DISPOSE                   : return(_DisposeP       (cl,obj,(APTR)msg));

		case MUIM_Settingsgroup_ConfigToGadgets: return(_ConfigToGadgets(cl,obj,(APTR)msg));
		case MUIM_Settingsgroup_GadgetsToConfig: return(_GadgetsToConfig(cl,obj,(APTR)msg));

	}

	return(DoSuperMethodA(cl,obj,msg));
}


// ****************************************************************************************************************
// ****************************************************************************************************************
// ****************************************************************************************************************
// ****************************************************************************************************************


/*

> In muic_framedisplaye there is:
> 
> MUIA_Framedisplay_Spec         0x80421794
> 
> The structure is 6 bytes long
> 
> with frame type,state,4 offset values
> 
> 
> Don't you want to discover how can we modify these values in an Area class?
> I think the suld be write somewer in his istance data.
> 
> It may be usefull for your NListviews :-)

setting MUI_Frame don't work ?

what would be usefull for me is to set MUIA_Listview_Input to FALSE as
it make bad things when it's TRUE for a Listview using a NList child.
And of course that tag is Initable but not Settable   :(
I think that only way will be to find it in instance data and change it
directly...




>  Hi..i'm writing a subclass with mcp.
> 
>  I'm having problems while loading the preferences from within the .mcc.
> 
>  I suppose i should do that in the setup method..can you send me a piece of
>  code that may help me?

getting config data can be made anywhere between (included) Setup/Cleanup
(most of time it's done in Setup).

some little example in Setup :

  { LONG *ptrd;
    if (DoMethod(obj, MUIM_GetConfigItem, MUICFG_xxx, &ptrd))
      data->xxx = *ptrd; /* ptrd is a pointer on the data stored by mcp class 
for MUICFG_xxx */
    else
      data->xxx = default_value_for_xxx;
  }


What i use to get pref backgrounds :

#define LOAD_BG(test,var_dest,cfg_attr,defaultval) \
  { \
    if (!test) \
    { \
      LONG ptrd; \
      if (DoMethod(obj, MUIM_GetConfigItem, cfg_attr, &ptrd)) \
        var_dest = ptrd; \
      else \
        var_dest = (LONG) defaultval; \
    } \
  }

then in Setup :

  LOAD_BG(data->BG_List_init,    data->NList_ListBackGround,    
MUICFG_NList_BG_List,    DEFAULT_BG_LIST);

/* data->BG_List_init is a BOOL which was set if MUIA_NList_ListBackground was 
set at init, which mean that default should not be used */

in that case, defines are :
#define MUICFG_NList_BG_List        0x9d510007
#define DEFAULT_BG_LIST      "2:m2"
                              ^^^^
this one should be considered obsolete... anyway i don't see why not using it.

Perhaps i should have used that insteed :
#define DEFAULT_BG_LIST    MUII_ListBack






In the mcc, are used for the background pref :

          Child, data->mcp_BG_List = MUI_NewObject(MUIC_Popimage,
            MUIA_CycleChain, 1,
            MUIA_Imageadjust_Type, MUIV_Imageadjust_Type_Background,
            MUIA_Window_Title, "List Background",
            MUIA_Draggable, TRUE,
            MUIA_ShortHelp, "Adjust List Background.",
          End,

in MUIM_Settingsgroup_ConfigToGadgets :

/*usable as is for any LONG data, data->mcp_xxx is the object */
  {
    LONG *ptrd;
    LONG num = 1;  /* default value for never saved config data */
    if (ptrd = (LONG *) DoMethod(msg->configdata, MUIM_Dataspace_Find, 
MUICFG_xxx))
      num = *ptrd; /* ptrd is a pointer on the data previously stored by mcp 
class for MUICFG_xxx */
    set(data->mcp_xxx,MUIA_xxx, num);
  }

/* for the background */
  LOAD_DATASPEC(data->mcp_BG_List, MUIA_Imagedisplay_Spec, 
MUICFG_NList_BG_List, DEFAULT_BG_LIST);



in MUIM_Settingsgroup_GadgetsToConfig :

/* usable for any LONG data, data->mcp_xxx is the object.
 * i don't know why, but my tests seemed to show that a size of 8 is need to
 * store LONG (should be 4 bytes for a LONG, but i had problems when using 4)
 */
  {
    LONG ptrd,num;
    get(data->mcp_xxx, MUIA_xxx, &ptrd);
    num = ptrd;
    DoMethod(msg->configdata, MUIM_Dataspace_Add, &num, 8, MUICFG_xxx);
  }

/* for the background */
  SAVE_DATASPEC2(data->mcp_BG_List, MUIA_Imagedisplay_Spec, 
MUICFG_NList_BG_List);



where :

#define LOAD_DATASPEC(obj,attr,cfg_attr,defaultval) \
  { \
    LONG ptrd; \
    if (ptrd = DoMethod(msg->configdata, MUIM_Dataspace_Find, cfg_attr)) \
      set(obj, attr, ptrd); \
    else \
      set(obj, attr, defaultval); \
  }

#define SAVE_DATASPEC2(obj,attr,cfg_attr) \
  { \
    LONG ptrd; \
    get(obj, attr, &ptrd); \
    if (ptrd) \
      DoMethod(msg->configdata, MUIM_Dataspace_Add, ptrd, sizeof(struct 
MUI_ImageSpec), cfg_attr); \
  }



regards,

Gilles Masson

*/