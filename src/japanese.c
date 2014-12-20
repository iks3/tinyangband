/* File: japanese.c */


/*
* Copyright (c) 1997 Ben Harrison
*
* This software may be copied and distributed for educational, research,
* and not for profit purposes provided that this copyright and statement
* are included in all such copies.  Other copyrights may also apply.
*/

#include "angband.h"

#ifdef JP

/*
* Find valid breakpoint in a Japanese string
*/
int null_pos_j(const char* s, int maxpos)
{
    int i = 0;
	
    while (s[i] != 0 && i < ((iskanji(s[i])) ? maxpos-1 : maxpos))
		i+= iskanji(s[i]) ? 2 : 1;
	
    return(i);
}

typedef struct convert_key convert_key;

struct convert_key
{
	cptr key1;
	cptr key2;
};

static const convert_key s2j_table[] = {
	{"mb","nb"}, {"mp","np"}, {"mv","nv"}, {"mm","nm"},
	{"x","ks"},
	/* sindar:シンダール  parantir:パランティア  feanor:フェアノール */
	{"ar$","a-ru$"}, {"ir$","ia$"}, {"or$","o-ru$"},
	{"ra","ラ"}, {"ri","リ"}, {"ru","ル"}, {"re","レ"}, {"ro","ロ"},
	{"ir","ia"}, {"ur","ua"}, {"er","ea"}, {"ar","aル"},
	{"sha","シャ"}, {"shi","シ"}, {"shu","シュ"}, {"she","シェ"}, {"sho","ショ"},
	{"tha","サ"}, {"thi","シ"}, {"thu","ス"}, {"the","セ"}, {"tho","ソ"},
	{"cha","ハ"}, {"chi","ヒ"}, {"chu","フ"}, {"che","ヘ"}, {"cho","ホ"},
	{"dha","ザ"}, {"dhi","ジ"}, {"dhu","ズ"}, {"dhe","ゼ"}, {"dho","ゾ"},
	{"ba","バ"}, {"bi","ビ"}, {"bu","ブ"}, {"be","ベ"}, {"bo","ボ"},
	{"ca","カ"}, {"ci","キ"}, {"cu","ク"}, {"ce","ケ"}, {"co","コ"},
	{"da","ダ"}, {"di","ディ"}, {"du","ドゥ"}, {"de","デ"}, {"do","ド"},
	{"fa","ファ"}, {"fi","フィ"}, {"fu","フ"}, {"fe","フェ"}, {"fo","フォ"},
	{"ga","ガ"}, {"gi","ギ"}, {"gu","グ"}, {"ge","ゲ"}, {"go","ゴ"},
	{"ha","ハ"}, {"hi","ヒ"}, {"hu","フ"}, {"he","ヘ"}, {"ho","ホ"},
	{"ja","ジャ"}, {"ji","ジ"}, {"ju","ジュ"}, {"je","ジェ"}, {"jo","ジョ"},
	{"ka","カ"}, {"ki","キ"}, {"ku","ク"}, {"ke","ケ"}, {"ko","コ"},
	{"la","ラ"}, {"li","リ"}, {"lu","ル"}, {"le","レ"}, {"lo","ロ"},
	{"ma","マ"}, {"mi","ミ"}, {"mu","ム"}, {"me","メ"}, {"mo","モ"},
	{"na","ナ"}, {"ni","ニ"}, {"nu","ヌ"}, {"ne","ネ"}, {"no","ノ"},
	{"pa","パ"}, {"pi","ピ"}, {"pu","プ"}, {"pe","ペ"}, {"po","ポ"},
	{"qu","ク"},
	{"sa","サ"}, {"si","シ"}, {"su","ス"}, {"se","セ"}, {"so","ソ"},
	{"ta","タ"}, {"ti","ティ"}, {"tu","トゥ"}, {"te","テ"}, {"to","ト"},
	{"va","ヴァ"}, {"vi","ヴィ"}, {"vu","ヴ"}, {"ve","ヴェ"}, {"vo","ヴォ"},
	{"wa","ワ"}, {"wi","ウィ"}, {"wu","ウ"}, {"we","ウェ"}, {"wo","ウォ"},
	{"ya","ヤ"}, {"yu","ユ"}, {"yo","ヨ"},
	{"za","ザ"}, {"zi","ジ"}, {"zu","ズ"}, {"ze","ゼ"}, {"zo","ゾ"},
	{"dh","ズ"}, {"ch","フ"}, {"th","ス"},
	{"b","ブ"}, {"c","ク"}, {"d","ド"}, {"f","フ"}, {"g","グ"},
	{"h","フ"}, {"j","ジュ"}, {"k","ク"}, {"l","ル"}, {"m","ム"},
	{"n","ン"}, {"p","プ"}, {"q","ク"}, {"r","ル"}, {"s","ス"},
	{"t","ト"}, {"v","ヴ"}, {"w","ウ"}, {"y","イ"},
	{"a","ア"}, {"i","イ"}, {"u","ウ"}, {"e","エ"}, {"o","オ"},
	{"-","ー"},
	{NULL,NULL}
};

/* 英語版の巻物の未判明時の名前を日本語の読みに変換する */
void sindarin_to_kana(char *kana, const char *sindarin)
{
	char buf[256];
	int idx;
	
	sprintf(kana, "%s$", sindarin);
	for (idx=0; kana[idx]; idx++)
		if (isupper(kana[idx])) kana[idx] = tolower(kana[idx]);
	
	for (idx = 0; s2j_table[idx].key1 != NULL; idx++)
	{
		cptr pat1 = s2j_table[idx].key1;
		cptr pat2 = s2j_table[idx].key2;
		int len = strlen(pat1);
		char *src = kana;
		char *dest = buf;
		
		while (*src) {
			if( strncmp(src, pat1, len) == 0 )
			{
				strcpy(dest, pat2);
				src += len;
				dest += strlen(pat2);
			}
			else
			{
				if( iskanji(*src) )
				{
					*dest = *src;
					src++;
					dest++;
				}
				*dest = *src;
				src++;
				dest++;
			}
		}

		*dest = 0;
		strcpy(kana, buf);
	}

	idx = 0;

	while( kana[idx] != '$' ) idx++;

	kana[idx] = '\0';
}


/* 日本語動詞活用 (打つ＞打って,打ち etc) */

#define CMPTAIL(y) strncmp(&in[l-(int)strlen(y)], y, strlen(y))

/* 殴る,蹴る -> 殴り,蹴る */
static void jverb1( const char *in , char *out)
{
	int l = strlen(in);
	strcpy(out, in);

	/* サ行変格活用 */
	if( CMPTAIL("する") == 0) sprintf(&out[l-4], "し"); else

	/* 上一段活用 */
	if( CMPTAIL("いる") == 0) sprintf(&out[l-4], "いて"); else

	/* 下一段活用 */
	if( CMPTAIL("える") == 0) sprintf(&out[l-4], "え"); else
	if( CMPTAIL("ける") == 0) sprintf(&out[l-4], "け"); else
	if( CMPTAIL("げる") == 0) sprintf(&out[l-4], "げ"); else
	if( CMPTAIL("せる") == 0) sprintf(&out[l-4], "せ"); else
	if( CMPTAIL("ぜる") == 0) sprintf(&out[l-4], "ぜ"); else
	if( CMPTAIL("てる") == 0) sprintf(&out[l-4], "て"); else
	if( CMPTAIL("でる") == 0) sprintf(&out[l-4], "で"); else
	if( CMPTAIL("ねる") == 0) sprintf(&out[l-4], "ね"); else
	if( CMPTAIL("へる") == 0) sprintf(&out[l-4], "へ"); else
	if( CMPTAIL("べる") == 0) sprintf(&out[l-4], "べ"); else
	if( CMPTAIL("める") == 0) sprintf(&out[l-4], "め"); else
	if( CMPTAIL("れる") == 0) sprintf(&out[l-4], "れ"); else

	/* 五段活用 */
	if( CMPTAIL("う") == 0) sprintf(&out[l-2], "い"); else
	if( CMPTAIL("く") == 0) sprintf(&out[l-2], "き"); else
	if( CMPTAIL("ぐ") == 0) sprintf(&out[l-2], "ぎ"); else
	if( CMPTAIL("す") == 0) sprintf(&out[l-2], "し"); else
	if( CMPTAIL("ず") == 0) sprintf(&out[l-2], "じ"); else
	if( CMPTAIL("つ") == 0) sprintf(&out[l-2], "ち"); else
	if( CMPTAIL("づ") == 0) sprintf(&out[l-2], "ぢ"); else
	if( CMPTAIL("ぬ") == 0) sprintf(&out[l-2], "に"); else
	if( CMPTAIL("ふ") == 0) sprintf(&out[l-2], "ひ"); else
	if( CMPTAIL("ぶ") == 0) sprintf(&out[l-2], "び"); else
	if( CMPTAIL("む") == 0) sprintf(&out[l-2], "み"); else
	if( CMPTAIL("る") == 0) sprintf(&out[l-2], "り"); else

	sprintf(&out[l], "そして");
}

/* 殴る,蹴る -> 殴って蹴る */
static void jverb2( const char *in , char *out)
{
	int l = strlen(in);
	strcpy(out,in);

	/* サ行変格活用 */
	if( CMPTAIL("する") == 0) sprintf(&out[l-4], "して"); else

	/* 上一段活用 */
	if( CMPTAIL("いる") == 0) sprintf(&out[l-4], "いて"); else

	/* 下一段活用 */
	if( CMPTAIL("える") == 0) sprintf(&out[l-4], "えて"); else
	if( CMPTAIL("ける") == 0) sprintf(&out[l-4], "けて"); else
	if( CMPTAIL("げる") == 0) sprintf(&out[l-4], "げて"); else
	if( CMPTAIL("せる") == 0) sprintf(&out[l-4], "せて"); else
	if( CMPTAIL("ぜる") == 0) sprintf(&out[l-4], "ぜて"); else
	if( CMPTAIL("てる") == 0) sprintf(&out[l-4], "てって"); else
	if( CMPTAIL("でる") == 0) sprintf(&out[l-4], "でて"); else
	if( CMPTAIL("ねる") == 0) sprintf(&out[l-4], "ねて"); else
	if( CMPTAIL("へる") == 0) sprintf(&out[l-4], "へて"); else
	if( CMPTAIL("べる") == 0) sprintf(&out[l-4], "べて"); else
	if( CMPTAIL("める") == 0) sprintf(&out[l-4], "めて"); else
	if( CMPTAIL("れる") == 0) sprintf(&out[l-4], "れて"); else

	/* 五段活用 */
	if( CMPTAIL("う") == 0) sprintf(&out[l-2], "って"); else
	if( CMPTAIL("く") == 0) sprintf(&out[l-2], "いて"); else
	if( CMPTAIL("ぐ") == 0) sprintf(&out[l-2], "いで"); else
	if( CMPTAIL("す") == 0) sprintf(&out[l-2], "して"); else
	if( CMPTAIL("ず") == 0) sprintf(&out[l-2], "じて"); else
	if( CMPTAIL("つ") == 0) sprintf(&out[l-2], "って"); else
	if( CMPTAIL("づ") == 0) sprintf(&out[l-2], "って"); else
	if( CMPTAIL("ぬ") == 0) sprintf(&out[l-2], "ねて"); else
	if( CMPTAIL("ふ") == 0) sprintf(&out[l-2], "へて"); else
	if( CMPTAIL("ぶ") == 0) sprintf(&out[l-2], "んで"); else
	if( CMPTAIL("む") == 0) sprintf(&out[l-2], "んで"); else
	if( CMPTAIL("る") == 0) sprintf(&out[l-2], "って"); else

	sprintf(&out[l], "ことにより");
}

/* 殴る,蹴る -> 殴ったり蹴ったり */
static void jverb3( const char *in , char *out)
{
	int l = strlen(in);
	strcpy(out, in);

	/* サ行変格活用 */
	if( CMPTAIL("する") == 0) sprintf(&out[l-4], "した"); else

	/* 上一段活用 */
	if( CMPTAIL("いる") == 0) sprintf(&out[l-4], "いた"); else

	/* 下一段活用 */
	if( CMPTAIL("える") == 0) sprintf(&out[l-4], "えた"); else
	if( CMPTAIL("ける") == 0) sprintf(&out[l-4], "けた"); else
	if( CMPTAIL("げる") == 0) sprintf(&out[l-4], "げた"); else
	if( CMPTAIL("せる") == 0) sprintf(&out[l-4], "せた"); else
	if( CMPTAIL("ぜる") == 0) sprintf(&out[l-4], "ぜた"); else
	if( CMPTAIL("てる") == 0) sprintf(&out[l-4], "てった"); else
	if( CMPTAIL("でる") == 0) sprintf(&out[l-4], "でた"); else
	if( CMPTAIL("ねる") == 0) sprintf(&out[l-4], "ねた"); else
	if( CMPTAIL("へる") == 0) sprintf(&out[l-4], "へた"); else
	if( CMPTAIL("べる") == 0) sprintf(&out[l-4], "べた"); else
	if( CMPTAIL("める") == 0) sprintf(&out[l-4], "めた"); else
	if( CMPTAIL("れる") == 0) sprintf(&out[l-4], "れた"); else

	/* 五段活用 */
	if( CMPTAIL("う") == 0) sprintf(&out[l-2], "った"); else
	if( CMPTAIL("く") == 0) sprintf(&out[l-2], "いた"); else
	if( CMPTAIL("ぐ") == 0) sprintf(&out[l-2], "いだ"); else
	if( CMPTAIL("す") == 0) sprintf(&out[l-2], "した"); else
	if( CMPTAIL("ず") == 0) sprintf(&out[l-2], "じた"); else
	if( CMPTAIL("つ") == 0) sprintf(&out[l-2], "った"); else
	if( CMPTAIL("づ") == 0) sprintf(&out[l-2], "った"); else
	if( CMPTAIL("ぬ") == 0) sprintf(&out[l-2], "ねた"); else
	if( CMPTAIL("ふ") == 0) sprintf(&out[l-2], "へた"); else
	if( CMPTAIL("ぶ") == 0) sprintf(&out[l-2], "んだ"); else
	if( CMPTAIL("む") == 0) sprintf(&out[l-2], "んだ"); else
	if( CMPTAIL("る") == 0) sprintf(&out[l-2], "った"); else

	sprintf(&out[l], "ことや");
}


void jverb( const char *in , char *out , int flag)
{
  switch (flag)
  {
	  case JVERB_AND: jverb1(in , out); break;
	  case JVERB_TO : jverb2(in , out); break;
	  case JVERB_OR : jverb3(in , out); break;
  }
}

#if 0
static bool iskinsoku(cptr ch)
{
	cptr kinsoku[] =
	{
		"、", "。", "，", "．", "・", "？", "！",
			"ヽ", "ヾ", "ゝ", "ゞ", "々", "ー", "～", 
			"）", "］", "｝", "」", "』", 
			"ぁ", "ぃ", "ぅ", "ぇ", "ぉ", "っ", "ゃ", "ゅ", "ょ",
			"ァ", "ィ", "ゥ", "ェ", "ォ", "ッ", "ャ", "ュ", "ョ",
			NULL
	};

	int i;
	
	for (i = 0; kinsoku[i]; i++)
	{
		if (streq(ch, kinsoku[i]))
		{
			return TRUE;
		}
	}
	return FALSE;
}
#endif

/*
 * Convert SJIS string to EUC string
 */
void sjis2euc(char *str)
{
	int i;
	unsigned char c1, c2;
	unsigned char *tmp;

	int len = strlen(str);

	C_MAKE(tmp, len+1, byte);

	for (i = 0; i < len; i++)
	{
		c1 = str[i];
		if (c1 & 0x80)
		{
			i++;
			c2 = str[i];
			if (c2 >= 0x9f)
			{
				c1 = c1 * 2 - (c1 >= 0xe0 ? 0xe0 : 0x60);
				c2 += 2;
			}
			else
			{
				c1 = c1 * 2 - (c1 >= 0xe0 ? 0xe1 : 0x61);
				c2 += 0x60 + (c2 < 0x7f);
			}
			tmp[i - 1] = c1;
			tmp[i] = c2;
		}
		else
			tmp[i] = c1;
	}
	tmp[len] = 0;
	strcpy(str, (char *)tmp);

	C_KILL(tmp, len+1, byte);
}


/*
 * Convert EUC string to SJIS string
 */
void euc2sjis(char *str)
{
	int i;
	unsigned char c1, c2;
	unsigned char *tmp;

	int len = strlen(str);

	C_MAKE(tmp, len+1, byte);

	for (i = 0; i < len; i++)
	{
		c1 = str[i];
		if (c1 & 0x80)
		{
			i++;
			c2 = str[i];
			if (c1 % 2)
			{
				c1 = (c1 >> 1) + (c1 < 0xdf ? 0x31 : 0x71);
				c2 -= 0x60 + (c2 < 0xe0);
			}
			else
			{
				c1 = (c1 >> 1) + (c1 < 0xdf ? 0x30 : 0x70);
				c2 -= 2;
			}

			tmp[i - 1] = c1;
			tmp[i] = c2;
		}
		else
			tmp[i] = c1;
	}
	tmp[len] = 0;
	strcpy(str, (char *)tmp);

	C_KILL(tmp, len+1, byte);
}


/*
 * strを環境に合った文字コードに変換し、変換前の文字コードを返す。
 * strの長さに制限はない。
 *
 * 0: Unknown
 * 1: ASCII (Never known to be ASCII in this function.)
 * 2: EUC
 * 3: SJIS
 */
byte codeconv(char *str)
{
	byte code = 0;
	int i;

	for (i = 0; str[i]; i++)
	{
		unsigned char c1;
		unsigned char c2;

		/* First byte */
		c1 = str[i];

		/* ASCII? */
		if (!(c1 & 0x80)) continue;

		/* Second byte */
		i++;
		c2 = str[i];

		if (((0xa1 <= c1 && c1 <= 0xdf) || (0xfd <= c1 && c1 <= 0xfe)) &&
		    (0xa1 <= c2 && c2 <= 0xfe))
		{
			/* Only EUC is allowed */
			if (!code)
			{
				/* EUC */
				code = 2;
			}

			/* Broken string? */
			else if (code != 2)
			{
				/* No conversion */
				return 0;
			}
		}

		else if (((0x81 <= c1 && c1 <= 0x9f) &&
			  ((0x40 <= c2 && c2 <= 0x7e) || (0x80 <= c2 && c2 <= 0xfc))) ||
			 ((0xe0 <= c1 && c1 <= 0xfc) &&
			  (0x40 <= c2 && c2 <= 0x7e)))
		{
			/* Only SJIS is allowed */
			if (!code)
			{
				/* SJIS */
				code = 3;
			}

			/* Broken string? */
			else if (code != 3)
			{
				/* No conversion */
				return 0;
			}
		}
	}


	switch (code)
	{
#ifdef EUC
	case 3:
		/* SJIS -> EUC */
		sjis2euc(str);
		break;
#endif

#ifdef SJIS
	case 2:
		/* EUC -> SJIS */
		euc2sjis(str);

		break;
#endif
	}

	/* Return kanji code */
	return code;
}

/*!
 * @brief 文字列sのxバイト目が漢字の1バイト目かどうか判定する
 * @param s 判定する文字列のポインタ
 * @param x 判定する位置(バイト)
 * @return 漢字の1バイト目ならばTRUE
 */
bool iskanji2(cptr s, int x)
{
	int i;

	for (i = 0; i < x; i++)
	{
		if (iskanji(s[i])) i++;
	}
	if ((x == i) && iskanji(s[x])) return TRUE;

	return FALSE;
}

/*!
 * @brief 文字列の文字コードがASCIIかどうかを判定する
 * @param str 判定する文字列へのポインタ
 * @return 文字列の文字コードがASCIIならTRUE、そうでなければFALSE
 */
static bool is_ascii_str(cptr str)
{
	for (;*str; str++) {
		if (!(0x00 < *str && *str <= 0x7f))
			return FALSE;
	}
	return TRUE;
}

/*!
 * @brief 文字列の文字コードがUTF-8かどうかを判定する
 * @param str 判定する文字列へのポインタ
 * @return 文字列の文字コードがUTF-8ならTRUE、そうでなければFALSE
 */
static bool is_utf8_str(cptr str)
{
	const unsigned char* p;
	for (p = (const unsigned char*)str; *p; p++) {
		int subseq_num = 0;
		if (0x00 < *p && *p <= 0x7f) continue;
		
		if ((*p & 0xe0) == 0xc0) subseq_num = 1;
		if ((*p & 0xf0) == 0xe0) subseq_num = 2;
		if ((*p & 0xf8) == 0xf0) subseq_num = 3;

		if (subseq_num == 0) return FALSE;
		while (subseq_num--) {
			p++;
			if (!*p || (*p & 0xc0) != 0x80) return FALSE;
		}
	}
	return TRUE;
}

#if defined(EUC)
#include <iconv.h>

static const struct ms_to_jis_unicode_conv_t {
	char from[3];
	char to[3];
} ms_to_jis_unicode_conv[] = {
	{{0xef, 0xbd, 0x9e}, {0xe3, 0x80, 0x9c}}, /* FULLWIDTH TILDE -> WAVE DASH */
	{{0xef, 0xbc, 0x8d}, {0xe2, 0x88, 0x92}}, /* FULLWIDTH HYPHEN-MINUS -> MINUS SIGN */
};

/*!
 * @brief EUCがシステムコードである環境下向けにUTF-8から変換処理を行うサブルーチン
 * @param str 変換する文字列のポインタ
 * @return なし
 */
static void ms_to_jis_unicode(char* str)
{
	unsigned char* p;
	for (p = (unsigned char*)str; *p; p++) {
		int subseq_num = 0;
		if (0x00 < *p && *p <= 0x7f) continue;

		if ((*p & 0xe0) == 0xc0) subseq_num = 1;
		if ((*p & 0xf0) == 0xe0) {
			int i;
			for (i = 0; i < sizeof(ms_to_jis_unicode_conv) / sizeof(ms_to_jis_unicode_conv[0]); ++ i) {
				const struct ms_to_jis_unicode_conv_t *c = &ms_to_jis_unicode_conv[i];
				if (memcmp(p, c->from, 3) == 0) {
					memcpy(p, c->to, 3);
				}
			}
			subseq_num = 2;
                }
		if ((*p & 0xf8) == 0xf0) subseq_num = 3;

		p += subseq_num;
	}
}

#elif defined(SJIS) && defined(WINDOWS)
#include <Windows.h>
#endif
/*!
 * @brief 文字コードがUTF-8の文字列をシステムの文字コードに変換する
 * @param utf8_str 変換するUTF-8の文字列へのポインタ
 * @param sys_str_buffer 変換したシステムの文字コードの文字列を格納するバッファへのポインタ
 * @param sys_str_buflen 変換したシステムの文字コードの文字列を格納するバッファの長さ
 * @return 変換に成功した場合TRUE、失敗した場合FALSEを返す
 */
static bool utf8_to_sys(char* utf8_str, char* sys_str_buffer, size_t sys_str_buflen)
{
#if defined(EUC)

	iconv_t cd = iconv_open("EUC-JP", "UTF-8");
	size_t utf8_len = strlen(utf8_str) + 1; /* include termination character */
	char *from = utf8_str;
	int ret;

	ms_to_jis_unicode(utf8_str);
	ret = iconv(cd, &from, &utf8_len, &sys_str_buffer, &sys_str_buflen);
	iconv_close(cd);
	return (ret >= 0);

#elif defined(SJIS) && defined(WINDOWS)

	LPWSTR utf16buf;
	int input_len = strlen(utf8_str) + 1; /* include termination character */

	C_MAKE(utf16buf, input_len, WCHAR);

	/* UTF-8 -> UTF-16 */
	if (MultiByteToWideChar( CP_UTF8, 0, utf8_str, input_len, utf16buf, input_len) == 0) {
		C_KILL(utf16buf, input_len, WCHAR);
		return FALSE;
	}

	/* UTF-8 -> SJIS(CP932) */
	if (WideCharToMultiByte( CP_ACP, 0, utf16buf, -1, sys_str_buffer, sys_str_buflen, NULL, NULL ) == 0) {
		C_KILL(utf16buf, input_len, WCHAR);
		return FALSE;
	}

	C_KILL(utf16buf, input_len, WCHAR);
	return TRUE;

#endif
}

/*!
 * @brief 受け取った文字列の文字コードを推定し、システムの文字コードへ変換する
 * @param strbuf 変換する文字列を格納したバッファへのポインタ。
 *               バッファは変換した文字列で上書きされる。
 *               UTF-8からSJISもしくはEUCへの変換を想定しているのでバッファの長さが足りなくなることはない。
 * @param buflen バッファの長さ。
 * @return なし
 */
void guess_convert_to_system_encoding(char* strbuf, int buflen)
{
	if (is_ascii_str(strbuf)) return;

	if (is_utf8_str(strbuf)) {
		char* work;
		C_MAKE(work, buflen, char);
		my_strcpy(work, strbuf, buflen);
		if (!utf8_to_sys(work, strbuf, buflen)) {
			msg_print("警告:文字コードの変換に失敗しました");
			msg_print(NULL);
		}
		C_KILL(work, buflen, char);
	}
}

#endif /* JP */
