/*====================================================================

			       照応解析

                                         Ryohei Sasano 2007. 8. 27

    $Id$
====================================================================*/

#include "knp.h"

#define CASE_CANDIDATE_MAX 20 /* 照応解析用格解析結果を保持する数 */
#define ELLIPSIS_RESULT_MAX 1 /* 省略解析結果を保持する数 */
#define INITIAL_SCORE -10000
#define ENTITY_DECAY_RATE 0.8

/* 解析結果を保持するためのENTITY_CASE_MGR
   先頭のCASE_CANDIDATE_MAX個に照応解析用格解析の結果の上位の保持、
   次のELLIPSIS_RESULT_MAX個には省略解析結果のベスト解の保持、
   最後の1個は現在の解析結果の保持に使用する */
CF_TAG_MGR work_ctm[CASE_CANDIDATE_MAX + ELLIPSIS_RESULT_MAX + 1];

/* 省略解析の対象とする格のリスト */
char *ELLIPSIS_CASE_LIST[] = {"ガ", "ヲ", "ニ", ""};

/*==================================================================*/
int read_one_annotation(SENTENCE_DATA *sp, TAG_DATA *tag_ptr, char *token, int co_flag)
/*==================================================================*/
{
    /* 解析結果からMENTION、ENTITYを作成する */
    /* co_flagがある場合は"="のみを処理、ない場合は"="以外を処理 */

    char flag, rel[SMALL_DATA_LEN], *cp;
    int tag_num, sent_num;
    MENTION_MGR *mention_mgr = &(tag_ptr->mention_mgr);
    MENTION *mention_ptr = NULL;

    if (!sscanf(token, "%[^/]/%c/%*[^/]/%d/%d/", rel, &flag, &tag_num, &sent_num)) 
	return FALSE;

    if (co_flag && !strcmp(rel, "=")) {
	mention_ptr = mention_mgr->mention;
	mention_ptr->entity = ((sp - sent_num)->tag_data + tag_num)->mention_mgr.mention->entity;
	mention_ptr->entity->activity_score += 1;
	strcpy(mention_ptr->cpp_string, "＊");
	strcpy(mention_ptr->spp_string, "＊");
	mention_ptr->flag = '=';

	/* entityのnameがNEでなく、tag_ptrがNEならばnameを上書き */
	if (!strchr(mention_ptr->entity->name, ':') &&
	    (cp = check_feature(tag_ptr->f, "NE"))) {
	    strcpy(mention_ptr->entity->name, cp + strlen("NE:"));
	}
    }

    else if (!co_flag &&
	     (flag == 'N' || flag == 'C' || flag == 'O' || flag == 'D') &&    
	     (!strcmp(rel, "ガ") || !strcmp(rel, "ヲ") || !strcmp(rel, "ニ") || !strcmp(rel, "ノ"))) {
	mention_ptr = mention_mgr->mention + mention_mgr->num;
	mention_ptr->entity = ((sp - sent_num)->tag_data + tag_num)->mention_mgr.mention->entity;
	
	mention_ptr->tag_num = mention_mgr->mention->tag_num;
	mention_ptr->sent_num = mention_mgr->mention->sent_num;
	mention_ptr->flag = flag;
	strcpy(mention_ptr->cpp_string, rel);
	strcpy(mention_ptr->spp_string, "＊");
	mention_mgr->num++;
    }

    if (!mention_ptr) return FALSE;
    mention_ptr->entity->mention[mention_ptr->entity->mentioned_num] = mention_ptr;
    mention_ptr->entity->mentioned_num++;
    if (flag == 'O' || !strcmp(rel, "=")) mention_ptr->entity->antecedent_num++;
    
    return TRUE;
}

/*==================================================================*/
	   int anaphora_result_to_entity(TAG_DATA *tag_ptr)
/*==================================================================*/
{
    /* 照応解析結果ENTITYに関連付ける */

    int i;
    char *cp;
    MENTION_MGR *mention_mgr = &(tag_ptr->mention_mgr);
    MENTION *mention_ptr = NULL;
    CF_TAG_MGR *ctm_ptr = tag_ptr->ctm_ptr; 

    /* 格・省略解析結果がない場合は終了 */
    if (!ctm_ptr) return FALSE;
    
    for (i = 0; i < ctm_ptr->result_num; i++) {
	mention_ptr = mention_mgr->mention + mention_mgr->num;
	mention_ptr->entity = ctm_ptr->elem_b_ptr[i]->mention_mgr.mention->entity;
	mention_ptr->tag_num = mention_mgr->mention->tag_num;
	mention_ptr->sent_num = mention_mgr->mention->sent_num;
	//mention_ptr->flag = ctm_ptr->flag[i]; /* 暫定的 */
	mention_ptr->flag = 'O'; /* 暫定的 */
	strcpy(mention_ptr->cpp_string, "＊"); /* 暫定的 */
	/* 入力側の表層格 */
	if (tag_ptr->tcf_ptr->cf.pp[ctm_ptr->tcf_element_num[i]][0] >= FUKUGOJI_START &&
	    tag_ptr->tcf_ptr->cf.pp[ctm_ptr->tcf_element_num[i]][0] <= FUKUGOJI_END) {
	    strcpy(mention_ptr->spp_string, 
		   pp_code_to_kstr(tag_ptr->tcf_ptr->cf.pp[ctm_ptr->tcf_element_num[i]][0]));
	}
	else { 
	    if ((cp = check_feature(ctm_ptr->elem_b_ptr[i]->f, "係"))) {
		strcpy(mention_ptr->spp_string, cp + strlen("係:"));
	    } 
	    else {
		strcpy(mention_ptr->spp_string, "＊");
	    }
	}
	mention_mgr->num++;

	mention_ptr->entity->mention[mention_ptr->entity->mentioned_num] = mention_ptr;
	mention_ptr->entity->mentioned_num++;
	mention_ptr->entity->activity_score += 0.5;
    }
    
    return TRUE;
}

/*==================================================================*/
     int set_tag_case_frame(SENTENCE_DATA *sp, TAG_DATA *tag_ptr)
/*==================================================================*/
{
    /* ENTITY_PRED_MGRを作成する関数
       make_data_cframeを用いて入力文の格構造を作成するため
       CF_PRED_MGRを作り、そのcfをコピーしている */

    int i;
    TAG_CASE_FRAME *tcf_ptr = tag_ptr->tcf_ptr;
    CF_PRED_MGR *cpm_ptr;
   
    /* cpmの作成 */
    cpm_ptr = (CF_PRED_MGR *)malloc_data(sizeof(CF_PRED_MGR), "set_tag_case_frame");
    init_case_frame(&(cpm_ptr->cf));
    cpm_ptr->pred_b_ptr = tag_ptr;

    /* 入力文側の格要素設定 */
    set_data_cf_type(cpm_ptr);
    make_data_cframe(sp, cpm_ptr);

    /* ENTITY_PRED_MGRを作成・入力文側の格要素をコピー */
    tcf_ptr->cf = cpm_ptr->cf;
    tcf_ptr->pred_b_ptr = tag_ptr;
    for (i = 0; i < CF_ELEMENT_MAX; i++) {
	tcf_ptr->elem_b_ptr[i] = cpm_ptr->elem_b_ptr[i];
    }

    free(cpm_ptr);
    return TRUE;
}

/*==================================================================*/
    int set_cf_candidate(TAG_DATA *tag_ptr, CASE_FRAME **cf_array)
/*==================================================================*/
{
    int i, l, frame_num = 0, hiragana_prefer_flag = 0;
    CFLIST *cfp;
    char *key;
    
    /* 格フレームcache */
    if (OptUseSmfix == TRUE && CFSimExist == TRUE) {
		
	if ((key = get_pred_id(tag_ptr->cf_ptr->cf_id)) != NULL) {
	    cfp = CheckCF(key);
	    free(key);

	    if (cfp) {
		for (l = 0; l < tag_ptr->cf_num; l++) {
		    for (i = 0; i < cfp->cfid_num; i++) {
			if (((tag_ptr->cf_ptr + l)->type == tag_ptr->tcf_ptr->cf.type) &&
			    ((tag_ptr->cf_ptr + l)->cf_similarity = 
			     get_cfs_similarity((tag_ptr->cf_ptr + l)->cf_id, 
						*(cfp->cfid + i))) > CFSimThreshold) {
			    *(cf_array + frame_num++) = tag_ptr->cf_ptr + l;
			    break;
			}
		    }
		}
		tag_ptr->e_cf_num = frame_num;
	    }
	}
    }

    if (frame_num == 0) {
	
	/* 表記がひらがなの場合: 
	   格フレームの表記がひらがなの場合が多ければひらがなの格フレームのみを対象に、
	   ひらがな以外が多ければひらがな以外のみを対象にする */
	if (!(OptCaseFlag & OPT_CASE_USE_REP_CF) && /* 代表表記ではない場合のみ */
	    check_str_type(tag_ptr->head_ptr->Goi) == TYPE_HIRAGANA) {
	    if (check_feature(tag_ptr->f, "代表ひらがな")) {
		hiragana_prefer_flag = 1;
	    }
	    else {
		hiragana_prefer_flag = -1;
	    }
	}
	
	for (l = 0; l < tag_ptr->cf_num; l++) {
	    if ((tag_ptr->cf_ptr + l)->type == tag_ptr->tcf_ptr->cf.type && 
		(hiragana_prefer_flag == 0 || 
		 (hiragana_prefer_flag > 0 && 
		  check_str_type((tag_ptr->cf_ptr + l)->entry) == TYPE_HIRAGANA) || 
		 (hiragana_prefer_flag < 0 && 
		  check_str_type((tag_ptr->cf_ptr + l)->entry) != TYPE_HIRAGANA))) {
		*(cf_array + frame_num++) = tag_ptr->cf_ptr + l;
	    }
	}
    }
    return frame_num;
}

/*==================================================================*/
double calc_score_of_ctm(CF_TAG_MGR *ctm_ptr, TAG_CASE_FRAME *tcf_ptr)
/*==================================================================*/
{
    /* 格フレームとの対応付けのスコアを計算する関数  */

    int i, e_num, debug = 0;
    double score;

    /* 対象の格フレームが選択される確率 */
    score = get_cf_probability(&(tcf_ptr->cf), ctm_ptr->cf_ptr);

    /* 対応付けられた要素に関するスコア */
    for (i = 0; i < ctm_ptr->result_num; i++) {
	e_num = ctm_ptr->cf_element_num[i];
	    
	score += 
	    get_ex_probability_with_para(ctm_ptr->tcf_element_num[i], &(tcf_ptr->cf), 
					 e_num, ctm_ptr->cf_ptr) +
	    get_case_interpret_probability(ctm_ptr->tcf_element_num[i], &(tcf_ptr->cf), 
					   e_num, ctm_ptr->cf_ptr);

	if (OptDisplay == OPT_DEBUG && debug) 
	    printf(";; %s-%s:%f:%f ", 
		   ctm_ptr->elem_b_ptr[i]->head_ptr->Goi2, 
		   pp_code_to_kstr(ctm_ptr->cf_ptr->pp[e_num][0]),
		   get_ex_probability_with_para(ctm_ptr->tcf_element_num[i], &(tcf_ptr->cf), 
						e_num, ctm_ptr->cf_ptr),
		   get_case_interpret_probability(ctm_ptr->tcf_element_num[i], &(tcf_ptr->cf), 
						  e_num, ctm_ptr->cf_ptr));      
    }
    /* 入力文の格要素のうち対応付けられなかった要素に関するスコア */
    for (i = 0; i < tcf_ptr->cf.element_num - ctm_ptr->result_num; i++) {
	if (OptDisplay == OPT_DEBUG && debug) 
	    printf("%s:%f/", 
		  (tcf_ptr->elem_b_ptr[ctm_ptr->non_match_element[i]])->head_ptr->Goi2,
		  score);

	score += FREQ0_ASSINED_SCORE +
	    get_case_interpret_probability(ctm_ptr->non_match_element[i], &(tcf_ptr->cf), 
					   NIL_ASSIGNED, ctm_ptr->cf_ptr);
    }
    if (OptDisplay == OPT_DEBUG && debug) printf("%f ", score);	   
    /* 格フレームの格が埋まっているかどうかに関するスコア */
    for (e_num = 0; e_num < ctm_ptr->cf_ptr->element_num; e_num++) {
	score += get_case_probability(e_num, ctm_ptr->cf_ptr, ctm_ptr->filled_element[e_num]);	
    }
    if (OptDisplay == OPT_DEBUG && debug) printf(";; %f\n", score);

    return score;
}

/*==================================================================*/
     int copy_ctm(CF_TAG_MGR *source_ctm, CF_TAG_MGR *target_ctm)
/*==================================================================*/
{
    int i;

    target_ctm->score = source_ctm->score;
    target_ctm->cf_ptr = source_ctm->cf_ptr;
    target_ctm->result_num = source_ctm->result_num;
    for (i = 0; i < CF_ELEMENT_MAX; i++) {
	target_ctm->filled_element[i] = source_ctm->filled_element[i];
	target_ctm->non_match_element[i] = source_ctm->non_match_element[i];
	target_ctm->cf_element_num[i] = source_ctm->cf_element_num[i];
	target_ctm->tcf_element_num[i] = source_ctm->tcf_element_num[i];
	target_ctm->entity_num[i] = source_ctm->entity_num[i];
	target_ctm->elem_b_ptr[i] = source_ctm->elem_b_ptr[i];
    }
}

/*==================================================================*/
      int preserve_ctm(CF_TAG_MGR *ctm_ptr, int start, int num)
/*==================================================================*/
{
    /* start番目からnum個のwork_ctmのスコアと比較し上位ならば保存する
       num個のwork_ctmのスコアは降順にソートされていることを仮定している
       保存された場合は1、されなかった場合は0を返す */

    int i, j;
    
    for (i = start; i < start + num; i++) {
	
	/* work_ctmに結果を保存 */
	if (ctm_ptr->score > work_ctm[i].score) {	    
	    for (j = CASE_CANDIDATE_MAX - 1; j > i; j--) {
		if (work_ctm[j - 1].score > INITIAL_SCORE) {
		    copy_ctm(&work_ctm[j - 1], &work_ctm[j]);
		}
	    }
	    copy_ctm(ctm_ptr, &work_ctm[i]);
	    return TRUE;
	}
    }
    return FALSE;
}

/*==================================================================*/
int case_analysis_for_anaphora(TAG_DATA *tag_ptr, CF_TAG_MGR *ctm_ptr, int i, int r_num)
/*==================================================================*/
{
    /* 候補の格フレームについて照応解析用格解析を実行する関数
       再帰的に呼び出す
       iにはtag_ptr->tcf_ptr->cf.element_numのうちチェックした数 
       r_numにはそのうち格フレームと関連付けられた要素の数が入る */
    
    int j, k, e_num;

    /* すでに埋まっている格フレームの格をチェック */
    memset(ctm_ptr->filled_element, 0, sizeof(int) * CF_ELEMENT_MAX);
    for (j = 0; j < r_num; j++) ctm_ptr->filled_element[ctm_ptr->cf_element_num[j]] = TRUE;

    /* まだチェックしていない要素がある場合 */
    if (i < tag_ptr->tcf_ptr->cf.element_num) {

	for (j = 0; tag_ptr->tcf_ptr->cf.pp[i][j] != END_M; j++) {
	    
	    /* 入力文のi番目の格要素を格フレームのcf.pp[i][j]格に割り当てる */
	    for (e_num = 0; e_num < ctm_ptr->cf_ptr->element_num; e_num++) {

		if (tag_ptr->tcf_ptr->cf.pp[i][j] == ctm_ptr->cf_ptr->pp[e_num][0]) {
		    /* 対象の格が既に埋まっている場合は不可 */
		    if (ctm_ptr->filled_element[e_num] == TRUE) return FALSE;

		    /* 直前格である場合は制限を加える(未実装・暫定的) */
		    if (0) return FALSE;
		    
		    /* 対応付け結果を記録 */
		    ctm_ptr->elem_b_ptr[r_num] = tag_ptr->tcf_ptr->elem_b_ptr[i];
		    ctm_ptr->cf_element_num[r_num] = e_num;
		    ctm_ptr->tcf_element_num[r_num] = i;
		    ctm_ptr->entity_num[r_num] = 
			ctm_ptr->elem_b_ptr[r_num]->mention_mgr.mention->entity->num;

		    /* i+1番目の要素のチェックへ */
		    case_analysis_for_anaphora(tag_ptr, ctm_ptr, i + 1, r_num + 1);
		}
	    }    
	}

	for (j = 0; tag_ptr->tcf_ptr->cf.pp[i][j] != END_M; j++) {
	    /* ガ、ヲ、ニ以外の格がある場合は割り当てない場合も考える */
	    if (!MatchPP(tag_ptr->tcf_ptr->cf.pp[i][j], "ガ") &&
		!MatchPP(tag_ptr->tcf_ptr->cf.pp[i][j], "ヲ") &&
		!MatchPP(tag_ptr->tcf_ptr->cf.pp[i][j], "ニ")) {		
		/* 入力文のi番目の要素が対応付けられなかったことを記録 */
		ctm_ptr->non_match_element[i - r_num] = i; 
		case_analysis_for_anaphora(tag_ptr, ctm_ptr, i + 1, r_num);
		break;
	    }
	}
    }

    /* すべてのチェックが終了した場合 */
    else {
	/* この段階でr_num個が対応付けられている */
	ctm_ptr->result_num = r_num;
	/* スコアを計算 */
	ctm_ptr->score = calc_score_of_ctm(ctm_ptr, tag_ptr->tcf_ptr);
	/* スコア上位を保存 */
	preserve_ctm(ctm_ptr, 0, CASE_CANDIDATE_MAX);
    }
}

/*==================================================================*/
int ellipsis_analysis(TAG_DATA *tag_ptr, CF_TAG_MGR *ctm_ptr, int i, int r_num)
/*==================================================================*/
{
    /* 候補となる格フレームと格要素の対応付けについて省略解析を実行する関数
       再帰的に呼び出す 
       iにはELLIPSIS_CASE_LIST[]のうちチェックした数が入る
       r_numには格フレームと関連付けられた要素の数が入る(格解析の結果関連付けられたものも含む) */

    int j, k, e_num;
   
    /* すでに埋まっている格フレームの格をチェック */
    memset(ctm_ptr->filled_element, 0, sizeof(int) * CF_ELEMENT_MAX);
    memset(ctm_ptr->filled_entity, 0, sizeof(int) * ENTITY_MAX);
    for (j = 0; j < r_num; j++) {
	ctm_ptr->filled_element[ctm_ptr->cf_element_num[j]] = TRUE;
	ctm_ptr->filled_entity[ctm_ptr->entity_num[j]] = TRUE;
    }

    /* まだチェックしていない省略解析対象格がある場合 */
    if (ELLIPSIS_CASE_LIST[i]) {
	/* 対象の格について */
	for (e_num = 0; ctm_ptr->cf_ptr->element_num; e_num++) {
	    if (ctm_ptr->cf_ptr->pp[e_num][0] != ELLIPSIS_CASE_LIST[i]) continue;
	    
	    /* すでに埋まっていた場合は次の格をチェックする */
	    if (ctm_ptr->filled_element[e_num] == TRUE) {
		ellipsis_analysis(tag_ptr, ctm_ptr, i + 1, r_num);
	    }
	    else {
		for (k = 0; k < entity_manager.num; k++) {
		    /* activity_scoreが1以下なら候補としない(暫定的) */
		    if (entity_manager.entity[k].activity_score < 1) continue;
		    /* 対象のENTITYがすでに対応付けられている場合は不可 */
		    if (ctm_ptr->filled_entity[k]) return FALSE;
		    
		    /* 対応付け結果を記録
		       (基本句との対応付けは取っていないためelem_b_ptrは使用しない) */
		    ctm_ptr->cf_element_num[r_num] = e_num;
		    ctm_ptr->entity_num[r_num] =
			ctm_ptr->elem_b_ptr[r_num]->mention_mgr.mention->entity->num;
		    
		    /* 次の格のチェックへ */
		    ellipsis_analysis(tag_ptr, ctm_ptr, i + 1, r_num + 1);
		}
	    }
	    break;
	}
    }
    
    /* すべてのチェックが終了した場合 */
    else {
	1;
    }
    
    /* スコア上位を保存 */
    return preserve_ctm(ctm_ptr, CASE_CANDIDATE_MAX, ELLIPSIS_RESULT_MAX);
}

/*==================================================================*/
	    int ellipsis_analysis_main(TAG_DATA *tag_ptr)
/*==================================================================*/
{
    /* ある基本句を対象として省略解析を行う関数 */
    /* 格フレームごとにループを回す */

    int i, j, k, frame_num = 0;
    CASE_FRAME **cf_array;
    CF_TAG_MGR *ctm_ptr = work_ctm + CASE_CANDIDATE_MAX + ELLIPSIS_RESULT_MAX;
    
    /* 使用する格フレームの設定 */
    cf_array = (CASE_FRAME **)malloc_data(sizeof(CASE_FRAME *)*tag_ptr->cf_num, 
					  "ellipsis_analysis_main");
    frame_num = set_cf_candidate(tag_ptr, cf_array);

    /* work_ctmのスコアを初期化 */
    for (i = 0; i < CASE_CANDIDATE_MAX + ELLIPSIS_RESULT_MAX; i++) work_ctm[i].score = INITIAL_SCORE;
	  
    /* 候補の格フレームについて照応解析用格解析を実行 */
    for (i = 0; i < frame_num; i++) {

	/* OR の格フレームを除く */
	if (((*(cf_array + i))->etcflag & CF_SUM) && frame_num != 1) {
	    continue;
	}
	
	/* 用言のみ対象(暫定的) */
	if (tag_ptr->tcf_ptr->cf.type != CF_PRED) continue;

	/* ctm_ptrの初期化 */
	ctm_ptr->score = INITIAL_SCORE;

	/* 格フレームを指定 */
 	ctm_ptr->cf_ptr = *(cf_array + i);

	/* 照応解析用格解析(上位CASE_CANDIDATE_MAX個の結果を保持する) */
	case_analysis_for_anaphora(tag_ptr, ctm_ptr, 0, 0);	
    }
    if (work_ctm[0].score == INITIAL_SCORE) return FALSE;
    
    if (OptDisplay == OPT_DEBUG) {
	for (i = 0; i < CASE_CANDIDATE_MAX; i++) {
	    if (work_ctm[i].score == INITIAL_SCORE) break;
	    printf(";;%2d %f %s", i, work_ctm[i].score, work_ctm[i].cf_ptr->cf_id);

	    for (j = 0; j < work_ctm[i].result_num; j++) {
		/* printf(" %s:%d:%s", */
		printf(" %s:%s",
		       pp_code_to_kstr(work_ctm[i].cf_ptr->pp[work_ctm[i].cf_element_num[j]][0]),
		       /* work_ctm[i].entity_num[j], */
		       work_ctm[i].elem_b_ptr[j]->head_ptr->Goi2);
	    }
	    for (j = 0; j < work_ctm[i].cf_ptr->element_num; j++) {
		if (!work_ctm[i].filled_element[j] && 
		    (MatchPP(work_ctm[i].cf_ptr->pp[j][0], "ガ") || 
		     MatchPP(work_ctm[i].cf_ptr->pp[j][0], "ヲ") || 
		     MatchPP(work_ctm[i].cf_ptr->pp[j][0], "ニ")))	    
		    printf(" %s:×", pp_code_to_kstr(work_ctm[i].cf_ptr->pp[j][0]));
			   
	    }
	    printf("\n", work_ctm[i].cf_ptr->cf_id);
	}
    }

    /* 上記の対応付けに対して省略解析を実行する */
    for (i = 0; i < CASE_CANDIDATE_MAX; i++) {
	copy_ctm(&work_ctm[i], ctm_ptr);
	ellipsis_analysis(tag_ptr, ctm_ptr, 0, ctm_ptr->result_num);
    }
    
    /* BEST解を保存 */
    tag_ptr->ctm_ptr = 
	(CF_TAG_MGR *)malloc_data(sizeof(CF_TAG_MGR), "ellipsis_analysis_main");
    copy_ctm(&work_ctm[CASE_CANDIDATE_MAX], tag_ptr->ctm_ptr);

    free(cf_array);

    return TRUE;
}

/*==================================================================*/
	    int make_context_structure(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j;
    char *cp;
    TAG_DATA *tag_ptr;
    MENTION_MGR *mention_mgr;
    ENTITY *entity_ptr;
    
    /* ENTITYを生成 */
    for (i = 0; i < sp->Tag_num; i++) { /* 解析文のタグ単位:i番目のタグについて */

	tag_ptr = sp->tag_data + i; 
	
	/* 自分自身(MENTION)を生成 */
	mention_mgr = &(tag_ptr->mention_mgr);
	mention_mgr->mention->tag_num = i;
	mention_mgr->mention->sent_num = sp->Sen_num;
	mention_mgr->mention->entity = NULL;
	mention_mgr->num = 1;

	/* 入力から正解を読み込む場合 */
	if (OptReadFeature) {

	    /* featureから格解析結果を取得 */
	    if (cp = check_feature(tag_ptr->f, "格解析結果")) {		
		cp += strlen("格解析結果:");
		cp = strchr(cp, ':') + 1;	
		for (; *cp; cp++) {
		    if (*cp == ':' || *cp == ';') {
			read_one_annotation(sp, tag_ptr, cp + 1, TRUE);
		    }
		}
	    }
	}
	/* 自動解析の場合 */
	else if (cp = check_feature(tag_ptr->f, "Ｔ共参照")) {
	    read_one_annotation(sp, tag_ptr, cp + strlen("Ｔ共参照:"), TRUE);
	}

	/* 新しいENTITYである場合 */
	if (!mention_mgr->mention->entity) {
	    if (entity_manager.num >= ENTITY_MAX - 1) { 
		fprintf(stderr, "Entity buffer overflowed!\n");
		exit(1);
	    }
	    entity_ptr = entity_manager.entity + entity_manager.num;
	    entity_ptr->num = entity_manager.num;
	    entity_manager.num++;				
	    entity_ptr->mention[0] = mention_mgr->mention;
	    entity_ptr->mentioned_num = 1;
	    entity_ptr->antecedent_num = 0;
	    /* 先行詞になりやすさ(基本的に文節主辞なら1) */
	    entity_ptr->activity_score = 
		(tag_ptr->inum == 0 && 
		 check_feature(tag_ptr->f, "先行詞候補") &&
		 !check_feature(tag_ptr->f, "NE内") &&
		 !check_feature(tag_ptr->f, "Ｔ非先行詞")) ? 1 : 0;
	    /* nameの設定(NE > 照応詞候補 > 主辞形態素の原型) */
	    if (cp = check_feature(tag_ptr->f, "NE")) {
		strcpy(entity_ptr->name, cp + strlen("NE:"));
	    }
	    else if (cp = check_feature(tag_ptr->f, "照応詞候補")) {
		strcpy(entity_ptr->name, cp + strlen("照応詞候補:"));
	    }
	    else {
		strcpy(entity_ptr->name, tag_ptr->head_ptr->Goi2);
	    }
	    mention_mgr->mention->entity = entity_ptr;
	    //pp_kstr_to_code("＊");
	    strcpy(mention_mgr->mention->cpp_string, "＊"); /* "＊"を入れたことに相当 */
	    strcpy(mention_mgr->mention->spp_string, "＊");
	    mention_mgr->mention->flag = 'S'; /* 自分自身 */
	}
    }

    /* 入力から正解を読み込む場合 */
    if (OptReadFeature) {
	for (i = 0; i < sp->Tag_num; i++) { /* 解析文のタグ単位:i番目のタグについて */

	    tag_ptr = sp->tag_data + i; 
	
	    /* featureから格解析結果を取得 */
	    if (cp = check_feature(tag_ptr->f, "格解析結果")) {		
		cp += strlen("格解析結果:");
		cp = strchr(cp, ':') + 1;	
		for (; *cp; cp++) {
		    if (*cp == ':' || *cp == ';') {
			read_one_annotation(sp, tag_ptr, cp + 1, FALSE);
		    }
		}
	    }
	}	
	return TRUE;
    }
    
    /* 省略解析 */
    for (i = sp->Tag_num - 1; i >= 0; i--) { /* 解析文のタグ単位:i番目のタグについて */

	tag_ptr = sp->tag_data + i; 
	tag_ptr->tcf_ptr = NULL;
	tag_ptr->ctm_ptr = NULL;
	
	if (tag_ptr->cf_ptr &&
	    !check_feature(tag_ptr->f, "省略解析なし") &&
	    !check_feature(tag_ptr->f, "NE") &&
	    !check_feature(tag_ptr->f, "NE内") &&
	    !check_feature(tag_ptr->f, "共参照") &&
	    !check_feature(tag_ptr->f, "共参照内")) {

	    /* tag_ptr->tcf_ptrを作成 */
	    tag_ptr->tcf_ptr = 
		(TAG_CASE_FRAME *)malloc_data(sizeof(TAG_CASE_FRAME), "make_context_structure");
	    set_tag_case_frame(sp, tag_ptr);

	    /* 省略解析メイン */
	    ellipsis_analysis_main(tag_ptr);

	    /* tcfを解放 (暫定的) */
	    for (j = 0; j < CF_ELEMENT_MAX; j++) {
		free(tag_ptr->tcf_ptr->cf.ex[j]);
		tag_ptr->tcf_ptr->cf.ex[j] = NULL;
		free(tag_ptr->tcf_ptr->cf.sm[j]);
		tag_ptr->tcf_ptr->cf.sm[j] = NULL;
		free(tag_ptr->tcf_ptr->cf.ex_list[j][0]);
		free(tag_ptr->tcf_ptr->cf.ex_list[j]);
		free(tag_ptr->tcf_ptr->cf.ex_freq[j]);
	    }
	    free(tag_ptr->tcf_ptr);
	}
    }

    /* 解析結果をENTITYと関連付ける */
    for (i = 0; i < sp->Tag_num; i++) {
	anaphora_result_to_entity(sp->tag_data + i);
    }    
}

/*==================================================================*/
		   void print_entities(int sen_num)
/*==================================================================*/
{
    int i, j;
    MENTION *mention_ptr;
    ENTITY *entity_ptr;

    printf(";;SENTENCE %d\n", sen_num); 
    for (i = 0; i < entity_manager.num; i++) {
	entity_ptr = entity_manager.entity + i;

	/* if (!entity_ptr->antecedent_num) continue; */
	/* if (entity_ptr->mentioned_num == 1) continue; */
	if (entity_ptr->activity_score == 0) continue;

	printf(";; ENTITY %d [ %s ] %f {\n", i, entity_ptr->name, entity_ptr->activity_score);
	for (j = 0; j < entity_ptr->mentioned_num; j++) {
	    mention_ptr = entity_ptr->mention[j];
	    printf(";;\tMENTION%3d {", j);
	    printf(" SEN:%3d", mention_ptr->sent_num);
	    printf(" TAG:%3d", mention_ptr->tag_num);
	    printf(" CPP: %s", mention_ptr->cpp_string);
	    printf(" SPP: %s", mention_ptr->spp_string);
	    printf(" FLAG: %c", mention_ptr->flag);
	    printf(" WORD: %s", ((sentence_data + mention_ptr->sent_num - 1)->tag_data + mention_ptr->tag_num)->head_ptr->Goi2);
	    printf(" }\n");
	}
	printf(";; }\n;;\n");
    }
}

/*==================================================================*/
			 void decay_entity()
/*==================================================================*/
{
    /* ENTITYの活性値を減衰させる */

    int i;

    for (i = 0; i < entity_manager.num; i++) {
	entity_manager.entity[i].activity_score *= ENTITY_DECAY_RATE;
    }
}

/*==================================================================*/
	      void anaphora_analysis(SENTENCE_DATA *sp)
/*==================================================================*/
{
    decay_entity();
    make_context_structure(sentence_data + sp->Sen_num - 1);
    /* if (OptDisplay == OPT_DEBUG) */ print_entities(sp->Sen_num);
}
