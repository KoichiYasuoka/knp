/*====================================================================

			     jumanrc 関連

                                               S.Kurohashi 1999.11.13

    $Id$
====================================================================*/
#include "knp.h"

extern char Jumangram_Dirname[];
extern int LineNoForError, LineNo;
char *Knprule_Dirname = NULL;
char *Knpdict_Dirname = NULL;

RuleVector *RULE = NULL;
int CurrentRuleNum = 0;
int RuleNumMax = 0;

char *DICT[DICT_MAX];
int knp_dict_file_already_defined = 0;

/*==================================================================*/
			void init_configfile()
/*==================================================================*/
{
    int i;
    for (i = 0; i < DICT_MAX; i++) {
	DICT[i] = NULL;
    }
}

/*==================================================================*/
	    void check_duplicated(int value, char *string)
/*==================================================================*/
{
    /* 値が 0 でないときはエラー */
    if (value) {
	fprintf(stderr, "%s is duplicately specified in .jumanrc\n", string);
	exit(0);	
    }
}

/*==================================================================*/
		   void clear_rule_configuration()
/*==================================================================*/
{
    if (CurrentRuleNum) {
	free(RULE);
	RULE = NULL;
	CurrentRuleNum = 0;
	RuleNumMax = 0;
    }
    if (Knprule_Dirname) {
	free(Knprule_Dirname);
	Knprule_Dirname = NULL;
    }
    if (Knpdict_Dirname) {
	free(Knpdict_Dirname);
	Knpdict_Dirname = NULL;
    }
}

/*==================================================================*/
		    char *check_tilde(char *file)
/*==================================================================*/
{
    char *home, *ret;

    if (*file == '~' && (home = getenv("HOME"))) {
	ret = (char *)malloc_data(strlen(home)+strlen(file), "check_tilde");
	sprintf(ret, "%s%s", home, strchr(file, '/'));
    }
    else {
	ret = strdup(file);
    }
    return ret;
}

/*==================================================================*/
			void read_rc(FILE *in)
/*==================================================================*/
{
#ifdef  _WIN32
    char buf[FILENAME_MAX];
    /* MS Windows の場合は、juman.ini, knp.ini を見に行く
       dicfile == gramfile */
    GetPrivateProfileString("juman", "dicfile", "", Jumangram_Dirname, FILENAME_MAX, "juman.ini");
    GetPrivateProfileString("knp", "ruledir", "", buf, FILENAME_MAX, "knp.ini");
    if (buf[0]) {
	Knprule_Dirname = strdup(buf);
    }
    GetPrivateProfileString("knp", "dictdir", "", buf, FILENAME_MAX, "knp.ini");
    if (buf[0]) {
	Knpdict_Dirname = strdup(buf);
    }
#else
    CELL *cell1,*cell2;
    char *dicttype;

    LineNo = 0 ;
    Jumangram_Dirname[0] = '\0';

    while (!s_feof(in))  {
	LineNoForError = LineNo;
	cell1 = s_read(in);

	if (!strcmp(DEF_GRAM_FILE, _Atom(car(cell1)))) {
	    if (!Atomp(cell2 = car(cdr(cell1)))) {
		fprintf(stderr, "error in .jumanrc\n");
		exit(0);
	    } else 
		strcpy(Jumangram_Dirname, _Atom(cell2));
	}
	/* KNP ルールディレクトリ */
	else if (!strcmp(DEF_KNP_DIR, _Atom(car(cell1)))) {
	    if (!Atomp(cell2 = car(cdr(cell1)))) {
		fprintf(stderr, "error in .jumanrc\n");
		exit(0);
	    }
	    else
		Knprule_Dirname = check_tilde(_Atom(cell2));
	}
	/* KNP ルールファイル */
	else if (!strcmp(DEF_KNP_FILE, _Atom(car(cell1)))) {
	    cell1 = cdr(cell1);

	    while (!Null(car(cell1))) {
		if (CurrentRuleNum >= RuleNumMax) {
		    RuleNumMax += RuleIncrementStep;
		    RULE = (RuleVector *)realloc_data(RULE, sizeof(RuleVector)*RuleNumMax, "read_rc");
		}

		/* デフォルト値設定 */
		(RULE+CurrentRuleNum)->file = (char *)strdup(_Atom(car(car(cell1))));
		(RULE+CurrentRuleNum)->mode = RLOOP_MRM;
		(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_NONE;
		(RULE+CurrentRuleNum)->type = 0;
		(RULE+CurrentRuleNum)->direction = 0;

		cell2 = cdr(car(cell1));

		while (!Null(car(cell2))) {
		    if (!strcmp(_Atom(car(cell2)), "同形異義語")) {
			check_duplicated((RULE+CurrentRuleNum)->type, "Rule type");
			(RULE+CurrentRuleNum)->type = HomoRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "形態素")) {
			check_duplicated((RULE+CurrentRuleNum)->type, "Rule type");
			(RULE+CurrentRuleNum)->type = MorphRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "文節")) {
			check_duplicated((RULE+CurrentRuleNum)->type, "Rule type");
			(RULE+CurrentRuleNum)->type = BnstRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "係り受け")) {
			check_duplicated((RULE+CurrentRuleNum)->type, "Rule type");
			(RULE+CurrentRuleNum)->type = DpndRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "呼応")) {
			check_duplicated((RULE+CurrentRuleNum)->type, "Rule type");
			(RULE+CurrentRuleNum)->type = KoouRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "固有表現形態素")) {
			check_duplicated((RULE+CurrentRuleNum)->type, "Rule type");
			(RULE+CurrentRuleNum)->type = NeMorphRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "固有表現句-PRE")) {
			check_duplicated((RULE+CurrentRuleNum)->type, "Rule type");
			(RULE+CurrentRuleNum)->type = NePhrasePreRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "固有表現句")) {
			check_duplicated((RULE+CurrentRuleNum)->type, "Rule type");
			(RULE+CurrentRuleNum)->type = NePhraseRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "固有表現句-AUX")) {
			check_duplicated((RULE+CurrentRuleNum)->type, "Rule type");
			(RULE+CurrentRuleNum)->type = NePhraseAuxRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "文脈")) {
			check_duplicated((RULE+CurrentRuleNum)->type, "Rule type");
			(RULE+CurrentRuleNum)->type = ContextRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "ルールループ先行")) {
			(RULE+CurrentRuleNum)->mode = RLOOP_RMM;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "BREAK")) {
			/* RLOOP_BREAK_NONE は 0 なのでひっかからない */
			check_duplicated((RULE+CurrentRuleNum)->breakmode, "Break mode");
			(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_NORMAL;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "BREAKJUMP")) {
			/* RLOOP_BREAK_NONE は 0 なのでひっかからない */
			check_duplicated((RULE+CurrentRuleNum)->breakmode, "Break mode");
			(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_JUMP;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "順方向")) {
			check_duplicated((RULE+CurrentRuleNum)->direction, "Direction");
			(RULE+CurrentRuleNum)->direction = LtoR;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "逆方向")) {
			check_duplicated((RULE+CurrentRuleNum)->direction, "Direction");
			(RULE+CurrentRuleNum)->direction = RtoL;
		    }
		    else {
			fprintf(stderr, "%s is invalid in .jumanrc\n", _Atom(car(cell2)));
			exit(0);
		    }
		    cell2 = cdr(cell2);
		}

		/* ルールのタイプが指定されていないとき */
		if (!(RULE+CurrentRuleNum)->type) {
		    fprintf(stderr, "Rule type for \'%s\' is not specified in .jumanrc\n", 
			    (RULE+CurrentRuleNum)->file);
		    exit(0);
		}

		/* デフォルトの方向 */
		if (!(RULE+CurrentRuleNum)->direction)
		    (RULE+CurrentRuleNum)->direction = LtoR;

		CurrentRuleNum++;
		cell1 = cdr(cell1);
	    }
	}
	/* KNP 辞書ディレクトリ */
	else if (!strcmp(DEF_KNP_DICT_DIR, _Atom(car(cell1)))) {
	    if (!Atomp(cell2 = car(cdr(cell1)))) {
		fprintf(stderr, "error in .jumanrc\n");
		exit(0);
	    }
	    else
		Knpdict_Dirname = check_tilde(_Atom(cell2));
	}
	/* KNP 辞書ファイル */
	else if (!strcmp(DEF_KNP_DICT_FILE, _Atom(car(cell1))) && 
		 !knp_dict_file_already_defined) {
	    cell1 = cdr(cell1);
	    knp_dict_file_already_defined = 1;

	    while (!Null(car(cell1))) {
		dicttype = _Atom(car(cdr(car(cell1))));
		if (!strcmp(dicttype, "格フレームINDEXDB")) {
		    DICT[CF_INDEX_DB] = strdup(_Atom(car(car(cell1))));
		}
		else if (!strcmp(dicttype, "格フレームDATA")) {
		    DICT[CF_DATA] = strdup(_Atom(car(car(cell1))));
		}
		else if (!strcmp(dicttype, "分類語彙表DB")) {
		    DICT[BGH_DB] = strdup(_Atom(car(car(cell1))));
		}
		else if (!strcmp(dicttype, "表層格DB")) {
		    DICT[SCASE_DB] = strdup(_Atom(car(car(cell1))));
		}
		else if (!strcmp(dicttype, "NTT単語DB")) {
		    DICT[SM_DB] = strdup(_Atom(car(car(cell1))));
		}
		else if (!strcmp(dicttype, "NTT意味素DB")) {
		    DICT[SM2CODE_DB] = strdup(_Atom(car(car(cell1))));
		}
		else if (!strcmp(dicttype, "NTT固有名詞変換テーブルDB")) {
		    DICT[SMP2SMG_DB] = strdup(_Atom(car(car(cell1))));
		}
		else {
		    fprintf(stderr, "%s is invalid in .jumanrc\n", _Atom(car(cdr(car(cell1)))));
		    exit(0);
		}
		cell1 = cdr(cell1);
	    }
	}
	else if (!strcmp(DEF_CASE_THESAURUS, _Atom(car(cell1)))) {
	    if (!Atomp(cell2 = car(cdr(cell1)))) {
		fprintf(stderr, "error in .jumanrc\n");
		exit(0);
	    }
	    else {
		if (!strcasecmp(_Atom(cell2), "ntt")) {
		    Thesaurus = USE_NTT;
		    if (OptDisplay == OPT_DEBUG) {
			fprintf(Outfp, "Thesaurus for case analysis ... NTT\n");
		    }
		}
		else if (!strcasecmp(_Atom(cell2), "bgh")) {
		    Thesaurus = USE_BGH;
		    if (OptDisplay == OPT_DEBUG) {
			fprintf(Outfp, "Thesaurus for case analysis ... BGH\n");
		    }
		}
		else {
		    fprintf(stderr, "%s is invalid in .jumanrc\n", _Atom(cell2));
		    exit(0);
		}
	    }
	}
	else if (!strcmp(DEF_PARA_THESAURUS, _Atom(car(cell1)))) {
	    if (!Atomp(cell2 = car(cdr(cell1)))) {
		fprintf(stderr, "error in .jumanrc\n");
		exit(0);
	    }
	    else {
		if (!strcasecmp(_Atom(cell2), "ntt")) {
		    ParaThesaurus = USE_NTT;
		    if (OptDisplay == OPT_DEBUG) {
			fprintf(Outfp, "Thesaurus for para analysis ... NTT\n");
		    }
		}
		else if (!strcasecmp(_Atom(cell2), "bgh")) {
		    ParaThesaurus = USE_BGH;
		    if (OptDisplay == OPT_DEBUG) {
			fprintf(Outfp, "Thesaurus for para analysis ... BGH\n");
		    }
		}
		else if (!strcasecmp(_Atom(cell2), "none")) {
		    ParaThesaurus = USE_NONE;
		    if (OptDisplay == OPT_DEBUG) {
			fprintf(Outfp, "Thesaurus for para analysis ... NONE\n");
		    }
		}
		else {
		    fprintf(stderr, "%s is invalid in .jumanrc\n", _Atom(cell2));
		    exit(0);
		}
	    }
	}
    }
#endif

    /* jumanrc にルールが指定されていない場合のデフォルトルール */
    if (CurrentRuleNum == 0) {
	if (OptDisplay == OPT_DEBUG) {
	    fprintf(Outfp, "Setting default rules ... ");
	}

	RuleNumMax = 8;
	RULE = (RuleVector *)realloc_data(RULE, sizeof(RuleVector)*RuleNumMax, "read_rc");

	/* mrph_homo 同形異義語 */
	(RULE+CurrentRuleNum)->file = strdup("mrph_homo");
	(RULE+CurrentRuleNum)->mode = RLOOP_MRM;
	(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_NONE;
	(RULE+CurrentRuleNum)->type = HomoRuleType;
	(RULE+CurrentRuleNum)->direction = LtoR;
	CurrentRuleNum++;

	/* mrph_basic 形態素 ルールループ先行 */
	(RULE+CurrentRuleNum)->file = strdup("mrph_basic");
	(RULE+CurrentRuleNum)->mode = RLOOP_RMM;
	(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_NONE;
	(RULE+CurrentRuleNum)->type = MorphRuleType;
	(RULE+CurrentRuleNum)->direction = LtoR;
	CurrentRuleNum++;

	/* bnst_basic 文節 逆方向 ルールループ先行 */
	(RULE+CurrentRuleNum)->file = strdup("bnst_basic");
	(RULE+CurrentRuleNum)->mode = RLOOP_RMM;
	(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_NONE;
	(RULE+CurrentRuleNum)->type = BnstRuleType;
	(RULE+CurrentRuleNum)->direction = RtoL;
	CurrentRuleNum++;

	/* bnst_type 文節 逆方向 BREAK */
	(RULE+CurrentRuleNum)->file = strdup("bnst_type");
	(RULE+CurrentRuleNum)->mode = RLOOP_MRM;
	(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_NORMAL;
	(RULE+CurrentRuleNum)->type = BnstRuleType;
	(RULE+CurrentRuleNum)->direction = RtoL;
	CurrentRuleNum++;

	/* bnst_etc 文節 逆方向 ルールループ先行 */
	(RULE+CurrentRuleNum)->file = strdup("bnst_etc");
	(RULE+CurrentRuleNum)->mode = RLOOP_RMM;
	(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_NONE;
	(RULE+CurrentRuleNum)->type = BnstRuleType;
	(RULE+CurrentRuleNum)->direction = RtoL;
	CurrentRuleNum++;

	/* kakari_uke 係り受け */
	(RULE+CurrentRuleNum)->file = strdup("kakari_uke");
	(RULE+CurrentRuleNum)->mode = RLOOP_MRM;
	(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_NONE;
	(RULE+CurrentRuleNum)->type = DpndRuleType;
	(RULE+CurrentRuleNum)->direction = LtoR;
	CurrentRuleNum++;

	/* koou 呼応 */
	(RULE+CurrentRuleNum)->file = strdup("koou");
	(RULE+CurrentRuleNum)->mode = RLOOP_MRM;
	(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_NONE;
	(RULE+CurrentRuleNum)->type = KoouRuleType;
	(RULE+CurrentRuleNum)->direction = LtoR;
	CurrentRuleNum++;

	/* ne-juman 固有表現句 */
	(RULE+CurrentRuleNum)->file = strdup("ne-juman");
	(RULE+CurrentRuleNum)->mode = RLOOP_MRM;
	(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_NONE;
	(RULE+CurrentRuleNum)->type = NePhraseRuleType;
	(RULE+CurrentRuleNum)->direction = LtoR;
	CurrentRuleNum++;

	if (OptDisplay == OPT_DEBUG) {
	    fprintf(Outfp, "done.\n");
	}
    }
}

/*==================================================================*/
		    void server_read_rc(FILE *fp)
/*==================================================================*/
{
    clear_rule_configuration();
    set_cha_getc();
    read_rc(fp);
    unset_cha_getc();
}

/*==================================================================*/
     void check_data_newer_than_rule(time_t data, char *datapath)
/*==================================================================*/
{
    /* ルールファイルとの時間チェック */

    char *rulename;
    int status;
    struct stat sb;

    rulename = strdup(datapath);
    *(rulename+strlen(rulename)-5) = '\0';
    strcat(rulename, ".rule");
    status = stat(rulename, &sb);
    if (!status) {
	if (data < sb.st_mtime) {
	    fprintf(stderr, "%s: older than rule file!\n", datapath);
	}
    }
    free(rulename);
}

/*==================================================================*/
		char *check_rule_filename(char *file)
/*==================================================================*/
{
    /* ルールファイル (*.data) の fullpath を返す関数 */

    char *fullname, *home;
    int status;
    struct stat sb;

    if (!Knprule_Dirname) {
#ifdef KNP_RULE
	Knprule_Dirname = strdup(KNP_RULE);
#else
	fprintf(stderr, "Please specify rule directory in .jumanrc\n");
	exit(0);
#endif
    }

    fullname = (char *)malloc_data(strlen(Knprule_Dirname)+strlen(file)+7, "check_rule_filename");
    sprintf(fullname, "%s/%s.data", Knprule_Dirname, file);
    /* dir + filename + ".data" */
    status = stat(fullname, &sb);

    if (status < 0) {
	*(fullname+strlen(fullname)-5) = '\0';
	/* dir + filename */
	status = stat(fullname, &sb);
	if (status < 0) {
	    /* filename + ".data" */
	    if (*file == '~' && (home = getenv("HOME"))) {
		free(fullname);
		fullname = (char *)malloc_data(strlen(home)+strlen(file), "check_dict_filename");
		sprintf(fullname, "%s%s.data", home, strchr(file, '/'));
	    }
	    else {
		sprintf(fullname, "%s.data", file);
	    }
	    status = stat(fullname, &sb);
	    if (status < 0) {
		*(fullname+strlen(fullname)-5) = '\0';
		/* filename */
		status = stat(fullname, &sb);
		if (status < 0) {
		    fprintf(stderr, "%s: No such file.\n", fullname);
		    exit(1);
		}
	    }
	}
    }

    /* ルールファイルとの時間チェック */
    check_data_newer_than_rule(sb.st_mtime, fullname);

    return fullname;
}

/*==================================================================*/
	   char *check_dict_filename(char *file, int flag)
/*==================================================================*/
{
    char *fullname, *home;
    int status;
    struct stat sb;

    if (!Knpdict_Dirname) {
#ifdef KNP_DICT
	Knpdict_Dirname = strdup(KNP_DICT);
#else
	fprintf(stderr, "Please specify dict directory in .jumanrc\n");
	exit(0);
#endif
    }

    fullname = (char *)malloc_data(strlen(Knpdict_Dirname)+strlen(file)+2, "check_dict_filename");
    sprintf(fullname, "%s/%s", Knpdict_Dirname, file);

    /* dir + filename */
    status = stat(fullname, &sb);

    if (status < 0) {
	free(fullname);
	if (*file == '~' && (home = getenv("HOME"))) {
	    fullname = (char *)malloc_data(strlen(home)+strlen(file), "check_dict_filename");
	    sprintf(fullname, "%s%s", home, strchr(file, '/'));
	}
	else {
	    fullname = strdup(file);
	}
	status = stat(fullname, &sb);
	if (status < 0) {
	    /* flag が FALSE のときはファイルが存在するかどうかチェックしない */
	    if (flag == FALSE) {
		return fullname;
	    }
	    fprintf(stderr, "%s: No such file.\n", fullname);
	    exit(1);
	}
    }
    return fullname;
}

/*====================================================================
				 END
====================================================================*/
