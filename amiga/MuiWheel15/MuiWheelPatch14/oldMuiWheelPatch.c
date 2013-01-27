/*
**	MuiWheelPatch - AmigaDOS 2.04 utility
**
**	Copyright © 1998-99 by Alessandro Zummo
**		All Rights Reserved
**
*/

#include <exec/execbase.h>
#include <exec/memory.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/muimaster.h>
#include <proto/intuition.h>

#include <exec/semaphores.h>

#include <devices/inputevent.h>

#include <libraries/mui.h>

#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>

#include <mousewheel.h>

#include <mui/MuiWheel_mcc.h>

#include "prefssemaphore.h"

#include <debug.h>

#define REG(x) register __ ## x
#define ASM    __asm
#define SAVEDS __saveds


STRPTR ver = "$VER: MuiWheelPatch 1.3b4 "__AMIGADATE__" Copyright (c) 1998-99 Alessandro Zummo";

struct Library 			*MUIMasterBase	= NULL;
struct IntuitionBase	*IntuitionBase	= NULL;

LONG	__stack = 8192;

#define SEMAPHORENAME "MuiWheel"

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


#define _between(a,x,b) ((x)>=(a) && (x)<=(b))
#define _isinobject(x,y) (_between(_left(obj),(x),_right(obj)) && _between(_top(obj),(y),_bottom(obj)))

									
struct ObjNode
{
	struct MinNode 					on_MinNode;

	Object							*on_Obj;

	struct 	MUI_InputHandlerNode 	on_ihn;
	
	CxObj							*on_f_up,
									*on_f_down;
};


struct AppNode
{
	struct MinNode 					an_MinNode;

	Object							*an_App;

	struct MinList					*an_ObjList;

	struct MUI_InputHandlerNode		an_ihn_up;
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

	if(!(ps = (struct PrefsSemaphore *) FindSemaphore("MuiWheel")))
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
				
								Numeric 							= MUI_GetClass("Numeric.mui"	);
								OldNumericDispatcher 				= (ULONG (* __asm)(REG(a0) struct IClass *, REG(a2) Object *, REG(a1) Msg )) Numeric->cl_Dispatcher.h_Entry;
								Numeric->cl_Dispatcher.h_Entry		= (ULONG (* )()) NewNumericDispatcher;
				
								Cycle  								= MUI_GetClass("Cycle.mui"		);
								OldCycleDispatcher 					= (ULONG (* __asm)(REG(a0) struct IClass *, REG(a2) Object *, REG(a1) Msg )) Cycle->cl_Dispatcher.h_Entry;
								Cycle->cl_Dispatcher.h_Entry		= (ULONG (* )()) NewCycleDispatcher;
				
								Prop   								= MUI_GetClass("Prop.mui"		);
								OldPropDispatcher 					= (ULONG (* __asm)(REG(a0) struct IClass *, REG(a2) Object *, REG(a1) Msg )) Prop->cl_Dispatcher.h_Entry;
								Prop->cl_Dispatcher.h_Entry			= (ULONG (* )()) NewPropDispatcher;
				
								List   								= MUI_GetClass("List.mui"		);
								OldListDispatcher 					= (ULONG (* __asm)(REG(a0) struct IClass *, REG(a2) Object *, REG(a1) Msg )) List->cl_Dispatcher.h_Entry;
								List->cl_Dispatcher.h_Entry			= (ULONG (* )()) NewListDispatcher;
				
								Virtgroup   						= MUI_GetClass("Virtgroup.mui"	);
								OldVirtgroupDispatcher 				= (ULONG (* __asm)(REG(a0) struct IClass *, REG(a2) Object *, REG(a1) Msg )) Virtgroup->cl_Dispatcher.h_Entry;
								Virtgroup->cl_Dispatcher.h_Entry	= (ULONG (* )()) NewVirtgroupDispatcher;
			
								Permit();
		
								Wait(SIGBREAKF_CTRL_D); 
								Wait(SIGBREAKF_CTRL_D); 
								Wait(SIGBREAKF_CTRL_D); 
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

	if(ps = (struct PrefsSemaphore *)FindSemaphore("MuiWheel"))
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

void __asm __inline AppSetup(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(d0) BYTE pri)
{
	struct PrefsSemaphore *ps;

	if(ps = (struct PrefsSemaphore *)FindSemaphore("MuiWheel"))
	{
		struct AppNode *an;

		if(an = AllocMem(sizeof(struct AppNode), MEMF_ANY))
		{
			CxObj	*broker, *f_up, *f_down, *t_up, *t_down;

			struct MsgPort *mp;

			broker 	= xget(obj, MUIA_Applicatian_Broker);
//			mp 		= xget(obj, MUIA_Applicatian_BrokerPort);

			an.an_mp = CreateMsgPort();

			if(broker)
			{
				f_up   = CxFilter(NULL);
				f_down = CxFilter(NULL);
	
				AttachCxObj(broker, f_up);
				AttachCxObj(broker, f_down);
		
				if(f_up && f_down)
				{
					SetFilterIX(f_up,   &ps->ps_ix_up  );
					SetFilterIX(f_down, &ps->ps_ix_down);
	
					AttachCxObj(f_up,   CxCustom(handle_wheel, EVT_UP  ));
					AttachCxObj(f_down, CxCustom(handle_wheel, EVT_DOWN));
	
//					AttachCxObj(f_up,	CxTranslate(NULL));
//					AttachCxObj(f_down,	CxTranslate(NULL));
				}
			}

			an->an_App	= obj;
/*
			on->on_ihn_up.ihn_Object	= obj;
			on->on_ihn_up.ihn_Signals	= 0;
			on->on_ihn_up.ihn_Flags		= 0;
			on->on_ihn_up.ihn_Method	= MUIM_WHEEL_UP;
*/

			Forbid();

			AddTail((struct List *)&ps->ps_AppList, (struct Node*)an);

			Permit();

//			DoMethod(obj, MUIM_Application_AddInputHandler, &an->an_ihn);

		}
	}
}

void __asm __inline Setup(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(d0) BYTE pri)
{
	struct PrefsSemaphore *ps;

	if(ps = (struct PrefsSemaphore *)FindSemaphore("MuiWheel"))
	{
		struct ObjNode *on;

		if(on = AllocMem(sizeof(struct ObjNode), MEMF_ANY))
		{
			CxObj	*broker, *f_up, *f_down, *t_up, *t_down;

			struct MsgPort *mp;

			broker 	= xget(_app(obj), MUIA_Application_Broker);
			onmp 		= xget(_app(obj), MUIA_Application_BrokerPort);

			if(broker)
			{
				on->on_f_up   = CxFilter(NULL);
				on->on_f_down = CxFilter(NULL);
	
				AttachCxObj(broker, on->on_f_up);
				AttachCxObj(broker, on->on_f_down);
		
				if(on->on_f_up && on->on_f_down)
				{
					SetFilterIX(on->on_f_up,   &ps->ps_ix_up  );
					SetFilterIX(on->on_f_down, &ps->ps_ix_down);
	
					AttachCxObj(on->on_f_up,   CxSender(mp, 0x7FFFFF0A));
					AttachCxObj(on->on_f_down, CxSender(mp, 0x7FFFFF0B));
	
					AttachCxObj(on->on_f_up,	CxTranslate(NULL));
					AttachCxObj(on->on_f_down,	CxTranslate(NULL));
				}
			}

			on->on_Obj	= obj;
/*
			on->on_ihn.ihn_Object	= obj;
			on->on_ihn.ihn_Signals	= 1L << mp->mp_SigBit;
			on->on_ihn.ihn_Flags	= 0;
			on->on_ihn.ihn_Method	= MUIM_WHEEL_UP;
*/

			Forbid();

			AddTail((struct List *)&ps->ps_ObjList, (struct Node*)on);

			Permit();

//			DoMethod(_win(obj), MUIM_Window_AddEventHandler, &on->on_ehn);
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
			{
				struct PrefsSemaphore *ps;

				if(ps = (struct PrefsSemaphore *)FindSemaphore("MuiWheel"))
				{
					if(ps->ps_NumericEnabled)
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

				if(ps = (struct PrefsSemaphore *)FindSemaphore("MuiWheel"))
				{
					if(ps->ps_CycleEnabled)
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
			{
				struct PrefsSemaphore *ps;

				if(ps = (struct PrefsSemaphore *)FindSemaphore("MuiWheel"))
				{
					if(ps->ps_PropEnabled)
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
				LONG first 		= xget(obj, MUIA_List_First);
				LONG visible 	= xget(obj, MUIA_List_Visible);

				struct PrefsSemaphore *ps;

				if(ps = (struct PrefsSemaphore *)FindSemaphore("MuiWheel"))
				{
					if(ps->ps_ListEnabled)
					if(imsg->Qualifier & IEQUALIFIER_MIDBUTTON)
					{
						if(ps->ps_ListQualSkip != 0)
						if(ps->ps_ListQualSkip > 0)
						{
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
						{
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

						if(ps->ps_ListSkip != 0)
						if(ps->ps_ListSkip > 0)
						{
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
						{
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
			if(imsg && _isinobject(imsg->MouseX, imsg->MouseY))
			{
				struct PrefsSemaphore *ps;

				if(ps = (struct PrefsSemaphore *)FindSemaphore("MuiWheel"))
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
						}
					}
				}
			}
			return(rc);
		}
	}
	return((*OldVirtgroupDispatcher)(cl,obj,msg));
}
