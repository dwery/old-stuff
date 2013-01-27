/*
**	MuiWheelPatch - AmigaDOS 2.04 utility
**
**	Copyright © 1998-2001 by Alessandro Zummo
**		All Rights Reserved
**
*/

#include <exec/execbase.h>
#include <exec/memory.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/muimaster.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/commodities.h>

#include <exec/semaphores.h>

#include <devices/inputevent.h>

#include <libraries/mui.h>

#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>

#include <mousewheel.h>

#include <mui/MuiWheel_mcc.h>

#include "prefssemaphore.h"
#include <mui/muiundoc.h>

#include <debug.h>

#define REG(x) register __ ## x
#define ASM    __asm
#define SAVEDS __saveds


STRPTR ver = "$VER: MuiWheelPatch 1.5 "__AMIGADATE__" Copyright (c) 1998-2001 Alessandro Zummo";

struct Library 			*MUIMasterBase	= NULL;
struct IntuitionBase	*IntuitionBase	= NULL;

LONG	__stack = 8192;

#define SEMAPHORENAME "MuiWheel"
#define MUIWHEELMAGIC 0x20000929

// Prototypes

ULONG __asm (*OldNumericDispatcher)(REG(a0) struct IClass *, REG(a2) Object *, REG(a1) Msg );
ULONG __asm __saveds NewNumericDispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);

ULONG __asm (*OldListDispatcher)(REG(a0) struct IClass *, REG(a2) Object *, REG(a1) Msg );
ULONG __asm __saveds NewListDispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);

ULONG __asm (*OldCycleDispatcher)(REG(a0) struct IClass *, REG(a2) Object *, REG(a1) Msg );
ULONG __asm __saveds NewCycleDispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);

ULONG __asm (*OldPropDispatcher)(REG(a0) struct IClass *, REG(a2) Object *, REG(a1) Msg );
ULONG __asm __saveds NewPropDispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);

ULONG __asm (*OldScrollgroupDispatcher)(REG(a0) struct IClass *, REG(a2) Object *, REG(a1) Msg );
ULONG __asm __saveds NewScrollgroupDispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);

ULONG __asm (*OldVirtgroupDispatcher)(REG(a0) struct IClass *, REG(a2) Object *, REG(a1) Msg );
ULONG __asm __saveds NewVirtgroupDispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);

ULONG __asm (*OldAreaDispatcher)(REG(a0) struct IClass *, REG(a2) Object *, REG(a1) Msg );
ULONG __asm __saveds NewAreaDispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);

ULONG __asm (*OldApplicationDispatcher)(REG(a0) struct IClass *, REG(a2) Object *, REG(a1) Msg );
ULONG __asm __saveds NewApplicationDispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);


#define _between(a,x,b) ((x)>=(a) && (x)<=(b))
#define _isinobject(x,y) (_between(_left(obj),(x),_right(obj)) && _between(_top(obj),(y),_bottom(obj)))

									
struct ObjNode
{
	struct MinNode 					on_MinNode;

	Object							*on_Obj;

	struct 	MUI_EventHandlerNode 	on_ehn;
};



#define LoadConfig(class, skip, qualskip) \
	DoMethod(obj, MUIM_GetConfigItem, MUICFG_MuiWheel_ ## class ## _Enabled, &kptrd);\
	if(kptrd)\
		ps->ps_ ## class ## Enabled = *kptrd ? TRUE : FALSE;\ 
	else\
		ps->ps_ ## class ## Enabled = TRUE;\
	if(DoMethod(obj, MUIM_GetConfigItem, MUICFG_MuiWheel_ ## class ## _Value, &kptrd))\
		ps->ps_ ## class ## Skip = *kptrd;\
	else\
		ps->ps_ ## class ## Skip = skip;\
	if(DoMethod(obj, MUIM_GetConfigItem, MUICFG_MuiWheel_ ## class ## _QualValue, &kptrd))\
		ps->ps_ ## class ## QualSkip = *kptrd;\
	else\
		ps->ps_ ## class ## QualSkip = qualskip;


int main( void )
{
	struct PrefsSemaphore 		*ps;

	SysBase	=*(struct ExecBase **)(4L);

	Forbid();
	ps = (struct PrefsSemaphore *) FindSemaphore("MuiWheel");
	Permit();

	if(!ps)
	if(DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 36))
	{
		if(IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library", 36))
		{
			if(MUIMasterBase = OpenLibrary("muimaster.library", 19))
			{
				if(MUIMasterBase->lib_Version == 19)
				{
					struct IClass *Numeric;
					struct IClass *Cycle;
					struct IClass *Prop;
					struct IClass *List;
					struct IClass *Virtgroup;
					struct IClass *App;
	
					if(ps = AllocMem(sizeof(struct PrefsSemaphore), MEMF_ANY | MEMF_CLEAR))
					{
						if(ps->ps_Name = AllocMem(strlen(SEMAPHORENAME) + 1, MEMF_ANY))
						{
							Object *obj;

							CopyMem(SEMAPHORENAME, ps->ps_Name, strlen(SEMAPHORENAME) + 1);

							// Build the semaphore
	
							ps->ps_ss.ss_Link.ln_Name 	= ps->ps_Name;
							ps->ps_ss.ss_Link.ln_Pri  	= 50;
		
							// Just a fake object. We use it to load the prefs.

							if(obj = ApplicationObject, End)
							{
								LONG *kptrd;
				
								LoadConfig(Numeric		,1 ,2 );
								LoadConfig(Cycle		,1 ,1 );
								LoadConfig(Prop			,4 ,8 );
								LoadConfig(List			,8 ,16);
								LoadConfig(Virtgroup	,4 ,8 );
				
								MUI_DisposeObject(obj);
							
	
								NewList((struct List *)&ps->ps_ObjList);
				
								AddSemaphore(ps);
			
								Forbid();
				
								Numeric 							= MUI_GetClass(MUIC_Numeric		);
								OldNumericDispatcher 				= (ULONG (* __asm)(REG(a0) struct IClass *, REG(a2) Object *, REG(a1) Msg )) Numeric->cl_Dispatcher.h_Entry;
								Numeric->cl_Dispatcher.h_Entry		= (ULONG (* )()) NewNumericDispatcher;
				
								Cycle  								= MUI_GetClass(MUIC_Cycle		);
								OldCycleDispatcher 					= (ULONG (* __asm)(REG(a0) struct IClass *, REG(a2) Object *, REG(a1) Msg )) Cycle->cl_Dispatcher.h_Entry;
								Cycle->cl_Dispatcher.h_Entry		= (ULONG (* )()) NewCycleDispatcher;
				
								Prop   								= MUI_GetClass(MUIC_Prop		);
								OldPropDispatcher 					= (ULONG (* __asm)(REG(a0) struct IClass *, REG(a2) Object *, REG(a1) Msg )) Prop->cl_Dispatcher.h_Entry;
								Prop->cl_Dispatcher.h_Entry			= (ULONG (* )()) NewPropDispatcher;
				
								List   								= MUI_GetClass(MUIC_List		);
								OldListDispatcher 					= (ULONG (* __asm)(REG(a0) struct IClass *, REG(a2) Object *, REG(a1) Msg )) List->cl_Dispatcher.h_Entry;
								List->cl_Dispatcher.h_Entry			= (ULONG (* )()) NewListDispatcher;
				
								Virtgroup   						= MUI_GetClass(MUIC_Virtgroup	);
								OldVirtgroupDispatcher 				= (ULONG (* __asm)(REG(a0) struct IClass *, REG(a2) Object *, REG(a1) Msg )) Virtgroup->cl_Dispatcher.h_Entry;
								Virtgroup->cl_Dispatcher.h_Entry	= (ULONG (* )()) NewVirtgroupDispatcher;

								App			   						= MUI_GetClass(MUIC_Application);
								OldApplicationDispatcher 			= (ULONG (* __asm)(REG(a0) struct IClass *, REG(a2) Object *, REG(a1) Msg )) App->cl_Dispatcher.h_Entry;
								App->cl_Dispatcher.h_Entry			= (ULONG (* )()) NewApplicationDispatcher;
			
								Permit();
		
								// Don't use this. It was meant only for debugging purposes.
								Wait(SIGBREAKF_CTRL_D); 
				
								return(0);			
							}
							else
								Printf("MuiWheelPatch: Failed to create the mui-application.\n");
						}
						else
							Printf("MuiWheelPatch: No enough memory.\n");
					}
					else
						Printf("MuiWheelPatch: No enough memory.\n");
				}
				else
					Printf("MuiWheelPatch: You need muimaster.library version 19, you have version %ld.\n", MUIMasterBase->lib_Version);

				CloseLibrary(MUIMasterBase);
			}
			CloseLibrary(IntuitionBase);
		}
		CloseLibrary(DOSBase);	
	}
	return(5);
}



LONG __inline xget(Object *obj,ULONG attribute)
{
	LONG x;
	get(obj,attribute,&x);
	return(x);
}

struct ObjNode * __inline RemObjNode(Object *Obj)
{
	struct ObjNode *rc = NULL;

	struct PrefsSemaphore *ps;

	Forbid();
	ps = (struct PrefsSemaphore *)FindSemaphore("MuiWheel");
	Permit();

	if(ps)
	{
		struct ObjNode *on;

		Forbid();

		for(on = (struct ObjNode *)ps->ps_ObjList.mlh_Head; on->on_MinNode.mln_Succ; on = (struct ObjNode *) on->on_MinNode.mln_Succ)
		{
			if(on->on_Obj == Obj)	
			{
				rc = on;
				Remove((struct Node *)rc);
				break;
			}
		}

		Permit();
	}
	return(rc);
}

void __asm __inline Setup(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(d0) BYTE pri)
{
	struct PrefsSemaphore *ps;

	Forbid();
	ps = (struct PrefsSemaphore *)FindSemaphore("MuiWheel");
	Permit();

	if(ps)
	{
		struct ObjNode *on;

		if(on = AllocMem(sizeof(struct ObjNode), MEMF_ANY))
		{
			on->on_Obj	= obj;

			on->on_ehn.ehn_Priority 	= pri;
			on->on_ehn.ehn_Flags	 	= 0;
			on->on_ehn.ehn_Events 		= IDCMP_RAWKEY;
			on->on_ehn.ehn_Object 		= obj;
			on->on_ehn.ehn_Class 		= cl;

			Forbid();

			AddTail((struct List *)&ps->ps_ObjList, (struct Node*)on);

			Permit();

			DoMethod(_win(obj), MUIM_Window_AddEventHandler, &on->on_ehn);
		}
	}
}

void __asm __inline Cleanup(REG(a2) Object *obj)
{
	struct ObjNode *on;

	if(on = RemObjNode(obj))
	{
		// Node has already been removed.

		DoMethod(_win(obj), MUIM_Window_RemEventHandler, &on->on_ehn);

		FreeMem(on, sizeof(struct ObjNode));
	}
}

// Dispatchers...

#define IfQual(imsg) (imsg->Qualifier & IEQUALIFIER_MIDBUTTON)


ULONG __asm __saveds NewNumericDispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
    switch(msg->MethodID)
    {
		case MUIM_Setup:
			Setup(NULL, obj, 10);
			break;

		case MUIM_Cleanup:
			Cleanup(obj);
			break;

		case MUIM_HandleEvent:
		{
			struct IntuiMessage *imsg 	= ((struct MUIP_HandleEvent *)msg)->imsg;
			ULONG rc 					= (*OldNumericDispatcher)(cl,obj,msg);

			if(!(rc & MUI_EventHandlerRC_Eat))
			if(imsg && _isinobject(imsg->MouseX, imsg->MouseY))
			if(xget(obj, MUIA_Disabled) != TRUE)
			{
				struct PrefsSemaphore *ps;

				Forbid();
				ps = (struct PrefsSemaphore *)FindSemaphore("MuiWheel");
				Permit();

				if(ps && ps->ps_NumericEnabled)
				{
					switch(imsg->Code)
					{
						case MW_WHEEL_UP_KEY:
						{
							DoMethod(obj, MUIM_Numeric_Increase, IfQual(imsg) ? ps->ps_NumericQualSkip : ps->ps_NumericSkip);
							return(MUI_EventHandlerRC_Eat);
							break;
						}	
						case MW_WHEEL_DOWN_KEY:
						{
							DoMethod(obj, MUIM_Numeric_Decrease, IfQual(imsg) ? ps->ps_NumericQualSkip : ps->ps_NumericSkip);
							return(MUI_EventHandlerRC_Eat);
							break;
						}	
					}
				}
			}
			return(rc);
		}
	}
	return((*OldNumericDispatcher)(cl,obj,msg));
}

ULONG __asm __saveds NewCycleDispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
    switch(msg->MethodID)
    {
		case MUIM_Setup:
			Setup(cl, obj, 10);
			break;

		case MUIM_Cleanup:
			Cleanup(obj);
			break;

		case MUIM_HandleEvent:
		{
			struct IntuiMessage *imsg 	= ((struct MUIP_HandleEvent *)msg)->imsg;
			ULONG rc 					= (*OldCycleDispatcher)(cl,obj,msg);

			if(!(rc & MUI_EventHandlerRC_Eat))
			if(imsg && _isinobject(imsg->MouseX, imsg->MouseY))
			if(xget(obj, MUIA_Disabled) != TRUE)
			{
				struct PrefsSemaphore *ps;

				Forbid();
				ps = (struct PrefsSemaphore *)FindSemaphore("MuiWheel");
				Permit();

				if(ps && ps->ps_CycleEnabled)
				{
					switch(imsg->Code)
					{
						case MW_WHEEL_UP_KEY:
						{
							set(obj, MUIA_Cycle_Active, MUIV_Cycle_Active_Prev);
							return(MUI_EventHandlerRC_Eat);
							break;
						}	
						case MW_WHEEL_DOWN_KEY:
						{
							set(obj, MUIA_Cycle_Active, MUIV_Cycle_Active_Next);
							return(MUI_EventHandlerRC_Eat);
							break;
						}	
					}
				}
			}
			return(rc);
		}
	}
	return((*OldCycleDispatcher)(cl,obj,msg));
}

ULONG __asm __saveds NewPropDispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
    switch(msg->MethodID)
    {
		case MUIM_Setup:
			Setup(NULL, obj, 10);
			break;

		case MUIM_Cleanup:
			Cleanup(obj);
			break;

		case MUIM_HandleEvent:
		{
			struct IntuiMessage *imsg 	= ((struct MUIP_HandleEvent *)msg)->imsg;
			ULONG rc 					= (*OldPropDispatcher)(cl,obj,msg);

			if(!(rc & MUI_EventHandlerRC_Eat))
			if(imsg && _isinobject(imsg->MouseX, imsg->MouseY))
			if(xget(obj, MUIA_Disabled) != TRUE)
			{
				struct PrefsSemaphore *ps;

				Forbid();
				ps = (struct PrefsSemaphore *)FindSemaphore("MuiWheel");
				Permit();

				if(ps && ps->ps_PropEnabled)
				{
					switch(imsg->Code)
					{
						case MW_WHEEL_UP_KEY:
						{
							DoMethod(obj, MUIM_Prop_Decrease, IfQual(imsg) ? ps->ps_PropQualSkip : ps->ps_PropSkip);
							return(MUI_EventHandlerRC_Eat);
							break;
						}	
						case MW_WHEEL_DOWN_KEY:
						{
							DoMethod(obj, MUIM_Prop_Increase, IfQual(imsg) ? ps->ps_PropQualSkip : ps->ps_PropSkip);
							return(MUI_EventHandlerRC_Eat);
							break;
						}	
					}
				}
			}
			return(rc);
		}
	}
	return((*OldPropDispatcher)(cl,obj,msg));
}

ULONG __asm __saveds NewListDispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
    switch(msg->MethodID)
    {
		case MUIM_Setup:
			Setup(cl, obj, 10);
			break;

		case MUIM_Cleanup:
			Cleanup(obj);
			break;

		case MUIM_HandleEvent:
		{
			struct IntuiMessage *imsg 	= ((struct MUIP_HandleEvent *)msg)->imsg;
			ULONG rc 					= (*OldListDispatcher)(cl,obj,msg);

			if(!(rc & MUI_EventHandlerRC_Eat))
			if(imsg && _isinobject(imsg->MouseX, imsg->MouseY))
			{
				struct PrefsSemaphore *ps;

				Forbid();
				ps = (struct PrefsSemaphore *)FindSemaphore("MuiWheel");
				Permit();

				if(ps && ps->ps_ListEnabled)
				{
					if(imsg->Qualifier & IEQUALIFIER_MIDBUTTON)
					{
						if(ps->ps_ListQualSkip > 0)
						{
							LONG first 		= xget(obj, MUIA_List_First);
							LONG visible 	= xget(obj, MUIA_List_Visible);

							switch(imsg->Code)
							{
								case MW_WHEEL_UP_KEY:
									DoMethod(obj, MUIM_List_Jump, ((first - (ps->ps_ListQualSkip - 1) ) > 0) ? (first - ps->ps_ListQualSkip) : 0);
									return(MUI_EventHandlerRC_Eat);
									break;
			
								case MW_WHEEL_DOWN_KEY:
									DoMethod(obj, MUIM_List_Jump, first + visible + (ps->ps_ListQualSkip - 1));
									return(MUI_EventHandlerRC_Eat);
									break;
							}
						}
						else
						if(ps->ps_ListQualSkip < 0)
						{
							LONG first 		= xget(obj, MUIA_List_First);
							LONG visible 	= xget(obj, MUIA_List_Visible);

							switch(imsg->Code)
							{
								case MW_WHEEL_UP_KEY:
									DoMethod(obj, MUIM_List_Jump, first + visible + ((-(ps->ps_ListQualSkip)) - 1));
									return(MUI_EventHandlerRC_Eat);
									break;
			
								case MW_WHEEL_DOWN_KEY:
									DoMethod(obj, MUIM_List_Jump, ((first - ((-(ps->ps_ListQualSkip)) - 1) ) > 0) ? (first - (-(ps->ps_ListQualSkip))) : 0);
									return(MUI_EventHandlerRC_Eat);
									break;
							}
						}
					}
					else
					{
						if(ps->ps_ListSkip > 0)
						{
							LONG first 		= xget(obj, MUIA_List_First);
							LONG visible 	= xget(obj, MUIA_List_Visible);

							switch(imsg->Code)
							{
								case MW_WHEEL_UP_KEY:
									DoMethod(obj, MUIM_List_Jump, ((first - (ps->ps_ListSkip - 1) ) > 0) ? (first - ps->ps_ListSkip) : 0);
									return(MUI_EventHandlerRC_Eat);
									break;
			
								case MW_WHEEL_DOWN_KEY:
									DoMethod(obj, MUIM_List_Jump, first + visible + (ps->ps_ListSkip - 1));
									return(MUI_EventHandlerRC_Eat);
									break;
							}
						}
						else
						if(ps->ps_ListSkip < 0)
						{
							LONG first 		= xget(obj, MUIA_List_First);
							LONG visible 	= xget(obj, MUIA_List_Visible);

							switch(imsg->Code)
							{
								case MW_WHEEL_UP_KEY:
									DoMethod(obj, MUIM_List_Jump, first + visible + ((-(ps->ps_ListSkip)) - 1));
									return(MUI_EventHandlerRC_Eat);
									break;
			
								case MW_WHEEL_DOWN_KEY:
									DoMethod(obj, MUIM_List_Jump, ((first - ((-(ps->ps_ListSkip)) - 1) ) > 0) ? (first - (-(ps->ps_ListSkip))) : 0);
									return(MUI_EventHandlerRC_Eat);
									break;
							}
						}
					}
				}
			}
			return(rc);
		}
	}
	return((*OldListDispatcher)(cl,obj,msg));
}

ULONG __asm __saveds NewVirtgroupDispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
    switch(msg->MethodID)
    {
		case MUIM_Setup:
			Setup(cl, obj, 0);
			break;

		case MUIM_Cleanup:
			Cleanup(obj);
			break;

		case MUIM_HandleEvent:
		{
			struct IntuiMessage *imsg 	= ((struct MUIP_HandleEvent *)msg)->imsg;
			ULONG rc 					= (*OldVirtgroupDispatcher)(cl,obj,msg);

			if(!(rc & MUI_EventHandlerRC_Eat))
			if(_flags(obj) & MADF_OBJECTVISIBLE)			
			if(imsg && _isinobject(imsg->MouseX, imsg->MouseY))
			{
				struct PrefsSemaphore *ps;

				Forbid();
				ps = (struct PrefsSemaphore *)FindSemaphore("MuiWheel");
				Permit();

				if(ps)
				{
					if(ps->ps_VirtgroupEnabled)
					if(imsg->Qualifier & IEQUALIFIER_MIDBUTTON)
					{
						switch(imsg->Code)
						{
							case MW_WHEEL_UP_KEY:
								set(obj, MUIA_Virtgroup_Top, xget(obj, MUIA_Virtgroup_Top) - ((_height(obj)) / 2));
								return(MUI_EventHandlerRC_Eat);
								break;
		
							case MW_WHEEL_DOWN_KEY:
								set(obj, MUIA_Virtgroup_Top, xget(obj, MUIA_Virtgroup_Top) + ((_height(obj)) / 2));
								return(MUI_EventHandlerRC_Eat);
								break;

							case MW_WHEEL_LEFT_KEY:
								set(obj, MUIA_Virtgroup_Left, xget(obj, MUIA_Virtgroup_Left) - ((_width(obj)) / 2));
								return(MUI_EventHandlerRC_Eat);
								break;
		
							case MW_WHEEL_RIGHT_KEY:
								set(obj, MUIA_Virtgroup_Left, xget(obj, MUIA_Virtgroup_Left) + ((_width(obj)) / 2));
								return(MUI_EventHandlerRC_Eat);
								break;
						}
					}
					else
					{
						switch(imsg->Code)
						{
							case MW_WHEEL_UP_KEY:
								set(obj, MUIA_Virtgroup_Top, xget(obj, MUIA_Virtgroup_Top) - ((_height(obj)) / 4));
								return(MUI_EventHandlerRC_Eat);
								break;
		
							case MW_WHEEL_DOWN_KEY:
								set(obj, MUIA_Virtgroup_Top, xget(obj, MUIA_Virtgroup_Top) + ((_height(obj)) / 4));
								return(MUI_EventHandlerRC_Eat);
								break;

							case MW_WHEEL_LEFT_KEY:
								set(obj, MUIA_Virtgroup_Left, xget(obj, MUIA_Virtgroup_Left) - ((_width(obj)) / 4));
								return(MUI_EventHandlerRC_Eat);
								break;
		
							case MW_WHEEL_RIGHT_KEY:
								set(obj, MUIA_Virtgroup_Left, xget(obj, MUIA_Virtgroup_Left) + ((_width(obj)) / 4));
								return(MUI_EventHandlerRC_Eat);
								break;
						}
					}
				}
			}
			return(rc);
		}
	}
	return((*OldVirtgroupDispatcher)(cl,obj,msg));
}

ULONG __asm __saveds NewApplicationDispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
    if( msg->MethodID == OM_GET )
    {
		struct opGet *opg = ((struct opGet *) msg);

		if( opg->opg_AttrID == MUIA_Application_MsgPort )
		{
			struct List *wlist = (struct List *) xget(obj, MUIA_Application_WindowList);

			Object *winobj;
			Object *state = (Object *) wlist->lh_Head;

			while( winobj = (Object *) NextObject( &state ) )
			{
				struct Window *win = (struct Window *) xget(winobj, MUIA_Window_Window);

				if( win && win->UserData == 0x00000000 )
						win->UserData = (char *) MUIWHEELMAGIC;
			}
		}
	}
	return( (*OldApplicationDispatcher)(cl,obj,msg) );
}
