/*
 *		MCC_MuiWheel (c) by Alessandro Zummo
 *
 *		MuiWheel_mcc.h
*/

/*** Include stuff ***/

#ifndef MUIWHEEL_MCC_H
#define MUIWHEEL_MCC_H

#ifndef LIBRARIES_MUI_H
#include "libraries/mui.h"
#endif


/*** MUI Defines ***/

#define MUIC_MuiWheelP "MuiWheel.mcp"

#define TAGBASE_AZUMMO (TAG_USER | (0x2553 << 16))


#define MUICFG_MuiWheel_KeyBindings     		(TAGBASE_AZUMMO | 0x1010)

#define MUICFG_MuiWheel_List_Enabled   			(TAGBASE_AZUMMO | 0x1023)
#define MUICFG_MuiWheel_List_VValue   			(TAGBASE_AZUMMO | 0x1011)
#define MUICFG_MuiWheel_List_VQualValue			(TAGBASE_AZUMMO | 0x1012)
#define MUICFG_MuiWheel_List_VQualifier			(TAGBASE_AZUMMO | 0x1015)
#define MUICFG_MuiWheel_List_Value   			(TAGBASE_AZUMMO | 0x1011)
#define MUICFG_MuiWheel_List_QualValue			(TAGBASE_AZUMMO | 0x1012)
#define MUICFG_MuiWheel_List_Qualifier			(TAGBASE_AZUMMO | 0x1015)

#define MUICFG_MuiWheel_List_HValue				(TAGBASE_AZUMMO | 0x1013)
#define MUICFG_MuiWheel_List_HQualValue     	(TAGBASE_AZUMMO | 0x1014)
#define MUICFG_MuiWheel_List_HQualifier    		(TAGBASE_AZUMMO | 0x1016)

#define MUICFG_MuiWheel_Numeric_Enabled 		(TAGBASE_AZUMMO | 0x1024)
#define MUICFG_MuiWheel_Numeric_Value	     	(TAGBASE_AZUMMO | 0x1017)
#define MUICFG_MuiWheel_Numeric_QualValue     	(TAGBASE_AZUMMO | 0x1018)
#define MUICFG_MuiWheel_Numeric_Qualifier     	(TAGBASE_AZUMMO | 0x1019)

#define MUICFG_MuiWheel_Scrollgroup_Enabled 	(TAGBASE_AZUMMO | 0x1025)
#define MUICFG_MuiWheel_Scrollgroup_Value	    (TAGBASE_AZUMMO | 0x101A)
#define MUICFG_MuiWheel_Scrollgroup_QualValue   (TAGBASE_AZUMMO | 0x101B)
#define MUICFG_MuiWheel_Scrollgroup_Qualifier  	(TAGBASE_AZUMMO | 0x101C)

#define MUICFG_MuiWheel_Cycle_Enabled		 	(TAGBASE_AZUMMO | 0x1026)
#define MUICFG_MuiWheel_Cycle_Value	     		(TAGBASE_AZUMMO | 0x101D)
#define MUICFG_MuiWheel_Cycle_QualValue     	(TAGBASE_AZUMMO | 0x101E)
#define MUICFG_MuiWheel_Cycle_Qualifier     	(TAGBASE_AZUMMO | 0x101F)

#define MUICFG_MuiWheel_Prop_Enabled		 	(TAGBASE_AZUMMO | 0x1027)
#define MUICFG_MuiWheel_Prop_Value	     		(TAGBASE_AZUMMO | 0x1020)
#define MUICFG_MuiWheel_Prop_QualValue     		(TAGBASE_AZUMMO | 0x1021)
#define MUICFG_MuiWheel_Prop_Qualifier	     	(TAGBASE_AZUMMO | 0x1022)

#define MUICFG_MuiWheel_Virtgroup_Enabled	 	(TAGBASE_AZUMMO | 0x1028)
#define MUICFG_MuiWheel_Virtgroup_Value	    	(TAGBASE_AZUMMO | 0x1029)
#define MUICFG_MuiWheel_Virtgroup_QualValue   	(TAGBASE_AZUMMO | 0x102A)
#define MUICFG_MuiWheel_Virtgroup_Qualifier  	(TAGBASE_AZUMMO | 0x102B)


#define KEYTAG_List_VertScroll					(TAGBASE_AZUMMO | 0x0001)
#define KEYTAG_List_QualVertScroll				(TAGBASE_AZUMMO | 0x0002)
#define KEYTAG_List_HorizScroll					(TAGBASE_AZUMMO | 0x0003)
#define KEYTAG_List_QualHorizScroll				(TAGBASE_AZUMMO | 0x0004)



#endif /* MUIWHEEL_MCC_H */
