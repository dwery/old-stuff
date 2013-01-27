struct PrefsSemaphore
{
	struct SignalSemaphore ps_ss;

	struct MinList	ps_ObjList;

	struct MinList	ps_AppList;

	BOOL	ps_NumericEnabled;
	LONG	ps_NumericSkip,
			ps_NumericQualSkip;

	BOOL	ps_CycleEnabled;
	LONG	ps_CycleSkip,
			ps_CycleQualSkip;

	BOOL	ps_PropEnabled;
	LONG	ps_PropSkip,
			ps_PropQualSkip;

	BOOL	ps_ListEnabled;
	LONG	ps_ListSkip,
			ps_ListQualSkip;

	BOOL	ps_VirtgroupEnabled;
	LONG	ps_VirtgroupSkip,
			ps_VirtgroupQualSkip;

	char 	*ps_Name;

	IX		ps_ix_up,
			ps_ix_down;


};
