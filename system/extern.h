/*====================================================================

				EXTREN

                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
extern int		Process_type;
extern int		Mrph_num;
extern int		Mrph_all_num;
extern int		Bnst_num;
extern int		New_Bnst_num;
extern int		Para_num;
extern int		Para_M_num;
extern int		Revised_para_num;

extern int		Sen_num;
extern char 		Comment[];
extern char		SID[];
extern char		*ErrorComment;
extern char 		PM_Memo[];

extern char  		key_str[], cont_str[];
extern datum 		key, content;

extern int		IPALExist;
extern int		SMExist;
extern int		SM2CODEExist;
extern int		SMP2SMGExist;
DBM_FILE		sm_db;
DBM_FILE		sm2code_db;
DBM_FILE		smp2smg_db;

extern MRPH_DATA 	mrph_data[];
extern BNST_DATA 	bnst_data[];
extern PARA_DATA	para_data[];
extern PARA_MANAGER	para_manager[];
extern NamedEntity	NE_data[];

extern SENTENCE_DATA	sentence_data[];

extern int 		match_matrix[][BNST_MAX];
extern int 		path_matrix[][BNST_MAX];
extern int		restrict_matrix[][BNST_MAX];
extern int		restrict_table[];
extern int 		Dpnd_matrix[][BNST_MAX];
extern int 		Quote_matrix[][BNST_MAX];
extern int 		Mask_matrix[][BNST_MAX];

extern char		G_Feature[][64];

extern TOTAL_MGR	Best_mgr;
extern TOTAL_MGR	Op_Best_mgr;

extern int 		OptAnalysis;
extern int 		OptExpress;
extern int 		OptDisplay;
extern int 		OptExpandP;
extern int 		OptInhibit;
extern int		OptCheck;
extern int		OptNE;
extern int		OptHelpsys;
extern int		OptLearn;
extern int		OptCFMode;
extern char		OptIgnoreChar;
extern char		*OptOptionalCase;

extern enum OPTION1	Option1;
extern enum OPTION2	Option2;
extern enum OPTION3	Option3;

extern CLASS    	Class[CLASSIFY_NO + 1][CLASSIFY_NO + 1];
extern TYPE     	Type[TYPE_NO];
extern FORM     	Form[TYPE_NO][FORM_NO];
extern int 		CLASS_num;

extern HomoRule 	HomoRuleArray[];
extern int 		CurHomoRuleSize;
extern MrphRule 	MrphRuleArray[];
extern int 		CurMrphRuleSize;

extern BnstRule	 	BnstRule1Array[];
extern int 		CurBnstRule1Size;
extern BnstRule	 	BnstRule2Array[];
extern int 		CurBnstRule2Size;
extern BnstRule	 	BnstRule3Array[];
extern int 		CurBnstRule3Size;

extern BnstRule		UkeRuleArray[];
extern int 		CurUkeRuleSize;
extern BnstRule		KakariRuleArray[];
extern int 		CurKakariRuleSize;

extern KoouRule		KoouRuleArray[];
extern int		CurKoouRuleSize;
extern DpndRule		DpndRuleArray[];
extern int		CurDpndRuleSize;

extern MrphRule 	NERuleArray[];
extern int 		CurNERuleSize;

extern MrphRule 	CNpreRuleArray[];
extern int 		CurCNpreRuleSize;

extern MrphRule 	CNRuleArray[];
extern int 		CurCNRuleSize;

extern MrphRule 	CNauxRuleArray[];
extern int 		CurCNauxRuleSize;

extern BnstRule		ContRuleArray[];
extern int 		ContRuleSize;

extern DicForRule	*DicForRuleVArray;
extern int		CurDicForRuleVSize;
extern DicForRule	*DicForRulePArray;
extern int		CurDicForRulePSize;

extern MrphRule         HelpsysArray[];
extern int              CurHelpsysSize;

extern int DicForRuleDBExist;

extern char 		*Case_name[];

/*====================================================================
				 END
====================================================================*/
