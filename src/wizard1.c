/* File: wizard1.c */

/* Purpose: Spoiler generation -BEN- */

#include "angband.h"


#ifdef ALLOW_SPOILERS


/*
 * The spoiler file being created
 */
static FILE *fff = NULL;



/*
 * Extract a textual representation of an attribute
 */
static cptr attr_to_text(byte a)
{
	switch (a)
	{
#ifdef JP
	case TERM_DARK:    return ("   XXX��");
	case TERM_WHITE:   return ("    ��");
	case TERM_SLATE:   return ("  �ĳ���");
	case TERM_ORANGE:  return ("�����");
	case TERM_RED:     return ("    �ֿ�");
	case TERM_GREEN:   return ("    �п�");
	case TERM_BLUE:    return ("    �Ŀ�");
	case TERM_UMBER:   return ("  ���ῧ");
	case TERM_L_DARK:  return ("    ����");
	case TERM_L_WHITE: return ("���ĳ���");
	case TERM_VIOLET:  return ("    �翧");
	case TERM_YELLOW:  return ("    ����");
	case TERM_L_RED:   return ("  ���ֿ�");
	case TERM_L_GREEN: return ("  ���п�");
	case TERM_L_BLUE:  return ("  ���Ŀ�");
	case TERM_L_UMBER: return ("�����ῧ");
#else
	case TERM_DARK:    return ("    xxx");
	case TERM_WHITE:   return ("  White");
	case TERM_SLATE:   return ("  Slate");
	case TERM_ORANGE:  return (" Orange");
	case TERM_RED:     return ("    Red");
	case TERM_GREEN:   return ("  Green");
	case TERM_BLUE:    return ("   Blue");
	case TERM_UMBER:   return ("  Umber");
	case TERM_L_DARK:  return (" L.Dark");
	case TERM_L_WHITE: return ("L.Slate");
	case TERM_VIOLET:  return (" Violet");
	case TERM_YELLOW:  return (" Yellow");
	case TERM_L_RED:   return ("  L.Red");
	case TERM_L_GREEN: return ("L.Green");
	case TERM_L_BLUE:  return (" L.Blue");
	case TERM_L_UMBER: return ("L.Umber");
#endif
	}

	/* Oops */
#ifdef JP
	return ("    �Ѥ�");
#else
	return ("   Icky");
#endif
}



/*
 * A tval grouper
 */
typedef struct
{
	byte tval;
	cptr name;
} grouper;



/*
 * Item Spoilers by: benh@phial.com (Ben Harrison)
 */


/*
 * The basic items categorized by type
 */
static grouper group_item[] =
{
#ifdef JP
	{ TV_SHOT,          "�ͷ�ʪ" },
	{ TV_ARROW,         NULL },
	{ TV_BOLT,          NULL },
	{ TV_BOW,           "��" },

	{ TV_SWORD,         "���" },
	{ TV_POLEARM,       NULL },
	{ TV_HAFTED,        NULL },
	{ TV_DIGGING,       NULL },

	{ TV_SOFT_ARMOR,    "�ɶ���Ρ�" },
	{ TV_HARD_ARMOR,    NULL },
	{ TV_DRAG_ARMOR,    NULL },
	{ TV_CLOAK,         "�ɶ�ʤ���¾��" },
	{ TV_SHIELD,        NULL },
	{ TV_HELM,          NULL },
	{ TV_CROWN,         NULL },
	{ TV_GLOVES,        NULL },
	{ TV_BOOTS,         NULL },

	{ TV_AMULET,        "���ߥ��å�" },
	{ TV_RING,          "����" },
	{ TV_SCROLL,        "��ʪ" },
	{ TV_POTION,        "��" },
	{ TV_FOOD,          "����" },

	{ TV_ROD,           "��å�" },
	{ TV_WAND,          "����" },
	{ TV_STAFF,         "�����å�" },

	{ TV_LIFE_BOOK,     "��ˡ�����̿��" },
	{ TV_SORCERY_BOOK,  "��ˡ�����ѡ�" },

	{ TV_CHEST,         "Ȣ" },
	{ TV_FIGURINE,      "�ͷ�" },
	{ TV_STATUE,        "��" },
	{ TV_CORPSE,        "����" },
	{ TV_SPIKE,         "����¾" },
	{ TV_LITE,          NULL },
	{ TV_FLASK,         NULL },
	{ TV_JUNK,          NULL },
	{ TV_BOTTLE,        NULL },
	{ TV_SKELETON,      NULL },
#else
	{ TV_SHOT,          "Ammo" },
	{ TV_ARROW,         NULL },
	{ TV_BOLT,          NULL },
	{ TV_BOW,           "Bows" },

	{ TV_SWORD,         "Weapons" },
	{ TV_POLEARM,       NULL },
	{ TV_HAFTED,        NULL },
	{ TV_DIGGING,       NULL },

	{ TV_SOFT_ARMOR,    "Armour (Body)" },
	{ TV_HARD_ARMOR,    NULL },
	{ TV_DRAG_ARMOR,    NULL },
	{ TV_CLOAK,         "Armour (Misc)" },
	{ TV_SHIELD,        NULL },
	{ TV_HELM,          NULL },
	{ TV_CROWN,         NULL },
	{ TV_GLOVES,        NULL },
	{ TV_BOOTS,         NULL },

	{ TV_AMULET,        "Amulets" },
	{ TV_RING,          "Rings" },
	{ TV_SCROLL,        "Scrolls" },
	{ TV_POTION,        "Potions" },
	{ TV_FOOD,          "Food" },

	{ TV_ROD,           "Rods" },
	{ TV_WAND,          "Wands" },
	{ TV_STAFF,         "Staffs" },

	{ TV_LIFE_BOOK,     "Books (Life)" },
	{ TV_SORCERY_BOOK,  "Books (Sorcery)" },

	{ TV_CHEST,         "Chests" },
	{ TV_FIGURINE,      "Magical Figurines" },
	{ TV_STATUE,        "Statues" },
	{ TV_CORPSE,        "Corpses" },
	{ TV_SPIKE,         "Various" },
	{ TV_LITE,          NULL },
	{ TV_FLASK,         NULL },
	{ TV_JUNK,          NULL },
	{ TV_BOTTLE,        NULL },
	{ TV_SKELETON,      NULL },
#endif
	
	{ 0, "" }
};


/*
 * Describe the kind
 */
static void kind_info(char *buf, char *dam, char *wgt, int *lev, s32b *val, int k)
{
	object_type forge;
	object_type *q_ptr;


	/* Get local object */
	q_ptr = &forge;

	/* Prepare a fake item */
	object_prep(q_ptr, k);

	/* It is known */
	q_ptr->ident |= (IDENT_KNOWN);

	/* Cancel bonuses */
	q_ptr->pval = 0;
	q_ptr->to_a = 0;
	q_ptr->to_h = 0;
	q_ptr->to_d = 0;


	/* Level */
	(*lev) = get_object_level(q_ptr);

	/* Value */
	(*val) = object_value(q_ptr);


	/* Hack */
	if (!buf || !dam || !wgt) return;


	/* Description (too brief) */
	object_desc(buf, q_ptr, OD_NAME_ONLY | OD_STORE);


	/* Misc info */
	strcpy(dam, "");

	/* Damage */
	switch (q_ptr->tval)
	{
		/* Bows */
		case TV_BOW:
		{
			break;
		}

		/* Ammo */
		case TV_SHOT:
		case TV_BOLT:
		case TV_ARROW:
		{
			sprintf(dam, "%dd%d", q_ptr->dd, q_ptr->ds);
			break;
		}

		/* Weapons */
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_DIGGING:
		{
			sprintf(dam, "%dd%d", q_ptr->dd, q_ptr->ds);
			break;
		}

		/* Armour */
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_CLOAK:
		case TV_CROWN:
		case TV_HELM:
		case TV_SHIELD:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
		{
			sprintf(dam, "%d", q_ptr->ac);
			break;
		}
	}


	/* Weight */
	sprintf(wgt, "%3d.%d", q_ptr->weight / 10, q_ptr->weight % 10);
}


/*
 * Create a spoiler file for items
 */
static void spoil_obj_desc(cptr fname)
{
	int i, k, s, t, n = 0;

	u16b who[200];

	char buf[1024];

	char wgt[80];
	char dam[80];


	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, fname);

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	/* Open the file */
	fff = my_fopen(buf, "w");

	/* Oops */
	if (!fff)
	{
		msg_print("Cannot create spoiler file.");
		return;
	}


	/* Header */
	fprintf(fff, "Spoiler File -- Basic Items (%s %d.%d.%d)\n\n\n",
		VERSION_NAME, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

	/* More Header */
	fprintf(fff, "%-45s     %8s%7s%5s%9s\n",
		"Description", "Dam/AC", "Wgt", "Lev", "Cost");
	fprintf(fff, "%-45s     %8s%7s%5s%9s\n",
		"----------------------------------------",
		"------", "---", "---", "----");

	/* List the groups */
	for (i = 0; TRUE; i++)
	{
		/* Write out the group title */
		if (group_item[i].name)
		{
			/* Hack -- bubble-sort by cost and then level */
			for (s = 0; s < n - 1; s++)
			{
				for (t = 0; t < n - 1; t++)
				{
					int i1 = t;
					int i2 = t + 1;

					int e1;
					int e2;

					s32b t1;
					s32b t2;

					kind_info(NULL, NULL, NULL, &e1, &t1, who[i1]);
					kind_info(NULL, NULL, NULL, &e2, &t2, who[i2]);

					if ((t1 > t2) || ((t1 == t2) && (e1 > e2)))
					{
						int tmp = who[i1];
						who[i1] = who[i2];
						who[i2] = tmp;
					}
				}
			}

			/* Spoil each item */
			for (s = 0; s < n; s++)
			{
				int e;
				s32b v;

				/* Describe the kind */
				kind_info(buf, dam, wgt, &e, &v, who[s]);

				/* Dump it */
				fprintf(fff, "     %-45s%8s%7s%5d%9ld\n",
					buf, dam, wgt, e, (long)(v));
			}

			/* Start a new set */
			n = 0;

			/* Notice the end */
			if (!group_item[i].tval) break;

			/* Start a new set */
			fprintf(fff, "\n\n%s\n\n", group_item[i].name);
		}

		/* Acquire legal item types */
		for (k = 1; k < max_k_idx; k++)
		{
			object_kind *k_ptr = &k_info[k];

			/* Skip wrong tval's */
			if (k_ptr->tval != group_item[i].tval) continue;

			/* Hack -- Skip instant-artifacts */
			if (k_ptr->gen_flags & (TRG_INSTA_ART)) continue;

			/* Save the index */
			who[n++] = k;
		}
	}


	/* Check for errors */
	if (ferror(fff) || my_fclose(fff))
	{
		msg_print("Cannot close spoiler file.");
		return;
	}

	/* Message */
	msg_print("Successfully created a spoiler file.");
}


/*
 * Artifact Spoilers by: randy@PICARD.tamu.edu (Randy Hutson)
 */


/*
 * Returns a "+" string if a number is non-negative and an empty
 * string if negative
 */
#define POSITIZE(v) (((v) >= 0) ? "+" : "")

/*
 * These are used to format the artifact spoiler file. INDENT1 is used
 * to indent all but the first line of an artifact spoiler. INDENT2 is
 * used when a line "wraps". (Bladeturner's resistances cause this.)
 */
#define INDENT1 "    "
#define INDENT2 "      "

/*
 * MAX_LINE_LEN specifies when a line should wrap.
 */
#define MAX_LINE_LEN 75

/*
 * Given an array, determine how many elements are in the array
 */
#define N_ELEMENTS(a) (sizeof (a) / sizeof ((a)[0]))

/*
 * The artifacts categorized by type
 */
static grouper group_artifact[] =
{
#ifdef JP
	{ TV_SWORD,             "���" },
	{ TV_POLEARM,           "��/��" },
	{ TV_HAFTED,            "�ߴ�" },
	{ TV_DIGGING,           "����٥�/�Ĥ�Ϥ�" },
	{ TV_BOW,               "����ƻ��" },
	{ TV_ARROW,             "��" },

	{ TV_SOFT_ARMOR,        "��" },
	{ TV_HARD_ARMOR,        NULL },
	{ TV_DRAG_ARMOR,        NULL },

	{ TV_CLOAK,             "������" },
	{ TV_SHIELD,            "��" },
	{ TV_HELM,              "��/��" },
	{ TV_CROWN,             NULL },
	{ TV_GLOVES,            "�Ƽ�" },
	{ TV_BOOTS,             "��" },

	{ TV_LITE,              "����" },
	{ TV_AMULET,            "���ߥ��å�" },
	{ TV_RING,              "����" },
#else
	{ TV_SWORD,             "Edged Weapons" },
	{ TV_POLEARM,           "Polearms" },
	{ TV_HAFTED,            "Hafted Weapons" },
	{ TV_DIGGING,           "Shovels/Picks" },
	{ TV_BOW,               "Bows" },
	{ TV_ARROW,             "Ammo" },

	{ TV_SOFT_ARMOR,        "Body Armor" },
	{ TV_HARD_ARMOR,        NULL },
	{ TV_DRAG_ARMOR,        NULL },

	{ TV_CLOAK,             "Cloaks" },
	{ TV_SHIELD,            "Shields" },
	{ TV_HELM,              "Helms/Crowns" },
	{ TV_CROWN,             NULL },
	{ TV_GLOVES,            "Gloves" },
	{ TV_BOOTS,             "Boots" },

	{ TV_LITE,              "Light Sources" },
	{ TV_AMULET,            "Amulets" },
	{ TV_RING,              "Rings" },
#endif
	{ 0, NULL }
};



/*
 * Pair together a constant flag with a textual description.
 *
 * Used by both "init.c" and "wiz-spo.c".
 *
 * Note that it sometimes more efficient to actually make an array
 * of textual names, where entry 'N' is assumed to be paired with
 * the flag whose value is "1L << N", but that requires hard-coding.
 */

typedef struct flag_desc flag_desc;

struct flag_desc
{
	const u32b flag;
	const char *const desc;
};



/*
 * These are used for "+3 to STR, DEX", etc. These are separate from
 * the other pval affected traits to simplify the case where an object
 * affects all stats.  In this case, "All stats" is used instead of
 * listing each stat individually.
 */

static flag_desc stat_flags_desc[] =
{
#ifdef JP
	{ TR1_STR,        "����" },
	{ TR1_INT,        "��ǽ" },
	{ TR1_WIS,        "����" },
	{ TR1_DEX,        "���Ѥ�" },
	{ TR1_CON,        "�ѵ���" },
	{ TR1_CHR,        "̥��" }
#else
	{ TR1_STR,        "STR" },
	{ TR1_INT,        "INT" },
	{ TR1_WIS,        "WIS" },
	{ TR1_DEX,        "DEX" },
	{ TR1_CON,        "CON" },
	{ TR1_CHR,        "CHR" }
#endif
};

/*
 * Besides stats, these are the other player traits
 * which may be affected by an object's pval
 */

static flag_desc pval_flags1_desc[] =
{
#ifdef JP
	{ TR1_STEALTH,    "��̩" },
	{ TR1_SEARCH,     "õ��" },
	{ TR1_INFRA,      "�ֳ�������" },
	{ TR1_TUNNEL,     "�η�" },
	{ TR1_BLOWS,      "������" },
	{ TR1_SPEED,      "���ԡ���" },
	{ TR1_MAGIC_MASTERY, "��ˡƻ�����" }
#else
	{ TR1_STEALTH,    "Stealth" },
	{ TR1_SEARCH,     "Searching" },
	{ TR1_INFRA,      "Infravision" },
	{ TR1_TUNNEL,     "Tunneling" },
	{ TR1_BLOWS,      "Attacks" },
	{ TR1_SPEED,      "Speed" },
	{ TR1_MAGIC_MASTERY, "Magic device" }
#endif
};

/*
 * Slaying preferences for weapons
 */

static flag_desc slay_flags_desc[] =
{
#ifdef JP
	{ TR1_SLAY_HUMAN,         "�ʹ�" },
	{ TR1_SLAY_ANIMAL,        "ưʪ" },
	{ TR1_SLAY_EVIL,          "�ٰ�" },
	{ TR1_SLAY_UNDEAD,        "����ǥå�" },
	{ TR1_SLAY_DEMON,         "����" },
	{ TR1_SLAY_ORC,           "������" },
	{ TR1_SLAY_TROLL,         "�ȥ��" },
	{ TR1_SLAY_GIANT,         "���" },
	{ TR1_SLAY_DRAGON,        "�ɥ饴��" },
	{ TR1_KILL_DRAGON,        "*�ɥ饴��*" },
#else
	{ TR1_SLAY_HUMAN,         "Human" },
	{ TR1_SLAY_ANIMAL,        "Animal" },
	{ TR1_SLAY_EVIL,          "Evil" },
	{ TR1_SLAY_UNDEAD,        "Undead" },
	{ TR1_SLAY_DEMON,         "Demon" },
	{ TR1_SLAY_ORC,           "Orc" },
	{ TR1_SLAY_TROLL,         "Troll" },
	{ TR1_SLAY_GIANT,         "Giant" },
	{ TR1_SLAY_DRAGON,        "Dragon" },
	{ TR1_KILL_DRAGON,        "Xdragon" }
#endif
};

/*
 * Elemental brands for weapons
 *
 * Clearly, TR1_IMPACT is a bit out of place here. To simplify
 * coding, it has been included here along with the elemental
 * brands. It does seem to fit in with the brands and slaying
 * more than the miscellaneous section.
 */
static flag_desc brand_flags_desc[] =
{
#ifdef JP
	{ TR1_BRAND_ACID,         "�ϲ�" },
	{ TR1_BRAND_ELEC,         "�ŷ�" },
	{ TR1_BRAND_FIRE,         "�ƴ�" },
	{ TR1_BRAND_COLD,         "���" },
	{ TR1_BRAND_POIS,         "�ǻ�" },

	{ TR1_CHAOTIC,            "����" },
	{ TR1_VAMPIRIC,           "�۷�" },
	{ TR1_IMPACT,             "�Ͽ�" },
	{ TR1_VORPAL,             "�ڤ�̣" },
#else
	{ TR1_BRAND_ACID,         "Acid Brand" },
	{ TR1_BRAND_ELEC,         "Lightning Brand" },
	{ TR1_BRAND_FIRE,         "Flame Tongue" },
	{ TR1_BRAND_COLD,         "Frost Brand" },
	{ TR1_BRAND_POIS,         "Poisoned" },

	{ TR1_CHAOTIC,            "Mark of Chaos" },
	{ TR1_VAMPIRIC,           "Vampiric" },
	{ TR1_IMPACT,             "Earthquake impact on hit" },
	{ TR1_VORPAL,             "Very sharp" },
#endif
};


/*
 * The 15 resistables
 */
static const flag_desc resist_flags_desc[] =
{
#ifdef JP
	{ TR2_RES_ACID,   "��" },
	{ TR2_RES_ELEC,   "�ŷ�" },
	{ TR2_RES_FIRE,   "�б�" },
	{ TR2_RES_COLD,   "�䵤" },
	{ TR2_RES_POIS,   "��" },
	{ TR2_RES_FEAR,   "����"},
	{ TR2_RES_LITE,   "����" },
	{ TR2_RES_DARK,   "�Ź�" },
	{ TR2_RES_BLIND,  "����" },
	{ TR2_RES_CONF,   "����" },
	{ TR2_RES_SOUND,  "�첻" },
	{ TR2_RES_SHARDS, "����" },
	{ TR2_RES_NETHER, "�Ϲ�" },
	{ TR2_RES_NEXUS,  "���̺���" },
	{ TR2_RES_CHAOS,  "������" },
	{ TR2_RES_DISEN,  "����" },
#else
	{ TR2_RES_ACID,   "Acid" },
	{ TR2_RES_ELEC,   "Lightning" },
	{ TR2_RES_FIRE,   "Fire" },
	{ TR2_RES_COLD,   "Cold" },
	{ TR2_RES_POIS,   "Poison" },
	{ TR2_RES_FEAR,   "Fear"},
	{ TR2_RES_LITE,   "Light" },
	{ TR2_RES_DARK,   "Dark" },
	{ TR2_RES_BLIND,  "Blindness" },
	{ TR2_RES_CONF,   "Confusion" },
	{ TR2_RES_SOUND,  "Sound" },
	{ TR2_RES_SHARDS, "Shards" },
	{ TR2_RES_NETHER, "Nether" },
	{ TR2_RES_NEXUS,  "Nexus" },
	{ TR2_RES_CHAOS,  "Chaos" },
	{ TR2_RES_DISEN,  "Disenchantment" },
#endif
};

/*
 * Elemental immunities (along with poison)
 */

static const flag_desc immune_flags_desc[] =
{
#ifdef JP
	{ TR2_IM_ACID,    "��" },
	{ TR2_IM_ELEC,    "�ŷ�" },
	{ TR2_IM_FIRE,    "�б�" },
	{ TR2_IM_COLD,    "�䵤" },
#else
	{ TR2_IM_ACID,    "Acid" },
	{ TR2_IM_ELEC,    "Lightning" },
	{ TR2_IM_FIRE,    "Fire" },
	{ TR2_IM_COLD,    "Cold" },
#endif
};

/*
 * Sustain stats -  these are given their "own" line in the
 * spoiler file, mainly for simplicity
 */
static const flag_desc sustain_flags_desc[] =
{
#ifdef JP
	{ TR2_SUST_STR,   "����" },
	{ TR2_SUST_INT,   "��ǽ" },
	{ TR2_SUST_WIS,   "����" },
	{ TR2_SUST_DEX,   "���Ѥ�" },
	{ TR2_SUST_CON,   "�ѵ���" },
	{ TR2_SUST_CHR,   "̥��" },
#else
	{ TR2_SUST_STR,   "STR" },
	{ TR2_SUST_INT,   "INT" },
	{ TR2_SUST_WIS,   "WIS" },
	{ TR2_SUST_DEX,   "DEX" },
	{ TR2_SUST_CON,   "CON" },
	{ TR2_SUST_CHR,   "CHR" },
#endif
};

/*
 * Miscellaneous magic given by an object's "flags2" field
 */

static const flag_desc misc_flags2_desc[] =
{
#ifdef JP
	{ TR2_THROW,      "��ڳ" },
	{ TR2_REFLECT,    "ȿ��" },
	{ TR2_FREE_ACT,   "�����Τ餺" },
	{ TR2_HOLD_LIFE,  "��̿�ϰݻ�" },
#else
	{ TR2_THROW,      "Throwing" },
	{ TR2_REFLECT,    "Reflection" },
	{ TR2_FREE_ACT,   "Free Action" },
	{ TR2_HOLD_LIFE,  "Hold Life" },
#endif
};

/*
 * Miscellaneous magic given by an object's "flags3" field
 *
 * Note that cursed artifacts and objects with permanent light
 * are handled "directly" -- see analyze_misc_magic()
 */

static const flag_desc misc_flags3_desc[] =
{
#ifdef JP
	{ TR3_SH_FIRE,            "�бꥪ����" },
	{ TR3_SH_ELEC,            "�ŷ⥪����" },
	{ TR3_SH_COLD,            "�䵤������" },
	{ TR3_NO_TELE,            "ȿ�ƥ�ݡ���" },
	{ TR3_NO_MAGIC,           "ȿ��ˡ" },
	{ TR3_WRAITH,             "ͩ�β�" },
	{ TR3_FEATHER,            "��ͷ" },
	{ TR3_SEE_INVIS,          "�Ļ�Ʃ��" },
	{ TR3_TELEPATHY,          "�ƥ�ѥ���" },
	{ TR3_SLOW_DIGEST,        "�پò�" },
	{ TR3_REGEN,              "��®����" },
	{ TR3_WARNING,            "�ٹ�" },
	{ TR3_XTRA_SHOTS,         "�ɲüͷ�" },        /* always +1? */
	{ TR3_DRAIN_EXP,          "�и��͵ۼ�" },
	{ TR3_AGGRAVATE,          "ȿ��" },
	{ TR3_BLESSED,            "��ʡ" },
	{ TR3_DEC_MANA,           "�������ϸ���" },
#else
	{ TR3_SH_FIRE,            "Fiery Aura" },
	{ TR3_SH_ELEC,            "Electric Aura" },
	{ TR3_SH_COLD,            "Cold Aura" },
	{ TR3_NO_TELE,            "Prevent Teleportation" },
	{ TR3_NO_MAGIC,           "Anti-Magic" },
	{ TR3_WRAITH,             "Wraith Form" },
	{ TR3_FEATHER,            "Levitation" },
	{ TR3_SEE_INVIS,          "See Invisible" },
	{ TR3_TELEPATHY,          "ESP" },
	{ TR3_SLOW_DIGEST,        "Slow Digestion" },
	{ TR3_WARNING,            "Warning" },
	{ TR3_REGEN,              "Regeneration" },
	{ TR3_XTRA_SHOTS,         "+1 Extra Shot" },        /* always +1? */
	{ TR3_DRAIN_EXP,          "Drains Experience" },
	{ TR3_AGGRAVATE,          "Aggravates" },
	{ TR3_BLESSED,            "Blessed Blade" },
	{ TR3_DEC_MANA,           "Decrease Need Mana" },
#endif
};


/*
 * A special type used just for deailing with pvals
 */
typedef struct
{
	/*
	 * This will contain a string such as "+2", "-10", etc.
	 */
	char pval_desc[12];

	/*
	 * A list of various player traits affected by an object's pval such
	 * as stats, speed, stealth, etc.  "Extra attacks" is NOT included in
	 * this list since it will probably be desirable to format its
	 * description differently.
	 *
	 * Note that room need only be reserved for the number of stats - 1
	 * since the description "All stats" is used if an object affects all
	 * all stats. Also, room must be reserved for a sentinel NULL pointer.
	 *
	 * This will be a list such as ["STR", "DEX", "Stealth", NULL] etc.
	 *
	 * This list includes extra attacks, for simplicity.
	 */
	cptr pval_affects[N_ELEMENTS(stat_flags_desc) - 1 +
			  N_ELEMENTS(pval_flags1_desc) + 1];

} pval_info_type;


/*
 * An "object analysis structure"
 *
 * It will be filled with descriptive strings detailing an object's
 * various magical powers. The "ignore X" traits are not noted since
 * all artifacts ignore "normal" destruction.
 */

typedef struct
{
	/* "The Longsword Dragonsmiter (6d4) (+20, +25)" */
	char description[160];

	/* Description of what is affected by an object's pval */
	pval_info_type pval_info;

	/* A list of an object's slaying preferences */
	cptr slays[N_ELEMENTS(slay_flags_desc) + 1];

	/* A list if an object's elemental brands */
	cptr brands[N_ELEMENTS(brand_flags_desc) + 1];

	/* A list of immunities granted by an object */
	cptr immunities[N_ELEMENTS(immune_flags_desc) + 1];

	/* A list of resistances granted by an object */
	cptr resistances[N_ELEMENTS(resist_flags_desc) + 1];

	/* A list of stats sustained by an object */
	cptr sustains[N_ELEMENTS(sustain_flags_desc)  - 1 + 1];

	/* A list of various magical qualities an object may have */
	cptr misc_magic[N_ELEMENTS(misc_flags2_desc) + N_ELEMENTS(misc_flags3_desc)
			+ 1       /* Permanent Light */
			+ 1       /* TY curse */
			+ 1       /* type of curse */
			+ 1];     /* sentinel NULL */

	/* A string describing an artifact's activation */
	cptr activation;

	/* "Level 20, Rarity 30, 3.0 lbs, 20000 Gold" */
	char misc_desc[80];
} obj_desc_list;


/*
 * Write out `n' of the character `c' to the spoiler file
 */
static void spoiler_out_n_chars(int n, char c)
{
	while (--n >= 0) fputc(c, fff);
}


/*
 * Write out `n' blank lines to the spoiler file
 */
static void spoiler_blanklines(int n)
{
	spoiler_out_n_chars(n, '\n');
}


/*
 * Write a line to the spoiler file and then "underline" it with hypens
 */
static void spoiler_underline(cptr str)
{
	fprintf(fff, "%s\n", str);
	spoiler_out_n_chars(strlen(str), '-');
	fprintf(fff, "\n");
}



/*
 * This function does most of the actual "analysis". Given a set of bit flags
 * (which will be from one of the flags fields from the object in question),
 * a "flag description structure", a "description list", and the number of
 * elements in the "flag description structure", this function sets the
 * "description list" members to the appropriate descriptions contained in
 * the "flag description structure".
 *
 * The possibly updated description pointer is returned.
 */
static cptr *spoiler_flag_aux(const u32b art_flags, const flag_desc *flag_ptr,
			      cptr *desc_ptr, const int n_elmnts)
{
	int i;

	for (i = 0; i < n_elmnts; ++i)
	{
		if (art_flags & flag_ptr[i].flag)
		{
			*desc_ptr++ = flag_ptr[i].desc;
		}
	}

	return desc_ptr;
}


/*
 * Acquire a "basic" description "The Cloak of Death [1,+10]"
 */
static void analyze_general (object_type *o_ptr, char *desc_ptr)
{
	/* Get a "useful" description of the object */
	object_desc(desc_ptr, o_ptr, OD_NAME_AND_ENCHANT | OD_STORE);
}


/*
 * List "player traits" altered by an artifact's pval. These include stats,
 * speed, infravision, tunneling, stealth, searching, and extra attacks.
 */
static void analyze_pval (object_type *o_ptr, pval_info_type *p_ptr)
{
	const u32b all_stats = (TR1_STR | TR1_INT | TR1_WIS |
				TR1_DEX | TR1_CON | TR1_CHR);

	u32b f1, f2, f3;

	cptr *affects_list;

	/* If pval == 0, there is nothing to do. */
	if (!o_ptr->pval)
	{
		/* An "empty" pval description indicates that pval == 0 */
		p_ptr->pval_desc[0] = '\0';
		return;
	}

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3);

	affects_list = p_ptr->pval_affects;

	/* Create the "+N" string */
	sprintf(p_ptr->pval_desc, "%s%d", POSITIZE(o_ptr->pval), 
		(int)(o_ptr->pval));

	/* First, check to see if the pval affects all stats */
	if ((f1 & all_stats) == all_stats)
	{
#ifdef JP
		*affects_list++ = "��ǽ��";
#else
		*affects_list++ = "All stats";
#endif
	}

	/* Are any stats affected? */
	else if (f1 & all_stats)
	{
		affects_list = spoiler_flag_aux(f1, stat_flags_desc,
						affects_list,
						N_ELEMENTS(stat_flags_desc));
	}

	/* And now the "rest" */
	affects_list = spoiler_flag_aux(f1, pval_flags1_desc,
					affects_list,
					N_ELEMENTS(pval_flags1_desc));

	/* Terminate the description list */
	*affects_list = NULL;
}


/* Note the slaying specialties of a weapon */
static void analyze_slay (object_type *o_ptr, cptr *slay_list)
{
	u32b f1, f2, f3;

	object_flags(o_ptr, &f1, &f2, &f3);

	slay_list = spoiler_flag_aux(f1, slay_flags_desc, slay_list,
				     N_ELEMENTS(slay_flags_desc));

	/* Terminate the description list */
	*slay_list = NULL;
}

/* Note an object's elemental brands */
static void analyze_brand (object_type *o_ptr, cptr *brand_list)
{
	u32b f1, f2, f3;

	object_flags(o_ptr, &f1, &f2, &f3);

	brand_list = spoiler_flag_aux(f1, brand_flags_desc, brand_list,
				      N_ELEMENTS(brand_flags_desc));

	/* Terminate the description list */
	*brand_list = NULL;
}


/* Note the resistances granted by an object */
static void analyze_resist (object_type *o_ptr, cptr *resist_list)
{
	u32b f1, f2, f3;

	object_flags(o_ptr, &f1, &f2, &f3);

	resist_list = spoiler_flag_aux(f2, resist_flags_desc,
				       resist_list, N_ELEMENTS(resist_flags_desc));

	/* Terminate the description list */
	*resist_list = NULL;
}


/* Note the immunities granted by an object */
static void analyze_immune (object_type *o_ptr, cptr *immune_list)
{
	u32b f1, f2, f3;

	object_flags(o_ptr, &f1, &f2, &f3);

	immune_list = spoiler_flag_aux(f2, immune_flags_desc,
				       immune_list, N_ELEMENTS(immune_flags_desc));

	/* Terminate the description list */
	*immune_list = NULL;
}

/* Note which stats an object sustains */

static void analyze_sustains (object_type *o_ptr, cptr *sustain_list)
{
	const u32b all_sustains = (TR2_SUST_STR | TR2_SUST_INT | TR2_SUST_WIS |
				   TR2_SUST_DEX | TR2_SUST_CON | TR2_SUST_CHR);

	u32b f1, f2, f3;

	object_flags(o_ptr, &f1, &f2, &f3);

	/* Simplify things if an item sustains all stats */
	if ((f2 & all_sustains) == all_sustains)
	{
#ifdef JP
		*sustain_list++ = "��ǽ��";
#else
		*sustain_list++ = "All stats";
#endif
	}

	/* Should we bother? */
	else if ((f2 & all_sustains))
	{
		sustain_list = spoiler_flag_aux(f2, sustain_flags_desc,
						sustain_list,
						N_ELEMENTS(sustain_flags_desc));
	}

	/* Terminate the description list */
	*sustain_list = NULL;
}


/*
 * Note miscellaneous powers bestowed by an artifact such as see invisible,
 * free action, permanent light, etc.
 */
static void analyze_misc_magic (object_type *o_ptr, cptr *misc_list)
{
	u32b f1, f2, f3;

	object_flags(o_ptr, &f1, &f2, &f3);

	misc_list = spoiler_flag_aux(f2, misc_flags2_desc, misc_list,
				     N_ELEMENTS(misc_flags2_desc));

	misc_list = spoiler_flag_aux(f3, misc_flags3_desc, misc_list,
				     N_ELEMENTS(misc_flags3_desc));

	/*
	 * Artifact lights -- large radius light.
	 */
	if ((o_ptr->tval == TV_LITE) && artifact_p(o_ptr))
	{
#ifdef JP
		*misc_list++ = "�ʵ׸���(Ⱦ��3)";
#else
		*misc_list++ = "Permanent Light(3)";
#endif
	}

	/*
	 * Glowing artifacts -- small radius light.
	 */
	if (f3 & (TR3_LITE))
	{
#ifdef JP
		*misc_list++ = "�ʵ׸���(Ⱦ��1)";
#else
		*misc_list++ = "Permanent Light(1)";
#endif
	}

	/*
	 * Handle cursed objects here to avoid redundancies such as noting
	 * that a permanently cursed object is heavily cursed as well as
	 * being "lightly cursed".
	 */

	if (f3 & TR3_TY_CURSE)
	{
#ifdef JP
		*misc_list++ = "���Ťα�ǰ";
#else
		*misc_list++ = "Ancient Curse";
#endif
	}

	if (f3 & TR3_PERMA_CURSE)
	{
#ifdef JP
		*misc_list++ = "�ʱ�μ���";
#else
		*misc_list++ = "Permanently Cursed";
#endif
	}
	else if (f3 & TR3_HEAVY_CURSE)
	{
#ifdef JP
		*misc_list++ = "���Ϥʼ���";
#else
		*misc_list++ = "Heavily Cursed";
#endif
	}
	else if (f3 & TR3_CURSED)
	{
#ifdef JP
		*misc_list++ = "����";
#else
		*misc_list++ = "Cursed";
#endif
	}

	/* Terminate the description list */
	*misc_list = NULL;
}




/*
 * Determine the minimum depth an artifact can appear, its rarity, its weight,
 * and its value in gold pieces
 */
static void analyze_misc (object_type *o_ptr, char *misc_desc)
{
	artifact_type *a_ptr = &a_info[o_ptr->name1];

#ifdef JP
	sprintf(misc_desc, "��٥� %u, ������ %u, %d.%d kg, ��%ld",
		a_ptr->level, a_ptr->rarity,
		lbtokg1(a_ptr->weight), lbtokg2(a_ptr->weight), (long int)a_ptr->cost);
#else
	sprintf(misc_desc, "Level %u, Rarity %u, %d.%d lbs, %ld Gold",
		a_ptr->level, a_ptr->rarity,
		a_ptr->weight / 10, a_ptr->weight % 10, (long int)a_ptr->cost);
#endif
}


/*
 * Fill in an object description structure for a given object
 */
static void object_analyze(object_type *o_ptr, obj_desc_list *desc_ptr)
{
	analyze_general(o_ptr, desc_ptr->description);

	analyze_pval(o_ptr, &desc_ptr->pval_info);

	analyze_brand(o_ptr, desc_ptr->brands);

	analyze_slay(o_ptr, desc_ptr->slays);

	analyze_immune(o_ptr, desc_ptr->immunities);

	analyze_resist(o_ptr, desc_ptr->resistances);

	analyze_sustains(o_ptr, desc_ptr->sustains);

	analyze_misc_magic(o_ptr, desc_ptr->misc_magic);

	analyze_misc(o_ptr, desc_ptr->misc_desc);

	desc_ptr->activation = item_activation(o_ptr);
}

static void print_header(void)
{
	char buf[80];

	sprintf(buf, "Artifact Spoilers for %s Version %d.%d.%d",
		VERSION_NAME, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
	spoiler_underline(buf);
}

/*
 * This is somewhat ugly.
 *
 * Given a header ("Resist", e.g.), a list ("Fire", "Cold", Acid", e.g.),
 * and a separator character (',', e.g.), write the list to the spoiler file
 * in a "nice" format, such as:
 *
 *      Resist Fire, Cold, Acid
 *
 * That was a simple example, but when the list is long, a line wrap
 * should occur, and this should induce a new level of indention if
 * a list is being spread across lines. So for example, Bladeturner's
 * list of resistances should look something like this
 *
 *     Resist Acid, Lightning, Fire, Cold, Poison, Light, Dark, Blindness,
 *       Confusion, Sound, Shards, Nether, Nexus, Chaos, Disenchantment
 *
 * However, the code distinguishes between a single list of many items vs.
 * many lists. (The separator is used to make this determination.) A single
 * list of many items will not cause line wrapping (since there is no
 * apparent reason to do so). So the lists of Ulmo's miscellaneous traits
 * might look like this:
 *
 *     Free Action; Hold Life; See Invisible; Slow Digestion; Regeneration
 *     Blessed Blade
 *
 * So comparing the two, "Regeneration" has no trailing separator and
 * "Blessed Blade" was not indented. (Also, Ulmo's lists have no headers,
 * but that's not relevant to line wrapping and indention.)
 */

/* ITEM_SEP separates items within a list */
#define ITEM_SEP ','


/* LIST_SEP separates lists */
#define LIST_SEP ';'


static void spoiler_outlist(cptr header, cptr *list, char separator)
{
	int line_len, buf_len;
	char line[MAX_LINE_LEN+1], buf[80];

	/* Ignore an empty list */
	if (*list == NULL) return;

	/* This function always indents */
	strcpy(line, INDENT1);

	/* Create header (if one was given) */
	if (header && (header[0]))
	{
		strcat(line, header);
		strcat(line, " ");
	}

	line_len = strlen(line);

	/* Now begin the tedious task */
	while (1)
	{
		/* Copy the current item to a buffer */
		strcpy(buf, *list);

		/* Note the buffer's length */
		buf_len = strlen(buf);

		/*
		 * If there is an item following this one, pad with separator and
		 * a space and adjust the buffer length
		 */

		if (list[1])
		{
			sprintf(buf + buf_len, "%c ", separator);
			buf_len += 2;
		}

		/*
		 * If the buffer will fit on the current line, just append the
		 * buffer to the line and adjust the line length accordingly.
		 */

		if (line_len + buf_len <= MAX_LINE_LEN)
		{
			strcat(line, buf);
			line_len += buf_len;
		}

		/* Apply line wrapping and indention semantics described above */
		else
		{
			/*
			 * Don't print a trailing list separator but do print a trailing
			 * item separator.
			 */
			if ((line_len > 1) && (line[line_len - 1] == ' ') &&
			    (line[line_len - 2] == LIST_SEP))
			{
				/* Ignore space and separator */
				line[line_len - 2] = '\0';

				/* Write to spoiler file */
				fprintf(fff, "%s\n", line);

				/* Begin new line at primary indention level */
				sprintf(line, "%s%s", INDENT1, buf);
			}

			else
			{
				/* Write to spoiler file */
				fprintf(fff, "%s\n", line);

				/* Begin new line at secondary indention level */
				sprintf(line, "%s%s", INDENT2, buf);
			}

			line_len = strlen(line);
		}

		/* Advance, with break */
		if (!*++list) break;
	}

	/* Write what's left to the spoiler file */
	fprintf(fff, "%s\n", line);
}


/* Create a spoiler file entry for an artifact */

static void spoiler_print_art(obj_desc_list *art_ptr)
{
	pval_info_type *pval_ptr = &art_ptr->pval_info;

	char buf[80];

	/* Don't indent the first line */
	fprintf(fff, "%s\n", art_ptr->description);

	/* An "empty" pval description indicates that the pval affects nothing */
	if (pval_ptr->pval_desc[0])
	{
		/* Mention the effects of pval */
#ifdef JP
		sprintf(buf, "%s�ν���:", pval_ptr->pval_desc);
#else
		sprintf(buf, "%s to", pval_ptr->pval_desc);
#endif
		spoiler_outlist(buf, pval_ptr->pval_affects, ITEM_SEP);
	}

	/* Now deal with the description lists */

#ifdef JP
	spoiler_outlist("��:", art_ptr->slays, ITEM_SEP);

	spoiler_outlist("���°��:", art_ptr->brands, LIST_SEP);

	spoiler_outlist("�ȱ�:", art_ptr->immunities, ITEM_SEP);

	spoiler_outlist("����:", art_ptr->resistances, ITEM_SEP);

	spoiler_outlist("�ݻ�:", art_ptr->sustains, ITEM_SEP);
#else
	spoiler_outlist("Slay", art_ptr->slays, ITEM_SEP);

	spoiler_outlist("", art_ptr->brands, LIST_SEP);

	spoiler_outlist("Immunity to", art_ptr->immunities, ITEM_SEP);

	spoiler_outlist("Resist", art_ptr->resistances, ITEM_SEP);

	spoiler_outlist("Sustain", art_ptr->sustains, ITEM_SEP);
#endif
	spoiler_outlist("", art_ptr->misc_magic, LIST_SEP);


	/* Write out the possible activation at the primary indention level */
	if (art_ptr->activation)
	{
#ifdef JP
		fprintf(fff, "%sȯư: %s\n", INDENT1, art_ptr->activation);
#else
		fprintf(fff, "%sActivates for %s\n", INDENT1, art_ptr->activation);
#endif
	}

	/* End with the miscellaneous facts */
	fprintf(fff, "%s%s\n\n", INDENT1, art_ptr->misc_desc);
}


void dump_object(FILE *fp, object_type *o_ptr)
{
	char buf[80];
	obj_desc_list obj;
	pval_info_type *pval_ptr = &obj.pval_info;

	fff = fp;
	object_analyze(o_ptr, &obj);

	/* An "empty" pval description indicates that the pval affects nothing */
	if (pval_ptr->pval_desc[0])
	{
		/* Mention the effects of pval */
#ifdef JP
		sprintf(buf, "  %s�ν���:", pval_ptr->pval_desc);
#else
		sprintf(buf, "  %s to", pval_ptr->pval_desc);
#endif
		spoiler_outlist(buf, pval_ptr->pval_affects, ITEM_SEP);
	}

	/* Now deal with the description lists */

#ifdef JP
	spoiler_outlist("  ��:", obj.slays, ITEM_SEP);

	spoiler_outlist("  ���°��:", obj.brands, LIST_SEP);

	spoiler_outlist("  �ȱ�:", obj.immunities, ITEM_SEP);

	spoiler_outlist("  ����:", obj.resistances, ITEM_SEP);

	spoiler_outlist("  �ݻ�:", obj.sustains, ITEM_SEP);
#else
	spoiler_outlist("  Slay", obj.slays, ITEM_SEP);

	spoiler_outlist("  ", obj.brands, LIST_SEP);

	spoiler_outlist("  Immunity to", obj.immunities, ITEM_SEP);

	spoiler_outlist("  Resist", obj.resistances, ITEM_SEP);

	spoiler_outlist("  Sustain", obj.sustains, ITEM_SEP);
#endif
	spoiler_outlist("  ", obj.misc_magic, LIST_SEP);


	/* Write out the possible activation at the primary indention level */
	if (obj.activation)
	{
#ifdef JP
		fprintf(fff, "%s  ȯư: %s\n", INDENT1, obj.activation);
#else
		fprintf(fff, "%s  Activates for %s\n", INDENT1, obj.activation);
#endif
	}
}

/*
 * Hack -- Create a "forged" artifact
 */
static bool make_fake_artifact(object_type *o_ptr, int name1)
{
	int i;

	artifact_type *a_ptr = &a_info[name1];


	/* Ignore "empty" artifacts */
	if (!a_ptr->name) return FALSE;

	/* Acquire the "kind" index */
	i = lookup_kind(a_ptr->tval, a_ptr->sval);

	/* Oops */
	if (!i) return (FALSE);

	/* Create the artifact */
	object_prep(o_ptr, i);

	/* Save the name */
	o_ptr->name1 = name1;

	/* Extract the fields */
	o_ptr->pval = a_ptr->pval;
	/* o_ptr->xtra2 = a_ptr->activate; */
	o_ptr->ac = a_ptr->ac;
	o_ptr->dd = a_ptr->dd;
	o_ptr->ds = a_ptr->ds;
	o_ptr->to_a = a_ptr->to_a;
	o_ptr->to_h = a_ptr->to_h;
	o_ptr->to_d = a_ptr->to_d;
	o_ptr->weight = a_ptr->weight;

	/* Success */
	return (TRUE);
}


/*
 * Create a spoiler file for artifacts
 */
static void spoil_artifact(cptr fname)
{
	int i, j;

	object_type forge;
	object_type *q_ptr;

	obj_desc_list artifact;

	char buf[1024];


	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, fname);

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	/* Open the file */
	fff = my_fopen(buf, "w");

	/* Oops */
	if (!fff)
	{
		msg_print("Cannot create spoiler file.");
		return;
	}

	/* Dump the header */
	print_header();

	/* List the artifacts by tval */
	for (i = 0; group_artifact[i].tval; i++)
	{
		/* Write out the group title */
		if (group_artifact[i].name)
		{
			spoiler_blanklines(2);
			spoiler_underline(group_artifact[i].name);
			spoiler_blanklines(1);
		}

		/* Now search through all of the artifacts */
		for (j = 1; j < max_a_idx; ++j)
		{
			artifact_type *a_ptr = &a_info[j];

			/* We only want objects in the current group */
			if (a_ptr->tval != group_artifact[i].tval) continue;

			/* Get local object */
			q_ptr = &forge;

			/* Wipe the object */
			object_wipe(q_ptr);

			/* Attempt to "forge" the artifact */
			if (!make_fake_artifact(q_ptr, j)) continue;

			/* Analyze the artifact */
			object_analyze(q_ptr, &artifact);

			/* Write out the artifact description to the spoiler file */
			spoiler_print_art(&artifact);
		}
	}

	/* Check for errors */
	if (ferror(fff) || my_fclose(fff))
	{
		msg_print("Cannot close spoiler file.");
		return;
	}

	/* Message */
	msg_print("Successfully created a spoiler file.");
}


/*
 * Create a spoiler file for monsters   -BEN-
 */
static void spoil_mon_desc(cptr fname)
{
	int i, n = 0;

	u16b why = 2;
	s16b *who;

	char buf[1024];

	char nam[80];
	char lev[80];
	char rar[80];
	char spd[80];
	char ac[80];
	char hp[80];
	char exp[80];


	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, fname);

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	/* Open the file */
	fff = my_fopen(buf, "w");

	/* Oops */
	if (!fff)
	{
		msg_print("Cannot create spoiler file.");
		return;
	}

	/* Dump the header */
	fprintf(fff, "Monster Spoilers for %s Version %d.%d.%d\n",
		VERSION_NAME, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
	fprintf(fff, "------------------------------------------\n\n");

	/* Dump the header */
	fprintf(fff, "%-40.40s%4s%4s%6s%8s%4s  %11.11s\n",
		"Name", "Lev", "Rar", "Spd", "Hp", "Ac", "Visual Info");
	fprintf(fff, "%-40.40s%4s%4s%6s%8s%4s  %11.11s\n",
		"----", "---", "---", "---", "--", "--", "-----------");


	/* Allocate the "who" array */
	C_MAKE(who, max_r_idx, s16b);

	/* Scan the monsters */
	for (i = 1; i < max_r_idx; i++)
	{
		monster_race *r_ptr = &r_info[i];

		/* Use that monster */
		if (r_ptr->name) who[n++] = i;
	}

	/* Select the sort method */
	ang_sort_comp = ang_sort_comp_hook;
	ang_sort_swap = ang_sort_swap_hook;

	/* Sort the array by dungeon depth of monsters */
	ang_sort(who, &why, n);


	/* Scan again */
	for (i = 0; i < n; i++)
	{
		monster_race *r_ptr = &r_info[who[i]];

		cptr name = (r_name + r_ptr->name);

		/* Get the "name" */
		if (r_ptr->flags1 & (RF1_QUESTOR))
		{
			sprintf(nam, "[Q] %s", name);
		}
		else if (r_ptr->flags1 & (RF1_UNIQUE))
		{
			sprintf(nam, "[U] %s", name);
		}
		else
		{
#ifdef JP
			sprintf(nam, "    %s", name);
#else
			sprintf(nam, "The %s", name);
#endif
		}


#ifdef JP
		nam[null_pos_j(nam, 39)] = 0;
#endif
		/* Level */
		sprintf(lev, "%d", r_ptr->level);

		/* Rarity */
		sprintf(rar, "%d", r_ptr->rarity);

		/* Speed */
		if (r_ptr->speed >= 110)
		{
			sprintf(spd, "+%d", (r_ptr->speed - 110));
		}
		else
		{
			sprintf(spd, "-%d", (110 - r_ptr->speed));
		}

		/* Armor Class */
		sprintf(ac, "%d", r_ptr->ac);

		/* Hitpoints */
		if ((r_ptr->flags1 & (RF1_FORCE_MAXHP)) || (r_ptr->hside == 1))
		{
			sprintf(hp, "%d", r_ptr->hdice * r_ptr->hside);
		}
		else
		{
			sprintf(hp, "%dd%d", r_ptr->hdice, r_ptr->hside);
		}


		/* Experience */
		sprintf(exp, "%ld", (long)(r_ptr->mexp));

		/* Hack -- use visual instead */
#ifdef JP
		sprintf(exp, "%s'%c'", attr_to_text(r_ptr->d_attr), r_ptr->d_char);
#else
		sprintf(exp, "%s '%c'", attr_to_text(r_ptr->d_attr), r_ptr->d_char);
#endif

		/* Dump the info */
		fprintf(fff, "%-40.40s%4s%4s%6s%8s%4s  %11.11s\n",
			nam, lev, rar, spd, hp, ac, exp);
	}

	/* Free the "who" array */
	C_KILL(who, max_r_idx, s16b);

	/* End it */
	fprintf(fff, "\n");


	/* Check for errors */
	if (ferror(fff) || my_fclose(fff))
	{
		msg_print("Cannot close spoiler file.");
		return;
	}

	/* Worked */
	msg_print("Successfully created a spoiler file.");
}


/*
 * Monster spoilers by: smchorse@ringer.cs.utsa.edu (Shawn McHorse)
 *
 * Primarily based on code already in mon-desc.c, mostly by -BEN-
 */

/*
 * Pronoun arrays
 */
static cptr wd_che[3] =
#ifdef JP
{ "����", "��", "���" };
#else
{ "It", "He", "She" };
#endif


/*
 * Buffer text to the given file. (-SHAWN-)
 * This is basically c_roff() from mon-desc.c with a few changes.
 */
static void spoil_out(cptr str)
{
	cptr r;

	/* Line buffer */
	static char roff_buf[256];

#ifdef JP
	bool iskanji2=0;
#endif

	/* Current pointer into line roff_buf */
	static char *roff_p = roff_buf;

	/* Last space saved into roff_buf */
	static char *roff_s = NULL;

	/* Special handling for "new sequence" */
	if (!str)
	{
		if (roff_p != roff_buf) roff_p--;
		while (*roff_p == ' ' && roff_p != roff_buf) roff_p--;
		if (roff_p == roff_buf) fprintf(fff, "\n");
		else
		{
			*(roff_p + 1) = '\0';
			fprintf(fff, "%s\n\n", roff_buf);
		}
		roff_p = roff_buf;
		roff_s = NULL;
		roff_buf[0] = '\0';
		return;
	}

	/* Scan the given string, character at a time */
	for (; *str; str++)
	{
#ifdef JP
		char cbak;
		int k_flag = iskanji(*str);
#endif
		char ch = *str;
		int wrap = (ch == '\n');

#ifdef JP
		if (!isprint(ch) && !k_flag && !iskanji2) ch = ' ';
		if(k_flag && !iskanji2) iskanji2 = TRUE; else iskanji2 = FALSE;
		if ( roff_p >= roff_buf+( (k_flag) ? 74 : 75) ) wrap=1;
		if ((ch == ' ') && (roff_p + 2 >= roff_buf + ((k_flag) ? 74 : 75))) wrap = 1;
#else
		if (!isprint(ch)) ch = ' ';
		if (roff_p >= roff_buf + 75) wrap = 1;
		if ((ch == ' ') && (roff_p + 2 >= roff_buf + 75)) wrap = 1;
#endif

		/* Handle line-wrap */
		if (wrap)
		{
			*roff_p = '\0';
			r = roff_p;
#ifdef JP
			cbak=' ';
#endif
			if (roff_s && (ch != ' '))
			{
#ifdef JP
				cbak=*roff_s;
#endif
				*roff_s = '\0';
				r = roff_s + 1;
			}
			fprintf(fff, "%s\n", roff_buf);
			roff_s = NULL;
			roff_p = roff_buf;
#ifdef JP
			if(cbak != ' ') *roff_p++ = cbak; 
#endif
			while (*r) *roff_p++ = *r++;
		}

		/* Save the char */
		if ((roff_p > roff_buf) || (ch != ' '))
		{
#ifdef JP
		  if( !k_flag ){
			if (ch == ' ' || ch == '(' ) roff_s = roff_p;
		  }
		  else{
		    if( iskanji2 && 
			strncmp(str, "��", 2) != 0 && 
			strncmp(str, "��", 2) != 0 &&
			strncmp(str, "��", 2) != 0 &&
			strncmp(str, "��", 2) != 0) roff_s = roff_p;
		  }
#else
			if (ch == ' ') roff_s = roff_p;
#endif
			*roff_p++ = ch;
		}
	}
}


/*
 * Create a spoiler file for monsters (-SHAWN-)
 */
static void spoil_mon_info(cptr fname)
{
	char buf[1024];
	int msex, vn, i, j, k, n, count = 0;
	bool breath, magic;
	cptr p, q;
	cptr vp[64];
	u32b flags1, flags2, flags3, flags4, flags5, flags6;

	u16b why = 2;
	s16b *who;

#ifdef JP
	char            jverb_buf[64];
#else
	bool sin;
#endif

	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, fname);

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	/* Open the file */
	fff = my_fopen(buf, "w");

	/* Oops */
	if (!fff)
	{
		msg_print("Cannot create spoiler file.");
		return;
	}


	/* Dump the header */
	sprintf(buf, "Monster Spoilers for %s Version %d.%d.%d\n",
		VERSION_NAME, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
	spoil_out(buf);
	spoil_out("------------------------------------------\n\n");

	/* Allocate the "who" array */
	C_MAKE(who, max_r_idx, s16b);

	/* Scan the monsters */
	for (i = 1; i < max_r_idx; i++)
	{
		monster_race *r_ptr = &r_info[i];

		/* Use that monster */
		if (r_ptr->name) who[count++] = i;
	}

	/* Select the sort method */
	ang_sort_comp = ang_sort_comp_hook;
	ang_sort_swap = ang_sort_swap_hook;

	/* Sort the array by dungeon depth of monsters */
	ang_sort(who, &why, count);


	/* Scan again */
	for (n = 0; n < count; n++)
	{
		monster_race *r_ptr = &r_info[who[n]];

		/* Extract the flags */
		flags1 = r_ptr->flags1;
		flags2 = r_ptr->flags2;
		flags3 = r_ptr->flags3;
		flags4 = r_ptr->flags4;
		flags5 = r_ptr->flags5;
		flags6 = r_ptr->flags6;
		breath = FALSE;
		magic = FALSE;

		/* Extract a gender (if applicable) */
		if (flags1 & (RF1_FEMALE)) msex = 2;
		else if (flags1 & (RF1_MALE)) msex = 1;
		else msex = 0;


		/* Prefix */
		if (flags1 & (RF1_QUESTOR))
		{
			spoil_out("[Q] ");
		}
		else if (flags1 & (RF1_UNIQUE))
		{
			spoil_out("[U] ");
		}
		else
		{
#ifndef JP
			spoil_out("The ");
#endif
		}

		/* Name */
#ifdef JP
		sprintf(buf, "%s/%s  (", (r_name + r_ptr->name), (r_name + r_ptr->E_name));  /* ---)--- */
#else
		sprintf(buf, "%s  (", (r_name + r_ptr->name));  /* ---)--- */
#endif
		spoil_out(buf);

		/* Color */
		spoil_out(attr_to_text(r_ptr->d_attr));

		/* Symbol --(-- */
		sprintf(buf, " '%c')\n", r_ptr->d_char);
		spoil_out(buf);

		/* Indent */
		sprintf(buf, "=== ");
		spoil_out(buf);

		/* Number */
		sprintf(buf, "Num:%d  ", who[n]);
		spoil_out(buf);

		/* Level */
		sprintf(buf, "Lev:%d  ", r_ptr->level);
		spoil_out(buf);

		/* Rarity */
		sprintf(buf, "Rar:%d  ", r_ptr->rarity);
		spoil_out(buf);

		/* Speed */
		if (r_ptr->speed >= 110)
		{
			sprintf(buf, "Spd:+%d  ", (r_ptr->speed - 110));
		}
		else
		{
			sprintf(buf, "Spd:-%d  ", (110 - r_ptr->speed));
		}
		spoil_out(buf);

		/* Hitpoints */
		if ((flags1 & (RF1_FORCE_MAXHP)) || (r_ptr->hside == 1))
		{
			sprintf(buf, "Hp:%d  ", r_ptr->hdice * r_ptr->hside);
		}
		else
		{
			sprintf(buf, "Hp:%dd%d  ", r_ptr->hdice, r_ptr->hside);
		}
		spoil_out(buf);

		/* Armor Class */
		sprintf(buf, "Ac:%d  ", r_ptr->ac);
		spoil_out(buf);

		/* Experience */
		sprintf(buf, "Exp:%ld\n", (long)(r_ptr->mexp));
		spoil_out(buf);

		/* Describe */
		spoil_out(r_text + r_ptr->text);
#ifdef JP
		spoil_out(" ");
		spoil_out("����");

		if (flags2 & RF2_ELDRITCH_HORROR) spoil_out("������Ͷ��");/*nuke me*/
		if (flags3 & RF3_ANIMAL)          spoil_out("��������");
		if (flags3 & RF3_EVIL)            spoil_out("�ٰ��ʤ�");
		if (flags3 & RF3_GOOD)            spoil_out("���ɤ�");
		if (flags3 & RF3_UNDEAD)          spoil_out("����ǥåɤ�");

		if (flags3 & RF3_DRAGON)   spoil_out("�ɥ饴��");
		else if (flags3 & RF3_DEMON)    spoil_out("�ǡ����");
		else if (flags3 & RF3_GIANT)    spoil_out("���㥤�����");
		else if (flags3 & RF3_TROLL)    spoil_out("�ȥ��");
		else if (flags3 & RF3_ORC)      spoil_out("������");
		else if (flags3 & RF3_HUMAN)    spoil_out("�ʹ�");/*nuke me*/
		else                            spoil_out("��󥹥���");
#else
		spoil_out("  ");
		spoil_out("This");

		if (flags2 & (RF2_ELDRITCH_HORROR)) spoil_out (" sanity-blasting");
		if (flags3 & (RF3_ANIMAL)) spoil_out(" natural");
		if (flags3 & (RF3_EVIL)) spoil_out(" evil");
		if (flags3 & (RF3_GOOD)) spoil_out(" good");
		if (flags3 & (RF3_UNDEAD)) spoil_out(" undead");

		if (flags3 & (RF3_DRAGON)) spoil_out(" dragon");
		else if (flags3 & (RF3_DEMON)) spoil_out(" demon");
		else if (flags3 & (RF3_GIANT)) spoil_out(" giant");
		else if (flags3 & (RF3_TROLL)) spoil_out(" troll");
		else if (flags3 & (RF3_ORC)) spoil_out(" orc");
		else if (flags3 & (RF3_HUMAN)) spoil_out (" human");
		else spoil_out(" creature");
#endif

#ifdef JP
		spoil_out("��");
#else
		spoil_out(" moves");
#endif

		if ((flags1 & (RF1_RAND_50)) && (flags1 & (RF1_RAND_25)))
		{
#ifdef JP
			spoil_out("���ʤ��Ե�§��");
#else
			spoil_out(" extremely erratically");
#endif
		}
		else if (flags1 & (RF1_RAND_50))
		{
#ifdef JP
			spoil_out("��ʬ�Ե�§��");
#else
			spoil_out(" somewhat erratically");
#endif
		}
		else if (flags1 & (RF1_RAND_25))
		{
#ifdef JP
			spoil_out("�����Ե�§��");
#else
			spoil_out(" a bit erratically");
#endif
		}
		else
		{
#ifdef JP
			spoil_out("���̤�");
#else
			spoil_out(" normally");
#endif
		}

#ifdef JP
		spoil_out("ư���Ƥ���");
#endif
		if (flags1 & (RF1_NEVER_MOVE))
		{
#ifdef JP
			spoil_out("���������Ԥ����פ��ʤ�");
#else
			spoil_out(", but does not deign to chase intruders");
#endif
		}
#ifdef JP
		spoil_out("��");
#else
		spoil_out(".  ");
#endif

		if (!r_ptr->level || (flags1 & (RF1_FORCE_DEPTH)))
		{
#ifdef JP
			if(r_ptr->level == 0) sprintf(buf, "%s���Ͼ�ˤΤ߽и����롣", wd_che[msex]);
			else sprintf(buf, "%s��%d���ʹߤˤ����и����ʤ���", wd_che[msex], r_ptr->level);
#else
			sprintf(buf, "%s is never found out of depth.  ", wd_che[msex]);
#endif
			spoil_out(buf);
		}

		if (flags1 & (RF1_FORCE_SLEEP))
		{
#ifdef JP
			sprintf(buf, "%s�����ʤ�̲�äƤ��롣", wd_che[msex]);
#else
			sprintf(buf, "%s is always created sluggish.  ", wd_che[msex]);
#endif
			spoil_out(buf);
		}

		if ((flags2 & RF2_AURA_FIRE) && (flags2 & RF2_AURA_ELEC) && (flags3 & RF3_AURA_COLD))
		{
#ifdef JP
			sprintf(buf, "%s�ϱ��ɹ�ȥ��ѡ�������ޤ�Ƥ��롣", wd_che[msex]);
#else
			sprintf(buf, "%s is surrounded by flames and electricity.  ", wd_che[msex]);
#endif
			spoil_out(buf);
		}
		else if ((flags2 & RF2_AURA_FIRE) && (flags2 & RF2_AURA_ELEC))
		{
#ifdef JP
			sprintf(buf, "%s�ϱ�ȥ��ѡ�������ޤ�Ƥ��롣", wd_che[msex]);
#else
			sprintf(buf, "%s is surrounded by flames and electricity.  ", wd_che[msex]);
#endif
			spoil_out(buf);
		}
		else if ((flags2 & RF2_AURA_FIRE) && (flags3 & RF3_AURA_COLD))
		{
#ifdef JP
			sprintf(buf, "%s�ϱ��ɹ����ޤ�Ƥ��롣", wd_che[msex]);
#else
			sprintf(buf, "%s is surrounded by flames and electricity.  ", wd_che[msex]);
#endif
			spoil_out(buf);
		}
		else if ((flags3 & RF3_AURA_COLD) && (flags2 & RF2_AURA_ELEC))
		{
#ifdef JP
			sprintf(buf, "%s��ɹ�ȥ��ѡ�������ޤ�Ƥ��롣", wd_che[msex]);
#else
			sprintf(buf, "%s is surrounded by ice and electricity.  ", wd_che[msex]);
#endif
			spoil_out(buf);
		}
		else if (flags2 & RF2_AURA_FIRE)
		{
#ifdef JP
			sprintf(buf, "%s�ϱ����ޤ�Ƥ��롣", wd_che[msex]);
#else
			sprintf(buf, "%s is surrounded by flames.  ", wd_che[msex]);
#endif
			spoil_out(buf);
		}
		else if (flags3 & RF3_AURA_COLD)
		{
#ifdef JP
			sprintf(buf, "%s��ɹ����ޤ�Ƥ��롣", wd_che[msex]);
#else
			sprintf(buf, "%s is surrounded by ice.  ", wd_che[msex]);
#endif
			spoil_out(buf);
		}
		else if (flags2 & RF2_AURA_ELEC)
		{
#ifdef JP
			sprintf(buf, "%s�ϥ��ѡ�������ޤ�Ƥ��롣", wd_che[msex]);
#else
			sprintf(buf, "%s is surrounded by electricity.  ", wd_che[msex]);
#endif
			spoil_out(buf);
		}

		if (flags2 & (RF2_REFLECTING))
		{
#ifdef JP
			sprintf(buf, "%s����μ�ʸ��ķ���֤���", wd_che[msex]);
#else
			sprintf(buf, "%s reflects bolt spells.  ", wd_che[msex]);
#endif
			spoil_out(buf);
		}

		/* Describe escorts */
		if ((flags1 & RF1_ESCORT) || (flags1 & RF1_ESCORTS))
		{
#ifdef JP
			sprintf(buf, "%s���̾��Ҥ�ȼ�äƸ���롣", wd_che[msex]);
#else
			sprintf(buf, "%s usually appears with escorts.  ", wd_che[msex]);
#endif
			spoil_out(buf);
		}

		/* Describe friends */
		else if ((flags1 & RF1_FRIEND) || (flags1 & RF1_FRIENDS))
		{
#ifdef JP
			sprintf(buf, "%s���̾ｸ�ĤǸ���롣", wd_che[msex]);
#else
			sprintf(buf, "%s usually appears in groups.  ", wd_che[msex]);
#endif
			spoil_out(buf);
		}

		/* Collect inate attacks */
		vn = 0;
#ifdef JP
		if (flags4 & RF4_SHRIEK)  vp[vn++] = "���Ĥǽ��������";
		if (flags4 & RF4_THROW)   vp[vn++] = "����ꤲ��";
		if (flags4 & RF4_ROCKET)  vp[vn++] = "���åȤ�ȯ�ͤ���";
		if (flags4 & RF4_ARROW_1) vp[vn++] = "�����";
		if (flags4 & RF4_ARROW_2) vp[vn++] = "���������";
		if (flags4 & RF4_ARROW_3) vp[vn++] = "�ͷ�򤹤�";
		if (flags4 & RF4_ARROW_4) vp[vn++] = "����ͷ�򤹤�";
#else
		if (flags4 & RF4_SHRIEK)  vp[vn++] = "shriek for help";
		if (flags4 & RF4_THROW)   vp[vn++] = "throw a rock";
		if (flags4 & RF4_ROCKET)  vp[vn++] = "shoot a rocket";
		if (flags4 & RF4_ARROW_1) vp[vn++] = "fire an arrow";
		if (flags4 & RF4_ARROW_2) vp[vn++] = "fire arrows";
		if (flags4 & RF4_ARROW_3) vp[vn++] = "fire a missile";
		if (flags4 & RF4_ARROW_4) vp[vn++] = "fire missiles";
#endif

		if (vn)
		{
#ifdef JP
			sprintf(buf, "%s��", wd_che[msex]);
#else
			sprintf(buf, "%s", wd_che[msex]);
#endif
			spoil_out(buf);

			for (i = 0; i < vn; i++)
			{
#ifdef JP
				if(i != vn-1)
				{
					jverb(vp[i],jverb_buf, JVERB_OR);
					spoil_out(jverb_buf);
					spoil_out("�ꡢ");
				}
				else  spoil_out(vp[i]);
#else
				if (!i) spoil_out(" may ");
				else if (i < vn-1) spoil_out(", ");
				else spoil_out(" or ");
				spoil_out(vp[i]);
#endif
			}
#ifdef JP
			spoil_out("���Ȥ��Ǥ��롣");
#else
			spoil_out(".  ");
#endif
		}

		/* Collect breaths */
		vn = 0;
#ifdef JP
		if (flags4 & (RF4_BR_ACID)) vp[vn++] = "��";
		if (flags4 & (RF4_BR_ELEC)) vp[vn++] = "���";
		if (flags4 & (RF4_BR_FIRE)) vp[vn++] = "�б�";
		if (flags4 & (RF4_BR_COLD)) vp[vn++] = "�䵤";
		if (flags4 & (RF4_BR_POIS)) vp[vn++] = "��";
		if (flags4 & (RF4_BR_NETH)) vp[vn++] = "�Ϲ�";
		if (flags4 & (RF4_BR_LITE)) vp[vn++] = "����";
		if (flags4 & (RF4_BR_DARK)) vp[vn++] = "�Ź�";
		if (flags4 & (RF4_BR_CONF)) vp[vn++] = "����";
		if (flags4 & (RF4_BR_SOUN)) vp[vn++] = "�첻";
		if (flags4 & (RF4_BR_CHAO)) vp[vn++] = "������";
		if (flags4 & (RF4_BR_DISE)) vp[vn++] = "����";
		if (flags4 & (RF4_BR_NEXU)) vp[vn++] = "���̺���";
		if (flags4 & (RF4_BR_TIME)) vp[vn++] = "���ֵ�ž";
		if (flags4 & (RF4_BR_INER)) vp[vn++] = "����";
		if (flags4 & (RF4_BR_GRAV)) vp[vn++] = "����";
		if (flags4 & (RF4_BR_SHAR)) vp[vn++] = "����";
		if (flags4 & (RF4_BR_PLAS)) vp[vn++] = "�ץ饺��";
		if (flags4 & (RF4_BR_WALL)) vp[vn++] = "�ե�����";
		if (flags4 & (RF4_BR_MANA)) vp[vn++] = "����";
		if (flags4 & (RF4_BR_NUKE)) vp[vn++] = "�������Ѵ�ʪ";
		if (flags4 & (RF4_BR_DISI)) vp[vn++] = "ʬ��";
#else
		if (flags4 & (RF4_BR_ACID)) vp[vn++] = "acid";
		if (flags4 & (RF4_BR_ELEC)) vp[vn++] = "lightning";
		if (flags4 & (RF4_BR_FIRE)) vp[vn++] = "fire";
		if (flags4 & (RF4_BR_COLD)) vp[vn++] = "frost";
		if (flags4 & (RF4_BR_POIS)) vp[vn++] = "poison";
		if (flags4 & (RF4_BR_NETH)) vp[vn++] = "nether";
		if (flags4 & (RF4_BR_LITE)) vp[vn++] = "light";
		if (flags4 & (RF4_BR_DARK)) vp[vn++] = "darkness";
		if (flags4 & (RF4_BR_CONF)) vp[vn++] = "confusion";
		if (flags4 & (RF4_BR_SOUN)) vp[vn++] = "sound";
		if (flags4 & (RF4_BR_CHAO)) vp[vn++] = "chaos";
		if (flags4 & (RF4_BR_DISE)) vp[vn++] = "disenchantment";
		if (flags4 & (RF4_BR_NEXU)) vp[vn++] = "nexus";
		if (flags4 & (RF4_BR_TIME)) vp[vn++] = "time";
		if (flags4 & (RF4_BR_INER)) vp[vn++] = "inertia";
		if (flags4 & (RF4_BR_GRAV)) vp[vn++] = "gravity";
		if (flags4 & (RF4_BR_SHAR)) vp[vn++] = "shards";
		if (flags4 & (RF4_BR_PLAS)) vp[vn++] = "plasma";
		if (flags4 & (RF4_BR_WALL)) vp[vn++] = "force";
		if (flags4 & (RF4_BR_MANA)) vp[vn++] = "mana";
		if (flags4 & (RF4_BR_NUKE)) vp[vn++] = "toxic waste";
		if (flags4 & (RF4_BR_DISI)) vp[vn++] = "disintegration";
#endif

		if (vn)
		{
			breath = TRUE;
#ifdef JP
			sprintf(buf, "%s��", wd_che[msex]);
#else
			sprintf(buf, "%s", wd_che[msex]);
#endif
			spoil_out(buf);

			for (i = 0; i < vn; i++)
			{
#ifdef JP
				if ( i != 0 ) spoil_out("��");
#else
				if (!i) spoil_out(" may breathe ");
				else if (i < vn-1) spoil_out(", ");
				else spoil_out(" or ");
#endif
				spoil_out(vp[i]);
			}
#ifdef JP
			spoil_out("�Υ֥쥹��");
			if (flags2 & (RF2_POWERFUL)) spoil_out("���Ϥ�");
			spoil_out("�Ǥ����Ȥ���");
#else
			if (flags2 & (RF2_POWERFUL)) spoil_out(" powerfully");
#endif
		}

		/* Collect spells */
		vn = 0;
#ifdef JP
		if (flags5 & (RF5_BA_ACID))           vp[vn++] = "�����åɡ��ܡ���";
		if (flags5 & (RF5_BA_ELEC))           vp[vn++] = "����������ܡ���";
		if (flags5 & (RF5_BA_FIRE))           vp[vn++] = "�ե��������ܡ���";
		if (flags5 & (RF5_BA_COLD))           vp[vn++] = "���������ܡ���";
		if (flags5 & (RF5_BA_POIS))           vp[vn++] = "������";
		if (flags5 & (RF5_BA_NETH))           vp[vn++] = "�Ϲ���";
		if (flags5 & (RF5_BA_WATE))           vp[vn++] = "�������������ܡ���";
		if (flags4 & (RF4_BA_NUKE))           vp[vn++] = "����ǽ��";
		if (flags5 & (RF5_BA_MANA))           vp[vn++] = "���Ϥ���";
		if (flags5 & (RF5_BA_DARK))           vp[vn++] = "�Ź�����";
		if (flags4 & (RF4_BA_LITE))           vp[vn++] = "���������С�����";
		if (flags4 & (RF4_BA_CHAO))           vp[vn++] = "��������";
		if (flags6 & (RF6_HAND_DOOM))         vp[vn++] = "���Ǥμ�";
		if (flags5 & (RF5_DRAIN_MANA))        vp[vn++] = "���ϵۼ�";
		if (flags5 & (RF5_MIND_BLAST))        vp[vn++] = "��������";
		if (flags5 & (RF5_BRAIN_SMASH))       vp[vn++] = "Ǿ����";
		if (flags5 & (RF5_CAUSE_1))           vp[vn++] = "�ڽ��ܼ���";
		if (flags5 & (RF5_CAUSE_2))           vp[vn++] = "�Ž��ܼ���";
		if (flags5 & (RF5_CAUSE_3))           vp[vn++] = "��̿���ܼ���";
		if (flags5 & (RF5_CAUSE_4))           vp[vn++] = "�λ��";
		if (flags5 & (RF5_BO_ACID))           vp[vn++] = "�����åɡ��ܥ��";
		if (flags5 & (RF5_BO_ELEC))           vp[vn++] = "����������ܥ��";
		if (flags5 & (RF5_BO_FIRE))           vp[vn++] = "�ե��������ܥ��";
		if (flags5 & (RF5_BO_COLD))           vp[vn++] = "���������ܥ��";
		if (flags5 & (RF5_BO_POIS))           vp[vn++] = "�ݥ����󡦥ܥ��";
		if (flags5 & (RF5_BO_NETH))           vp[vn++] = "�Ϲ�����";
		if (flags5 & (RF5_BO_WATE))           vp[vn++] = "�������������ܥ��";
		if (flags5 & (RF5_BO_MANA))           vp[vn++] = "���Ϥ���";
		if (flags5 & (RF5_BO_PLAS))           vp[vn++] = "�ץ饺�ޡ��ܥ��";
		if (flags5 & (RF5_BO_ICEE))           vp[vn++] = "�˴�����";
		if (flags5 & (RF5_MISSILE))           vp[vn++] = "�ޥ��å��ߥ�����";
		if (flags5 & (RF5_SCARE))             vp[vn++] = "����";
		if (flags5 & (RF5_BLIND))             vp[vn++] = "�ܤ���ޤ�";
		if (flags5 & (RF5_CONF))              vp[vn++] = "����";
		if (flags5 & (RF5_SLOW))              vp[vn++] = "��®";
		if (flags5 & (RF5_HOLD))              vp[vn++] = "����";
		if (flags6 & (RF6_HASTE))             vp[vn++] = "��®";
		if (flags6 & (RF6_HEAL))              vp[vn++] = "����";
		if (flags6 & (RF6_INVULNER))          vp[vn++] = "̵Ũ��";
		if (flags6 & (RF6_BLINK))             vp[vn++] = "���硼�ȥƥ�ݡ���";
		if (flags6 & (RF6_TPORT))             vp[vn++] = "�ƥ�ݡ���";
		if (flags6 & (RF6_XXX3))              vp[vn++] = "����";
		if (flags6 & (RF6_XXX4))              vp[vn++] = "����";
		if (flags6 & (RF6_TELE_TO))           vp[vn++] = "�ƥ�ݡ��ȥХå�";
		if (flags6 & (RF6_TELE_AWAY))         vp[vn++] = "�ƥ�ݡ��ȥ�������";
		if (flags6 & (RF6_TELE_LEVEL))        vp[vn++] = "��٥�ƥ�ݡ���";
		if (flags6 & (RF6_XXX5))              vp[vn++] = "����";
		if (flags6 & (RF6_DARKNESS))          vp[vn++] = "�Ű�";
		if (flags6 & (RF6_TRAPS))             vp[vn++] = "�ȥ�å�";
		if (flags6 & (RF6_FORGET))            vp[vn++] = "�����õ�";
		if (flags6 & (RF6_RAISE_DEAD))        vp[vn++] = "�������";
		if (flags6 & (RF6_S_MONSTER))         vp[vn++] = "��󥹥������ξ���";
		if (flags6 & (RF6_S_MONSTERS))        vp[vn++] = "��󥹥���ʣ������";
		if (flags6 & (RF6_S_KIN))             vp[vn++] = "�߱群��";
		if (flags6 & (RF6_S_ANT))             vp[vn++] = "���꾤��";
		if (flags6 & (RF6_S_SPIDER))          vp[vn++] = "���⾤��";
		if (flags6 & (RF6_S_HOUND))           vp[vn++] = "�ϥ���ɾ���";
		if (flags6 & (RF6_S_HYDRA))           vp[vn++] = "�ҥɥ龤��";
		if (flags6 & (RF6_S_ANGEL))           vp[vn++] = "ŷ�Ⱦ���";
		if (flags6 & (RF6_S_DEMON))           vp[vn++] = "�ǡ���󾤴�";
		if (flags6 & (RF6_S_UNDEAD))          vp[vn++] = "����ǥåɾ���";
		if (flags6 & (RF6_S_DRAGON))          vp[vn++] = "�ɥ饴�󾤴�";
		if (flags6 & (RF6_S_HI_UNDEAD))       vp[vn++] = "���Ϥʥ���ǥåɾ���";
		if (flags6 & (RF6_S_HI_DRAGON))       vp[vn++] = "���ɥ饴�󾤴�";
		if (flags6 & (RF6_S_CYBER))           vp[vn++] = "�����С��ǡ���󾤴�";
		if (flags6 & (RF6_S_HI_DEMON))        vp[vn++] = "���ǡ���󾤴�";
		if (flags6 & (RF6_S_UNIQUE))          vp[vn++] = "��ˡ�����󥹥�������";
#else
		if (flags5 & (RF5_BA_ACID))           vp[vn++] = "produce acid balls";
		if (flags5 & (RF5_BA_ELEC))           vp[vn++] = "produce lightning balls";
		if (flags5 & (RF5_BA_FIRE))           vp[vn++] = "produce fire balls";
		if (flags5 & (RF5_BA_COLD))           vp[vn++] = "produce frost balls";
		if (flags5 & (RF5_BA_POIS))           vp[vn++] = "produce poison balls";
		if (flags5 & (RF5_BA_NETH))           vp[vn++] = "produce nether balls";
		if (flags5 & (RF5_BA_WATE))           vp[vn++] = "produce water balls";
		if (flags4 & (RF4_BA_NUKE))           vp[vn++] = "produce balls of radiation";
		if (flags5 & (RF5_BA_MANA))           vp[vn++] = "produce mana storms";
		if (flags5 & (RF5_BA_DARK))           vp[vn++] = "produce darkness storms";
		if (flags4 & (RF4_BA_LITE))           vp[vn++] = "invoke star burst";
		if (flags4 & (RF4_BA_CHAO))           vp[vn++] = "produce chaos balls";
		if (flags6 & (RF6_HAND_DOOM))         vp[vn++] = "invoke the Hand of Doom";
		if (flags5 & (RF5_DRAIN_MANA))        vp[vn++] = "drain mana";
		if (flags5 & (RF5_MIND_BLAST))        vp[vn++] = "cause mind blasting";
		if (flags5 & (RF5_BRAIN_SMASH))       vp[vn++] = "cause brain smashing";
		if (flags5 & (RF5_CAUSE_1))           vp[vn++] = "cause light wounds and cursing";
		if (flags5 & (RF5_CAUSE_2))           vp[vn++] = "cause serious wounds and cursing";
		if (flags5 & (RF5_CAUSE_3))           vp[vn++] = "cause critical wounds and cursing";
		if (flags5 & (RF5_CAUSE_4))           vp[vn++] = "cause mortal wounds";
		if (flags5 & (RF5_BO_ACID))           vp[vn++] = "produce acid bolts";
		if (flags5 & (RF5_BO_ELEC))           vp[vn++] = "produce lightning bolts";
		if (flags5 & (RF5_BO_FIRE))           vp[vn++] = "produce fire bolts";
		if (flags5 & (RF5_BO_COLD))           vp[vn++] = "produce frost bolts";
		if (flags5 & (RF5_BO_POIS))           vp[vn++] = "produce poison bolts";
		if (flags5 & (RF5_BO_NETH))           vp[vn++] = "produce nether bolts";
		if (flags5 & (RF5_BO_WATE))           vp[vn++] = "produce water bolts";
		if (flags5 & (RF5_BO_MANA))           vp[vn++] = "produce mana bolts";
		if (flags5 & (RF5_BO_PLAS))           vp[vn++] = "produce plasma bolts";
		if (flags5 & (RF5_BO_ICEE))           vp[vn++] = "produce ice bolts";
		if (flags5 & (RF5_MISSILE))           vp[vn++] = "produce magic missiles";
		if (flags5 & (RF5_SCARE))             vp[vn++] = "terrify";
		if (flags5 & (RF5_BLIND))             vp[vn++] = "blind";
		if (flags5 & (RF5_CONF))              vp[vn++] = "confuse";
		if (flags5 & (RF5_SLOW))              vp[vn++] = "slow";
		if (flags5 & (RF5_HOLD))              vp[vn++] = "paralyze";
		if (flags6 & (RF6_HASTE))             vp[vn++] = "haste-self";
		if (flags6 & (RF6_HEAL))              vp[vn++] = "heal-self";
		if (flags6 & (RF6_INVULNER))          vp[vn++] = "make invulnerable";
		if (flags6 & (RF6_BLINK))             vp[vn++] = "blink-self";
		if (flags6 & (RF6_TPORT))             vp[vn++] = "teleport-self";
		if (flags6 & (RF6_XXX3))              vp[vn++] = "do something";
		if (flags6 & (RF6_XXX4))              vp[vn++] = "do something";
		if (flags6 & (RF6_TELE_TO))           vp[vn++] = "teleport to";
		if (flags6 & (RF6_TELE_AWAY))         vp[vn++] = "teleport away";
		if (flags6 & (RF6_TELE_LEVEL))        vp[vn++] = "teleport level";
		if (flags6 & (RF6_XXX5))              vp[vn++] = "do something";
		if (flags6 & (RF6_DARKNESS))          vp[vn++] = "create darkness";
		if (flags6 & (RF6_TRAPS))             vp[vn++] = "create traps";
		if (flags6 & (RF6_FORGET))            vp[vn++] = "cause amnesia";
		if (flags6 & (RF6_RAISE_DEAD))        vp[vn++] = "raise dead";
		if (flags6 & (RF6_S_MONSTER))         vp[vn++] = "summon a monster";
		if (flags6 & (RF6_S_MONSTERS))        vp[vn++] = "summon monsters";
		if (flags6 & (RF6_S_KIN))             vp[vn++] = "summon aid";
		if (flags6 & (RF6_S_ANT))             vp[vn++] = "summon ants";
		if (flags6 & (RF6_S_SPIDER))          vp[vn++] = "summon spiders";
		if (flags6 & (RF6_S_HOUND))           vp[vn++] = "summon hounds";
		if (flags6 & (RF6_S_HYDRA))           vp[vn++] = "summon hydras";
		if (flags6 & (RF6_S_ANGEL))           vp[vn++] = "summon an angel";
		if (flags6 & (RF6_S_DEMON))           vp[vn++] = "summon a demon";
		if (flags6 & (RF6_S_UNDEAD))          vp[vn++] = "summon an undead";
		if (flags6 & (RF6_S_DRAGON))          vp[vn++] = "summon a dragon";
		if (flags6 & (RF6_S_HI_UNDEAD))       vp[vn++] = "summon greater undead";
		if (flags6 & (RF6_S_HI_DRAGON))       vp[vn++] = "summon greater dragons";
		if (flags6 & (RF6_S_CYBER))           vp[vn++] = "summon Cyberdemons";
		if (flags6 & (RF6_S_HI_DEMON))        vp[vn++] = "summon greater demon";
		if (flags6 & (RF6_S_UNIQUE))          vp[vn++] = "summon unique monsters";
#endif

		if (vn)
		{
			magic = TRUE;
			if (breath)
			{
#ifdef JP
				spoil_out("�ꡢ�ʤ�����");
#else
				spoil_out(", and is also");
#endif
			}
			else
			{
#ifdef JP
				sprintf(buf, "%s��", wd_che[msex]);
#else
				sprintf(buf, "%s is", wd_che[msex]);
#endif
				spoil_out(buf);
			}

#ifdef JP
			/* Adverb */
			if (flags2 & (RF2_SMART)) spoil_out("Ū�Τ�");

			/* Verb Phrase */
			spoil_out("��ˡ��Ȥ����Ȥ��Ǥ���");
#else
			spoil_out(" magical, casting spells");
			if (flags2 & (RF2_SMART)) spoil_out(" intelligently");
#endif

			for (i = 0; i < vn; i++)
			{
#ifdef JP
				if ( i != 0 ) spoil_out("��");
#else
				if (!i) spoil_out(" which ");
				else if (i < vn-1) spoil_out(", ");
				else spoil_out(" or ");
#endif
				spoil_out(vp[i]);
			}
#ifdef JP
			spoil_out("�μ�ʸ�򾧤��뤳�Ȥ���");
#endif
		}

		if (breath || magic)
		{
#ifdef JP
			sprintf(buf, "��(��Ψ:��1/%d)��",
				200 / (r_ptr->freq_inate + r_ptr->freq_spell));
			spoil_out(buf);
#else
			sprintf(buf, "; 1 time in %d.  ",
				200 / (r_ptr->freq_inate + r_ptr->freq_spell));
			spoil_out(buf);
#endif
		}

		/* Collect special abilities. */
		vn = 0;
#ifdef JP
		if (flags2 & (RF2_OPEN_DOOR)) vp[vn++] = "�ɥ��򳫤���";
		if (flags2 & (RF2_BASH_DOOR)) vp[vn++] = "�ɥ����Ǥ��ˤ�";
		if (flags2 & (RF2_PASS_WALL)) vp[vn++] = "�ɤ򤹤�ȴ����";
		if (flags2 & (RF2_KILL_WALL)) vp[vn++] = "�ɤ򷡤�ʤ�";
		if (flags2 & (RF2_MOVE_BODY)) vp[vn++] = "�夤��󥹥����򲡤��Τ���";
		if (flags2 & (RF2_KILL_BODY)) vp[vn++] = "�夤��󥹥������ݤ�";
		if (flags2 & (RF2_TAKE_ITEM)) vp[vn++] = "�����ƥ�򽦤�";
		if (flags2 & (RF2_KILL_ITEM)) vp[vn++] = "�����ƥ�����";
#else
		if (flags2 & (RF2_OPEN_DOOR)) vp[vn++] = "open doors";
		if (flags2 & (RF2_BASH_DOOR)) vp[vn++] = "bash down doors";
		if (flags2 & (RF2_PASS_WALL)) vp[vn++] = "pass through walls";
		if (flags2 & (RF2_KILL_WALL)) vp[vn++] = "bore through walls";
		if (flags2 & (RF2_MOVE_BODY)) vp[vn++] = "push past weaker monsters";
		if (flags2 & (RF2_KILL_BODY)) vp[vn++] = "destroy weaker monsters";
		if (flags2 & (RF2_TAKE_ITEM)) vp[vn++] = "pick up objects";
		if (flags2 & (RF2_KILL_ITEM)) vp[vn++] = "destroy objects";
#endif


		if (vn)
		{
			spoil_out(wd_che[msex]);
#ifdef JP
			spoil_out("��");
#endif
			for (i = 0; i < vn; i++)
			{
#ifdef JP
				if(i != vn-1)
				{
					jverb(vp[i],jverb_buf, JVERB_AND);
					spoil_out(jverb_buf);
					spoil_out("��");
				}
				else  spoil_out(vp[i]);
#else
				if (!i) spoil_out(" can ");
				else if (i < vn-1) spoil_out(", ");
				else spoil_out(" and ");
				spoil_out(vp[i]);
#endif
			}
#ifdef JP
			spoil_out("���Ȥ��Ǥ��롣");
#else
			spoil_out(".  ");
#endif
		}

		if (flags2 & (RF2_INVISIBLE))
		{
			spoil_out(wd_che[msex]);
#ifdef JP
			spoil_out("��Ʃ�����ܤ˸����ʤ���");
#else
			spoil_out(" is invisible.  ");
#endif
		}
		if (flags2 & (RF2_COLD_BLOOD))
		{
			spoil_out(wd_che[msex]);
#ifdef JP
			spoil_out("�����ưʪ�Ǥ��롣");
#else
			spoil_out(" is cold blooded.  ");
#endif
		}
		if (flags2 & (RF2_EMPTY_MIND))
		{
			spoil_out(wd_che[msex]);
#ifdef JP
			spoil_out("�ϥƥ�ѥ����Ǥϴ��ΤǤ��ʤ���");
#else
			spoil_out(" is not detected by telepathy.  ");
#endif
		}
		if (flags2 & (RF2_WEIRD_MIND))
		{
			spoil_out(wd_che[msex]);
#ifdef JP
			spoil_out("�Ϥޤ�˥ƥ�ѥ����Ǵ��ΤǤ��롣");
#else
			spoil_out(" is rarely detected by telepathy.  ");
#endif
		}
		if (flags2 & (RF2_MULTIPLY))
		{
			spoil_out(wd_che[msex]);
#ifdef JP
			spoil_out("����ȯŪ���������롣");
#else
			spoil_out(" breeds explosively.  ");
#endif
		}
		if (flags2 & (RF2_REGENERATE))
		{
			spoil_out(wd_che[msex]);
#ifdef JP
			spoil_out("�����᤯���Ϥ�������롣");
#else
			spoil_out(" regenerates quickly.  ");
#endif
		}

		/* Collect susceptibilities */
		vn = 0;
#ifdef JP
		if (flags3 & (RF3_HURT_ROCK)) vp[vn++] = "���������";
		if (flags3 & (RF3_HURT_LITE)) vp[vn++] = "���뤤��";
		if (flags3 & (RF3_HURT_FIRE)) vp[vn++] = "��";
		if (flags3 & (RF3_HURT_COLD)) vp[vn++] = "�䵤";
#else
		if (flags3 & (RF3_HURT_ROCK)) vp[vn++] = "rock remover";
		if (flags3 & (RF3_HURT_LITE)) vp[vn++] = "bright light";
		if (flags3 & (RF3_HURT_FIRE)) vp[vn++] = "fire";
		if (flags3 & (RF3_HURT_COLD)) vp[vn++] = "cold";
#endif

		if (vn)
		{
#ifdef JP
			sprintf(buf, "%s�ˤ�", wd_che[msex]);
#else
			sprintf(buf, "%s", wd_che[msex]);
#endif
			spoil_out(buf);

			for (i = 0; i < vn; i++)
			{
#ifdef JP
				if ( i != 0 ) spoil_out("��");
#else
				if (i == 0) spoil_out(" is hurt by ");
				else if (i < vn-1) spoil_out(", ");
				else spoil_out(" and ");
#endif
				spoil_out(vp[i]);
			}

			/* End */
#ifdef JP
			spoil_out("�ǥ��᡼����Ϳ�����롣");
#else
			spoil_out(".  ");
#endif
		}

		/* Collect immunities */
		vn = 0;
#ifdef JP
		if (flags3 & (RF3_IM_ACID)) vp[vn++] = "��";
		if (flags3 & (RF3_IM_ELEC)) vp[vn++] = "�ŷ�";
		if (flags3 & (RF3_IM_FIRE)) vp[vn++] = "��";
		if (flags3 & (RF3_IM_COLD)) vp[vn++] = "�䵤";
		if (flags3 & (RF3_IM_POIS)) vp[vn++] = "��";
#else
		if (flags3 & (RF3_IM_ACID)) vp[vn++] = "acid";
		if (flags3 & (RF3_IM_ELEC)) vp[vn++] = "lightning";
		if (flags3 & (RF3_IM_FIRE)) vp[vn++] = "fire";
		if (flags3 & (RF3_IM_COLD)) vp[vn++] = "cold";
		if (flags3 & (RF3_IM_POIS)) vp[vn++] = "poison";
#endif

		if (vn)
		{
#ifdef JP
			sprintf(buf, "%s��", wd_che[msex]);
#else
			sprintf(buf, "%s", wd_che[msex]);
#endif
			spoil_out(buf);
			for (i = 0; i < vn; i++)
			{
#ifdef JP
				if ( i != 0 ) spoil_out("��");
#else
				if (!i) spoil_out(" resists ");
				else if (i < vn-1) spoil_out(", ");
				else spoil_out(" and ");
#endif
				spoil_out(vp[i]);
			}
#ifdef JP
			spoil_out("����������äƤ��롣");
#else
			spoil_out(".  ");
#endif
		}

		/* Collect resistances */
		vn = 0;
#ifdef JP
		if (flags4 & RF4_BR_LITE)    vp[vn++] = "����";
		if ((flags4 & RF4_BR_DARK) || (flags3 & RF3_ORC) || (flags3 & RF3_HURT_LITE)) vp[vn++] = "����";
		if ((flags3 & RF3_RES_NETH) || (flags3 & RF3_UNDEAD)) vp[vn++] = "�Ϲ�";
		if (flags3 & (RF3_RES_WATE)) vp[vn++] = "��";
		if (flags3 & (RF3_RES_PLAS)) vp[vn++] = "�ץ饺��";
		if (flags4 & RF4_BR_SHAR) vp[vn++] = "����";
		if (flags4 & RF4_BR_SOUN) vp[vn++] = "�첻";
		if (flags4 & RF4_BR_CONF) vp[vn++] = "����";
		if (flags4 & RF4_BR_CHAO) vp[vn++] = "������";
		if (flags3 & (RF3_RES_NEXU)) vp[vn++] = "���̺���";
		if (flags3 & (RF3_RES_DISE)) vp[vn++] = "����";
		if (flags4 & RF4_BR_WALL) vp[vn++] = "�ե�����";
		if (flags4 & RF4_BR_INER) vp[vn++] = "����";
		if (flags4 & RF4_BR_TIME) vp[vn++] = "���ֵ�ž";
		if (flags4 & RF4_BR_GRAV) vp[vn++] = "����";
		if ((flags3 & RF3_RES_TELE) && !(r_ptr->flags1 & RF1_UNIQUE)) vp[vn++] = "�ƥ�ݡ���";
#else
		if (flags4 & RF4_BR_LITE)    vp[vn++] = "light";
		if ((flags4 & RF4_BR_DARK) || (flags3 & RF3_ORC) || (flags3 & RF3_HURT_LITE)) vp[vn++] = "dark";
		if ((flags3 & RF3_RES_NETH) || (flags3 & RF3_UNDEAD)) vp[vn++] = "nether";
		if (flags3 & (RF3_RES_WATE)) vp[vn++] = "water";
		if (flags3 & (RF3_RES_PLAS)) vp[vn++] = "plasma";
		if (flags4 & RF4_BR_SHAR) vp[vn++] = "shards";
		if (flags4 & RF4_BR_SOUN) vp[vn++] = "sound";
		if (flags4 & RF4_BR_CONF) vp[vn++] = "confusion";
		if (flags4 & RF4_BR_CHAO) vp[vn++] = "chaos";
		if (flags3 & (RF3_RES_NEXU)) vp[vn++] = "nexus";
		if (flags3 & (RF3_RES_DISE)) vp[vn++] = "disenchantment";
		if (flags4 & RF4_BR_WALL) vp[vn++] = "force";
		if (flags4 & RF4_BR_INER) vp[vn++] = "inertia";
		if (flags4 & RF4_BR_TIME) vp[vn++] = "time";
		if (flags4 & RF4_BR_GRAV) vp[vn++] = "gravity";
		if ((flags3 & RF3_RES_TELE) && !(r_ptr->flags1 & RF1_UNIQUE)) vp[vn++] = "teleportation";
#endif

		if (vn)
		{
#ifdef JP
			sprintf(buf, "%s��", wd_che[msex]);
#else
			sprintf(buf, "%s", wd_che[msex]);
#endif
			spoil_out(buf);
			for (i = 0; i < vn; i++)
			{
#ifdef JP
				if ( i != 0 ) spoil_out("��");
#else
				if (!i) spoil_out(" resists ");
				else if (i < vn-1) spoil_out(", ");
				else spoil_out(" and ");
#endif
				spoil_out(vp[i]);
			}
#ifdef JP
			spoil_out("����������äƤ��롣");
#else
			spoil_out(".  ");
#endif
		}

		/* Collect non-effects */
		vn = 0;
#ifdef JP
		if (flags3 & (RF3_NO_STUN)) vp[vn++] = "ۯ۰�Ȥ��ʤ�";
		if (flags3 & (RF3_NO_FEAR)) vp[vn++] = "���ݤ򴶤��ʤ�";
		if (flags3 & (RF3_NO_CONF)) vp[vn++] = "���𤷤ʤ�";
		if (flags3 & (RF3_NO_SLEEP)) vp[vn++] = "̲�餵��ʤ�";
		if ((flags3 & RF3_RES_TELE) && (r_ptr->flags1 & RF1_UNIQUE)) vp[vn++] = "�ƥ�ݡ��Ȥ���ʤ�";
#else
		if (flags3 & (RF3_NO_STUN)) vp[vn++] = "stunned";
		if (flags3 & (RF3_NO_FEAR)) vp[vn++] = "frightened";
		if (flags3 & (RF3_NO_CONF)) vp[vn++] = "confused";
		if (flags3 & (RF3_NO_SLEEP)) vp[vn++] = "slept";
		if ((flags3 & RF3_RES_TELE) && (r_ptr->flags1 & RF1_UNIQUE)) vp[vn++] = "teleported";
#endif


		if (vn)
		{
#ifdef JP
			sprintf(buf, "%s��", wd_che[msex]);
#else
			sprintf(buf, "%s", wd_che[msex]);
#endif
			spoil_out(buf);
			for (i = 0; i < vn; i++)
			{
#ifdef JP
				if ( i != 0 ) spoil_out("����");
#else
				if (!i) spoil_out(" cannot be ");
				else if (i < vn-1) spoil_out(", ");
				else spoil_out(" or ");
#endif
				spoil_out(vp[i]);
			}
#ifdef JP
			spoil_out("��");
#else
			spoil_out(".  ");
#endif
		}

		spoil_out(wd_che[msex]);
#ifdef JP
		spoil_out("�Ͽ�����");
		if (r_ptr->sleep > 200)     spoil_out("��̵�뤷�����Ǥ��뤬");
		else if (r_ptr->sleep > 95) spoil_out("���Ф��ƤۤȤ����դ�ʧ��ʤ���");
		else if (r_ptr->sleep > 75) spoil_out("���Ф��Ƥ��ޤ���դ�ʧ��ʤ���");
		else if (r_ptr->sleep > 45) spoil_out("�򸫲ᤴ�������Ǥ��뤬");
		else if (r_ptr->sleep > 25) spoil_out("��ۤ�ξ����ϸ��Ƥ���");
		else if (r_ptr->sleep > 10) spoil_out("�򤷤Ф餯�ϸ��Ƥ���");
		else if (r_ptr->sleep > 5)  spoil_out("���ʬ��տ������Ƥ���");
		else if (r_ptr->sleep > 3)  spoil_out("����տ������Ƥ���");
		else if (r_ptr->sleep > 1)  spoil_out("�򤫤ʤ���տ������Ƥ���");
		else if (r_ptr->sleep > 0)  spoil_out("��ٲ����Ƥ���");
		else spoil_out("�򤫤ʤ�ٲ����Ƥ���");
		spoil_out("��");
		sprintf(buf, " %d �ե������褫�鿯���Ԥ˵��դ����Ȥ����롣",
			10 * r_ptr->aaf);
		spoil_out(buf);
#else
		if (r_ptr->sleep > 200)     spoil_out(" prefers to ignore");
		else if (r_ptr->sleep > 95) spoil_out(" pays very little attention to");
		else if (r_ptr->sleep > 75) spoil_out(" pays little attention to");
		else if (r_ptr->sleep > 45) spoil_out(" tends to overlook");
		else if (r_ptr->sleep > 25) spoil_out(" takes quite a while to see");
		else if (r_ptr->sleep > 10) spoil_out(" takes a while to see");
		else if (r_ptr->sleep > 5)  spoil_out(" is fairly observant of");
		else if (r_ptr->sleep > 3)  spoil_out(" is observant of");
		else if (r_ptr->sleep > 1)  spoil_out(" is very observant of");
		else if (r_ptr->sleep > 0)  spoil_out(" is vigilant for");
		else spoil_out(" is ever vigilant for");

		sprintf(buf, " intruders, which %s may notice from %d feet.  ",
			wd_che[msex], 10 * r_ptr->aaf);
		spoil_out(buf);
#endif

		i = 0;
		if (flags1 & (RF1_DROP_60)) i += 1;
		if (flags1 & (RF1_DROP_90)) i += 2;
		if (flags1 & (RF1_DROP_1D2)) i += 2;
		if (flags1 & (RF1_DROP_2D2)) i += 4;
		if (flags1 & (RF1_DROP_3D2)) i += 6;
		if (flags1 & (RF1_DROP_4D2)) i += 8;

		/* Drops gold and/or items */
		if (i)
		{
#ifdef JP
			sprintf(buf, "%s��", wd_che[msex]);
#else
			sprintf(buf, "%s will carry", wd_che[msex]);
			sin = FALSE;
#endif
			spoil_out(buf);

			if (i == 1)
			{
#ifdef JP
				spoil_out("1�Ĥ�");
#else
				spoil_out(" a");
				sin = TRUE;
#endif
			}
			else if (i == 2)
			{
#ifdef JP
				spoil_out("1�Ĥ�2�Ĥ�");
#else
				spoil_out(" one or two");
#endif
			}
			else
			{
#ifdef JP
				sprintf(buf, "%u�ĤޤǤ�", i);
#else
				sprintf(buf, " up to %u", i);
#endif
				spoil_out(buf);
			}

			if (flags1 & (RF1_DROP_GREAT))
			{
#ifdef JP
				spoil_out("���̤ʥ����ƥ�");
#else
				if (sin) spoil_out("n");
				spoil_out(" exceptional object");
#endif
			}
			else if (flags1 & (RF1_DROP_GOOD))
			{
#ifdef JP
				spoil_out("����Υ����ƥ�");
#else
				spoil_out(" good object");
#endif
			}
			else if (flags1 & (RF1_DROP_USEFUL))
			{
#ifdef JP
				spoil_out("�����ƥ�");
#else
				spoil_out(" useful object");
#endif
			}
			else if (flags1 & (RF1_ONLY_ITEM))
			{
#ifdef JP
				spoil_out("�����ƥ�");
#else
				spoil_out(" object");
#endif
			}
			else if (flags1 & (RF1_ONLY_GOLD))
			{
#ifdef JP
				spoil_out("����");
#else
				spoil_out(" treasure");
#endif
			}
			else
			{
#ifdef JP
				spoil_out("�����ƥ�����");
#else
				spoil_out(" object");
				if (i > 1) spoil_out("s");
				spoil_out(" or treasure");
#endif
			}
#ifndef JP
			if (i > 1) spoil_out("s");
#endif

			if (flags1 & (RF1_DROP_CHOSEN))
			{
#ifdef JP
				if(i > 1) spoil_out("����äƤ��ꡢ�ʤ�����");
				spoil_out("����Υ����ƥ�");
#else
				spoil_out(", in addition to chosen objects");
#endif
			}

#ifdef JP
			spoil_out("����äƤ��뤳�Ȥ����롣");
#else
			spoil_out(".  ");
#endif
		}

		/* Count the actual attacks */
		for (i = 0, j = 0; j < 4; j++)
		{
			if (r_ptr->blow[j].method) i++;
		}

		/* Examine the actual attacks */
		for (k = 0, j = 0; j < 4; j++)
		{
			if (!r_ptr->blow[j].method) continue;

			/* No method yet */
			p = NULL;

			/* Acquire the method */
			switch (r_ptr->blow[j].method)
			{
#ifdef JP
				case RBM_HIT:		p = "����"; break;
				case RBM_TOUCH:		p = "����"; break;
				case RBM_PUNCH:		p = "�ѥ������"; break;
				case RBM_KICK:		p = "����"; break;
				case RBM_CLAW:		p = "�Ҥä���"; break;
				case RBM_BITE:		p = "����"; break;
				case RBM_STING:		p = "�ɤ�"; break;
				case RBM_SLASH:		p = "�¤�"; break;
				case RBM_BUTT:		p = "�Ѥ��ͤ�"; break;
				case RBM_CRUSH:		p = "�������ꤹ��"; break;
				case RBM_ENGULF:	p = "���߹���"; break;
				case RBM_CHARGE: 	p = "������褳��"; break;
				case RBM_CRAWL:		p = "�Τξ���礤���"; break;
				case RBM_DROOL:		p = "�����򤿤餹"; break;
				case RBM_SPIT:		p = "�ĤФ��Ǥ�"; break;
				case RBM_EXPLODE:	p = "��ȯ����"; break;
				case RBM_GAZE:		p = "�ˤ��"; break;
				case RBM_WAIL:		p = "�㤭����"; break;
				case RBM_SPORE:		p = "˦�Ҥ����Ф�"; break;
				case RBM_XXX4:   break;
				case RBM_BEG:		p = "��򤻤���"; break;
				case RBM_INSULT:	p = "�����"; break;
				case RBM_MOAN:		p = "���᤯"; break;
				case RBM_SHOW:  	p = "�Τ�"; break;
#else
				case RBM_HIT:    p = "hit"; break;
				case RBM_TOUCH:  p = "touch"; break;
				case RBM_PUNCH:  p = "punch"; break;
				case RBM_KICK:   p = "kick"; break;
				case RBM_CLAW:   p = "claw"; break;
				case RBM_BITE:   p = "bite"; break;
				case RBM_STING:  p = "sting"; break;
				case RBM_SLASH:	 p = "slash"; break;
				case RBM_BUTT:   p = "butt"; break;
				case RBM_CRUSH:  p = "crush"; break;
				case RBM_ENGULF: p = "engulf"; break;
				case RBM_CHARGE: p = "charge";  break;
				case RBM_CRAWL:  p = "crawl on you"; break;
				case RBM_DROOL:  p = "drool on you"; break;
				case RBM_SPIT:   p = "spit"; break;
				case RBM_EXPLODE: p = "explode";  break;
				case RBM_GAZE:   p = "gaze"; break;
				case RBM_WAIL:   p = "wail"; break;
				case RBM_SPORE:  p = "release spores"; break;
				case RBM_XXX4:   break;
				case RBM_BEG:    p = "beg"; break;
				case RBM_INSULT: p = "insult"; break;
				case RBM_MOAN:   p = "moan"; break;
				case RBM_SHOW:   p = "sing"; break;
#endif
			}


			/* Default effect */
			q = NULL;

			/* Acquire the effect */
			switch (r_ptr->blow[j].effect)
			{
#ifdef JP
				case RBE_HURT:    	q = "���⤹��"; break;
				case RBE_POISON:  	q = "�Ǥ򤯤�魯"; break;
				case RBE_UN_BONUS:	q = "����������"; break;
				case RBE_UN_POWER:	q = "���Ϥ�ۤ����"; break;
				case RBE_EAT_GOLD:	q = "������"; break;
				case RBE_EAT_ITEM:	q = "�����ƥ�����"; break;
				case RBE_EAT_FOOD:	q = "���ʤ��ο����򿩤٤�"; break;
				case RBE_EAT_LITE:	q = "�������ۼ�����"; break;
				case RBE_ACID:    	q = "�������Ф�"; break;
				case RBE_ELEC:      q = "���Ť�����"; break;

				case RBE_FIRE:    	q = "ǳ�䤹"; break;
				case RBE_COLD:    	q = "��餻��"; break;
				case RBE_BLIND:   	q = "���ܤˤ���"; break;
				case RBE_CONFUSE: 	q = "���𤵤���"; break;
				case RBE_TERRIFY: 	q = "���ݤ�����"; break;
				case RBE_PARALYZE:	q = "���㤵����"; break;
				case RBE_LOSE_STR:	q = "���Ϥ򸺾�������"; break;
				case RBE_LOSE_INT:	q = "��ǽ�򸺾�������"; break;
				case RBE_LOSE_WIS:	q = "�����򸺾�������"; break;
				case RBE_LOSE_DEX:	q = "���Ѥ��򸺾�������"; break;
				case RBE_LOSE_CON:	q = "�ѵ��Ϥ򸺾�������"; break;
				case RBE_LOSE_CHR:	q = "̥�Ϥ򸺾�������"; break;
				case RBE_LOSE_ALL:	q = "�����ơ������򸺾�������"; break;
				case RBE_SHATTER:	q = "ʴ�դ���"; break;
				case RBE_EXP_10:	q = "�и��ͤ򸺾�(10d6+)������"; break;
				case RBE_EXP_20:	q = "�и��ͤ򸺾�(20d6+)������"; break;
				case RBE_EXP_40:	q = "�и��ͤ򸺾�(40d6+)������"; break;
				case RBE_EXP_80:	q = "�и��ͤ򸺾�(80d6+)������"; break;
				case RBE_DISEASE:	q = "�µ��ˤ���"; break;
				case RBE_TIME:      q = "���ֵ�ž������"; break;
				case RBE_EXP_VAMP:      q = "��̿�Ϥ�ۼ�����"; break;
#else
				case RBE_HURT:          q = "attack"; break;
				case RBE_POISON:        q = "poison"; break;
				case RBE_UN_BONUS:      q = "disenchant"; break;
				case RBE_UN_POWER:      q = "drain charges"; break;
				case RBE_EAT_GOLD:      q = "steal gold"; break;
				case RBE_EAT_ITEM:      q = "steal items"; break;
				case RBE_EAT_FOOD:      q = "eat your food"; break;
				case RBE_EAT_LITE:      q = "absorb light"; break;
				case RBE_ACID:          q = "shoot acid"; break;
				case RBE_ELEC:          q = "electrocute"; break;

				case RBE_FIRE:          q = "burn"; break;
				case RBE_COLD:          q = "freeze"; break;
				case RBE_BLIND:         q = "blind"; break;
				case RBE_CONFUSE:       q = "confuse"; break;
				case RBE_TERRIFY:       q = "terrify"; break;
				case RBE_PARALYZE:      q = "paralyze"; break;
				case RBE_LOSE_STR:      q = "reduce strength"; break;
				case RBE_LOSE_INT:      q = "reduce intelligence"; break;
				case RBE_LOSE_WIS:      q = "reduce wisdom"; break;
				case RBE_LOSE_DEX:      q = "reduce dexterity"; break;
				case RBE_LOSE_CON:      q = "reduce constitution"; break;
				case RBE_LOSE_CHR:      q = "reduce charisma"; break;
				case RBE_LOSE_ALL:      q = "reduce all stats"; break;
				case RBE_SHATTER:       q = "shatter"; break;
				case RBE_EXP_10:        q = "lower experience (by 10d6+)"; break;
				case RBE_EXP_20:        q = "lower experience (by 20d6+)"; break;
				case RBE_EXP_40:        q = "lower experience (by 40d6+)"; break;
				case RBE_EXP_80:        q = "lower experience (by 80d6+)"; break;
				case RBE_DISEASE:       q = "disease"; break;
				case RBE_TIME:          q = "time"; break;
				case RBE_EXP_VAMP:      q = "drain life force"; break;
#endif
			}


			if (!k)
			{
#ifdef JP
				sprintf(buf, "%s��", wd_che[msex]);
#else
				sprintf(buf, "%s can ", wd_che[msex]);
#endif
				spoil_out(buf);
			}
			else if (k < i-1)
			{
#ifdef JP
				spoil_out("��");
#else
				spoil_out(", ");
#endif
			}
			else
			{
#ifdef JP
				spoil_out("��");
#else
				spoil_out(", and ");
#endif
			}

#ifdef JP
			if (p)
			{
				if (r_ptr->blow[j].d_dice && r_ptr->blow[j].d_side)
				{
					if(!k) spoil_out(" ");
					if (r_ptr->blow[j].d_side == 1)
						sprintf(buf, "%d ", r_ptr->blow[j].d_dice);
					else
						sprintf(buf, "%dd%d ",
						r_ptr->blow[j].d_dice, r_ptr->blow[j].d_side);
					spoil_out(buf);
					spoil_out("�Υ��᡼����");
				}
				if(q) jverb( p ,jverb_buf, JVERB_TO);
				else if(k!=i-1) jverb( p ,jverb_buf, JVERB_AND);
				else strcpy(jverb_buf, p);
				spoil_out(jverb_buf);
				/* Describe the effect (if any) */
				if (q)
				{
					if(k!=i-1) jverb( q,jverb_buf, JVERB_AND);
					else strcpy(jverb_buf,q); 
					spoil_out(jverb_buf);
				}
			}
#else
			/* Describe the method */
			if (p) spoil_out(p);

			/* Describe the effect, if any */
			if (r_ptr->blow[j].effect)
			{
				if (q)
				{
					spoil_out(" to ");
					spoil_out(q);
				}
				if (r_ptr->blow[j].d_dice && r_ptr->blow[j].d_side)
				{
					spoil_out(" with damage");
					if (r_ptr->blow[j].d_side == 1)
						sprintf(buf, " %d", r_ptr->blow[j].d_dice);
					else
						sprintf(buf, " %dd%d",
						r_ptr->blow[j].d_dice, r_ptr->blow[j].d_side);
					spoil_out(buf);
				}
			}
#endif
			k++;
		}

		if (k)
		{
#ifdef JP
			spoil_out("��");
#else
			spoil_out(".  ");
#endif
		}
		else if (flags1 & (RF1_NEVER_BLOW))
		{
#ifdef JP
			sprintf(buf, "%s��ʪ��Ū�ʹ�����ˡ������ʤ���", wd_che[msex]);
#else
			sprintf(buf, "%s has no physical attacks.  ", wd_che[msex]);
#endif
			spoil_out(buf);
		}

		spoil_out(NULL);
	}

	/* Free the "who" array */
	C_KILL(who, max_r_idx, s16b);

	/* Check for errors */
	if (ferror(fff) || my_fclose(fff))
	{
		msg_print("Cannot close spoiler file.");
		return;
	}

	msg_print("Successfully created a spoiler file.");
}


/*
 * Forward declare
 */
extern void do_cmd_spoilers(void);

/*
 * Create Spoiler files -BEN-
 */
void do_cmd_spoilers(void)
{
	int i;


	/* Save the screen */
	screen_save();


	/* Drop priv's */
	safe_setuid_drop();


	/* Interact */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Info */
#ifdef JP
		prt("��ꤿ�����ݥ���ե����������ǲ�������", 2, 0);
#else
		prt("Create a spoiler file.", 2, 0);
#endif

		/* Prompt for a file */
#ifdef JP
		prt("(1) �����ƥ�ꥹ��          (obj-desc.spo)", 5, 5);
		prt("(2) �����ƥ��ե����ȥꥹ��  (artifact.spo)", 6, 5);
		prt("(3) �ʰץ�󥹥����ꥹ��    (mon-desc.spo)", 7, 5);
		prt("(4) �ܺ٥�󥹥����ꥹ��    (mon-info.spo)", 8, 5);
#else
		prt("(1) Brief Object Info (obj-desc.spo)", 5, 5);
		prt("(2) Brief Artifact Info (artifact.spo)", 6, 5);
		prt("(3) Brief Monster Info (mon-desc.spo)", 7, 5);
		prt("(4) Full Monster Info (mon-info.spo)", 8, 5);
#endif

		/* Prompt */
#ifdef JP
		prt("���ޥ��:", 18, 0);
#else
		prt("Command: ", 12, 0);
#endif


		/* Get a choice */
		i = inkey();

		/* Escape */
		if (i == ESCAPE)
		{
			break;
		}

		/* Option (1) */
		else if (i == '1')
		{
			spoil_obj_desc("obj-desc.spo");
		}

		/* Option (2) */
		else if (i == '2')
		{
			spoil_artifact("artifact.spo");
		}

		/* Option (3) */
		else if (i == '3')
		{
			spoil_mon_desc("mon-desc.spo");
		}

		/* Option (4) */
		else if (i == '4')
		{
			spoil_mon_info("mon-info.spo");
		}

		/* Oops */
		else
		{
			bell();
		}

		/* Flush messages */
		msg_print(NULL);
	}


	/* Grab priv's */
	safe_setuid_grab();


	/* Restore the screen */
	screen_load();
}


#else

#ifdef MACINTOSH
static int i = 0;
#endif /* MACINTOSH */

#endif
