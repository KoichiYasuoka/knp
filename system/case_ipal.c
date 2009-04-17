/*====================================================================

		       格構造解析: 格フレーム側

                                               S.Kurohashi 93. 5.31

    $Id$

====================================================================*/
#include "knp.h"
#define INITIAL_SCORE -10000

FILE *cf_fp;
DBM_FILE cf_db;
FILE *cf_noun_fp;
DBM_FILE cf_noun_db;
DBM_FILE cf_sim_db;
DBM_FILE cf_case_db;
DBM_FILE cf_ex_db;
DBM_FILE case_db;
DBM_FILE cfp_db;
DBM_FILE renyou_db;
DBM_FILE adverb_db;
DBM_FILE para_db;
DBM_FILE noun_co_db;
DBM_FILE chi_pa_db;

CASE_FRAME 	*Case_frame_array = NULL; 	/* 格フレーム */
int 	   	Case_frame_num;			/* 格フレーム数 */
int 	   	MAX_Case_frame_num = 0;		/* 最大格フレーム数 */

char *GENERAL_SOTO_WORDS ="展開/てんかい:1／要因/よういん:1／構え/かまえv:1／状況/じょうきょう:1／段階/だんかい:1／傾向/けいこう:1／制度/せいど:1／ケース/けーす:1／声/こえ:1／可能/かのうa+性/せい:1／見込み/みこみv:1／材料/ざいりょう:1／環境/かんきょう:1／必要だ/ひつようだ:1／考え/かんがえv:1／狙い/ねらいv:1／公算/こうさん:1／事実/じじつ:1／予定/よてい:1／効果/こうか:1／意向/いこう:1／政権/せいけん:1／体制/たいせい:1／問題/もんだい:1／懸念/けねん:1／地域/ちいき:1／方法/ほうほう:1／対策/たいさく:1／例/れい:1／事業/じぎょう:1／仕組み/しくみv:1／点/てん:1／作品/さくひん:1／理由/りゆう:1／疑い/うたがいv:1／恰好/かっこう:1／方針/ほうしん:1／様子/ようす:1／姿勢/しせい:1／恐れ/おそれv:1／模様/もよう:1／事件/じけん:1／作業/さぎょう:1／機会/きかい:1／見通し/みとおしv:1／政策/せいさく:1／内容/ないよう:1／措置/そち:1／背景/はいけい:1／面/おもて:1／方向/ほうこう:1／結果/けっか:1／状態/じょうたい:1／方式/ほうしき:1／商品/しょうひん:1／目的/もくてき:1／場面/ばめん:1／事態/じたい:1／動き/うごきv:1／計画/けいかく:1／案/あん:1／の/の:1／ん/ん:1／こと/こと:1／はず/はず:1／わけ/わけ:1／つもり/つもり:1／ところ/ところ:1／為/ため:1／お陰/おかげ:1／せい/せい:1／あまり/あまり:1／通り/とおり:1／様/よう:1／代わり/かわり:1／他/ほか:1／ついで/ついで:1／まま/まま:1／時/とき:1／折り/おり:1／間/あいだ:1／うち/うち:1／後/あと:1／後/あと:1／前/まえ:1／最中/さいちゅう:1／際/さい:1／場合/ばあい:1／途中/とちゅう:1／一方/いっぽう:1／結果/けっか:1／反面/はんめん:1／半面/はんめん:1／限り/かぎり:1／方/ほう:1／くせ/くせ:1／程度/ていど:1／以外/いがい:1／頃/ころ:1／程/ほど:1／上/うえ:1／毎/ごと:1／故/ゆえ:1／くらい/くらい:1／旨/むね:1／途端/とたん:1／矢先/やさき:1／以上/いじょう:1／たび/たび:1";
/* 副詞的名詞「中/なか:1」は保留 */

char *db_buf = NULL;
int db_buf_size = 0;

CF_FRAME CF_frame;
int MAX_cf_frame_length = 0;
unsigned char *cf_str_buf;

int	CFExist;
int	CFNounExist;
int	CFSimExist;
int	CFCaseExist;
int	CFExExist;
int	CaseExist;
int	CfpExist;
int	RenyouExist;
int	AdverbExist;
int	ParaExist;
int	NounCoExist;
int     CHISpecPAExist;
int     CHIPAExist;

int	PrintDeletedSM = 0;
int	SM_AGENT_THRESHOLD = 0.40;

/*==================================================================*/
	   void init_cf_structure(CASE_FRAME *p, int size)
/*==================================================================*/
{
    memset(p, 0, sizeof(CASE_FRAME)*size);
}

/*==================================================================*/
			  void realloc_cf()
/*==================================================================*/
{
    Case_frame_array = (CASE_FRAME *)realloc_data(Case_frame_array, 
						  sizeof(CASE_FRAME)*(MAX_Case_frame_num+ALLOCATION_STEP), 
						  "realloc_cf");
    init_cf_structure(Case_frame_array+MAX_Case_frame_num, ALLOCATION_STEP);
    MAX_Case_frame_num += ALLOCATION_STEP;
}

/*==================================================================*/
			    void init_cf()
/*==================================================================*/
{
    char *index_db_filename, *data_filename;

    if (DICT[CF_DATA]) {
	data_filename = check_dict_filename(DICT[CF_DATA], TRUE);
    }
    else {
	data_filename = check_dict_filename(CF_DAT_NAME, FALSE);
    }

    if (DICT[CF_INDEX_DB]) {
	index_db_filename = check_dict_filename(DICT[CF_INDEX_DB], TRUE);
    }
    else {
	index_db_filename = check_dict_filename(CF_DB_NAME, FALSE);
    }

    if (OptDisplay == OPT_DEBUG) {
	fprintf(Outfp, "Opening %s ... ", data_filename);
    }

    if ((cf_fp = fopen(data_filename, "rb")) == NULL) {
	if (OptDisplay == OPT_DEBUG) {
	    fputs("failed.\n", Outfp);
	}
#ifdef DEBUG
	fprintf(stderr, ";; Cannot open CF DATA <%s>.\n", data_filename);
#endif
	CFExist = FALSE;
    }
    else if ((cf_db = DB_open(index_db_filename, O_RDONLY, 0)) == NULL) {
	if (OptDisplay == OPT_DEBUG) {
	    fprintf(Outfp, "done.\nOpening %s ... failed.\n", index_db_filename);
	}
	fprintf(stderr, ";; Cannot open CF INDEX Database <%s>.\n", index_db_filename);
	/* 格フレーム DATA は読めるのに、DB が読めないときは終わる */
	exit(1);
    } 
    else {
	if (OptDisplay == OPT_DEBUG) {
	    fprintf(Outfp, "done.\nOpening %s ... done.\n", index_db_filename);
	}
	CFExist = TRUE;
    }

    free(data_filename);
    free(index_db_filename);

    /* 格フレーム類似度DB (cfsim.db) */
    cf_sim_db = open_dict(CF_SIM_DB, CF_SIM_DB_NAME, &CFSimExist);

    /* 格確率DB (cfcase.db) */
    cf_case_db = open_dict(CF_CASE_DB, CF_CASE_DB_NAME, &CFCaseExist);

    /* 用例確率DB (cfex.db) *
    cf_ex_db = open_dict(CF_EX_DB, CF_EX_DB_NAME, &CFExExist);
    */
    CFExExist = FALSE;

    /* 格フレーム選択確率DB (cfp.db) */
    cfp_db = open_dict(CFP_DB, CFP_DB_NAME, &CfpExist);

    /* 格解釈確率DB (case.db) */
    case_db = open_dict(CASE_DB, CASE_DB_NAME, &CaseExist);

    /* 連用確率DB (renyou.db) */
    renyou_db = open_dict(RENYOU_DB, RENYOU_DB_NAME, &RenyouExist);

    /* 副詞確率DB (adverb.db) */
    adverb_db = open_dict(ADVERB_DB, ADVERB_DB_NAME, &AdverbExist);

    /* 並列確率DB (para.db) */
    para_db = open_dict(PARA_DB, PARA_DB_NAME, &ParaExist);

    /* 名詞共起確率DB (noun-co.db) */
    noun_co_db = open_dict(NOUN_CO_DB, NOUN_CO_DB_NAME, &NounCoExist);

    if (Language == CHINESE) {
	/* Chinese CHI_PA DB (chi_pa.db) */
	chi_pa_db = open_dict(CHI_PA_DB, CHI_PA_DB_NAME, &CHIPAExist);
    }
}

/*==================================================================*/
			 void init_noun_cf()
/*==================================================================*/
{
    char *index_db_filename, *data_filename;

    if (DICT[CF_NOUN_DATA]) {
	data_filename = check_dict_filename(DICT[CF_NOUN_DATA], TRUE);
    }
    else {
	data_filename = check_dict_filename(CF_NOUN_DAT_NAME, FALSE);
    }

    if (DICT[CF_NOUN_INDEX_DB]) {
	index_db_filename = check_dict_filename(DICT[CF_NOUN_INDEX_DB], TRUE);
    }
    else {
	index_db_filename = check_dict_filename(CF_NOUN_DB_NAME, FALSE);
    }

    if (OptDisplay == OPT_DEBUG) {
	fprintf(Outfp, "Opening %s ... ", data_filename);
    }

    if ((cf_noun_fp = fopen(data_filename, "rb")) == NULL) {
	if (OptDisplay == OPT_DEBUG) {
	    fputs("failed.\n", Outfp);
	}
#ifdef DEBUG
	fprintf(stderr, ";; Cannot open CF(noun) DATA <%s>.\n", data_filename);
#endif
	CFNounExist = FALSE;
    }
    else if ((cf_noun_db = DB_open(index_db_filename, O_RDONLY, 0)) == NULL) {
	if (OptDisplay == OPT_DEBUG) {
	    fprintf(Outfp, "done.\nOpening %s ... failed.\n", index_db_filename);
	}
	fprintf(stderr, ";; Cannot open CF(noun) INDEX Database <%s>.\n", index_db_filename);
	/* 格フレーム DATA は読めるのに、DB が読めないときは終わる */
	exit(1);
    } 
    else {
	if (OptDisplay == OPT_DEBUG) {
	    fprintf(Outfp, "done.\nOpening %s ... done.\n", index_db_filename);
	}
	CFNounExist = TRUE;
    }

    free(data_filename);
    free(index_db_filename);
}

/*==================================================================*/
		 void clear_mgr_cf(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j;

    for (i = 0; i < CPM_MAX; i++) {
	for (j = 0; j < CF_ELEMENT_MAX; j++) {
	    free(sp->Best_mgr->cpm[i].cf.ex[j]);
	    sp->Best_mgr->cpm[i].cf.ex[j] = NULL;

	    free(sp->Best_mgr->cpm[i].cf.sm[j]);
	    sp->Best_mgr->cpm[i].cf.sm[j] = NULL;

	    free(sp->Best_mgr->cpm[i].cf.ex_list[j][0]);
	    free(sp->Best_mgr->cpm[i].cf.ex_list[j]);
	    free(sp->Best_mgr->cpm[i].cf.ex_freq[j]);
	}
    }
}

/*==================================================================*/
		   void init_mgr_cf(TOTAL_MGR *tmp)
/*==================================================================*/
{
    int i;

    for (i = 0; i < CPM_MAX; i++) {
	init_case_frame(&tmp->cpm[i].cf);
    }
}

/*==================================================================*/
	    void init_case_analysis_cpm(SENTENCE_DATA *sp)
/*==================================================================*/
{
    if (OptAnalysis == OPT_CASE || 
	OptAnalysis == OPT_CASE2 ||
	OptUseNCF) {

	/* 格フレーム領域確保 */
	Case_frame_array = (CASE_FRAME *)malloc_data(sizeof(CASE_FRAME)*ALL_CASE_FRAME_MAX, 
						     "init_case_analysis_cpm");
	MAX_Case_frame_num = ALL_CASE_FRAME_MAX;
	init_cf_structure(Case_frame_array, MAX_Case_frame_num);

	/* Best_mgrのcpm領域確保 */
	init_mgr_cf(sp->Best_mgr);

	/* 名詞-意味素HASHの初期化 */
	memset(smlist, 0, sizeof(SMLIST)*TBLSIZE);
    }
}

/*==================================================================*/
			   void close_cf()
/*==================================================================*/
{
    if (CFExist == TRUE) {
	fclose(cf_fp);
	DB_close(cf_db);
    }
}

/*==================================================================*/
			 void close_noun_cf()
/*==================================================================*/
{
    if (CFNounExist == TRUE) {
	fclose(cf_noun_fp);
	DB_close(cf_noun_db);
    }
}

/*==================================================================*/
	char *get_ipal_address(unsigned char *word, int flag)
/*==================================================================*/
{
    if (flag == CF_PRED) {
	if (CFExist == FALSE) {
	    return NULL;
	}

	return db_get(cf_db, word);
    }
    else {
	if (CFNounExist == FALSE) {
	    return NULL;
	}

	return db_get(cf_noun_db, word);
    }
}

/*==================================================================*/
	 void get_cf_alignment(char *str, int cf_aligned_num)
/*==================================================================*/
{
    char *buf, *token, *cp, *strtok_context;
    int count = 0, c1, c2;

    buf = strdup(str);
    token = strtok_r(buf, ";", &strtok_context);
    CF_frame.cf_align[cf_aligned_num].cf_id = strdup(token);
    token = strtok_r(NULL, ";", &strtok_context);
    while (token) {
	if ((cp = strstr(token, "＝")) != NULL) {
	    *cp = '\0';
	    c1 = pp_kstr_to_code(token);
	    c2 = pp_kstr_to_code(cp + strlen("＝"));
	    if (c1 == END_M) { /* c2はNILのときEND_Mになる */
		if (OptDisplay == OPT_DEBUG) {
		    fprintf(stderr, ";; Can't understand <%s> as cf alignment\n", token);
		}
	    }
	    else if (count < CF_ELEMENT_MAX - 1) { /* 溢れチェック */
		CF_frame.cf_align[cf_aligned_num].aligned_case[count][0] = c1;
		CF_frame.cf_align[cf_aligned_num].aligned_case[count][1] = c2;
		count++;
	    }
	}
	token = strtok_r(NULL, ";", &strtok_context);
    }
    free(buf);

    CF_frame.cf_align[cf_aligned_num].aligned_case[count][0] = END_M;
    CF_frame.cf_align[cf_aligned_num].aligned_case[count][1] = END_M;
}

/*==================================================================*/
  CF_FRAME *get_ipal_frame(unsigned int address, int size, int flag)
/*==================================================================*/
{
    int i, c1, c2, count = 0, cf_aligned_num = 0;
    char *cp;
    FILE *fp;

    if (flag == CF_PRED) {
	fp = cf_fp;
    }
    else {
	fp = cf_noun_fp;
    }

    if (size > MAX_cf_frame_length) {
	MAX_cf_frame_length += ALLOCATION_STEP*((size-MAX_cf_frame_length)/ALLOCATION_STEP+1);
	CF_frame.DATA = 
	    (unsigned char *)realloc_data(CF_frame.DATA, 
					  sizeof(unsigned char)*MAX_cf_frame_length, 
					  "get_ipal_frame");	
	cf_str_buf = 
	    (unsigned char *)realloc_data(cf_str_buf, 
					  sizeof(unsigned char)*MAX_cf_frame_length, 
					  "get_ipal_frame");
    }

    fseek(fp, (unsigned long)address, 0);
    if (fread(CF_frame.DATA, size, 1, fp) < 1) {
	fprintf(stderr, ";; Error in fread.\n");
	exit(1);
    }

    /* 読み, 表記, 素性を設定 */
    CF_frame.casenum = 0;
    for (i = 0; i < size-1; i++) {
	if (*(CF_frame.DATA+i) == '\0') {
	    if (count == 0)
		CF_frame.yomi = CF_frame.DATA+i+1;
	    else if (count == 1)
		CF_frame.hyoki = CF_frame.DATA+i+1;
	    else if (count == 2)
		CF_frame.feature = CF_frame.DATA+i+1;
	    else {
		if (count%3 == 0) {
		    CF_frame.cs[count/3-1].kaku_keishiki = CF_frame.DATA+i+1;
		    CF_frame.casenum++;
		    if (CF_frame.casenum > CASE_MAX_NUM) {
			fprintf(stderr, ";; # of cases is more than MAX (%d).\n", CASE_MAX_NUM);
		    }
		}
		else if (count%3 == 1)
		    CF_frame.cs[count/3-1].meishiku = CF_frame.DATA+i+1;
		else if (count%3 == 2)
		    CF_frame.cs[count/3-1].imisosei = CF_frame.DATA+i+1;
	    }
	    count++;
	}
    }

    count = 0;
    CF_frame.voice = 0;
    CF_frame.etcflag = CF_NORMAL;
    if (*CF_frame.feature) {
	char *string, *token, *buf;
	string = strdup(CF_frame.feature);
	token = strtok(string, " ");
	while (token) {
	    if (!strcmp(token, "和フレーム")) {
		CF_frame.etcflag |= CF_SUM;
	    }
	    else if (!strcmp(token, "格フレーム変化")) {
		CF_frame.etcflag |= CF_CHANGE;
	    }
	    else if (!strcmp(token, "ヲ使役")) {
		CF_frame.voice |= CF_CAUSATIVE_WO;
	    }
	    else if (!strcmp(token, "ニ使役")) {
		CF_frame.voice |= CF_CAUSATIVE_NI;
	    }
	    else if (!strcmp(token, "直受1")) {
		CF_frame.voice |= CF_PASSIVE_1;
	    }
	    else if (!strcmp(token, "直受2")) {
		CF_frame.voice |= CF_PASSIVE_2;
	    }
	    else if (!strcmp(token, "間受")) {
		CF_frame.voice |= CF_PASSIVE_I;
	    }
	    else if (!strcmp(token, "可能")) {
		CF_frame.voice |= CF_POSSIBLE;
	    }
	    else if (!strcmp(token, "尊敬")) {
		CF_frame.voice |= CF_POLITE;
	    }
	    else if (!strcmp(token, "自発")) {
		CF_frame.voice |= CF_SPONTANE;
		;
	    }
	    else if ((!strncmp(token, "格フレーム対応:", strlen("格フレーム対応:")))) {
		if (cf_aligned_num < CF_ALIGNMENT_MAX - 1) {
		    get_cf_alignment(token + strlen("格フレーム対応:"), cf_aligned_num++);
		}
	    }
	    /* merged cases */
	    else if ((cp = strstr(token, "＝")) != NULL && flag == CF_PRED) {
		buf = strdup(token);
		cp = buf+(cp-token);
		*cp = '\0';
		/* if (!strncmp(buf+strlen(buf)-2, "格", 2)) *(buf+strlen(buf)-2) = '\0';
		if (!strncmp(cp+strlen(cp+2), "格", 2)) *(cp+strlen(cp+2)) = '\0'; */
		c1 = pp_kstr_to_code(buf);
		c2 = pp_kstr_to_code(cp + strlen("＝"));
		free(buf);

		if (c1 == END_M || c2 == END_M) {
		    if (OptDisplay == OPT_DEBUG) {
			fprintf(stderr, ";; Can't understand <%s> as merged cases\n", token);
		    }
		}
		else if (count < CF_ELEMENT_MAX - 1) { /* 溢れチェック */
		    /* 数が小さい格を前に入れる */
		    if (c1 > c2) {
			CF_frame.samecase[count][0] = c2;
			CF_frame.samecase[count][1] = c1;
		    }
		    else {
			CF_frame.samecase[count][0] = c1;
			CF_frame.samecase[count][1] = c2;
		    }
		    count++;
		}
	    }
	    token = strtok(NULL, " ");
	}
	free(string);
    }

    CF_frame.cf_align[cf_aligned_num].cf_id = NULL;
    CF_frame.cf_align[cf_aligned_num].aligned_case[0][0] = END_M;
    CF_frame.cf_align[cf_aligned_num].aligned_case[0][1] = END_M;
    CF_frame.samecase[count][0] = END_M;
    CF_frame.samecase[count][1] = END_M;

    if (cp = strchr(CF_frame.DATA, ':')) {
	strncpy(CF_frame.pred_type, cp+1, 2);
	CF_frame.pred_type[2] = '\0';
    }
    else {
	CF_frame.pred_type[0] = '\0';
    }

    return &CF_frame;
}

/*==================================================================*/
unsigned char *extract_ipal_str(unsigned char *dat, unsigned char *ret, int flag)
/*==================================================================*/
{
    int freq;

    /* flag == TRUE: 頻度付きで返す */

    if (*dat == '\0' || !strcmp(dat, "＊")) {
	return NULL;
    }

    while (1) {
	if (*dat == '\0') {
	    *ret = '\0';
	    return dat;
	}
	/* 頻度が記述してある場合 */
	else if (*dat == ':') {
	    if (flag) {
		*ret++ = *dat;
	    }
	    /* flag == FALSE: 頻度を返さない */
	    else {
		*ret++ = '\0';	/* ':' -> '\0' */
	    }
	    dat++;
	}
	/* 空白でも切る */
	else if (*dat == ' ') {
	    *ret = '\0';
	    return dat+1;
	}
	else if (*dat < 0x80) { /* OK? */
	    *ret++ = *dat++;
	}
	else if (!strncmp(dat, "／", strlen("／")) || 
		 !strncmp(dat, "，", strlen("，")) ||
		 !strncmp(dat, "、", strlen("、")) ||
		 !strncmp(dat, "＊", strlen("＊"))) {
	    *ret = '\0';
	    return dat+strlen("／"); /* OK? */
	}
	else {
	    *ret++ = *dat++;
	    *ret++ = *dat++;
	}
    }
}

/*==================================================================*/
int _make_ipal_cframe_pp(CASE_FRAME *c_ptr, unsigned char *cp, int num, int flag)
/*==================================================================*/
{
    /* 助詞の読みだし */

    unsigned char *point;
    int pp_num = 0;

    /* 直前格 */
    if (*(cp+strlen(cp)-1) == '*') {
	c_ptr->adjacent[num] = TRUE;
	*(cp+strlen(cp)-1) = '\0';
    }
    else if (!strcmp(cp+strlen(cp)-strlen("＠"), "＠")) {
	c_ptr->adjacent[num] = TRUE;
	*(cp+strlen(cp)-strlen("＠")) = '\0';
    }

    /* 任意格 */
    if (!strcmp(cp+strlen(cp)-strlen("＊"), "＊")) {
	c_ptr->oblig[num] = FALSE;
    }
    else {
	c_ptr->oblig[num] = TRUE;
    }

    point = cp; 
    while ((point = extract_ipal_str(point, cf_str_buf, FALSE))) {

	if (flag == CF_PRED) {
	    c_ptr->pp[num][pp_num] = pp_kstr_to_code(cf_str_buf);
	    if (c_ptr->pp[num][pp_num] == END_M) {
		if (OptDisplay == OPT_DEBUG) {
		    fprintf(stderr, ";; Unknown case (%s) in PP!\n", cf_str_buf);
		}
		return FALSE;
	    }
	    c_ptr->pp_str[num] = NULL;
	}
	else {
	    c_ptr->pp[num][pp_num] = 0;
	    c_ptr->pp_str[num] = strdup(cf_str_buf);
	    c_ptr->oblig[num] = TRUE;
	}

	pp_num++;
    }

    c_ptr->pp[num][pp_num] = END_M;
    return TRUE;
}

/*==================================================================*/
	 char *append_str(char **dst, char *src, char *delim)
/*==================================================================*/
{
    if (src && *src) {
	if (*dst == NULL) {
	    *dst = strdup(src);
	}
	else {
	    *dst = (char *)realloc_data(*dst, strlen(*dst) + (delim ? strlen(delim) : 0) + strlen(src) + 1, "append_str");
	    if (delim) {
		strcat(*dst, delim);
	    }
	    strcat(*dst, src);
	}
    }

    return *dst;
}

/*==================================================================*/
void _make_ipal_cframe_sm(CASE_FRAME *c_ptr, unsigned char *cp, int num, int flag)
/*==================================================================*/
{
    /* 意味マーカの読みだし */

    unsigned char *point;
    int size, sm_num = 0, sm_print_num = 0, mlength, sm_delete_sm_max = 0, sm_specify_sm_max = 0;
    char buf[SM_ELEMENT_MAX * SM_CODE_SIZE], *sm_delete_sm = NULL, *sm_specify_sm = NULL, *temp, *str;

    if (*cp == '\0') {
	return;
    }

    if (flag & USE_BGH) {
	size = BGH_CODE_SIZE;
    }
    else if (flag & USE_NTT) {
	size = SM_CODE_SIZE;
    }
    else {
	return;
    }

    str = strdup(cp);
    *str = '\0';
    point = cp;
    buf[0] = '\0';
    while ((point = extract_ipal_str(point, cf_str_buf, FALSE))) {
	/* 意味素制限 */
        if (cf_str_buf[0] == '+') {
	    if (c_ptr->sm_specify[num] == NULL) {
		c_ptr->sm_specify_size[num] = SM_ELEMENT_MAX;
		c_ptr->sm_specify[num] = (char *)malloc_data(sizeof(char)*c_ptr->sm_specify_size[num] * size + 1, "_make_ipal_cframe_sm");
		*c_ptr->sm_specify[num] = '\0';

		if (flag & USE_NTT) {
		    sm_specify_sm_max = sizeof(char) * ALLOCATION_STEP;
		    sm_specify_sm = (char *)malloc_data(sm_specify_sm_max, "_make_ipal_cframe_sm");
		    strcpy(sm_specify_sm, "意味素制限:");
		}
		else {
		    sm_specify_sm = strdup("意味素制限");
		}
	    }
	    else if (c_ptr->sm_specify_num[num] >= c_ptr->sm_specify_size[num]) {
		c_ptr->sm_specify[num] = (char *)realloc_data(c_ptr->sm_specify[num], 
							      sizeof(char)*(c_ptr->sm_specify_size[num] <<= 1) * size + 1, "_make_ipal_cframe_sm");
	    }

	    /* codeが書いてあるとき */
	    if (cf_str_buf[1] == '1') {
		strcat(c_ptr->sm_specify[num], &cf_str_buf[1]);

		if (flag & USE_NTT) { /* 表示用の意味素名への変換 (NTTのみ) */
		    temp = code2sm(&cf_str_buf[1]);
		    if (temp[0]) {
			/* -1 ではないのは '/' の分 */
			if (strlen(sm_specify_sm) + strlen(temp) > sm_specify_sm_max - 2) {
			    sm_specify_sm = (char *)realloc_data(sm_specify_sm, sm_specify_sm_max <<= 1, "_make_ipal_cframe_sm");
			}
			strcat(sm_specify_sm, "/");
			strcat(sm_specify_sm, temp);
		    }
		}
	    }
	    else if (flag & USE_NTT) { /* 意味素名での指定 (NTTのみ) */
		strcat(c_ptr->sm_specify[num], sm2code(&cf_str_buf[1]));
	    }
	    c_ptr->sm_specify_num[num]++;
	}
	/* 使ってはいけない意味素 */
        else if (cf_str_buf[0] == '-') {
	    if (c_ptr->sm_delete[num] == NULL) {
		c_ptr->sm_delete_size[num] = SM_ELEMENT_MAX;
		c_ptr->sm_delete[num] = (char *)malloc_data(sizeof(char)*c_ptr->sm_delete_size[num] * size + 1, "_make_ipal_cframe_sm");
		*c_ptr->sm_delete[num] = '\0';

		if (PrintDeletedSM && (flag & USE_NTT)) {
		    sm_delete_sm_max = sizeof(char) * ALLOCATION_STEP;
		    sm_delete_sm = (char *)malloc_data(sm_delete_sm_max, "_make_ipal_cframe_sm");
		    strcpy(sm_delete_sm, "意味素削除:");
		}
		else {
		    sm_delete_sm = strdup("意味素削除");
		}
	    }
	    else if (c_ptr->sm_delete_num[num] >= c_ptr->sm_delete_size[num]) {
		c_ptr->sm_delete[num] = (char *)realloc_data(c_ptr->sm_delete[num], sizeof(char)*(c_ptr->sm_delete_size[num] <<= 1) * size + 1, 
							     "_make_ipal_cframe_sm");
	    }

	    /* codeが書いてあるとき */
	    if (cf_str_buf[1] == '1') {
		strcat(c_ptr->sm_delete[num], &cf_str_buf[1]);

		if (PrintDeletedSM && (flag & USE_NTT)) { /* 表示用の意味素名への変換 (NTTのみ) */
		    temp = code2sm(&cf_str_buf[1]);
		    if (temp[0]) {
			/* -1 ではないのは '/' の分 */
			if (strlen(sm_delete_sm)+strlen(temp) > sm_delete_sm_max-2) {
			    sm_delete_sm = (char *)realloc_data(sm_delete_sm, sm_delete_sm_max <<= 1, "_make_ipal_cframe_sm");
			}
			strcat(sm_delete_sm, "/");
			strcat(sm_delete_sm, temp);
		    }
		}
	    }
	    else if (flag & USE_NTT) { /* 意味素名での指定 (NTTのみ) */
		strcat(c_ptr->sm_delete[num], sm2code(&cf_str_buf[1]));
	    }
	    c_ptr->sm_delete_num[num]++;
	}
	/* 普通の意味素 */
	else {
	    sm_num++;
	    sm_print_num++;
	    if (sm_num >= SM_ELEMENT_MAX){
		break;
	    }

	    if (!strncmp(cf_str_buf, "数量", strlen("数量"))) {
		/* 前回も<数量>のときは入れない */
		if (sm_num > 1 && !strncmp(&buf[size*(sm_num-2)], sm2code("数量"), size)) {
		    sm_num--;
		}
		else {
		    strcat(buf, sm2code("数量"));
		}
	    }
	    else if (!strncmp(cf_str_buf, "主体準", strlen("主体準"))) {
		strcat(buf, sm2code("主体"));
		if (MatchPP(c_ptr->pp[num][0], "ガ")) { /* 今は、ガ格以外に<主体準>を与えても<主体>と同じになる */
		    c_ptr->etcflag |= CF_GA_SEMI_SUBJECT;
		}
	    }
	    else {
		strcat(buf, sm2code(cf_str_buf));
	    }
 	
	    if ((flag & STOREtoCF) && 
		(EX_PRINT_NUM < 0 || sm_print_num <= EX_PRINT_NUM)) {
		if (str[0])	strcat(str, "/");
		strcat(str, cf_str_buf);
	    }
	}
    }

    if (buf[0]) {
	append_str(&(c_ptr->sm[num]), buf, NULL);
    }

    if (flag & STOREtoCF) {
	append_str(&(c_ptr->semantics[num]), str, "/");
	append_str(&(c_ptr->semantics[num]), sm_delete_sm, "/");
	append_str(&(c_ptr->semantics[num]), sm_specify_sm, "/");
	if (EX_PRINT_NUM >= 0 && sm_print_num > EX_PRINT_NUM) {
	    append_str(&(c_ptr->semantics[num]), "...", "/");
	}
    }
    free(str);
    if (sm_delete_sm) free(sm_delete_sm);
    if (sm_specify_sm) free(sm_specify_sm);
}

/*==================================================================*/
		  int split_freq(unsigned char *cp)
/*==================================================================*/
{
    int freq;

    while (1) {
	if (*cp == ':') {
	    sscanf(cp + 1, "%d", &freq);
	    *cp = '\0';
	    return freq;
	}
	else if (*cp == '\0') {
	    return 1;
	}
	cp++;
    }
    return 0;
}

/*==================================================================*/
	     double split_freq_for_gex(unsigned char *cp)
/*==================================================================*/
{   
    double freq;

    cp += 4; /* gex用 (先頭の"<NE:"、"<TH:", "<CT:"は読み飛ばす */

    while (1) {
	if (*cp == ':') {
	    sscanf(cp + 1, "%lf", &freq);
	    *(cp - 1) = '\0'; /* ">"を除く */
	    return freq;
	}
	else if (*cp == '\0') {
	    return 1;
	}
	cp++;
    }
    return 0;
}

/*==================================================================*/
void _make_ipal_cframe_ex(CASE_FRAME *c_ptr, unsigned char *cp, int num, 
			  int flag, int fflag)
/*==================================================================*/
{
    /* 例の読みだし */

    /* fflag: 頻度1を使うかどうか
              格が外の関係のときだけ使う */

    unsigned char *point, *point2;
    int max, count = 0, thesaurus = USE_NTT, freq, over_flag = 0, agent_count = 0;
    int sub_agent_flag = 0, ex_agent_flag;
    double freq_gex, agent_ratio = -1;
    char *code, **destination, *buf, *token;

    c_ptr->freq[num] = 0;

    if (*cp == '\0') {
	return;
    }

    /* 引くリソースによって関数などをセット */
    destination = &c_ptr->ex[num];
    if (flag & USE_BGH) {
	thesaurus = USE_BGH;
	max = EX_ELEMENT_MAX*BGH_CODE_SIZE;
    }
    else if (flag & USE_NTT) {
	thesaurus = USE_NTT;
	max = SM_ELEMENT_MAX*SM_CODE_SIZE;
    }

    /* 最大値やめないといけません */
    buf = (char *)malloc_data(sizeof(char)*max, "_make_ipal_cframe_ex");

    point = cp;
    *buf = '\0';
    while ((point = extract_ipal_str(point, cf_str_buf, TRUE))) {
	point2 = cf_str_buf;

	/* 用例中に記された汎化素性の読み込み */
	if (!strncmp(point2, "<NE:", 4) || 
	    !strncmp(point2, "<TH:", 4) || 
	    !strncmp(point2, "<CT:", 4)) {

	    /* 頻度の抽出 */
	    freq_gex = split_freq_for_gex(point2);

	    if (!strcmp(point2, "<TH:主体")) { /* <主体>の割合を下で利用 */
		agent_ratio = freq_gex;
	    }

	    if (c_ptr->gex_size[num] == 0) {
		c_ptr->gex_size[num] = 10;	/* 初期確保数 */
		c_ptr->gex_list[num] = (char **)malloc_data(sizeof(char *)*c_ptr->gex_size[num], 
							   "_make_ipal_cframe_ex");
		c_ptr->gex_freq[num] = (double *)malloc_data(sizeof(double)*c_ptr->gex_size[num], 
							 "_make_ipal_cframe_ex");
	    }
	    else if (c_ptr->gex_num[num] >= c_ptr->gex_size[num]) {
		c_ptr->gex_list[num] = (char **)realloc_data(c_ptr->gex_list[num], 
							    sizeof(char *)*(c_ptr->gex_size[num] <<= 1), 
							    "_make_ipal_cframe_ex");
		c_ptr->gex_freq[num] = (double *)realloc_data(c_ptr->gex_freq[num], 
							  sizeof(double)*c_ptr->gex_size[num], 
							  "_make_ipal_cframe_ex");
	    }
	    
	    c_ptr->gex_list[num][c_ptr->gex_num[num]] = strdup(point2 + 1);
	    c_ptr->gex_freq[num][c_ptr->gex_num[num]++] = freq_gex;

	    /* fprintf(stderr, "%s:%.10f freq_gex\n", point2 + 1, freq_gex); */

	    continue;
	}

	/* 頻度の抽出 */
	freq = split_freq(point2);

	if (!strcmp(point2, "<主体準>")) {
	    sub_agent_flag = 1;
	    continue;
	}

	/* fflag == TRUE: 頻度1を削除 */
	if (!(OptCaseFlag & OPT_CASE_USE_PROBABILITY) && 
	    fflag && freq < 2) {
	    continue;
	}

	if (*point2 != '\0') {
	    if (agent_ratio > 0) { /* <TH:主体>があるならば、動的<主体>チェックはしない */
		ex_agent_flag = 1;
	    }
	    else {
		ex_agent_flag = 0;
	    }
	    token = strtok(point2, "?");
	    while (token) { /* "?"で結合されたものは切って格納 */
		code = get_str_code(token, thesaurus);
		if (code) {
		    /* <主体>のチェック (for backward compatibility -> will be deleted) */
		    if (ex_agent_flag == 0 && 
			cf_match_element(code, (flag & USE_BGH) ? sm2code("主体") : "主体", FALSE)) {
			agent_count += freq;
			ex_agent_flag = 1;
		    }

		    if (!over_flag) {
			if (strlen(buf) + strlen(code) >= max) {
			    /* fprintf(stderr, "Too many EX <%s> (%2dth).\n", cf_str_buf, count); */
			    over_flag = 1;
			}
			else {
			    strcat(buf, code);
			}
		    }
		    free(code);
		}

		if (c_ptr->ex_size[num] == 0) {
		    c_ptr->ex_size[num] = 10;	/* 初期確保数 */
		    c_ptr->ex_list[num] = (char **)malloc_data(sizeof(char *)*c_ptr->ex_size[num], 
							       "_make_ipal_cframe_ex");
		    c_ptr->ex_freq[num] = (int *)malloc_data(sizeof(int)*c_ptr->ex_size[num], 
							     "_make_ipal_cframe_ex");
		}
		else if (c_ptr->ex_num[num] >= c_ptr->ex_size[num]) {
		    c_ptr->ex_list[num] = (char **)realloc_data(c_ptr->ex_list[num], 
								sizeof(char *)*(c_ptr->ex_size[num] <<= 1), 
								"_make_ipal_cframe_ex");
		    c_ptr->ex_freq[num] = (int *)realloc_data(c_ptr->ex_freq[num], 
							      sizeof(int)*c_ptr->ex_size[num], 
							      "_make_ipal_cframe_ex");
		}

		c_ptr->ex_list[num][c_ptr->ex_num[num]] = strdup(token);
		c_ptr->ex_freq[num][c_ptr->ex_num[num]++] = freq;

		token = strtok(NULL, "?");
	    }

	    c_ptr->freq[num] += freq;
	    count++;
	}
    }

    /* <主体>の割合を格フレームの<TH:主体>から取得する
       格フレーム旧バージョン(200706版以前): <TH:主体>がないので、上で動的に計算
       格フレーム新バージョン: <TH:主体>から取得するが、ない場合(0.01未満)は上で計算したagent_countになる
                               (上の動的計算は撲滅予定) */
    if (agent_ratio > 0) {
	agent_count = agent_ratio * c_ptr->freq[num];

	/* <主体>を意味素に追加 (-no-probcase用) */
	if (!sub_agent_flag && agent_ratio > SM_AGENT_THRESHOLD) {
	    _make_ipal_cframe_sm(c_ptr, "主体", num, flag);
	}
    }

    /* <主体>の追加 */
    if (agent_count || sub_agent_flag) {
	if (c_ptr->ex_size[num] == 0) {
	    c_ptr->ex_size[num] = 1;	/* 初期確保数 */
	    c_ptr->ex_list[num] = (char **)malloc_data(sizeof(char *)*c_ptr->ex_size[num], 
						       "_make_ipal_cframe_ex");
	    c_ptr->ex_freq[num] = (int *)malloc_data(sizeof(int)*c_ptr->ex_size[num], 
						     "_make_ipal_cframe_ex");
	}
	else if (c_ptr->ex_num[num] >= c_ptr->ex_size[num]) {
	    c_ptr->ex_list[num] = (char **)realloc_data(c_ptr->ex_list[num], 
							sizeof(char *)*(c_ptr->ex_size[num] <<= 1), 
							"_make_ipal_cframe_ex");
	    c_ptr->ex_freq[num] = (int *)realloc_data(c_ptr->ex_freq[num], 
						      sizeof(int)*c_ptr->ex_size[num], 
						      "_make_ipal_cframe_ex");
	}
	c_ptr->ex_list[num][c_ptr->ex_num[num]] = strdup("<主体>");
	c_ptr->ex_freq[num][c_ptr->ex_num[num]++] = agent_count ? agent_count : 1;
	if (c_ptr->freq[num] == 0) {
	    c_ptr->freq[num] = 1;
	}
    }

    *destination = strdup(buf);
    free(buf);
}

/*==================================================================*/
 int check_examples(char *cp, int cp_len, char **ex_list, int ex_num)
/*==================================================================*/
{
    int i;

    if (!ex_list) {
	return -1;
    }

    for (i = 0; i < ex_num; i++) {
	if (strlen(*(ex_list + i)) == cp_len && 
	    !strncmp(cp, *(ex_list + i), cp_len)) {
	    return i;
	}
    }
    return -1;
}

/*==================================================================*/
		int check_agentive(unsigned char *cp)
/*==================================================================*/
{
    unsigned char *point;

    point = cp;
    while ((point = extract_ipal_str(point, cf_str_buf, FALSE)))
	if (!strcmp(cf_str_buf, "Ａ")) return TRUE;
    return FALSE;
}

/*==================================================================*/
     void _make_ipal_cframe(CF_FRAME *i_ptr, CASE_FRAME *cf_ptr,
			    unsigned int address, int size, char *verb, int flag)
/*==================================================================*/
{
    int i, j, ga_p = FALSE, c1, c2, count = 0;
    char ast_cap[32], *token, *cp, *buf;

    cf_ptr->type = flag;
    cf_ptr->cf_address = address;
    cf_ptr->cf_size = size;
    strcpy(cf_ptr->cf_id, i_ptr->DATA); 
    cf_ptr->etcflag = i_ptr->etcflag;
    cf_ptr->entry = strdup(cf_ptr->cf_id);
    sscanf(cf_ptr->cf_id, "%[^:]", cf_ptr->entry); /* 格フレームの用言表記 (代表表記) */
    strcpy(cf_ptr->pred_type, i_ptr->pred_type);
    for (i = 0; i_ptr->cf_align[i].cf_id != NULL; i++) {
	cf_ptr->cf_align[i].cf_id = strdup(i_ptr->cf_align[i].cf_id);
	for (j = 0; i_ptr->cf_align[i].aligned_case[j][0] != END_M; j++) {
	    cf_ptr->cf_align[i].aligned_case[j][0] = i_ptr->cf_align[i].aligned_case[j][0];
	    cf_ptr->cf_align[i].aligned_case[j][1] = i_ptr->cf_align[i].aligned_case[j][1];
	}
	cf_ptr->cf_align[i].aligned_case[j][0] = END_M;
	cf_ptr->cf_align[i].aligned_case[j][1] = END_M;
    }
    cf_ptr->cf_align[i].cf_id = NULL;
    cf_ptr->cf_align[i].aligned_case[0][0] = END_M;
    cf_ptr->cf_align[i].aligned_case[0][1] = END_M;
    for (i = 0; i_ptr->samecase[i][0] != END_M; i++) {
	cf_ptr->samecase[i][0] = i_ptr->samecase[i][0];
	cf_ptr->samecase[i][1] = i_ptr->samecase[i][1];
    }
    cf_ptr->samecase[i][0] = END_M;
    cf_ptr->samecase[i][1] = END_M;

    if (*(i_ptr->feature)) {
	cf_ptr->feature = strdup(i_ptr->feature);
    }
    else {
	cf_ptr->feature = NULL;
    }
    cf_ptr->cf_similarity = 0;


    /* 格要素の追加 */

    j = 0;
    if (cf_ptr->voice == FRAME_PASSIVE_I ||
	cf_ptr->voice == FRAME_CAUSATIVE_WO_NI ||
	cf_ptr->voice == FRAME_CAUSATIVE_WO ||
	cf_ptr->voice == FRAME_CAUSATIVE_NI) {
	_make_ipal_cframe_pp(cf_ptr, "ガ", j, flag);
	_make_ipal_cframe_sm(cf_ptr, "主体準", j, 
			     Thesaurus == USE_NTT ? USE_NTT_WITH_STORE : USE_BGH_WITH_STORE);
	_make_ipal_cframe_ex(cf_ptr, "彼", j, Thesaurus, FALSE);
	j++;
    }
    else if (cf_ptr->voice == FRAME_CAUSATIVE_PASSIVE) {
	_make_ipal_cframe_pp(cf_ptr, "ニ", j, flag);
	_make_ipal_cframe_sm(cf_ptr, "主体準", j, 
			     Thesaurus == USE_NTT ? USE_NTT_WITH_STORE : USE_BGH_WITH_STORE);
	_make_ipal_cframe_ex(cf_ptr, "彼", j, Thesaurus, FALSE);
	j++;
    }

    /* 各格要素の処理 */

    for (i = 0; i < i_ptr->casenum && j < CASE_MAX_NUM; i++, j++) { 
	cf_ptr->adjacent[j] = FALSE;
	if (_make_ipal_cframe_pp(cf_ptr, i_ptr->cs[i].kaku_keishiki, j, flag) == FALSE) {
	    j--;
	    continue;
	}
	if (Thesaurus == USE_BGH) {
	    _make_ipal_cframe_ex(cf_ptr, i_ptr->cs[i].meishiku, j, USE_BGH_WITH_STORE, 
				 (OptCaseFlag & OPT_CASE_USE_EX_ALL) ? 0 : !MatchPP(cf_ptr->pp[j][0], "外の関係"));
	    _make_ipal_cframe_sm(cf_ptr, i_ptr->cs[i].imisosei, j, USE_BGH_WITH_STORE);
	}
	else if (Thesaurus == USE_NTT) {
	    _make_ipal_cframe_ex(cf_ptr, i_ptr->cs[i].meishiku, j, USE_NTT_WITH_STORE, 
				 (OptCaseFlag & OPT_CASE_USE_EX_ALL) ? 0 : !MatchPP(cf_ptr->pp[j][0], "外の関係"));
	    _make_ipal_cframe_sm(cf_ptr, i_ptr->cs[i].imisosei, j, USE_NTT_WITH_STORE);
	}

	/* 能動 : Agentive ガ格を任意的とする場合
	if (cf_ptr->voice == FRAME_ACTIVE &&
	    i == 0 && 
	    cf_ptr->pp[i][0] == pp_kstr_to_code("ガ") &&
	    check_agentive(i_ptr->DATA+i_ptr->jyutugoso) == TRUE)
	  cf_ptr->oblig[i] = FALSE;
	*/

	if (cf_ptr->voice == FRAME_CAUSATIVE_WO_NI ||	/* 使役 */
	    cf_ptr->voice == FRAME_CAUSATIVE_WO ||
	    cf_ptr->voice == FRAME_CAUSATIVE_NI) {
	    /* ガ → ヲ，ニ */
	    if (cf_ptr->pp[j][0] == pp_kstr_to_code("ガ")) {
		if (ga_p == FALSE) {
		    ga_p = TRUE;
		    if (cf_ptr->voice == FRAME_CAUSATIVE_WO_NI)
		      _make_ipal_cframe_pp(cf_ptr, "ヲ／ニ", j, flag);
		    else if (cf_ptr->voice == FRAME_CAUSATIVE_WO)
		      _make_ipal_cframe_pp(cf_ptr, "ヲ", j, flag);
		    else if (cf_ptr->voice == FRAME_CAUSATIVE_NI)
		      _make_ipal_cframe_pp(cf_ptr, "ニ", j, flag);
		} else {
		    _make_ipal_cframe_pp(cf_ptr, "ヲ", j, flag); /* ガ・ガ構文 */
		}
	    }
	}
	else if (cf_ptr->voice == FRAME_PASSIVE_I ||	/* 受身 */
		 cf_ptr->voice == FRAME_PASSIVE_1 ||
		 cf_ptr->voice == FRAME_PASSIVE_2) {
	    /* 間接 ガ→ニ，直接 ガ→ニ／ニヨッテ／．． */
	    if (!strcmp(i_ptr->cs[i].kaku_keishiki, "ガ")) {
		if (cf_ptr->voice == FRAME_PASSIVE_I)
		  _make_ipal_cframe_pp(cf_ptr, "ニ", j, flag);
		else if (cf_ptr->voice == FRAME_PASSIVE_1)
		  _make_ipal_cframe_pp(cf_ptr, 
				       "ニ／ニヨル／カラ", j, flag);
		else if (cf_ptr->voice == FRAME_PASSIVE_2)
		  _make_ipal_cframe_pp(cf_ptr, 
				       "ニ／ニヨル／カラ", j, flag);
	    }
	    /* 直接 ニ／ニヨッテ／．．→ガ */
	    else if ((cf_ptr->voice == FRAME_PASSIVE_1 && 
		      (!strcmp(i_ptr->cs[i].kaku_keishiki, "ヲ") ||
		       (sprintf(ast_cap, "ヲ＊") &&
			!strcmp(i_ptr->cs[i].kaku_keishiki, ast_cap)))) ||
		     (cf_ptr->voice == FRAME_PASSIVE_2 && 
		      (!strcmp(i_ptr->cs[i].kaku_keishiki, "ニ") ||
		       (sprintf(ast_cap, "ニ＊") &&
			!strcmp(i_ptr->cs[i].kaku_keishiki, ast_cap))))) {
		_make_ipal_cframe_pp(cf_ptr, "ガ", j, flag);
	    }
	}
	else if (cf_ptr->voice == FRAME_POSSIBLE) {	/* 可能 */
	    if (!strcmp(i_ptr->cs[i].kaku_keishiki, "ヲ")) {
		_make_ipal_cframe_pp(cf_ptr, "ガ／ヲ", j, flag);
	     }
	}
	else if (cf_ptr->voice == FRAME_SPONTANE) {	/* 自発 */
	    if (!strcmp(i_ptr->cs[i].kaku_keishiki, "ヲ")) {
		_make_ipal_cframe_pp(cf_ptr, "ガ", j, flag);
	     }
	}
    }
    cf_ptr->element_num = j;
}

/*==================================================================*/
TAG_DATA *get_quasi_closest_case_component(TAG_DATA *t_ptr, TAG_DATA *pre_ptr)
/*==================================================================*/
{
    if (t_ptr->num < 1 || 
	(t_ptr->type == IS_TAG_DATA && t_ptr->inum != 0)) {
	return NULL;
    }

    if (check_feature(t_ptr->f, "ID:（〜を）〜に")) {
	return t_ptr;
    }

    if (pre_ptr->type == IS_TAG_DATA && pre_ptr->inum != 0) {
	return NULL;
    }

    if (!check_feature(pre_ptr->f, "体言")) {
	return NULL;
    }

    if (check_feature(pre_ptr->f, "指示詞") || 
	(pre_ptr->SM_code[0] == '\0' && 
	 check_feature(pre_ptr->f, "係:ガ格"))) {
	return NULL;
    }

    if (check_feature(pre_ptr->f, "係:ヲ格") || 
	check_feature(pre_ptr->f, "係:ニ格") || 
	(!cf_match_element(pre_ptr->SM_code, "主体", FALSE) && 
	(check_feature(pre_ptr->f, "係:ガ格") || 
	 check_feature(pre_ptr->f, "係:カラ格") || 
	 check_feature(pre_ptr->f, "係:ヘ格") || 
	 check_feature(pre_ptr->f, "係:ヨリ格") || 
	 check_feature(pre_ptr->f, "係:ト格") || 
	 check_feature(pre_ptr->f, "係:マデ格")))) {
	return pre_ptr;
    }
    return NULL;
}

/*==================================================================*/
		   char *feature2case(TAG_DATA *tp)
/*==================================================================*/
{
    char *cp, *buffer;

    if (check_feature(tp->f, "ID:（〜を）〜に")) {
	buffer = (char *)malloc_data(strlen("ニ")+1, "feature2case");
	strcpy(buffer, "ニ");
	return buffer;
    }
    else if ((cp = check_feature(tp->f, "係"))) {
	buffer = strdup(cp+strlen("係:"));
	if (!strncmp(buffer+strlen(buffer)-strlen("格"), "格", strlen("格"))) {
	    *(buffer+strlen(buffer)-strlen("格")) = '\0';
	    if (pp_kstr_to_code(buffer) != END_M) {
		return buffer;
	    }
	}
	free(buffer);
    }
    return NULL;
}

/*==================================================================*/
	       void f_num_inc(int start, int *f_num_p)
/*==================================================================*/
{
    (*f_num_p)++;
    if ((start + *f_num_p) >= MAX_Case_frame_num) {
	realloc_cf();
	realloc_cmm();
    }
}

/*==================================================================*/
	  void clear_cf_element(CASE_FRAME *cf_ptr, int num)
/*==================================================================*/
{
    int k;

    if (cf_ptr->pp_str[num]) {
	free(cf_ptr->pp_str[num]);
	cf_ptr->pp_str[num] = NULL;
    }
    if (cf_ptr->ex[num]) {
	free(cf_ptr->ex[num]);
	cf_ptr->ex[num] = NULL;
    }
    if (cf_ptr->sm[num]) {
	free(cf_ptr->sm[num]);
	cf_ptr->sm[num] = NULL;
    }
    if (cf_ptr->sm_delete[num]) {
	free(cf_ptr->sm_delete[num]);
	cf_ptr->sm_delete[num] = NULL;
	cf_ptr->sm_delete_size[num] = 0;
	cf_ptr->sm_delete_num[num] = 0;
    }
    if (cf_ptr->sm_specify[num]) {
	free(cf_ptr->sm_specify[num]);
	cf_ptr->sm_specify[num] = NULL;
	cf_ptr->sm_specify_size[num] = 0;
	cf_ptr->sm_specify_num[num] = 0;
    }
    if (cf_ptr->ex_list[num]) {
	for (k = 0; k < cf_ptr->ex_num[num]; k++) {
	    if (cf_ptr->ex_list[num][k]) {
		free(cf_ptr->ex_list[num][k]);
	    }
	}
	free(cf_ptr->ex_list[num]);
	free(cf_ptr->ex_freq[num]);
	cf_ptr->ex_list[num] = NULL;
	cf_ptr->ex_size[num] = 0;
	cf_ptr->ex_num[num] = 0;
    }
    if (cf_ptr->gex_list[num]) {
	for (k = 0; k < cf_ptr->gex_num[num]; k++) {
	    if (cf_ptr->gex_list[num][k]) {
		free(cf_ptr->gex_list[num][k]);
	    }
	}
	free(cf_ptr->gex_list[num]);
	free(cf_ptr->gex_freq[num]);
	cf_ptr->gex_list[num] = NULL;
	cf_ptr->gex_size[num] = 0;
	cf_ptr->gex_num[num] = 0;
    }
    if (cf_ptr->semantics[num]) {
	free(cf_ptr->semantics[num]);
	cf_ptr->semantics[num] = NULL;
    }
}

/*==================================================================*/
int _make_ipal_cframe_subcontract(SENTENCE_DATA *sp, TAG_DATA *t_ptr, int start, 
				  char *verb, int voice, int flag)
/*==================================================================*/
{
    CF_FRAME *i_ptr;
    CASE_FRAME *cf_ptr;
    TAG_DATA *cbp;
    int f_num = 0, break_flag = 0, size, match, c;
    unsigned int address;
    char *pre_pos, *cp, *address_str, *vtype = NULL;

    if (!verb)
    	return f_num;

    cf_ptr = Case_frame_array + start;

    /* 直前格要素をくっつけて検索 */
    if (flag == CF_PRED) {
	/* ひらがなで曖昧性のあるときは、格解析で曖昧性解消するために
	   ここではすべての格フレームを検索しておく */
	if (check_str_type(t_ptr->head_ptr->Goi) == TYPE_HIRAGANA && 

	    check_feature(t_ptr->head_ptr->f, "品曖")) {
	    address_str = get_ipal_address(verb, flag);
	}
	else {
	    cbp = get_quasi_closest_case_component(t_ptr, t_ptr->num < 1 ? NULL : t_ptr - 1);
	    if ((OptCaseFlag & OPT_CASE_FIX_CF_SEARCH) && cbp) {
		char *buffer, *pp, *cbp_str;
		int cbp_str_malloc_flag = 0;

		pp = feature2case(cbp);
		if (pp) {
		    if (OptCaseFlag & OPT_CASE_USE_REP_CF) {
			if ((OptCaseFlag & OPT_CASE_USE_CREP_CF) && /* 正規化(主辞)代表表記 */
			    (cbp_str = get_bnst_head_canonical_rep(cbp->b_ptr, OptCaseFlag & OPT_CASE_USE_CN_CF))) {
			    ;
			}
			else if ((cbp_str = get_mrph_rep_from_f(cbp->head_ptr, FALSE)) == NULL) { /* feature中の代表表記 */
			    cbp_str = make_mrph_rn(cbp->head_ptr); /* なければ作る */
			    cbp_str_malloc_flag = 1;
			}
		    }
		    else {
			cbp_str = cbp->head_ptr->Goi;
		    }
		    buffer = (char *)malloc_data(strlen(cbp_str) + strlen(pp) + strlen(verb) + 3, 
						 "_make_ipal_cframe_subcontract");
		    sprintf(buffer, "%s-%s-%s", cbp_str, pp, verb);
		    address_str = get_ipal_address(buffer, flag);
		    free(buffer);
		    free(pp);
		    if (cbp_str_malloc_flag) {
			free(cbp_str);
		    }
		}
		if (!pp || !address_str) {
		    address_str = get_ipal_address(verb, flag);
		}
	    }
	    else {
		address_str = get_ipal_address(verb, flag);
	    }
	}
    }
    else {
	address_str = get_ipal_address(verb, flag);
    }

    /* なければ */
    if (!address_str)
	return f_num;

    if (flag == CF_NOUN && (vtype = check_feature(t_ptr->f, "体言"))) {
	vtype = "名";
    }
    else if ((vtype = check_feature(t_ptr->f, "用言"))) {
	vtype += strlen("用言:");
    }
    else if ((vtype = check_feature(t_ptr->f, "非用言格解析"))) {
	vtype += strlen("非用言格解析:");
    }

    for (cp = pre_pos = address_str; ; cp++) {
	if (*cp == '/' || *cp == '\0') {
	    if (*cp == '\0')
		break_flag = 1;
	    else 
		*cp = '\0';
	    
	    /* 格フレームの読みだし */
	    match = sscanf(pre_pos, "%d:%d", &address, &size);
	    if (match != 2) {
		fprintf(stderr, ";; CaseFrame Dictionary Index error (it seems version 1.).\n");
		exit(1);
	    }

	    i_ptr = get_ipal_frame(address, size, flag);
	    pre_pos = cp + 1;

	    /* 用言のタイプがマッチしなければ (準用言なら通過) */
	    if (vtype) {
		if (strncmp(vtype, i_ptr->pred_type, strlen("名"))) {
		    if (break_flag)
			break;
		    else
			continue;
		}
	    }

	    /* 能動態 or 格フレームに態が含まれる場合 */
	    if (voice == 0) {
		/* CF_NOUNの場合、ここでマッチ */
		(cf_ptr + f_num)->voice = FRAME_ACTIVE;
		_make_ipal_cframe(i_ptr, cf_ptr + f_num, address, size, verb, flag);

		/* 用言のときは、一般的外の関係名詞を追加 */
		if ((OptCaseFlag & OPT_CASE_ADD_SOTO_WORDS) && flag == CF_PRED) {
		    /* 外の関係がないとき */
		    if ((c = check_cf_case(cf_ptr + f_num, "外の関係")) < 0) {
			if ((cf_ptr + f_num)->element_num == CF_ELEMENT_MAX) { /* 格の数が上限 */
			    (cf_ptr + f_num)->element_num--; /* 最後の格を削除 */
			    clear_cf_element((cf_ptr + f_num), (cf_ptr + f_num)->element_num);
			}
			_make_ipal_cframe_pp(cf_ptr + f_num, "外の関係", (cf_ptr + f_num)->element_num, flag);
			_make_ipal_cframe_ex(cf_ptr + f_num, GENERAL_SOTO_WORDS, (cf_ptr + f_num)->element_num, Thesaurus, FALSE);
			(cf_ptr + f_num)->element_num++;
		    }
		    /* 外の関係がすでにあるときは用例を追加 */
		    else {
			_make_ipal_cframe_ex(cf_ptr + f_num, GENERAL_SOTO_WORDS, c, Thesaurus, FALSE);
		    }
		}

		/* 以下 flag == CF_PRED のはず */

		/* 格フレーム使役/格フレーム使役&受身*/
		if (t_ptr->voice & VOICE_SHIEKI || 
		    t_ptr->voice & VOICE_SHIEKI_UKEMI) {
		    /* ニ格がないとき */
		    if ((c = check_cf_case(cf_ptr + f_num, "ニ")) < 0) {
			_make_ipal_cframe_pp(cf_ptr + f_num, "ニ", (cf_ptr + f_num)->element_num, flag);
			_make_ipal_cframe_sm(cf_ptr + f_num, "主体", (cf_ptr + f_num)->element_num, 
					     Thesaurus == USE_NTT ? USE_NTT_WITH_STORE : USE_BGH_WITH_STORE);
			(cf_ptr+f_num)->element_num++;
		    }
		    /* ニ格はあるけど<主体>がないとき */
		    else if (sms_match(sm2code("主体"), (cf_ptr + f_num)->sm[c], SM_NO_EXPAND_NE) == FALSE) {
			_make_ipal_cframe_sm(cf_ptr + f_num, "主体", c, 
					     Thesaurus == USE_NTT ? USE_NTT_WITH_STORE : USE_BGH_WITH_STORE);
		    }
		    if (t_ptr->voice & VOICE_SHIEKI) {
			(cf_ptr + f_num)->voice = FRAME_CAUSATIVE_NI;
		    }
		    else if (t_ptr->voice & VOICE_SHIEKI_UKEMI) {
			(cf_ptr + f_num)->voice = FRAME_CAUSATIVE_PASSIVE;
		    }
		}
		/* 格フレーム受身 */
		else if (t_ptr->voice & VOICE_UKEMI || 
			 t_ptr->voice & VOICE_UNKNOWN) {
		    /* ニ/ニヨル/カラ格がないとき */
		    if ((c = check_cf_case(cf_ptr + f_num, "ニ")) < 0 && 
			(c = check_cf_case(cf_ptr + f_num, "ニヨル")) < 0 && 
			(c = check_cf_case(cf_ptr + f_num, "カラ")) < 0) {
			_make_ipal_cframe_pp(cf_ptr + f_num, "ニ", (cf_ptr + f_num)->element_num, flag);
			_make_ipal_cframe_sm(cf_ptr + f_num, "主体", (cf_ptr + f_num)->element_num, 
					     Thesaurus == USE_NTT ? USE_NTT_WITH_STORE : USE_BGH_WITH_STORE);
			(cf_ptr+f_num)->element_num++;
		    }
		    /* ニ/ニヨル/カラ格はあるけど<主体>がないとき */
		    else if (sms_match(sm2code("主体"), (cf_ptr + f_num)->sm[c], SM_NO_EXPAND_NE) == FALSE) {
			_make_ipal_cframe_sm(cf_ptr + f_num, "主体", c, 
					     Thesaurus == USE_NTT ? USE_NTT_WITH_STORE : USE_BGH_WITH_STORE);
		    }
		    (cf_ptr + f_num)->voice = FRAME_PASSIVE_1;
		}
		
		f_num_inc(start, &f_num);
		cf_ptr = Case_frame_array + start;
	    }

	    /* 使役 */
	    if (voice & VOICE_SHIEKI) {
		if (i_ptr->voice & CF_CAUSATIVE_WO && i_ptr->voice & CF_CAUSATIVE_NI)
		  (cf_ptr + f_num)->voice = FRAME_CAUSATIVE_WO_NI;
		else if (i_ptr->voice & CF_CAUSATIVE_WO)
		  (cf_ptr + f_num)->voice = FRAME_CAUSATIVE_WO;
		else if (i_ptr->voice & CF_CAUSATIVE_NI)
		  (cf_ptr + f_num)->voice = FRAME_CAUSATIVE_NI;
		
		_make_ipal_cframe(i_ptr, cf_ptr + f_num, address, size, verb, flag);
		f_num_inc(start, &f_num);
		cf_ptr = Case_frame_array + start;
	    }
	    
	    /* 受身 */
	    if (voice & VOICE_UKEMI) {
		/* 直接受身１ */
		if (i_ptr->voice & CF_PASSIVE_1) {
		    (cf_ptr + f_num)->voice = FRAME_PASSIVE_1;
		    _make_ipal_cframe(i_ptr, cf_ptr + f_num, address, size, verb, flag);
		    f_num_inc(start, &f_num);
		    cf_ptr = Case_frame_array + start;
		}
		/* 直接受身２ */
		if (i_ptr->voice & CF_PASSIVE_2) {
		    (cf_ptr + f_num)->voice = FRAME_PASSIVE_2;
		    _make_ipal_cframe(i_ptr, cf_ptr + f_num, address, size, verb, flag);
		    f_num_inc(start, &f_num);
		    cf_ptr = Case_frame_array + start;
		}
		/* 間接受身 */
		if (i_ptr->voice & CF_PASSIVE_I) {
		    (cf_ptr + f_num)->voice = FRAME_PASSIVE_I;
		    _make_ipal_cframe(i_ptr, cf_ptr + f_num, address, size, verb, flag);
		    f_num_inc(start, &f_num);
		    cf_ptr = Case_frame_array + start;
		}
	    }

	    /* もらう/ほしい */
	    if (voice & VOICE_MORAU || 
		voice & VOICE_HOSHII) {
		/* ニ使役 (間接受身でも同じ */
		if (i_ptr->voice & CF_CAUSATIVE_NI) {
		    (cf_ptr + f_num)->voice = FRAME_CAUSATIVE_NI;
		    _make_ipal_cframe(i_ptr, cf_ptr + f_num, address, size, verb, flag);
		    f_num_inc(start, &f_num);
		    cf_ptr = Case_frame_array + start;
		}
 	    }

	    /* せられる/させられる */
	    if (voice & VOICE_SHIEKI_UKEMI) {
		(cf_ptr + f_num)->voice = FRAME_CAUSATIVE_PASSIVE;
		_make_ipal_cframe(i_ptr, cf_ptr + f_num, address, size, verb, flag);
		f_num_inc(start, &f_num);
		cf_ptr = Case_frame_array + start;
	    } 

	    /* 可能，尊敬，自発 */
	    if (voice & VOICE_UKEMI) {
		if (i_ptr->voice & CF_POSSIBLE) {
		    (cf_ptr + f_num)->voice = FRAME_POSSIBLE;
		    _make_ipal_cframe(i_ptr, cf_ptr + f_num, address, size, verb, flag);
		    f_num_inc(start, &f_num);
		    cf_ptr = Case_frame_array + start;
		}
		if (i_ptr->voice & CF_POLITE) {
		    (cf_ptr + f_num)->voice = FRAME_POLITE;
		    _make_ipal_cframe(i_ptr, cf_ptr + f_num, address, size, verb, flag);
		    f_num_inc(start, &f_num);
		    cf_ptr = Case_frame_array + start;
		}
		if (i_ptr->voice & CF_SPONTANE) {
		    (cf_ptr + f_num)->voice = FRAME_SPONTANE;
		    _make_ipal_cframe(i_ptr, cf_ptr + f_num, address, size, verb, flag);
		    f_num_inc(start, &f_num);
		    cf_ptr = Case_frame_array + start;
		}
	    }
	    if (break_flag)
		break;
	}
    }
    free(address_str);
    return f_num;
}

/*==================================================================*/
int make_ipal_cframe_subcontract(SENTENCE_DATA *sp, TAG_DATA *t_ptr, int start, char *in_verb, int flag)
/*==================================================================*/
{
    int f_num = 0, plus_num;
    char *verb;

    if (flag == CF_NOUN) {
	return _make_ipal_cframe_subcontract(sp, t_ptr, start, in_verb, 0, flag);
    }

    verb = (char *)malloc_data(strlen(in_verb) + 4, "make_ipal_cframe_subcontract");
    strcpy(verb, in_verb);

    if (t_ptr->voice == VOICE_UNKNOWN) {
	t_ptr->voice = 0; /* 能動態でtry */
	f_num = _make_ipal_cframe_subcontract(sp, t_ptr, start, verb, 0, flag);

	/* 今のところ受身の場合を考えない */
	free(verb);
	return f_num;

	t_ptr->voice = VOICE_UNKNOWN;
    }

    /* 受身, 使役の格フレーム */
    if (t_ptr->voice == VOICE_UNKNOWN || 
	t_ptr->voice & VOICE_UKEMI ||
	t_ptr->voice & VOICE_SHIEKI || 
	t_ptr->voice & VOICE_SHIEKI_UKEMI) {
	int suffix = 0;

	if (t_ptr->voice & VOICE_SHIEKI) {
	    strcat(verb, ":C");
	    suffix = 2;
	}
	else if (t_ptr->voice & VOICE_UKEMI || 
		 t_ptr->voice & VOICE_UNKNOWN) {
	    strcat(verb, ":P");
	    suffix = 2;
	}
	else if (t_ptr->voice & VOICE_SHIEKI_UKEMI) {
	    strcat(verb, ":PC");
	    suffix = 3;
	}

	plus_num = _make_ipal_cframe_subcontract(sp, t_ptr, start + f_num, verb, 0, flag);
	if (plus_num != 0) {
	    free(verb);
	    return f_num + plus_num;
	}
	*(verb + strlen(verb) - suffix) = '\0'; /* みつからなかったらもとにもどす */
    }

    if (t_ptr->voice == VOICE_UNKNOWN) {
	f_num += _make_ipal_cframe_subcontract(sp, t_ptr, start + f_num, verb, VOICE_UKEMI, flag); /* 受身 */
    }
    else {
	f_num = _make_ipal_cframe_subcontract(sp, t_ptr, start, verb, t_ptr->voice, flag);
    }
    free(verb);    
    return f_num;
}

/*==================================================================*/
		char *make_pred_type(TAG_DATA *t_ptr)
/*==================================================================*/
{
    /* 用言タイプをかえす */

    if (t_ptr->voice == VOICE_UNKNOWN) { /* 文末のサ変名詞など */
	return ":?";
    }
    else if (t_ptr->voice & VOICE_SHIEKI) {
	return ":C";
    }
    else if (t_ptr->voice & VOICE_UKEMI) {
	return ":P";
    }
    else if (t_ptr->voice & VOICE_SHIEKI_UKEMI) {
	return ":PC";
    }
    else {
	return "";
    }
}

/*==================================================================*/
char *make_pred_string(TAG_DATA *t_ptr, MRPH_DATA *m_ptr, char *orig_form, int use_rep_flag, int flag)
/*==================================================================*/
{
    char *buffer, *main_pred = NULL, *cp, *rep_strt;
    int rep_length, main_pred_malloc_flag = 0, i, cpncf_flag = 0;

    /* orig_form == 1: 可能動詞のもとの形を用いるとき */

    /* m_ptr == NULL: 本動詞の形態素は t_ptr->head_ptr を用いる
       otherwise    : 本動詞の形態素として m_ptr を用いる (ALTのもの) */

    /* 用言タイプ, voiceの分(7)も確保しておく */

    /* 代表表記を使う場合で代表表記があるとき */
    if (use_rep_flag) {

	if (m_ptr) {
            rep_strt = get_mrph_rep(m_ptr);
            rep_length = get_mrph_rep_length(rep_strt);
            if (rep_length) {
                main_pred = (char *)malloc_data(rep_length + 1, "make_pred_string");
                strncpy(main_pred, rep_strt, rep_length);
                *(main_pred + rep_length) = '\0';
                main_pred_malloc_flag = 1;
            }
        }
	/* 省略解析をする場合は正規化代表表記を使用する */
	else if (flag == CF_NOUN && OptAnaphora &&
		 (cp = check_feature(t_ptr->b_ptr->f, "正規化代表表記"))) {
	    cpncf_flag = 1;
	    rep_strt = cp + strlen("正規化代表表記:");
	    rep_length = strlen(rep_strt);		

	    main_pred = (char *)malloc_data(rep_length + 1, "make_pred_string");
	    strncpy(main_pred, rep_strt, rep_length);
	    *(main_pred + rep_length) = '\0';
	    main_pred_malloc_flag = 1;
	}
	/* 複合名詞格フレームを用いる場合で、分類語彙表が引けている場合 */
	/* ただし、BGH中に"|"が含まれている場合は除く */
	else if (flag == CF_NOUN && OptUseCPNCF &&  
		 !check_feature(t_ptr->head_ptr->f, "特殊非見出語") &&		 
		 (cp = check_feature(t_ptr->f, "BGH")) && !strstr(cp, "|")) {

	    /* 形容詞語幹の場合は通常の代表表記を使用する */
	    if (check_feature(t_ptr->f, "名詞的形容詞語幹")) {
		rep_strt = get_mrph_rep(t_ptr->head_ptr);
		rep_length = get_mrph_rep_length(rep_strt);
	    }
	    /* 複合名詞格フレームを用いる場合は分類語彙表の見出しを用いる */
	    else {
		cpncf_flag = 1;
		rep_strt = cp + strlen("BGH:");
		rep_length = strlen(rep_strt);

		/* 後方の基本句の分類語彙表の見出しに含まれる基本句は省略解析の対象としない */
		/* 分類語彙表の見出しが処理対象の基本句より長い場合のみ実行*/
		cp = get_mrph_rep(t_ptr->head_ptr);
		if (0 && cp && strncmp(cp, rep_strt, strlen(cp) - 1)) {
		    i = 1;
		    while ((t_ptr - i) && (t_ptr - i)->head_ptr) {
			assign_cfeature(&((t_ptr - i)->f), "省略解析なし", FALSE);
			if (check_feature((t_ptr - i)->head_ptr->f, "文節始")) break;
			cp = get_mrph_rep((t_ptr - i)->head_ptr);
			if (!cp || !strncmp(cp, rep_strt, strlen(cp) - 1)) break;
			i++;
		    }
		}
	    }
	    if (rep_length) {
		main_pred = (char *)malloc_data(rep_length + 1, "make_pred_string");
		strncpy(main_pred, rep_strt, rep_length);
		*(main_pred + rep_length) = '\0';
		main_pred_malloc_flag = 1;
	    }
	}
	else {
	    /* 用言のとき、末尾の名詞性名詞接尾辞は無視する */
	    if ((flag & CF_PRED) &&
		!strcmp(Class[t_ptr->head_ptr->Hinshi][t_ptr->head_ptr->Bunrui].id, "名詞性名詞接尾辞")) {
		if ((main_pred = get_mrph_rep_from_f(t_ptr->head_ptr - 1, (flag & CF_PRED) && 
						     check_feature((t_ptr->head_ptr - 1)->f, "Ｔ代表表記変更前用言見出"))) == NULL) {
		    main_pred = make_mrph_rn(t_ptr->head_ptr - 1);
		    main_pred_malloc_flag = 1;
		}
	    }
	    /* 用言のとき、a化している形容詞語幹は、元の代表表記で引く(e.g., 「平和/条約」の「平和」) */
	    else if ((main_pred = get_mrph_rep_from_f(t_ptr->head_ptr, (flag & CF_PRED) && 
						 check_feature(t_ptr->head_ptr->f, "Ｔ代表表記変更前用言見出"))) == NULL) {
		main_pred = make_mrph_rn(t_ptr->head_ptr);
		main_pred_malloc_flag = 1;
	    }
	}
    }
    if (main_pred == NULL) {
	main_pred = m_ptr ? m_ptr->Goi : t_ptr->head_ptr->Goi;
    }

    /* 「（〜を）〜に」 のときは 「する」 で探す */
    if (check_feature(t_ptr->f, "ID:（〜を）〜に") && !cpncf_flag) {
	buffer = (char *)malloc_data(strlen("する/する") + 8, "make_pred_string"); /* 9(euc) + 8 */
	if (use_rep_flag) {
	    strcpy(buffer, "する/する");
	}
	else {
	    strcpy(buffer, "する");
	}
    }
    /* 「形容詞+なる」など */
    else if (check_feature(t_ptr->f, "Ｔ用言見出→") && !cpncf_flag &&
	     (!check_feature(t_ptr->head_ptr->f, "サ変") ||
	      strcmp(get_mrph_rep_from_f(t_ptr->head_ptr + 1, flag), "する/する"))) {
	if (use_rep_flag) {
	    if ((cp = get_mrph_rep_from_f(t_ptr->head_ptr + 1, flag))) {
		buffer = (char *)malloc_data(strlen(main_pred) + strlen(cp) + 9, 
					     "make_pred_string");
		strcpy(buffer, main_pred);
		strcat(buffer, "+");
		strcat(buffer, cp);
	    }
	    else {
		buffer = (char *)malloc_data(strlen(main_pred) + strlen((t_ptr->head_ptr + 1)->Goi) + 9, 
					     "make_pred_string");
		strcpy(buffer, main_pred);
		strcat(buffer, "+");
		strcat(buffer, (t_ptr->head_ptr + 1)->Goi);
	    }
	}
	else {
	    buffer = (char *)malloc_data(strlen(t_ptr->head_ptr->Goi2) + strlen((t_ptr->head_ptr + 1)->Goi) + 8, 
					 "make_pred_string");
	    strcpy(buffer, t_ptr->head_ptr->Goi2);
	    strcat(buffer, (t_ptr->head_ptr + 1)->Goi);
	}
    }
    /* 「形容詞語幹+的だ」など */
    else if (check_feature(t_ptr->f, "Ｔ用言見出←") && !cpncf_flag && 
	     !check_feature((t_ptr->head_ptr - 1)->f, "付属")) {
	if (use_rep_flag &&
	    (cp = get_mrph_rep_from_f(t_ptr->head_ptr - 1, FALSE))) {
	    buffer = (char *)malloc_data(strlen(cp) + strlen(main_pred) + 9, 
					 "make_pred_string");
	    strcpy(buffer, cp);
	    strcat(buffer, "+");
	}
	else {
	    buffer = (char *)malloc_data(strlen((t_ptr->head_ptr - 1)->Goi2) + strlen(main_pred) + 8, 
					 "make_pred_string");
	    strcpy(buffer, (t_ptr->head_ptr - 1)->Goi2);
	}
	strcat(buffer, main_pred);
    }
    else {
	if (orig_form) {
	    buffer = (char *)malloc_data(strlen(orig_form) + 8, "make_pred_string");
	    strcpy(buffer, orig_form);
	}
	else {
	    buffer = (char *)malloc_data(strlen(main_pred) + 8, "make_pred_string");
	    strcpy(buffer, main_pred);
	}
    }

    if (main_pred_malloc_flag) {
	free(main_pred);
    }

    return buffer;
}

/*==================================================================*/
int make_ipal_cframe(SENTENCE_DATA *sp, TAG_DATA *t_ptr, int start, int flag)
/*==================================================================*/
{
    int f_num = 0;
    char *cp, *pred_string, *new_pred_string;

    /* 自立語末尾語を用いて格フレーム辞書を引く */

    if (!t_ptr->jiritu_ptr) {
	return f_num;
    }
    
    pred_string = make_pred_string(t_ptr, NULL, NULL, OptCaseFlag & OPT_CASE_USE_REP_CF, flag);
    f_num += make_ipal_cframe_subcontract(sp, t_ptr, start, pred_string, flag);

    /* 代表表記が曖昧な用言の場合 */
    if (check_feature(t_ptr->head_ptr->f, "原形曖昧")) {
	FEATURE *fp;
	MRPH_DATA m;
	char *str;

	fp = t_ptr->head_ptr->f;
	while (fp) {
	    if (!strncmp(fp->cp, "ALT-", 4)) {
		sscanf(fp->cp + 4, "%[^-]-%[^-]-%[^-]-%d-%d-%d-%d-%[^\n]", 
		       m.Goi2, m.Yomi, m.Goi, 
		       &m.Hinshi, &m.Bunrui, 
		       &m.Katuyou_Kata, &m.Katuyou_Kei, m.Imi);
		new_pred_string = make_pred_string(t_ptr, &m, NULL, OptCaseFlag & OPT_CASE_USE_REP_CF, flag);
		/* 代表と異なるもの */
		if (strcmp(pred_string, new_pred_string)) {
		    f_num += make_ipal_cframe_subcontract(sp, t_ptr, start + f_num, new_pred_string, flag);
		}
		free(new_pred_string);
	    }
	    fp = fp->next;
	}
    }

    free(pred_string);

    /* ないときで、可能動詞のときは、もとの形を使う */
    if (f_num == 0 && 
	(cp = check_feature(t_ptr->head_ptr->f, "可能動詞"))) {
	pred_string = make_pred_string(t_ptr, NULL, cp + strlen("可能動詞:"), OptCaseFlag & OPT_CASE_USE_REP_CF, flag);
	f_num += make_ipal_cframe_subcontract(sp, t_ptr, start, pred_string, flag);	
	free(pred_string);
    }

    Case_frame_num += f_num;
    return f_num;
}

/*==================================================================*/
	  void assign_pred_feature_to_bp(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, pred_merged_rep_size = DATA_LEN;
    char *pred_string, *new_pred_string, *pred_merged_rep = NULL;
    TAG_DATA *t_ptr;

    /* 自立語末尾語を用いて用言代表表記を作成 */

    pred_merged_rep = (char *)malloc_data(pred_merged_rep_size, "assign_pred_feature_to_bp");

    for (i = 0; i < sp->Tag_num; i++) {
	t_ptr = sp->tag_data + i;
	if (!check_feature(t_ptr->f, "用言") || 
	    check_feature(t_ptr->f, "格解析なし")) {
	    continue;
	}

	pred_string = make_pred_string(t_ptr, NULL, NULL, OptCaseFlag & OPT_CASE_USE_REP_CF, CF_PRED);
	strcat(pred_string, make_pred_type(t_ptr));

	strcpy(pred_merged_rep, "用言代表表記:");
	strcat(pred_merged_rep, pred_string);

	/* 代表表記が曖昧な用言の場合 */
	if (check_feature(t_ptr->head_ptr->f, "原形曖昧")) {
	    FEATURE *fp;
	    MRPH_DATA m;
	    char *str;

	    fp = t_ptr->head_ptr->f;
	    while (fp) {
		if (!strncmp(fp->cp, "ALT-", 4)) {
		    sscanf(fp->cp + 4, "%[^-]-%[^-]-%[^-]-%d-%d-%d-%d-%[^\n]", 
			   m.Goi2, m.Yomi, m.Goi, 
			   &m.Hinshi, &m.Bunrui, 
			   &m.Katuyou_Kata, &m.Katuyou_Kei, m.Imi);
		    new_pred_string = make_pred_string(t_ptr, &m, NULL, OptCaseFlag & OPT_CASE_USE_REP_CF, CF_PRED);
		    /* 代表と異なるもの */
		    if (strcmp(pred_string, new_pred_string)) {
			strcat(new_pred_string, make_pred_type(t_ptr));

			if (strlen(pred_merged_rep) + strlen(new_pred_string) + 2 > pred_merged_rep_size) {
			    pred_merged_rep = (char *)realloc_data(pred_merged_rep, 
								   pred_merged_rep_size *= 2, "assign_pred_feature_to_bp");
			}
			strcat(pred_merged_rep, "?");
			strcat(pred_merged_rep, new_pred_string);
		    }
		    free(new_pred_string);
		}
		fp = fp->next;
	    }
	}

	free(pred_string);
	assign_cfeature(&(t_ptr->f), pred_merged_rep, FALSE);
    }

    free(pred_merged_rep);
}

/*==================================================================*/
	 int make_default_cframe(TAG_DATA *t_ptr, int start)
/*==================================================================*/
{
    int i, num = 0, f_num = 0, rep_name_malloc_flag = 0;
    CASE_FRAME *cf_ptr;
    char *cp, *rep_name;

    cf_ptr = Case_frame_array + start;
    cf_ptr->type = CF_PRED;

    if (MAX_cf_frame_length == 0) {
	cf_str_buf = 
	    (unsigned char *)realloc_data(cf_str_buf, 
					  sizeof(unsigned char)*ALLOCATION_STEP, 
					  "make_default_cframe");
    }

    cf_ptr->pred_type[0] = '\0';
    cf_ptr->cf_address = -1;
    if (cp = check_feature(t_ptr->f, "用言")) {
	_make_ipal_cframe_pp(cf_ptr, "ガ＊", num, CF_PRED);
	_make_ipal_cframe_sm(cf_ptr, "主体準", num++, 
			     Thesaurus == USE_NTT ? USE_NTT_WITH_STORE : USE_BGH_WITH_STORE);

	if (!strcmp(cp, "用言:判")) {
	    strcpy(cf_ptr->pred_type, "判");
	}
	else if (!strcmp(cp, "用言:動")) {
	    strcpy(cf_ptr->pred_type, "動");
	    _make_ipal_cframe_pp(cf_ptr, "ヲ＊", num++, CF_PRED);
	    _make_ipal_cframe_pp(cf_ptr, "ニ＊", num++, CF_PRED);
	    _make_ipal_cframe_pp(cf_ptr, "ヘ＊", num++, CF_PRED);
	    _make_ipal_cframe_pp(cf_ptr, "ヨリ＊", num++, CF_PRED);
	}
	else if (!strcmp(cp, "用言:形")) {
	    strcpy(cf_ptr->pred_type, "形");
	    _make_ipal_cframe_pp(cf_ptr, "ニ＊", num++, CF_PRED);
	    _make_ipal_cframe_pp(cf_ptr, "ヨリ＊", num++, CF_PRED);
	}
	else {
	    return FALSE;
	}
    }
    else {
	return FALSE;
    }

    cf_ptr->element_num = num;
    cf_ptr->etcflag = CF_NORMAL;
    /* 代表表記格フレームのときは、IDなどを代表表記にする */
    if (OptCaseFlag & OPT_CASE_USE_REP_CF) {
	if ((rep_name = get_mrph_rep_from_f(t_ptr->head_ptr, FALSE)) == NULL) {
	    rep_name = make_mrph_rn(t_ptr->head_ptr);
	    rep_name_malloc_flag = 1;
	}
	if (strlen(rep_name) >= REPNAME_LEN_MAX) { /* 長すぎるとき */
	    *(rep_name + REPNAME_LEN_MAX - 1) = '\0';
	}
	sprintf(cf_ptr->cf_id, "%s:%s0", rep_name, cf_ptr->pred_type);
	if (rep_name_malloc_flag) {
	    cf_ptr->entry = rep_name;
	}
	else {
	    cf_ptr->entry = strdup(rep_name);
	}
    }
    else {
	sprintf(cf_ptr->cf_id, "%s:%s0", t_ptr->head_ptr->Goi, cf_ptr->pred_type);
	cf_ptr->entry = strdup(t_ptr->head_ptr->Goi);
    }

    for (i = 0; i < num; i++) {
	cf_ptr->pp[i][1] = END_M;
    }

    cf_ptr->samecase[0][0] = END_M;
    cf_ptr->samecase[0][1] = END_M;

    f_num_inc(start, &f_num);
    Case_frame_num++;
    t_ptr->cf_num = 1;
    return TRUE;
}

/*==================================================================*/
  void make_caseframes(SENTENCE_DATA *sp, TAG_DATA *t_ptr, int flag)
/*==================================================================*/
{
    t_ptr->cf_num += make_ipal_cframe(sp, t_ptr, Case_frame_num, flag);

    /* ないときで用言のときは、defaultの格フレームをつくる */
    if (t_ptr->cf_num == 0 && flag == CF_PRED) {
	make_default_cframe(t_ptr, Case_frame_num);
    }
}

/*==================================================================*/
		void set_caseframes(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j, start, hiragana_count, pred_num = 0;
    TAG_DATA  *t_ptr;

    Case_frame_num = 0;

    for (i = 0, t_ptr = sp->tag_data; i < sp->Tag_num; i++, t_ptr++) {
	/* 正解コーパスを入力したときに自立語がない場合がある */
	t_ptr->cf_num = 0;

	if (t_ptr->jiritu_ptr != NULL && 
	    !check_feature(t_ptr->f, "格解析なし")) {
	    if (OptUseNCF && /* 名詞格フレーム */
		/* 「ＡのＢＣ」の係り受けをチェックする際、
		 先頭の格フレームが名詞格フレームであることを仮定しているため
		用言格フレームより先に読み込む必要あり(判定詞対策) */
		check_feature(t_ptr->f, "体言")) {
		make_caseframes(sp, t_ptr, CF_NOUN);
	    }
	    if (OptUseCF &&
		(check_feature(t_ptr->f, "用言") || /* 準用言はとりあえず対象外 */
		 (check_feature(t_ptr->f, "非用言格解析") && /* サ変名詞, 形容詞語幹 (確率的以外) */
		  (!(OptCaseFlag & OPT_CASE_USE_PROBABILITY) ||
		   (OptEllipsis & OPT_ELLIPSIS) || OptAnaphora ||
		   (t_ptr->inum == 1 && /* 確率的の場合は、「公開予定だ」のようなときのみ */
		    check_feature(t_ptr->b_ptr->f, "タグ単位受:-1")))))) { 
		
		set_pred_voice((BNST_DATA *)t_ptr); /* ヴォイス */

		make_caseframes(sp, t_ptr, CF_PRED);
		t_ptr->e_cf_num = t_ptr->cf_num;
	    }
	}
	else {
	    t_ptr->cf_ptr = NULL;
	}
    }

    /* 各タグ単位から格フレームへのリンク付け */
    start = 0;
    for (i = 0, t_ptr = sp->tag_data; i < sp->Tag_num; i++, t_ptr++) {
	if (t_ptr->cf_num) {
	    t_ptr->cf_ptr = Case_frame_array + start;
	    t_ptr->pred_num = pred_num++;

	    /* 表記がひらがなの場合: 
	       格フレームの表記がひらがなの場合が多ければひらがなの格フレームのみを対象に、
	       ひらがな以外が多ければひらがな以外のみを対象にするためのfeatureを付与 */
	    if (!(OptCaseFlag & OPT_CASE_USE_REP_CF) && /* 代表表記ではない場合のみ */
		check_str_type(t_ptr->head_ptr->Goi) == TYPE_HIRAGANA) {
		hiragana_count = 0;
		for (j = 0; j < t_ptr->cf_num; j++) {
		    if (check_str_type((t_ptr->cf_ptr + j)->entry) == TYPE_HIRAGANA) {
			hiragana_count++;
		    }
		}
		if (2 * hiragana_count > t_ptr->cf_num) {
		    assign_cfeature(&(t_ptr->f), "代表ひらがな", FALSE);
		}
	    }

	    start += t_ptr->cf_num;
	}
    }
}

/*==================================================================*/
		       void clear_cf(int flag)
/*==================================================================*/
{
    int i, j, k, end;

    end = flag ? MAX_Case_frame_num : Case_frame_num;

    for (i = 0; i < end; i++) {
	for (j = 0; j < CF_ELEMENT_MAX; j++) {
	    clear_cf_element(Case_frame_array + i, j);
	}
	if ((Case_frame_array+i)->entry) {
	    free((Case_frame_array+i)->entry);
	    (Case_frame_array+i)->entry = NULL;
	}
	if ((Case_frame_array+i)->feature) {
	    free((Case_frame_array+i)->feature);
	    (Case_frame_array+i)->feature = NULL;
	}
    }
}

/*==================================================================*/
	     int check_cf_case(CASE_FRAME *cfp, char *pp)
/*==================================================================*/
{
    int i;
    for (i = 0; i < cfp->element_num; i++) {
	if (MatchPP2(cfp->pp[i], pp)) {
	    return i;
	}
    }
    return -1;
}

/*==================================================================*/
		    char *malloc_db_buf(int size)
/*==================================================================*/
{
    if (db_buf_size == 0) {
	db_buf_size = DATA_LEN;
	db_buf = (char *)malloc_data(db_buf_size, "malloc_db_buf");
    }

    while (db_buf_size < size) {
	db_buf = (char *)realloc_data(db_buf, db_buf_size <<= 1, "malloc_db_buf");
    }

    return db_buf;
}

/*==================================================================*/
	    float get_cfs_similarity(char *cf1, char *cf2)
/*==================================================================*/
{
    char *key, *value;
    char *verb1, *verb2;
    float ret;
    int id1, id2;

    if (CFSimExist == FALSE || cf1 == NULL || cf2 == NULL) {
	return 0;
    }

    /* 同じとき */
    if (!strcmp(cf1, cf2)) {
	return 1.0;
    }

    verb1 = strdup(cf1);
    verb2 = strdup(cf2);
    sscanf(cf1, "%[^0-9]%d", verb1, &id1);
    sscanf(cf2, "%[^0-9]%d", verb2, &id2);

    key = (char *)malloc_data(sizeof(char) * (strlen(cf1) + strlen(cf2) + 2), 
			      "get_cfs_similarity");

    /* 前側のidが小さくなるようにkeyを生成 */
    if (id1 > id2) {
	sprintf(key, "%s%d-%s%d", verb2, id2, verb1, id1);
    }
    else {
	sprintf(key, "%s%d-%s%d", verb1, id1, verb2, id2);
    }

    value = db_get(cf_sim_db, key);
    if (value) {
	    ret = atof(value);
	    free(value);
    }
    free(key);
    free(verb1);
    free(verb2);

    return ret;
}

/*==================================================================*/
     double get_cf_probability(CASE_FRAME *cfd, CASE_FRAME *cfp)
/*==================================================================*/
{
    /* 格フレーム選択確率 P(食べる:動2|食べる:動)
       KNP格解析結果 (cfp.prob) */

    char *vtype, *key, *value, voice[3];
    double ret;
    int num;
    TAG_DATA *tp = cfd->pred_b_ptr;

    if (CfpExist == FALSE) {
	return 0;
    }

    if ((vtype = check_feature(tp->f, "用言"))) {
	vtype += strlen("用言:");
    }
    else if ((vtype = check_feature(tp->f, "非用言格解析"))) {
	vtype += strlen("非用言格解析:");
    }
    else {
	return UNKNOWN_CF_SCORE;
    }

    /* 用言表記は格フレームIDから抽出しない */

    key = malloc_db_buf(strlen(cfp->cf_id) + strlen(tp->head_ptr->Goi) + 8);
    if ((num = sscanf(cfp->cf_id, "%*[^:]:%*[^:]:%[PC]%*d", voice)) == 1) {
	sprintf(key, "%s|%s:%s:%s", cfp->cf_id, tp->head_ptr->Goi, vtype, voice);
    }
    else {
	sprintf(key, "%s|%s:%s", cfp->cf_id, tp->head_ptr->Goi, vtype);
    }

    value = db_get(cfp_db, key);

    if (value) {
	ret = atof(value);
	if (VerboseLevel >= VERBOSE3) {
	    fprintf(Outfp, ";; (CF) %s: P(%s) = %lf\n", tp->head_ptr->Goi, key, ret);
	}
	free(value);
	ret = log(ret);
    }
    else {
	if (VerboseLevel >= VERBOSE3) {
	    fprintf(Outfp, ";; (CF) %s: P(%s) = 0\n", tp->head_ptr->Goi, key);
	}
	ret = UNKNOWN_CF_SCORE;
    }

    return ret;
}

/*==================================================================*/
double get_case_probability_for_pred(char *case_str, CASE_FRAME *cfp, int aflag)
/*==================================================================*/
{
    /* 格確率 P(ガ格○|食べる) */

    char *key, *value, *verb;
    double ret;

    if (CFCaseExist == FALSE) {
	return 0;
    }

    /* 用言表記 */
    verb = strdup(cfp->cf_id);
    sscanf(cfp->cf_id, "%[^0-9]%*d", verb);

    key = malloc_db_buf(strlen(case_str) + strlen(cfp->cf_id) + 2);

    /* 用言表記でやった方がよいみたい */
    sprintf(key, "%s|%s", case_str, verb); /* cfp->cf_id); */
    value = db_get(cf_case_db, key);
    free(verb);

    if (value) {
	ret = atof(value);
	if (VerboseLevel >= VERBOSE2) {
	    fprintf(Outfp, ";; (C) P(%s) = %lf\n", key, ret);
	}
	free(value);
	if (aflag == FALSE) {
	    ret = 1 - ret;
	}
	if (ret == 0) {
	    ret = UNKNOWN_CASE_SCORE;
	}
	else {
	    ret = log(ret);
	}
    }
    else {
	if (VerboseLevel >= VERBOSE2) {
	    fprintf(Outfp, ";; (C) P(%s) = 0\n", key);
	}
	if (aflag == FALSE) {
	    ret = 0;
	}
	else {
	    ret = UNKNOWN_CASE_SCORE;
	}
    }

    return ret;
}

/*==================================================================*/
   double get_case_probability(int as2, CASE_FRAME *cfp, int aflag)
/*==================================================================*/
{
    /* 格確率 P(ガ格○|食べる:動2)
       KNP格解析結果から計算 (cfcases.prob) */

    char *key, *value, *verb;
    double ret;

    if (CFCaseExist == FALSE) {
	return 0;
    }

    /* 用言表記 */
    verb = strdup(cfp->cf_id);
    sscanf(cfp->cf_id, "%[^0-9]%*d", verb);

    key = malloc_db_buf(strlen(pp_code_to_kstr(cfp->pp[as2][0])) + 
			strlen(cfp->cf_id) + 2);

    /* 用言表記でやった方がよいみたい */
    sprintf(key, "%s|%s", pp_code_to_kstr(cfp->pp[as2][0]), verb); /* cfp->cf_id); */
    value = db_get(cf_case_db, key);

    /* if (!value) { * back-off *
	sprintf(key, "%s|%s", pp_code_to_kstr(cfp->pp[as2][0]), verb);
	value = db_get(cf_case_db, key);
	} */
    free(verb);

    if (value) {
	ret = atof(value);
	if (VerboseLevel >= VERBOSE3) {
	    fprintf(Outfp, ";; (C) P(%s) = %lf\n", key, ret);
	}
	free(value);
	if (aflag == FALSE) {
	    ret = 1 - ret;
	}
	if (ret == 0) {
	    ret = UNKNOWN_CASE_SCORE;
	}
	else {
	    ret = log(ret);
	}
    }
    else {
	if (VerboseLevel >= VERBOSE3) {
	    fprintf(Outfp, ";; (C) P(%s) = 0\n", key);
	}
	if (aflag == FALSE) {
	    ret = 0;
	}
	else {
	    ret = UNKNOWN_CASE_SCORE;
	}
    }

    return ret;
}

/*==================================================================*/
      double get_case_num_probability(CASE_FRAME *cfp, int num)
/*==================================================================*/
{
    /* 格の個数確率 P(2|食べる:動2)
       KNP格解析結果から計算 (cfcases.prob) */

    char *key, *value, *verb;
    double ret;

    if (CFCaseExist == FALSE) {
	return 0;
    }

    /* 用言表記 */
    verb = strdup(cfp->cf_id);
    sscanf(cfp->cf_id, "%[^0-9]%*d", verb);

    key = malloc_db_buf(strlen(cfp->cf_id) + 6);

    sprintf(key, "%d|N:%s", num, verb); /* cfp->cf_id */
    value = db_get(cf_case_db, key);

    /* if (!value) {
	sprintf(key, "%d|N:%s", num, verb);
	value = db_get(cf_case_db, key);
	} */
    free(verb);

    if (value) {
	ret = atof(value);
	if (VerboseLevel >= VERBOSE3) {
	    fprintf(Outfp, ";; (C) P(%s) = %lf\n", key, ret);
	}
	free(value);
	ret = log(ret);
    }
    else {
	if (VerboseLevel >= VERBOSE3) {
	    fprintf(Outfp, ";; (C) P(%s) = 0\n", key);
	}
	ret = UNKNOWN_CASE_SCORE;
    }

    return ret;
}

/*==================================================================*/
double _get_ex_category_probability(char *key, int as2, CASE_FRAME *cfp, FEATURE *fp)
/*==================================================================*/
{
    /* カテゴリ-用例確率 
       P(リンゴ/りんご|人工物-食べ物)*P(人工物-食べ物|食べる:動2:ガ格) */

    int i;
    double ret = 0, prob;
    char category[SMALL_DATA_LEN], *ex_category, *value;
    
    for (i = 0; i < cfp->gex_num[as2]; i++) {
	
	/* 格フレームに含まれているカテゴリ情報を抽出 */
	if (!strncmp(cfp->gex_list[as2][i], "CT", 2)) {
	    strcpy(category, cfp->gex_list[as2][i] + 3);
	    if (strchr(category, '>')) *strchr(category, '>') = '\0';
	    
	    /* 該当するカテゴリが対象の語のfeatureに含まれていた場合 */
	    if (check_category(fp, category)) {
		ex_category = (char *)malloc_data(sizeof(char) * (strlen(key) + strlen(category) + 5), 
						  "get_ex_category_probability");
		sprintf(ex_category, "%s|CT:%s", key, category);
		if ((value = db_get(case_db, ex_category))) {
		    if (VerboseLevel >= VERBOSE3) {
			fprintf(Outfp, ";; (EX-CATEGORY)%s %f %f\n", 
				ex_category, atof(value), cfp->gex_freq[as2][i]);
		    }
		    prob = atof(value) * cfp->gex_freq[as2][i];
		    if (ret < prob) ret = prob;
		    free(value);
		}
    		free(ex_category);		
	    }
	}	    
    }
    return ret;
}
	
/*==================================================================*/
double get_ex_ne_probability(char *cp, int as2, CASE_FRAME *cfp, int flag)
/*==================================================================*/
{
    /* 固有表現-用例確率 
       P(京大|LOCATION)*P(LOCATION|行く:動2:ニ格) */
    /* 入力key=NE:LOCATION:京大 */
    /* flagがたっている場合P(LOCATION|行く:動2:ニ格)を返す */
    /* またflagがたっている場合は、固有表現以外でも使用可 */

    int i;
    char *value, ne[SMALL_DATA_LEN], key[WORD_LEN_MAX*2];
    
    strcpy(ne, cp);
    strcpy(key, strchr(ne + 3, ':') + 1); /* key = 京大 */
    *strchr(ne + 3, ':') = '\0'; /* ne = NE:LOCATION */

    for (i = 0; i < cfp->gex_num[as2]; i++) {
	if (!strcmp(ne, cfp->gex_list[as2][i])) {

	    if (flag) {
		return cfp->gex_freq[as2][i];
	    }	
	    strcat(key, "|");
	    strcat(key, ne);
		
	    if ((value = db_get(case_db, key))) {
		if (VerboseLevel >= VERBOSE3) {
		    fprintf(Outfp, ";; (EX-NE)%s %f %f\n", 
			    key, atof(value), cfp->gex_freq[as2][i]);
		}
		return atof(value) * cfp->gex_freq[as2][i];
	    }
	}	
    }
    return 0;
}

/*==================================================================*/
double _get_ex_probability_internal(char *key, int as2, CASE_FRAME *cfp)
/*==================================================================*/
{
    int i;
    double ret = 0;

    for (i = 0; i < cfp->ex_num[as2]; i++) {
	if (!strcmp(key, cfp->ex_list[as2][i])) {
	    ret = (double)cfp->ex_freq[as2][i] / cfp->freq[as2];
	    if (VerboseLevel >= VERBOSE3) {
		fprintf(Outfp, ";; (EX) P(%s) = %lf\n", key, ret);
	    }
	    return ret;
	}
    }

    if (VerboseLevel >= VERBOSE3) {
	fprintf(Outfp, ";; (EX) P(%s) = 0\n", key);
    }
    return 0;
}

/*==================================================================*/
		double _get_ex_probability(char *key)
/*==================================================================*/
{
    char *value;
    double ret = 0;

    if (CFExExist == FALSE) {
	return 0;
    }

    value = db_get(cf_ex_db, key);

    if (value) {
	ret = exp(-1 * atof(value));
	if (VerboseLevel >= VERBOSE3) {
	    fprintf(Outfp, ";; P(%s) = %lf\n", key, ret);
	}
	free(value);
    }
    else {
	if (VerboseLevel >= VERBOSE3) {
	    fprintf(Outfp, ";; P(%s) = 0\n", key);
	}
    }

    return ret;
}

/*==================================================================*/
  double _get_sm_probability(TAG_DATA *dp, int as2, CASE_FRAME *cfp)
/*==================================================================*/
{
    int i;
    double ret, max = 0;
    char *key, *sm_code = dp->SM_code;
    char code[SM_CODE_SIZE + 1];
    code[SM_CODE_SIZE] = '\0';

    key = malloc_db_buf(SM_CODE_SIZE + 
			strlen(cfp->cf_id) + strlen(pp_code_to_kstr(cfp->pp[as2][0])) + 3);

    /* 各意味素ごとに調べて、maxをとる */
    for (i = 0; *(sm_code + i); i += SM_CODE_SIZE) {
	strncpy(code, sm_code + i, SM_CODE_SIZE);
	code[0] = '1';
	sprintf(key, "%s|%s,%s", 
		code, 
		cfp->cf_id, pp_code_to_kstr(cfp->pp[as2][0]));
	ret = _get_ex_probability(key);
	if (ret > max) {
	    max = ret;
	}
    }

    return max;
}

/*==================================================================*/
double _get_soto_default_probability(TAG_DATA *dp, int as2, CASE_FRAME *cfp)
/*==================================================================*/
{
    int i;
    double ret, max = 0;
    char *key, *sm_code = dp->SM_code;
    char code[SM_CODE_SIZE + 1];
    code[SM_CODE_SIZE] = '\0';

    if (CFExExist == FALSE) {
	return 0;
    }

    if (strlen(dp->head_ptr->Goi) > SM_CODE_SIZE) {
	key = malloc_db_buf(strlen(dp->head_ptr->Goi) + 18);
    }
    else {
	key = malloc_db_buf(SM_CODE_SIZE + 18);
    }

    /* 表記でsearch */
    sprintf(key, "%s|DEFAULT,外の関係", dp->head_ptr->Goi);
    if (ret = _get_ex_probability(key)) {
	return ret;
    }

    /* 意味素でsearch: maxをとる */
    for (i = 0; *(sm_code + i); i += SM_CODE_SIZE) {
	strncpy(code, sm_code + i, SM_CODE_SIZE);
	code[0] = '1';
	sprintf(key, "%s|DEFAULT,外の関係", code);
	if (ret = _get_ex_probability(key) && ret > max) {
	    max = ret;
	}
    }

    return max;
}

/*==================================================================*/
  double get_ex_probability(int as1, CASE_FRAME *cfd, TAG_DATA *dp,
			    int as2, CASE_FRAME *cfp, int sm_flag)
/*==================================================================*/
{
    /* 用例確率 P(弁当|食べる:動2,ヲ格)
       格フレームから計算 (cfex.prob) */

    char *key = NULL, *mrph_str, *cp, ne_prob[WORD_LEN_MAX*2], *value;
    double ret = FREQ0_ASSINED_SCORE, prob;
    int rep_malloc_flag = 0;
    
    /* dpの指定がなければ、as1とcfdから作る */
    if (dp == NULL) {
	dp = cfd->pred_b_ptr->cpm_ptr->elem_b_ptr[as1];
    }

    key = malloc_db_buf(strlen("<補文>") + strlen(cfp->cf_id) + strlen(pp_code_to_kstr(cfp->pp[as2][0])) + 3);
    *key = '\0';
    
    if (check_feature(dp->f, "補文")) {
	sprintf(key, "<補文>");
    }
    else if (sm_flag && /* FALSEの場合は主体を使わない */
	     (OptCaseFlag & OPT_CASE_GENERALIZE_AGENT) && /* 主体を汎化する場合(default) */
	     dat_match_sm(as1, cfd, dp, "主体")) {
	sprintf(key, "<主体>");
    }
    else if (check_feature(dp->f, "時間")) {
	sprintf(key, "<時間>");
    }
    else if (check_feature(dp->f, "数量")) {
	sprintf(key, "<数量>");
    }
    
    if (*key) {
	/* if (ret = _get_ex_probability(key)) { */
	if (prob = _get_ex_probability_internal(key, as2, cfp)) {
	    if (ret < log(prob)) ret = log(prob);
	}
    }
    	
    if (OptCaseFlag & OPT_CASE_USE_REP_CF) {
	if ((OptCaseFlag & OPT_CASE_USE_CREP_CF) && /* 正規化(主辞)代表表記 */
	    (cp = get_bnst_head_canonical_rep(dp->b_ptr, OptCaseFlag & OPT_CASE_USE_CN_CF))) {
	    mrph_str = strdup(cp);
	    rep_malloc_flag = 1;
	}
	else {
	    mrph_str = get_mrph_rep_from_f(dp->head_ptr, FALSE);
	    if (mrph_str == NULL) {
		mrph_str = make_mrph_rn(dp->head_ptr);
		rep_malloc_flag = 1;
	    }
	}
    }
    
    else {
	mrph_str = dp->head_ptr->Goi;
    }
    
    key = malloc_db_buf(strlen(mrph_str) + 
			strlen(cfp->cf_id) + strlen(pp_code_to_kstr(cfp->pp[as2][0])) + 3);
    sprintf(key, "%s", mrph_str);
    /* sprintf(key, "%s|%s,%s", dp->head_ptr->Goi, 
       cfp->cf_id, pp_code_to_kstr(cfp->pp[as2][0])); */
    if (rep_malloc_flag) {
	free(mrph_str);
	rep_malloc_flag = 0;
    }

    /* if (ret = _get_ex_probability(key)) { */
    if (prob = _get_ex_probability_internal(key, as2, cfp)) {
	if (ret < log(prob)) ret = log(prob);
    }
    /* 代表表記の場合はALTも調べる */
    else if (OptCaseFlag & OPT_CASE_USE_REP_CF) {
	int rep_length;
	FEATURE *fp = dp->head_ptr->f;
	MRPH_DATA m;

	while (fp) {
	    if (!strncmp(fp->cp, "ALT-", 4)) {
		sscanf(fp->cp + 4, "%[^-]-%[^-]-%[^-]-%d-%d-%d-%d-%[^\n]", 
		       m.Goi2, m.Yomi, m.Goi, 
		       &m.Hinshi, &m.Bunrui, 
		       &m.Katuyou_Kata, &m.Katuyou_Kei, m.Imi);
		mrph_str = get_mrph_rep(&m); /* 代表表記 */
		rep_length = get_mrph_rep_length(mrph_str);
		if (rep_length == 0) { /* なければ作る */
		    mrph_str = make_mrph_rn(&m);
		    rep_length = strlen(mrph_str);
		    rep_malloc_flag = 1;
		}

		key = malloc_db_buf(rep_length + 1);
		strncpy(key, mrph_str, rep_length);
		*(key + rep_length) = '\0';
		if (rep_malloc_flag) {
		    free(mrph_str);
		    rep_malloc_flag = 0;
		}

		if (prob = _get_ex_probability_internal(key, as2, cfp)) {
		    if (ret < log(prob)) ret = log(prob);
		}
	    }
	    fp = fp->next;
	}
    }
    if (!sm_flag) return ret;

    /* 固有表現の場合 */       
    if ((OptGeneralCF & OPT_CF_NE) && (cp = check_feature(dp->f, "NE"))) {
	if (prob = get_ex_ne_probability(cp, as2, cfp, FALSE)) {
	    if (ret < log(prob)) ret = log(prob);
	}
    } 

    /* 代表表記を用いる場合はカテゴリを参照する */
    if ((OptGeneralCF & OPT_CF_CATEGORY) && OptCaseFlag & OPT_CASE_USE_REP_CF) {
	mrph_str = get_mrph_rep_from_f(dp->head_ptr, FALSE);
	if (mrph_str == NULL) {
	    mrph_str = make_mrph_rn(dp->head_ptr);
	    rep_malloc_flag = 1;
	}
	key = malloc_db_buf(strlen(mrph_str) + 
			    strlen(cfp->cf_id) + strlen(pp_code_to_kstr(cfp->pp[as2][0])) + 3);
	sprintf(key, "%s", mrph_str);

	if (prob = _get_ex_category_probability(key, as2, cfp, dp->head_ptr->f)) {
	    if (ret < log(prob)) ret = log(prob);
	}
	if (rep_malloc_flag) {
	    free(mrph_str);
	    rep_malloc_flag = 0;
	}
    }

    /* else if (ret = _get_sm_probability(dp, as2, cfp)) { * 意味素にback-off *
	ret = log(ret);
    } */
    /* else if (MatchPP(cfp->pp[as2][0], "外の関係") && 
	     (ret = _get_soto_default_probability(dp, as2, cfp))) {
	ret = log(ret);
    } */
    return ret;
}

/*==================================================================*/
    double get_ex_probability_with_para(int as1, CASE_FRAME *cfd,
					int as2, CASE_FRAME *cfp)
/*==================================================================*/
{
    int j, count = 1, np_modifying_flag;
    TAG_DATA *tp = cfd->pred_b_ptr->cpm_ptr->elem_b_ptr[as1];
    double score;

    if (cfd->pred_b_ptr->num < tp->num) { /* 連体修飾 */
	np_modifying_flag = 1;
    }
    else {
	np_modifying_flag = 0;
    }

    /* 自分自身 */
    score = get_ex_probability(as1, cfd, NULL, as2, cfp, TRUE);

    if (OptParaNoFixFlag & OPT_PARA_GENERATE_SIMILARITY) { /* 類似度生成モデルのときは、並列要素を述語から生成しない */
	return score;
    }

    /* 並列の要素 */
    while (tp->next) {
	score += get_ex_probability(-1, cfd, tp->next, as2, cfp, TRUE);
	count++;
	tp = tp->next;
    }

    /* 自分と並列の要素 *
    if (tp->para_top_p) {
	for (j = 1; tp->child[j]; j++) { * 0は自分と同じ *
	    if (tp->child[j]->para_type == PARA_NORMAL) {
		score += get_ex_probability(-1, cfd, tp->child[j], as2, cfp);
		count++;
	    }
	}
    } */

    if (VerboseLevel >= VERBOSE3) {
	fprintf(Outfp, ";; (EX) is divided by %d => %.5f\n", count, score / count);
    }

    /* 並列確率的解析時: 並列要素間生成する場合と被連体修飾名詞は正規化
       並列決定的解析時: 常に正規化 */
    if (OptParaFix == TRUE || 
	((OptParaNoFixFlag & OPT_PARA_MULTIPLY_ALL_EX) || np_modifying_flag)) {
	return score / count;
    }
    else {
	return score;
    }
}

/*==================================================================*/
    double get_np_modifying_probability(int as1, CASE_FRAME *cfd)
/*==================================================================*/
{
    int dist = 0;
    char *type = NULL, *key, *value;
    double ret;

    if (CaseExist == FALSE) {
	return 0;
    }

    /* tp -> hp */
    if (cfd->pred_b_ptr->cpm_ptr->elem_b_ptr[as1]->num > cfd->pred_b_ptr->num) { /* 連体修飾 */
	if (type = check_feature(cfd->pred_b_ptr->f, "用言")) {
	    type += strlen("用言:");
	}

	/* 候補チェック */
	dist = get_dist_from_work_mgr(cfd->pred_b_ptr->b_ptr, 
				      cfd->pred_b_ptr->cpm_ptr->elem_b_ptr[as1]->b_ptr);
	if (dist <= 0) {
	    return UNKNOWN_CASE_SCORE;
	}
	else if (dist > 1) {
	    dist = 2;
	}
    }

    key = malloc_db_buf(10);
    sprintf(key, "%s,%d|R", type ? type : "NIL", dist);
    if (value = db_get(case_db, key)) {
	if (VerboseLevel >= VERBOSE3) {
	    fprintf(Outfp, ";; (RE) %s -> %s: P(%s,%d|R) = %s\n", 
		    type ? cfd->pred_b_ptr->head_ptr->Goi : "NIL", 
		    cfd->pred_b_ptr->cpm_ptr->elem_b_ptr[as1]->head_ptr->Goi, 
		    type ? type : "NIL", dist, value);
	}

	ret = log(atof(value));
	free(value);
    }
    else {
	ret = UNKNOWN_CASE_SCORE;
    }

    return ret;
}

/*==================================================================*/
double get_topic_generating_probability(int have_topic, TAG_DATA *g_ptr)
/*==================================================================*/
{
    int topic_score = 0;
    char *cp, *key, *value;
    double ret;

    if (CaseExist == FALSE) {
	return 0;
    }

    /* 提題スコア */
    if (cp = check_feature(g_ptr->f, "提題受")) {
	sscanf(cp, "%*[^:]:%d", &topic_score);
	if (topic_score > 0 && topic_score < 30) {
	    topic_score = 10;
	}
    }

    key = malloc_db_buf(7);
    sprintf(key, "%d|W:%d", have_topic, topic_score);
    if (value = db_get(case_db, key)) {
	if (VerboseLevel >= VERBOSE3) {
	    fprintf(Outfp, ";; (W) %s: P(%d|W:%d) = %s\n", 
		    g_ptr->head_ptr->Goi, have_topic, 
		    topic_score, value);
	}

	ret = log(atof(value));
	free(value);
    }
    else {
	ret = UNKNOWN_CASE_SCORE;
    }

    return ret;
}

/*==================================================================*/
	double get_key_probability(TAG_DATA *tag_ptr)
/*==================================================================*/
{
    char *key = NULL, *mrph_str, *cp;
    int rep_malloc_flag = 0;
    
    key = malloc_db_buf(strlen("<補文>") + 3);
    *key = '\0';
    
    if (check_feature(tag_ptr->f, "補文")) {
	sprintf(key, "<補文>");
    }
    else if (check_feature(tag_ptr->f, "時間")) {
	sprintf(key, "<時間>");
    }
    else if (check_feature(tag_ptr->f, "数量")) {
	sprintf(key, "<数量>");
    }

    if (*key) {
	return get_general_probability(key, "KEY");
    }

    if (OptCaseFlag & OPT_CASE_USE_REP_CF) {
	if ((OptCaseFlag & OPT_CASE_USE_CREP_CF) && /* 正規化(主辞)代表表記 */
	    (cp = get_bnst_head_canonical_rep(tag_ptr->b_ptr, OptCaseFlag & OPT_CASE_USE_CN_CF))) {
	    mrph_str = strdup(cp);
	    rep_malloc_flag = 1;
	}
	else {
	    mrph_str = get_mrph_rep_from_f(tag_ptr->head_ptr, FALSE);
	    if (mrph_str == NULL) {
		mrph_str = make_mrph_rn(tag_ptr->head_ptr);
		rep_malloc_flag = 1;
	    }
	}
    }

    else {
	mrph_str = tag_ptr->head_ptr->Goi;
    }

    key = malloc_db_buf(strlen(mrph_str) + 3);
    sprintf(key, "%s", mrph_str);
    if (rep_malloc_flag) {
	free(mrph_str);
	rep_malloc_flag = 0;
    }
    return get_general_probability(key, "KEY");
}

/*==================================================================*/
	double get_general_probability(char *key1, char *key2)
/*==================================================================*/
{
    char *value, *key;
    double ret;

    //return FREQ0_ASSINED_SCORE / 2;

    key = malloc_db_buf(strlen(key1) + strlen(key2) + 2);
    sprintf(key, "%s|%s", key1, key2);
    value = db_get(case_db, key);

    if (value) {
	ret = atof(value);
	ret = log(ret);
	free(value);
	return ret;
    }
    else {
	return FREQ0_ASSINED_SCORE;
    }
}

/*==================================================================*/
double get_case_interpret_probability(char *scase, char *cfcase, int ellipsis_flag)
/*==================================================================*/
{
    /* ellipsis_flag = 1 の場合は省略用 */

    char *value, *key;
    double ret;

    if (CaseExist == FALSE) {
	return 0;
    }
    
    key = malloc_db_buf(strlen(scase) + strlen(cfcase) + 20);
    if (ellipsis_flag) 
	sprintf(key, "%s|O:%s", scase, cfcase);
    else
	sprintf(key, "%s|C:%s", scase, cfcase);
    value = db_get(case_db, key);

    if (value) {
	ret = atof(value);
	ret = log(ret);
	free(value);
	return ret;
    }
    else {
	return UNKNOWN_CASE_SCORE;
    }
}

/*==================================================================*/
double get_case_surface_probability(char *scase, char *cfcase, int ellipsis_flag)
/*==================================================================*/
{
    /* ellipsis_flag = 1 の場合は省略用 */

    char *value, *key;
    double ret;

    if (CaseExist == FALSE) {
	return 0;
    }
    
    key = malloc_db_buf(strlen(scase) + strlen(cfcase) + 20);
    if (ellipsis_flag) 
	sprintf(key, "%s|O:%s", scase, cfcase);
    else
	sprintf(key, "%s|C:%s", scase, cfcase);
    value = db_get(case_db, key);

    if (value) {
	ret = atof(value);
	ret = log(ret);
	free(value);
	return ret;
    }
    else {
	return UNKNOWN_CASE_SCORE;
    }
}

/*==================================================================*/
double get_punctuation_generating_probability(int np_modifying_flag, int touten_flag, int dist, 
					      int closest_pred_flag, int topic_score, int wa_flag)
/*==================================================================*/
{
    /* 読点の生成 */

    char *value, *key;
    double ret;

    if (CaseExist == FALSE) {
	return 0;
    }

    key = malloc_db_buf(20);
    if (np_modifying_flag) {
	sprintf(key, "%d|P連格:%d", touten_flag, dist);
    }
    else {
	sprintf(key, "%d|P:%d,%d,%d,%d", touten_flag, dist, closest_pred_flag, topic_score, wa_flag);
    }
    value = db_get(case_db, key);

    if (value) {
	ret = atof(value);
	ret = log(ret);
	free(value);
	return ret;
    }
    else {
	return UNKNOWN_CASE_SCORE;
    }
}

/*==================================================================*/
double get_wa_generating_probability(int np_modifying_flag, int touten_flag, int dist, 
				     int closest_pred_flag, int topic_score, int wa_flag, 
				     int negation_flag, char *vtype)
/*==================================================================*/
{
    /* 「は」の生成 */

    char *value, *key;
    double ret;

    if (CaseExist == FALSE) {
	return 0;
    }

    if (np_modifying_flag || !vtype) {
	value = "1.0";
    }
    else {
	key = malloc_db_buf(22);

	/* sprintf(key, "%d|T:%d,%d,%d,%d,%d,%d", wa_flag, dist, closest_pred_flag, topic_score, touten_flag, negation_flag, strcmp(vtype, "判") == 0 ? 1 : 0); */
	sprintf(key, "%d|T:%d,%d,%d,%d,%d,%s", wa_flag, dist, closest_pred_flag, topic_score, touten_flag, negation_flag, vtype);
	value = db_get(case_db, key);
    }

    if (value) {
	ret = atof(value);
	ret = log(ret);

	if (!np_modifying_flag && vtype) {
	    free(value);
	}

	return ret;
    }
    else {
	return UNKNOWN_CASE_SCORE;
    }
}

/*==================================================================*/
	 char *make_cf_case_string(int as1, CASE_FRAME *cfd,
				   int as2, CASE_FRAME *cfp)
/*==================================================================*/
{
    /* 格フレームの格スロット表記をかえす */

    if (as2 != NIL_ASSIGNED) {
	return pp_code_to_kstr(cfp->pp[as2][0]);
    }
    else {
	/* 格スロットがあるのにNIL_ASSIGNED */
	if (CF_MatchPP(cfd->pp[as1][0], cfp)) {
	    return "--";
	}
	/* 格フレームがないとき、仮想的に格スロットを作ると考える */
	else {
	    return pp_code_to_kstr(cfd->pp[as1][0]);
	}
    }
}

/*==================================================================*/
     double get_case_function_probability(int as1, CASE_FRAME *cfd,
					  int as2, CASE_FRAME *cfp, 
					  int ellipsis_flag)
/*==================================================================*/
{
    int wa_flag, topic_score = 0, touten_flag, i, dist, negation_flag, 	np_modifying_flag, closest_pred_flag = 0;
    char *scase, *cfcase, *cp, *vtype;
    double score1, score2, score3;
    TAG_DATA *tp, *tp2, *hp;

    if (CaseExist == FALSE) {
	return 0;
    }

    /* tp -> hp */
    if (cfd->pred_b_ptr->cpm_ptr->elem_b_ptr[as1]->num > cfd->pred_b_ptr->num) { /* 連体修飾 */
	tp = cfd->pred_b_ptr;
	hp = cfd->pred_b_ptr->cpm_ptr->elem_b_ptr[as1];
	np_modifying_flag = 1;
    }
    else {
	tp = cfd->pred_b_ptr->cpm_ptr->elem_b_ptr[as1];
	hp = cfd->pred_b_ptr;
	np_modifying_flag = 0;
    }

    if (vtype = check_feature(hp->f, "用言")) {
	vtype += strlen("用言:");
    }

    /* 複合辞 */
    if (cfd->pp[as1][0] >= FUKUGOJI_START && cfd->pp[as1][0] <= FUKUGOJI_END) {
	scase = pp_code_to_kstr(cfd->pp[as1][0]); /* 入力側の表層格 */
	tp2 = &(current_sentence_data.tag_data[tp->num + 1]); /* 読点や「は」などをチェックするタグ単位 */
    }
    else {
	if (np_modifying_flag) {
	    scase = "連格";
	}
	else if ((scase = check_feature(tp->f, "係")) == NULL) {
	    return UNKNOWN_CASE_SCORE;
	}
	else {
	    scase += strlen("係:"); /* 入力側の表層格 */
	}
	tp2 = tp;
    }

    /* 隣に用言があるかどうか */
    if (np_modifying_flag == 0) {
	if (get_dist_from_work_mgr(tp2->b_ptr, current_sentence_data.tag_data[tp2->num + 1].b_ptr) > 0) {
	    closest_pred_flag = 1;
	}
    }

    /* 「は」, 読点, 否定のチェック */
    wa_flag = check_feature(tp2->f, "ハ") ? 1 : 0;
    touten_flag = check_feature(tp2->f, "読点") ? 1 : 0;
    negation_flag = check_feature(hp->f, "否定表現")   ? 1 
		  : check_feature(hp->f, "準否定表現") ? 1
		  : 0;

    /* 提題スコア */
    if (cp = check_feature(hp->f, "提題受")) {
	sscanf(cp, "%*[^:]:%d", &topic_score);
	if (topic_score > 0 && topic_score < 30) {
	    topic_score = 10;
	}
    }

    /* 候補チェック */
    dist = get_dist_from_work_mgr(tp2->b_ptr, hp->b_ptr);
    if (dist <= 0 && !ellipsis_flag) {
	return UNKNOWN_CASE_SCORE;
    }
    else if (dist > 1) {
	dist = 2;
    }

    /* 格の解釈 */
    cfcase = make_cf_case_string(as1, cfd, as2, cfp);
    score1 = get_case_interpret_probability(scase, cfcase, FALSE);
    if (ellipsis_flag) return score1;

    /* 読点の生成 */
    score2 = get_punctuation_generating_probability(np_modifying_flag, touten_flag, dist, 
						    closest_pred_flag, topic_score, wa_flag);

    /* 「は」の生成 */
    score3 = get_wa_generating_probability(np_modifying_flag, touten_flag, dist, 
					   closest_pred_flag, topic_score, wa_flag, negation_flag, vtype);

    if (VerboseLevel >= VERBOSE3) {
	fprintf(Outfp, ";; (CC) %s -> %s: P(%s,%d,%d|%s,%d,%d,%d,%d) = %lf (C:%lf * P:%lf * T:%lf)\n", 
		tp->head_ptr->Goi, hp->head_ptr->Goi, 
		scase, touten_flag, wa_flag, as2 == NIL_ASSIGNED ? "--" : pp_code_to_kstr(cfp->pp[as2][0]), 
		dist, closest_pred_flag, topic_score, negation_flag, score1 + score2 + score3, exp(score1), exp(score2), exp(score3));
    }

    return score1 + score2 + score3;
}

/*==================================================================*/
double calc_vp_modifying_probability(TAG_DATA *gp, CASE_FRAME *g_cf, TAG_DATA *dp, CASE_FRAME *d_cf)
/*==================================================================*/
{
    int touten_flag, dist, closest_pred_flag = 0;
    char *g_pred, *d_pred, *g_id, *d_id, *key, *value, *g_level;
    double ret1 = 0, ret2 = 0, ret3 = 0;

    /* EOS -> 文末 */
    if (gp == NULL) {
	g_pred = strdup("EOS");
    }
    else if (g_cf) {
	g_pred = strdup(g_cf->cf_id);
	sscanf(g_cf->cf_id, "%[^0-9]:%*d", g_pred);
    }
    else {
	g_pred = NULL;
    }

    /* 用言 -> 用言 */
    if (RenyouExist && g_pred && d_cf && 
	(gp != NULL || OptCaseFlag & OPT_CASE_GENERATE_EOS)) { /* 文末からの生成は-generate-eos(言語モデル用)時のみ */
	d_pred = strdup(d_cf->cf_id);
	sscanf(d_cf->cf_id, "%[^0-9]:%*d", d_pred);

	key = malloc_db_buf(strlen(g_pred) + strlen(d_pred) + 3);
	sprintf(key, "%s|%s", d_pred, g_pred);
	value = db_get(renyou_db, key);
	if (value) {
	    ret3 = atof(value);
	    free(value);
	}
	else {
	    sprintf(key, "NIL|%s", g_pred);
	    value = db_get(renyou_db, key);
	    if (value) {
		ret3 = atof(value);
		free(value);
	    }
	}

	if (VerboseLevel >= VERBOSE2) {
	    fprintf(Outfp, ";; (R) %s: P(%s|%s) = %lf\n", gp ? gp->head_ptr->Goi : "EOS", d_pred, g_pred, ret3);
	}

	free(g_pred);
	free(d_pred);
    }
    else {
	ret3 = 1;
    }

    if (gp == NULL) {
	if (ret3) {
	    return log(ret3);
	}
	else {
	    return UNKNOWN_RENYOU_SCORE;
	}
    }


    touten_flag = check_feature(dp->b_ptr->f, "読点") ? 1 : 0;

    if ((dist = get_dist_from_work_mgr(dp->b_ptr, gp->b_ptr)) < 0) {
	return UNKNOWN_RENYOU_SCORE;
    }

    if (get_dist_from_work_mgr(dp->b_ptr, dp->b_ptr + 1) > 0) {
	closest_pred_flag = 1;
    }

    /* 読点の生成 */
    key = malloc_db_buf(strlen("連用") + 8);
    sprintf(key, "%d|P連用:%d,%d", touten_flag, dist, closest_pred_flag);
    value = db_get(case_db, key);
    if (value) {
	ret1 = atof(value);
	if (VerboseLevel >= VERBOSE2) {
	    fprintf(Outfp, ";; (R_P) %s: P(%d|P連用:%d,%d) = %lf\n", gp->head_ptr->Goi, touten_flag, dist, closest_pred_flag, ret1);
	}
	free(value);
    }

    if (RenyouExist == FALSE) {
	if (ret1) {
	    return log(ret1);
	}
	else {
	    return UNKNOWN_RENYOU_SCORE;
	}
    }

    /* ID -> ID (未使用) */
    if (0 && (g_id = check_feature(gp->f, "ID")) && 
	(d_id = check_feature(dp->f, "ID"))) {
	g_id += 3;
	d_id += 3;
	key = malloc_db_buf(strlen(g_id) + strlen(d_id) + 3);
	sprintf(key, "%s|%s", d_id, g_id);
	value = db_get(renyou_db, key);
	if (value) {
	    ret2 = atof(value);
	    free(value);
	}
	else {
	    sprintf(key, "NIL|%s", g_id);
	    value = db_get(renyou_db, key);
	    if (value) {
		ret2 = atof(value);
		free(value);
	    }
	}

	if (VerboseLevel >= VERBOSE2) {
	    fprintf(Outfp, ";; (R) %s: P(%s|%s) = %lf\n", gp->head_ptr->Goi, d_id, g_id, ret2);
	}
    }
    else {
	ret2 = 1;
    }

    /* 格フレームID => 格フレームID *
    if (0 && g_cf && d_cf) {
	key = malloc_db_buf(strlen(g_cf->cf_id) + strlen(d_cf->cf_id) + 2);
	sprintf(key, "%s|%s", d_cf->cf_id, g_cf->cf_id);
	value = db_get(renyou_db, key);
	if (value) {
	    ret3 = atof(value);
	    if (VerboseLevel >= VERBOSE3) {
		fprintf(Outfp, ";; (R) %s: P(%s|%s) = %lf\n", gp->head_ptr->Goi, d_cf->cf_id, g_cf->cf_id, ret3);
	    }
	    free(value);
	}
    }
    else {
	ret3 = 1;
    }
    */

    /* レベル => レベル *
    if ((g_level = check_feature(gp->f, "レベル")) == NULL) {
	return UNKNOWN_RENYOU_SCORE;
    }
    g_level += 7;

    key = malloc_db_buf(strlen(g_level) + 6);
    sprintf(key, "%d|%d,%s", check_feature(dp->f, "読点") ? 1 : 0, dist, g_level);
    value = db_get(renyou_func_db, key);
    if (value) {
	ret3 = atof(value);
	if (VerboseLevel >= VERBOSE3) {
	    fprintf(Outfp, ";; (R) %s -> %s: P(,=%d|%d,%s) = %lf\n", dp->head_ptr->Goi, gp->head_ptr->Goi, 
		    check_feature(dp->f, "読点") ? 1 : 0, dist, g_level, ret3);
	}
	free(value);
    }
    ret3 = 1;
    */

    if (ret1) {
	ret1 = log(ret1);
    }
    else {
	ret1 = UNKNOWN_RENYOU_SCORE;
    }
    if (ret2) {
	ret2 = log(ret2);
    }
    else {
	ret2 = UNKNOWN_RENYOU_SCORE;
    }
    if (ret3) {
	ret3 = log(ret3);
    }
    else {
	ret3 = UNKNOWN_RENYOU_SCORE;
    }
    return ret1 + ret2 + ret3;
}

/*==================================================================*/
double calc_vp_modifying_num_probability(TAG_DATA *t_ptr, CASE_FRAME *cfp, int num)
/*==================================================================*/
{
    char *pred, *id, *key, *value;
    double ret1 = 0, ret2 = 0;

    /* 個数は未使用 */
    if (1 || RenyouExist == FALSE) {
	return 0;
    }

    /* ID */
    if (0 && (id = check_feature(t_ptr->f, "ID"))) {
	id += 3;
	key = malloc_db_buf(strlen(id) + 6);
	sprintf(key, "%d|N:%s", num, id);
	value = db_get(renyou_db, key);
	if (value) {
	    ret1 = atof(value);
	    if (VerboseLevel >= VERBOSE2) {
		fprintf(Outfp, ";; (RN) %s: P(%d|%s) = %lf\n", t_ptr->head_ptr->Goi, num, id, ret1);
	    }
	    free(value);
	}
    }
    else {
	ret1 = 1;
    }

    /* 用言 (未使用) */
    if (cfp) {
	pred = strdup(cfp->cf_id);
	sscanf(cfp->cf_id, "%[^0-9]:%*d", pred);
	key = malloc_db_buf(strlen(pred) + 6);
	sprintf(key, "%d|N:%s", num, pred);
	value = db_get(renyou_db, key);
	if (value) {
	    ret2 = atof(value);
	    free(value);
	}

	if (VerboseLevel >= VERBOSE2) {
	    fprintf(Outfp, ";; (RN) %s: P(%d|%s) = %lf\n", t_ptr->head_ptr->Goi, num, pred, ret2);
	}

	free(pred);
    }
    else {
	ret2 = 1;
    }

    /* 格フレーム *
    if (0 && cfp) {
	key = malloc_db_buf(strlen(cfp->cf_id) + 6);
	sprintf(key, "%d|N:%s", num, cfp->cf_id);
	value = db_get(renyou_db, key);
	if (value) {
	    ret2 = atof(value);
	    if (VerboseLevel >= VERBOSE3) {
		fprintf(Outfp, ";; (R) %s: P(%d|%s) = %lf\n", t_ptr->head_ptr->Goi, num, cfp->cf_id, ret2);
	    }
	    free(value);
	}
    }
    else {
	ret2 = 1;
    }
    */

    if (ret1 && ret2) {
	return log(ret1) + log(ret2);
    }
    else {
	return UNKNOWN_RENYOU_SCORE;
    }
}

/*==================================================================*/
double calc_adv_modifying_probability(TAG_DATA *gp, CASE_FRAME *cfp, TAG_DATA *dp)
/*==================================================================*/
{
    char *pred, *key, *value;
    int touten_flag, dist;
    double ret1, ret2;

    /* 副詞 -> 用言 */
    if (cfp) {
	if (AdverbExist == TRUE) {
	    pred = strdup(cfp->cf_id);
	    sscanf(cfp->cf_id, "%[^0-9]:%*d", pred);
	    key = malloc_db_buf(strlen(pred) + strlen(dp->head_ptr->Goi) + 3);
	    sprintf(key, "%s|%s", dp->head_ptr->Goi, pred);
	    value = db_get(adverb_db, key);
	    if (value) {
		ret1 = atof(value);
		free(value);
		if (VerboseLevel >= VERBOSE1) {
		    fprintf(Outfp, ";; (A) P(%s) = %lf\n", key, ret1);
		}
		ret1 = log(ret1);
	    }
	    else {
		sprintf(key, "NIL|%s", pred);
		value = db_get(adverb_db, key);
		if (value) {
		    ret1 = atof(value);
		    free(value);
		    if (VerboseLevel >= VERBOSE1) {
			fprintf(Outfp, ";; (A) P(%s) = %lf\n", key, ret1);
		    }
		    if (ret1 > 0) {
			ret1 = log(ret1);
		    }
		    else {
			ret1 = UNKNOWN_RENYOU_SCORE;
		    }
		}
		else {
		    if (VerboseLevel >= VERBOSE1) {
			fprintf(Outfp, ";; (A) P(%s) = 0\n", key);
		    }
		    ret1 = UNKNOWN_RENYOU_SCORE;
		}
	    }
	    free(pred);
	}
	else {
	    ret1 = 0;
	}

	/* 読点の生成 */
	touten_flag = check_feature(dp->b_ptr->f, "読点") ? 1 : 0;

	if ((dist = get_dist_from_work_mgr(dp->b_ptr, gp->b_ptr)) < 0) {
	    ret2 = UNKNOWN_RENYOU_SCORE;
	}
	else {
	    int closest_pred_flag = 0;

	    if (get_dist_from_work_mgr(dp->b_ptr, dp->b_ptr + 1) > 0) {
		closest_pred_flag = 1;
	    }
	    key = malloc_db_buf(strlen("副詞") + 8);
	    sprintf(key, "%d|P副詞:%d,%d", touten_flag, dist, closest_pred_flag);
	    value = db_get(case_db, key);
	    if (value) {
		ret2 = atof(value);
		if (VerboseLevel >= VERBOSE1) {
		    fprintf(Outfp, ";; (A_P) [%s -> %s] : P(%s) = %lf\n", dp->head_ptr->Goi, gp->head_ptr->Goi, key, ret2);
		}
		free(value);
		ret2 = log(ret2);
	    }
	    else {
		ret2 = UNKNOWN_RENYOU_SCORE;
		if (VerboseLevel >= VERBOSE1) {
		    fprintf(Outfp, ";; (A_P) [%s -> %s] : P(%s) = 0\n", dp->head_ptr->Goi, gp->head_ptr->Goi, key);
		}
	    }
	}
	return ret1 + ret2;
    }
    else {
	return 0;
    }
}

/*==================================================================*/
double calc_adv_modifying_num_probability(TAG_DATA *t_ptr, CASE_FRAME *cfp, int num)
/*==================================================================*/
{
    char *pred, *key, *value;
    double ret = 0;

    if (AdverbExist == FALSE) {
	return 0;
    }

    if (cfp) {
	pred = strdup(cfp->cf_id);
	sscanf(cfp->cf_id, "%[^0-9]:%*d", pred);
	key = malloc_db_buf(strlen(pred) + 6);
	sprintf(key, "%d|N:%s", num, pred);
	value = db_get(adverb_db, key);
	if (value) {
	    ret = atof(value);
	    free(value);
	}

	if (VerboseLevel >= VERBOSE2) {
	    fprintf(Outfp, ";; (AN) %s: P(%d|%s) = %lf\n", t_ptr->head_ptr->Goi, num, pred, ret);
	}

	free(pred);

	if (ret) {
	    return log(ret);
	}
	else {
	    return UNKNOWN_RENYOU_SCORE;
	}
    }
    else {
	return 0;
    }
}

/*==================================================================*/
		  int bin_sim_score(double score)
/*==================================================================*/
{
    if (score < 1.0) {
	return 0;
    }
    else if (score < 2.0) {
	return 1;
    }
    else if (score < 3.0) {
	return 2;
    }
    else if (score < 4.0) {
	return 3;
    }
    else {
	return 4;
    }
}

/*==================================================================*/
double get_para_exist_probability(char *para_key, double score, int flag, TAG_DATA *dp, TAG_DATA *gp)
/*==================================================================*/
{
    char *key, *value;
    double ret1 = 0, ret2 = 0;
    int binned_score = bin_sim_score(score), touten_flag, dist;

    if (CaseExist == FALSE) {
	return 0;
    }

    key = malloc_db_buf(strlen(para_key) + 12);
    if (flag) {
	if (OptParaNoFixFlag & OPT_PARA_GENERATE_SIMILARITY) {
	    sprintf(key, "1,%d|PARA:%s", binned_score, para_key);
	}
	else {
	    sprintf(key, "1|PARA:%s", para_key);
	}
    }
    else {
	sprintf(key, "0|PARA:%s", para_key);
    }
    value = db_get(case_db, key);
    if (value) {
	ret1 = atof(value);
	if (VerboseLevel >= VERBOSE2) {
	    fprintf(Outfp, ";; (PARA) : P(%s) = %lf\n", key, ret1);
	}
	free(value);
    }
    else {
	if (VerboseLevel >= VERBOSE2) {
	    fprintf(Outfp, ";; (PARA) : P(%s) = 0\n", key);
	}
    }

    /* 
    sprintf(key, "%s|PTYPE:%d", para_key, binned_score);
    value = db_get(case_db, key);
    if (value) {
	ret2 = atof(value);
	if (VerboseLevel >= VERBOSE2) {
	    fprintf(Outfp, ";; (PTYPE) : P(%s) = %lf\n", key, ret2);
	}
	free(value);
    }
    else {
	if (VerboseLevel >= VERBOSE2) {
	    fprintf(Outfp, ";; (PTYPE) : P(%s) = 0\n", key);
	}
    }
    */

    /* 名詞並列のときは読点を生成 */
    if (flag && !check_feature(dp->f, "用言")) {
	if (check_feature(dp->b_ptr->f, "読点") && 
	    !check_feature(dp->b_ptr->f, "読点並キ")) {
	    touten_flag = 1;
	}
	else {
	    touten_flag = 0;
	}

	if ((dist = get_dist_from_work_mgr(dp->b_ptr, gp->b_ptr)) < 0) {
	    ret2 = UNKNOWN_RENYOU_SCORE;
	}
	else {
	    key = malloc_db_buf(strlen("並列") + 6);
	    sprintf(key, "%d|P並列:%d", touten_flag, dist);
	    value = db_get(case_db, key);
	    if (value) {
		ret2 = atof(value);
		if (VerboseLevel >= VERBOSE2) {
		    fprintf(Outfp, ";; (PARA_P) [%s -> %s] : P(%s) = %lf\n", dp->head_ptr->Goi, gp->head_ptr->Goi, key, ret2);
		}
		free(value);
		ret2 = log(ret2);
	    }
	    else {
		ret2 = UNKNOWN_RENYOU_SCORE;
		if (VerboseLevel >= VERBOSE2) {
		    fprintf(Outfp, ";; (PARA_P) [%s -> %s] : P(%s) = 0\n", dp->head_ptr->Goi, gp->head_ptr->Goi, key);
		}
	    }
	}
    }

    if (ret1) {
	ret1 = log(ret1);
    }
    else {
	ret1 = UNKNOWN_RENYOU_SCORE;
    }
    /* if (ret2) {
	ret2 = log(ret2);
    }
    else {
	ret2 = UNKNOWN_RENYOU_SCORE;
	} */

    return ret1 + ret2;
}

/*==================================================================*/
double get_para_ex_probability(char *para_key, double score, TAG_DATA *dp, TAG_DATA *gp)
/*==================================================================*/
{
    char *key, *value;
    double ret;

    if (ParaExist == FALSE) {
	return 0;
    }

    if (!strcmp(dp->head_ptr->Goi, gp->head_ptr->Goi)) {
	return 0;
    }

    key = malloc_db_buf(strlen(dp->head_ptr->Goi) + strlen(para_key) + strlen(gp->head_ptr->Goi) + 5);

    if (OptParaNoFixFlag & OPT_PARA_GENERATE_SIMILARITY) {
	sprintf(key, "%s|%d,%s,%s", dp->head_ptr->Goi, bin_sim_score(score), para_key, gp->head_ptr->Goi);
    }
    else {
	sprintf(key, "%s|%s,%s", dp->head_ptr->Goi, para_key, gp->head_ptr->Goi);
    }

    value = db_get(para_db, key);
    if (value) {
	ret = atof(value);
	if (VerboseLevel >= VERBOSE2) {
	    fprintf(Outfp, ";; (PARA_EX) : P(%s) = %lf\n", key, ret);
	}
	free(value);
    }
    else {
	if (VerboseLevel >= VERBOSE2) {
	    fprintf(Outfp, ";; (PARA_EX) : P(%s) = 0\n", key);
	}
	return FREQ0_ASSINED_SCORE;
    }

    if (ret) {
	return log(ret);
    }
    else {
	return FREQ0_ASSINED_SCORE;
    }
}

/*==================================================================*/
double get_noun_co_ex_probability(TAG_DATA *dp, TAG_DATA *gp)
/*==================================================================*/
{
    char *key, *value;
    int touten_flag, dist, elem_num = 0, g_elem_num = 0;
    double ret1 = 0, ret2 = 0, tmp_ret;
    TAG_DATA *tmp_dp, *tmp_gp = gp;

    if (NounCoExist == FALSE) {
	return 0;
    }

    while (tmp_gp) {
	tmp_dp = dp;
	while (tmp_dp) {
	    if (tmp_dp->num > tmp_gp->num) {
		continue;
	    }

	    key = malloc_db_buf(strlen(tmp_dp->head_ptr->Goi) + strlen(tmp_gp->head_ptr->Goi) + 2);
	    sprintf(key, "%s|%s", tmp_dp->head_ptr->Goi, tmp_gp->head_ptr->Goi);

	    value = db_get(noun_co_db, key);
	    if (value) {
		tmp_ret = atof(value);
		if (VerboseLevel >= VERBOSE2) {
		    fprintf(Outfp, ";; (NOUN_EX) : P(%s) = %lf\n", key, tmp_ret);
		}
		free(value);
		ret1 += log(tmp_ret);
	    }
	    else {
		if (VerboseLevel >= VERBOSE2) {
		    fprintf(Outfp, ";; (NOUN_EX) : P(%s) = 0\n", key);
		}
		ret1 += FREQ0_ASSINED_SCORE;
	    }

	    elem_num++;
	    tmp_dp = tmp_dp->next;
	}
	g_elem_num++;
	tmp_gp = tmp_gp->next;
    }
    ret1 /= (OptParaNoFixFlag & OPT_PARA_MULTIPLY_ALL_EX) ? (double)elem_num : (double)g_elem_num; /* 非連体修飾名詞の方は常に正規化 */
    if (VerboseLevel >= VERBOSE2) {
	fprintf(Outfp, ";; (NOUN_EX) is divided by %d => %.5f\n", (OptParaNoFixFlag & OPT_PARA_MULTIPLY_ALL_EX) ? elem_num : g_elem_num, ret1);
    }

    /* 読点の生成
       係側が並列の場合は複数個 */

    elem_num = 0;
    tmp_dp = dp;
    while (tmp_dp) {
	touten_flag = check_feature(tmp_dp->b_ptr->f, "読点") ? 1 : 0;

	if ((dist = get_dist_from_work_mgr(tmp_dp->b_ptr, gp->b_ptr)) < 0) {
	    ret2 += FREQ0_ASSINED_SCORE;
	}
	else {
	    int closest_ok_flag = 0;

	    if (get_dist_from_work_mgr(tmp_dp->b_ptr, tmp_dp->b_ptr + 1) > 0) {
		closest_ok_flag = 1;
	    }
	    key = malloc_db_buf(strlen("連体") + 8);
	    sprintf(key, "%d|P連体:%d,%d", touten_flag, dist, closest_ok_flag);
	    value = db_get(case_db, key);
	    if (value) {
		tmp_ret += atof(value);
		if (VerboseLevel >= VERBOSE2) {
		    fprintf(Outfp, ";; (NOUN_N) [%s -> %s] : P(%s) = %lf\n", tmp_dp->head_ptr->Goi, gp->head_ptr->Goi, key, tmp_ret);
		}
		free(value);
		ret2 += log(tmp_ret);
	    }
	    else {
		ret2 += FREQ0_ASSINED_SCORE;
		if (VerboseLevel >= VERBOSE2) {
		    fprintf(Outfp, ";; (NOUN_N) [%s -> %s] : P(%s) = 0\n", tmp_dp->head_ptr->Goi, gp->head_ptr->Goi, key);
		}
	    }
	}

	elem_num++;
	tmp_dp = tmp_dp->next;
    }
    if (OptParaNoFixFlag & OPT_PARA_MULTIPLY_ALL_EX) {
	ret2 /= (double)elem_num;
	if (VerboseLevel >= VERBOSE1) {
	    fprintf(Outfp, ";; (NOUN_N) is divided by %d => %.5f\n", elem_num, ret2);
	}
    }

    return ret1 + ret2;
}

/*==================================================================*/
      double get_noun_co_num_probability(TAG_DATA *gp, int num)
/*==================================================================*/
{
    char *key, *value;
    double ret;

    if (NounCoExist == FALSE) {
	return 0;
    }

    key = malloc_db_buf(strlen(gp->head_ptr->Goi) + 6);
    sprintf(key, "%d|N:%s", num, gp->head_ptr->Goi);

    value = db_get(noun_co_db, key);
    if (value) {
	ret = atof(value);
	if (VerboseLevel >= VERBOSE2) {
	    fprintf(Outfp, ";; (NOUN_NUM) : P(%s) = %lf\n", key, ret);
	}
	free(value);
	ret = log(ret);
    }
    else {
	if (VerboseLevel >= VERBOSE2) {
	    fprintf(Outfp, ";; (NOUN_NUM) : P(%s) = 0\n", key);
	}
	ret = FREQ0_ASSINED_SCORE;
    }

    return ret;
}

/* get pa count for Chinese from gigaword */
/*==================================================================*/
   double get_chi_pa(BNST_DATA *ptr1, BNST_DATA *ptr2, int dist)
/*==================================================================*/
{
    char *key, *value;
    double ret;

    if (CHIPAExist == FALSE) {
	return 0;
    }

    key = malloc_db_buf(strlen(ptr1->head_ptr->Goi) + 
			strlen(ptr2->head_ptr->Goi) + 
			strlen(ptr1->head_ptr->Pos) + 
			strlen(ptr2->head_ptr->Pos) + 11);

    /* 用言表記でやった方がよいみたい */
    if (ptr1->num < ptr2->num) {
	sprintf(key, "%s_%s_%s_%s_R_%d", ptr1->head_ptr->Pos, ptr1->head_ptr->Goi, ptr2->head_ptr->Pos, ptr2->head_ptr->Goi, dist);
    }
    else {
	sprintf(key, "%s_%s_%s_%s_L_%d", ptr2->head_ptr->Pos, ptr2->head_ptr->Goi, ptr1->head_ptr->Pos, ptr1->head_ptr->Goi, dist);
    }

    value = db_get(chi_pa_db, key);

    if (value) {
	ret = atof(value);
	free(value);
    }
    else {
	ret = 0.0;
    }

    return ret;
}

/*====================================================================
                               END
====================================================================*/
