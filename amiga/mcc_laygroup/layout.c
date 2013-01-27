
SAVEDS ULONG ASM GroupLayoutFunc(REG(a0) struct Hook *h,REG(a2) Object *obj,REG(a1) struct MUI_LayoutMsg *lm)
{
   	/* get my private data from hook structure */
	// h_Data points to an istance data

	struct Data *Data = (struct Data*)h->h_Data;

	Object *cstate = (Object *)lm->lm_Children->mlh_Head;
	Object *child;

	#define hs  Data->lg_HSpace
	#define vs  Data->lg_VSpace
    #define lo  Data->lg_LOff
    #define to  Data->lg_TOff

	switch (lm->lm_Type)
  	{
    	case MUILM_MINMAX:
    	{
			// We are loading the preferences here since there isn't a better place
            // to do it. :-(

			LoadPrefs(obj, Data);

	      	/* find out biggest widths & heights of our children */

      		while (child = NextObject(&cstate))
   	  		{
       			if (Data->lg_MaxWidth  < MUI_MAXMAX && _defwidth (child) > Data->lg_MaxWidth)
					Data->lg_MaxWidth  = _defwidth (child);

        		if (Data->lg_MaxHeight < MUI_MAXMAX && _defheight(child) > Data->lg_MaxHeight)
					Data->lg_MaxHeight = _defheight(child);
   			}

      		/* set our dimensions */

      		lm->lm_MinMax.MinWidth  = lo + (hs * 2) + Data->lg_MaxWidth; 	// lo + hs + Data->lg_MaxWidth  + hs;
      		lm->lm_MinMax.MinHeight = to + (vs * 2) + Data->lg_MaxHeight;	// to + vs + Data->lg_MaxHeight + vs;

      		lm->lm_MinMax.DefWidth  = (lm->lm_MinMax.MinWidth)  + Data->lg_MaxWidth  + hs;
      		lm->lm_MinMax.DefHeight = (lm->lm_MinMax.MinHeight) + Data->lg_MaxHeight + vs;

      		lm->lm_MinMax.MaxWidth  = MUI_MAXMAX;
      		lm->lm_MinMax.MaxHeight = MUI_MAXMAX;

      		return(0);
    	}

    	case MUILM_LAYOUT:
    	{
	        LONG llo, lto,l,t,wc;
		    UWORD width, height;
			UWORD maxlineheight = 0;
            
            llo = Data->lg_LOff;
            lto = Data->lg_TOff;

			Data->lg_ColNumber = Data->lg_RowNumber = 0;

      		if( child = NextObject(&cstate))
      		{
        		width = _defwidth(child);
        		height= _defheight(child);

        		/* how many children match into one line */
        		wc = (lm->lm_Layout.Width)/(width+hs);

        		/* calc left offset to center the line into the group */
        		if(llo == -1) llo = (lm->lm_Layout.Width - (width*wc) - (hs*(wc-1))) / 2;
				if(lto == -1) lto = vs;

        		/* left is left offset */
        		l  = llo;

        		/* top is top offset space */
        		t  = lto;

        		/* set for all child there position */
        		cstate = (Object *)lm->lm_Children->mlh_Head;

				Data->lg_RowNumber++;

		        while (child = NextObject(&cstate))
        		{
					height = _defheight(child);
					width  =  _defwidth(child);

          			/* if line is filled */
          			if(l + width > lm->lm_Layout.Width )
          			{
            			/* left is the left offset */
            			l = llo;

            			/* rize the top coordinate for the next line */
            			t += maxlineheight + vs;

						maxlineheight = 0;

						Data->lg_RowNumber++;
          			}

					// Update the max height of this line
					maxlineheight = max(maxlineheight, height);

		        	/* set position for this object */
   	      			if (!MUI_Layout(child,l,t,width,height,0))
       	          		return(FALSE);

          			/* left = width + space */
          			l += width + hs;
        		}

	      		/* say MUI our total height */

    	  		lm->lm_Layout.Height = t + vs + ((l == llo) ? 0 : (height+vs));

				set(obj, MUIA_LayGroup_NumberOfRows, Data->lg_RowNumber);

      		}
	      	return(TRUE);
    	}
  }
  return(MUILM_UNKNOWN);
}

SAVEDS ULONG ASM AskLayoutGroupLayoutFunc(REG(a0) struct Hook *h,REG(a2) Object *obj,REG(a1) struct MUI_LayoutMsg *lm)
{
   	/* get my private data from hook structure */
	// h_Data points to an istance data

	struct Data *Data = (struct Data*)h->h_Data;

	Object *cstate = (Object *)lm->lm_Children->mlh_Head;
	Object *child;

	struct MUI_LayGroup_Layout al;

	#define hs  Data->lg_HSpace
	#define vs  Data->lg_VSpace
    #define lo  Data->lg_LOff
    #define to  Data->lg_TOff

	switch (lm->lm_Type)
  	{
    	case MUILM_MINMAX:
    	{
			// We are loading the preferences here since there isn't a better place
            // to do it. :-(

			LoadPrefs(obj, Data);

	      	/* find out biggest widths & heights of our children */

      		while (child = NextObject(&cstate))
   	  		{
				al.lgl_Object = child;
				DoMethod(obj, MUIM_LayGroup_AskLayout, &al);

       			if(Data->lg_MaxWidth  < MUI_MAXMAX && al.lgl_Width > Data->lg_MaxWidth)
					Data->lg_MaxWidth  = al.lgl_Width;
				
				if(Data->lg_MaxHeight < MUI_MAXMAX && al.lgl_Height > Data->lg_MaxHeight)
					Data->lg_MaxHeight = al.lgl_Height;

   			}

      		/* set our dimensions */

      		lm->lm_MinMax.MinWidth  = lo + (hs * 2) + Data->lg_MaxWidth; 	// lo + hs + Data->lg_MaxWidth  + hs;
      		lm->lm_MinMax.MinHeight = to + (vs * 2) + Data->lg_MaxHeight;	// to + vs + Data->lg_MaxHeight + vs;

      		lm->lm_MinMax.DefWidth  = (lm->lm_MinMax.MinWidth)  + Data->lg_MaxWidth  + hs;
      		lm->lm_MinMax.DefHeight = (lm->lm_MinMax.MinHeight) + Data->lg_MaxHeight + vs;

      		lm->lm_MinMax.MaxWidth  = MUI_MAXMAX;
      		lm->lm_MinMax.MaxHeight = MUI_MAXMAX;

      		return(0);
    	}

    	case MUILM_LAYOUT:
    	{
	        LONG llo, lto,l,t,wc;
		    UWORD width, height;
			UWORD maxlineheight = 0;
            
            llo = Data->lg_LOff;
            lto = Data->lg_TOff;

			Data->lg_ColNumber = Data->lg_RowNumber = 0;

      		if( child = NextObject(&cstate))
      		{
				al.lgl_Object = child;
				DoMethod(obj, MUIM_LayGroup_AskLayout, &al);

        		width  = al.lgl_Width;
        		height = al.lgl_Height;

        		/* how many childes match into one line */
        		wc = (lm->lm_Layout.Width)/(width+hs);

        		/* calc left offset to center the line into the group */
        		if(llo == -1) llo = (lm->lm_Layout.Width - (width*wc) - (hs*(wc-1))) / 2;
				if(lto == -1) lto = vs;

        		/* left is left offset */
        		l  = llo;

        		/* top is top offset space */
        		t  = lto;

        		/* set for all child there position */
        		cstate = (Object *)lm->lm_Children->mlh_Head;

				Data->lg_RowNumber++;

		        while (child = NextObject(&cstate))
        		{
					al.lgl_Object = child;
					DoMethod(obj, MUIM_LayGroup_AskLayout, &al);

	        		width  = al.lgl_Width;
    	    		height = al.lgl_Height;

          			/* if line is filled */
          			if(l + width > lm->lm_Layout.Width )
          			{
            			/* left is the left offset */
            			l = llo;

            			/* rize the top coordinate for the next line */
            			t += maxlineheight + vs;

						maxlineheight = 0;

						Data->lg_RowNumber++;
          			}

					// Update the max height of this line
					maxlineheight = max(maxlineheight, height);

		        	/* set position for this object */
   	      			if (!MUI_Layout(child,l,t,width,height,0))
       	          		return(FALSE);

          			/* left = width + space */
          			l += width + hs;
        		}

	      		/* say MUI our total height */

    	  		lm->lm_Layout.Height = t + vs + ((l == llo) ? 0 : (height+vs));

				set(obj, MUIA_LayGroup_NumberOfRows, Data->lg_RowNumber);
			}
	      	return(TRUE);
    	}
  }
  return(MUILM_UNKNOWN);
}

SAVEDS ULONG ASM SameSizeGroupLayoutFunc(REG(a0) struct Hook *h,REG(a2) Object *obj,REG(a1) struct MUI_LayoutMsg *lm)
{
   	/* get my private data from hook structure */
	// h_Data points to an istance data

	struct Data *Data = (struct Data*)h->h_Data;

	Object *cstate = (Object *)lm->lm_Children->mlh_Head;
	Object *child;

	#define hs  Data->lg_HSpace
	#define vs  Data->lg_VSpace
    #define lo  Data->lg_LOff
    #define to  Data->lg_TOff

	switch (lm->lm_Type)
  	{
    	case MUILM_MINMAX:
    	{
			// We are loading some of the preferences here since there isn't a better place
            // to do it. :-( (MUIM_Setup is called only after this routine)
		
			LoadPrefs(obj, Data);

	      	/* find out biggest widths & heights of our children */
			// Since thery're all the same, we use the values from the first one.

      		if(child = NextObject(&cstate))
  	  		{
				Data->lg_MaxWidth  = _defwidth (child);
				Data->lg_MaxHeight = _defheight(child);
   			}

      		/* set our dimensions */

      		lm->lm_MinMax.MinWidth  = lo + (hs * 2) + Data->lg_MaxWidth; 	// lo + hs + Data->lg_MaxWidth  + hs;
      		lm->lm_MinMax.MinHeight = to + (vs * 2) + Data->lg_MaxHeight;	// to + vs + Data->lg_MaxHeight + vs;

      		lm->lm_MinMax.DefWidth  = (lm->lm_MinMax.MinWidth)  + Data->lg_MaxWidth  + hs;
      		lm->lm_MinMax.DefHeight = (lm->lm_MinMax.MinHeight) + Data->lg_MaxHeight + vs;

      		lm->lm_MinMax.MaxWidth  = MUI_MAXMAX;
      		lm->lm_MinMax.MaxHeight = MUI_MAXMAX;

      		return(0);
    	}

    	case MUILM_LAYOUT:
    	{
	        LONG llo, lto,l,t,wc;
		    UWORD width, height, mwidth, mheight;
            
            llo = Data->lg_LOff;
            lto = Data->lg_TOff;

			Data->lg_ColNumber = Data->lg_RowNumber = 0;

      		if(child = NextObject(&cstate))
      		{
        		width = _defwidth(child);
        		height= _defheight(child);

				mwidth  = width + hs;
				mheight = height + vs;

        		/* how many childes match into one line */
        		wc = (lm->lm_Layout.Width)/(width+hs);

        		/* calc left offset to center the line into the group */ // correct this
        		if(llo == -1) llo = ((lm->lm_Layout.Width - (width*wc) - (hs*(wc-1))) / 2);
				if(lto == -1) lto = vs;

        		/* left is left offset */
        		l  = llo;

        		/* top is top offset space */
        		t  = lto;

        		/* set for all child there position */
        		cstate = (Object *)lm->lm_Children->mlh_Head;

				Data->lg_RowNumber++;

		        while (child = NextObject(&cstate))
        		{
          			/* if line is filled */
          			if(l + width/* + hs */> lm->lm_Layout.Width )
          			{
            			/* left is the left offset */
            			l = llo;

            			/* rize the top coordinate for the next line */
            			t += mheight;

						Data->lg_RowNumber++;

          			}

		        	/* set position for this object */
   	      			if (!MUI_Layout(child,l,t,width,height,0))
       	          		return(FALSE);

          			/* left = width + space */
          			l += mwidth;
        		}

	      		/* say MUI our total height */

    	  		lm->lm_Layout.Height = t + vs + ((l == llo) ? 0 : (mheight));

				set(obj, MUIA_LayGroup_NumberOfRows, Data->lg_RowNumber);
      		}
	      	return(TRUE);
    	}
  }
  return(MUILM_UNKNOWN);
}
