/*
 *		LayGroup.mcc (c) Copyright 1997-98 by Alessandro Zummo
 *
 *		Example for LayGroup.mcc
 *
 *		LayGroup-Demo.c
*/

#include <proto/exec.h>
#include <proto/muimaster.h>

#include <pragmas/muimaster_pragmas.h>
#include <libraries/mui.h>
#include <stdio.h>


#include "mui/LayGroup_mcc.h"

#define USE_PREFS_IMAGE_COLORS
#define USE_PREFS_IMAGE_BODY  
#include "prefs_image.c"      


#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))

struct Library *MUIMasterBase;

Object *prefs_image_object(void)                                        
{                                                                       
        Object *ret;                                                    
                                                                        
        ret = BodychunkObject,                                          
                MUIA_FixWidth             , PREFS_IMAGE_WIDTH,          
                MUIA_FixHeight            , PREFS_IMAGE_HEIGHT,         
                MUIA_Bitmap_Width         , PREFS_IMAGE_WIDTH,          
                MUIA_Bitmap_Height        , PREFS_IMAGE_HEIGHT,         
                MUIA_Bodychunk_Depth      , PREFS_IMAGE_DEPTH,          
                MUIA_Bodychunk_Body       , (UBYTE *)prefs_image_body,  
                MUIA_Bodychunk_Compression, PREFS_IMAGE_COMPRESSION,    
                MUIA_Bodychunk_Masking    , PREFS_IMAGE_MASKING,        
                MUIA_Bitmap_SourceColors  , (ULONG *)prefs_image_colors,
//              MUIA_Bitmap_Transparent   , 0,                          
                End;                                                    
                                                                        
        return(ret);                                                    
}                                                                       

int main(int argc,char *argv[])
{
	ULONG ret;

	if(MUIMasterBase = OpenLibrary(MUIMASTER_NAME, MUIMASTER_VLATEST))
	{
		APTR app;
		APTR window;
		APTR lg;

		ULONG sigs;

		app = ApplicationObject,
			MUIA_Application_Title      , "LayGroup-Demo",
			MUIA_Application_Version    , "$VER: LayGroup.mcc 19.0 "__AMIGADATE__,
			MUIA_Application_Copyright  , "© 1997-98 by Alessandro Zummo",
			MUIA_Application_Author     , "Alessandro Zummo",
			MUIA_Application_Description, "Demonstrates the LayGroup class.",
			MUIA_Application_Base       , "LAYGROUPDEMO",

			SubWindow, window = WindowObject,
				MUIA_Window_Title, "LayGroupClass",
				MUIA_Window_ID   , MAKE_ID('L','A','Y','G'),
//				MUIA_Window_Width, 240,
				WindowContents, VGroup,

					Child, lg = LayGroupObject,
						Child, prefs_image_object(),
						Child, prefs_image_object(),
						Child, prefs_image_object(),
						Child, prefs_image_object(),
						Child, prefs_image_object(),
						Child, prefs_image_object(),
						Child, prefs_image_object(),
						Child, prefs_image_object(),
						Child, prefs_image_object(),
						Child, prefs_image_object(),
						Child, prefs_image_object(),
						Child, prefs_image_object(),
						Child, prefs_image_object(),
						Child, prefs_image_object(),
						Child, prefs_image_object(),
						Child, prefs_image_object(),
						Child, prefs_image_object(),
						Child, prefs_image_object(),
						Child, prefs_image_object(),
						Child, prefs_image_object(),
						Child, prefs_image_object(),
						End,
					End,
				End,
			End;

		if(app)
		{
			/*** generate notifies ***/
			DoMethod(window, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
				app, 2,
				MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

			/*** ready to open the window ... ***/
			set(window,MUIA_Window_Open,TRUE);

			while (DoMethod(app,MUIM_Application_NewInput,&sigs) != MUIV_Application_ReturnID_Quit)
			{
				if (sigs)
				{
					sigs = Wait(sigs | SIGBREAKF_CTRL_C);
					if (sigs & SIGBREAKF_CTRL_C) break;
				}
			}

			set(window, MUIA_Window_Open, FALSE);

			/*** shutdown ***/
			MUI_DisposeObject(app);      /* dispose all objects. */

			ret = RETURN_OK;
		}
		else
		{
			printf("Could not open application!");
			ret = RETURN_FAIL;
		}

		CloseLibrary(MUIMasterBase);
	}
	else
	{
		printf("Could not open muimaster.library v%ld!", MUIMASTER_VLATEST);
		ret = RETURN_FAIL;
	}

	return(ret);
}
