/* File: artifact.c */

/* Purpose: Artifact code */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"


/* Chance of using syllables to form the name instead of the "template" files */
#define TABLE_NAME      50

#define A_CURSED        13
#define WEIRD_LUCK      12
#define BIAS_LUCK       20

/*
 * Bias luck needs to be higher than weird luck,
 * since it is usually tested several times...
 */
#define ACTIVATION_CHANCE 3


/*
 * Choose one random sustain
 */
void one_sustain(object_type *o_ptr)
{
	switch (randint0(6))
	{
		case 0: o_ptr->art_flags2 |= (TR2_SUST_STR); break;
		case 1: o_ptr->art_flags2 |= (TR2_SUST_INT); break;
		case 2: o_ptr->art_flags2 |= (TR2_SUST_WIS); break;
		case 3: o_ptr->art_flags2 |= (TR2_SUST_DEX); break;
		case 4: o_ptr->art_flags2 |= (TR2_SUST_CON); break;
		case 5: o_ptr->art_flags2 |= (TR2_SUST_CHR); break;
	}
}


/*
 * Choose one random high resistance
 */
void one_high_resistance(object_type *o_ptr)
{
	switch (randint0(7))
	{
		case  0: o_ptr->art_flags2 |= (TR2_RES_POIS);   break;
		case  1: o_ptr->art_flags2 |= (TR2_RES_DARK);   break;
		case  2: o_ptr->art_flags2 |= (TR2_RES_BLIND);  break;
		case  3: o_ptr->art_flags2 |= (TR2_RES_CONF);   break;
		case  4: o_ptr->art_flags2 |= (TR2_RES_SOUND);  break;
		case  5: o_ptr->art_flags2 |= (TR2_RES_NETHER); break;
		case  6: o_ptr->art_flags2 |= (TR2_RES_FEAR);   break;
	}
}

/*
 * Choose one random high resistance ( except poison and disenchantment )
 */
void one_lordly_high_resistance(object_type *o_ptr)
{
	switch (randint0(6))
	{
		case 0: o_ptr->art_flags2 |= (TR2_RES_DARK);   break;
		case 1: o_ptr->art_flags2 |= (TR2_RES_BLIND);  break;
		case 2: o_ptr->art_flags2 |= (TR2_RES_CONF);   break;
		case 3: o_ptr->art_flags2 |= (TR2_RES_SOUND);  break;
		case 4: o_ptr->art_flags2 |= (TR2_RES_NETHER); break;
		case 5: o_ptr->art_flags2 |= (TR2_RES_FEAR);   break;
	}
}


/*
 * Choose one random element resistance
 */
void one_ele_resistance(object_type *o_ptr)
{
	switch (randint0(4))
	{
		case  0: o_ptr->art_flags2 |= (TR2_RES_ACID); break;
		case  1: o_ptr->art_flags2 |= (TR2_RES_ELEC); break;
		case  2: o_ptr->art_flags2 |= (TR2_RES_COLD); break;
		case  3: o_ptr->art_flags2 |= (TR2_RES_FIRE); break;
	}
}


/*
 * Choose one random element or poison resistance
 */
void one_dragon_ele_resistance(object_type *o_ptr)
{
	if (one_in_(7))
	{
		o_ptr->art_flags2 |= (TR2_RES_POIS);
	}
	else
	{
		one_ele_resistance(o_ptr);
	}
}


/*
 * Choose one random resistance
 */
void one_resistance(object_type *o_ptr)
{
	if (one_in_(3))
	{
		one_ele_resistance(o_ptr);
	}
	else
	{
		one_high_resistance(o_ptr);
	}
}


/*
 * Choose one random ability
 */
void one_ability(object_type *o_ptr)
{
	switch (randint0(9))
	{
		case 0: o_ptr->art_flags3 |= (TR3_FEATHER);     break;
		case 1: o_ptr->art_flags3 |= (TR3_LITE);        break;
		case 2: o_ptr->art_flags3 |= (TR3_SEE_INVIS);   break;
		case 3: o_ptr->art_flags3 |= (TR3_TELEPATHY);   break;
		case 4: o_ptr->art_flags3 |= (TR3_SLOW_DIGEST); break;
		case 5: o_ptr->art_flags3 |= (TR3_REGEN);       break;
		case 6: o_ptr->art_flags2 |= (TR2_FREE_ACT);    break;
		case 7: o_ptr->art_flags2 |= (TR2_HOLD_LIFE);   break;
		case 8: o_ptr->art_flags3 |= (TR3_WARNING);   break;
	}
}


static void curse_artifact(object_type * o_ptr)
{
	if (o_ptr->pval) o_ptr->pval = 0 - (s16b)(o_ptr->pval + randint1(4));
	if (o_ptr->to_a) o_ptr->to_a = 0 - (s16b)(o_ptr->to_a + randint1(4));
	if (o_ptr->to_h) o_ptr->to_h = 0 - (s16b)(o_ptr->to_h + randint1(4));
	if (o_ptr->to_d) o_ptr->to_d = 0 - (s16b)(o_ptr->to_d + randint1(4));

	o_ptr->art_flags3 |= (TR3_HEAVY_CURSE | TR3_CURSED);

	if (randint1(4) == 1) o_ptr->art_flags3 |= TR3_PERMA_CURSE;
	if (randint1(3) == 1) o_ptr->art_flags3 |= TR3_TY_CURSE;
	if (randint1(2) == 1) o_ptr->art_flags3 |= TR3_AGGRAVATE;
	if (randint1(3) == 1) o_ptr->art_flags3 |= TR3_DRAIN_EXP;
	if (randint1(2) == 1) o_ptr->art_flags3 |= TR3_TELEPORT;
	else if (randint1(3) == 1) o_ptr->art_flags3 |= TR3_NO_TELE;

	if ((p_ptr->pclass != CLASS_WARRIOR) && (randint1(3) == 1))
		o_ptr->art_flags3 |= TR3_NO_MAGIC;

	o_ptr->ident |= IDENT_CURSED;
}


void random_plus(object_type * o_ptr)
{
	int this_type = (o_ptr->tval < TV_BOOTS ? 24 : 20);

	switch (artifact_bias)
	{
	case BIAS_WARRIOR:
		if (!(o_ptr->art_flags1 & TR1_STR))
		{
			o_ptr->art_flags1 |= TR1_STR;
			if (randint1(2) == 1) return; /* 50% chance of being a "free" power */
		}
		if (!(o_ptr->art_flags1 & TR1_CON))
		{
			o_ptr->art_flags1 |= TR1_CON;
			if (randint1(2) == 1) return;
		}
		if (!(o_ptr->art_flags1 & TR1_DEX))
		{
			o_ptr->art_flags1 |= TR1_DEX;
			if (randint1(2) == 1) return;
		}
		break;

	case BIAS_MAGE:
		if (!(o_ptr->art_flags1 & TR1_INT))
		{
			o_ptr->art_flags1 |= TR1_INT;
			if (randint1(2) == 1) return;
		}
		break;

	case BIAS_PRIESTLY:
		if (!(o_ptr->art_flags1 & TR1_WIS))
		{
			o_ptr->art_flags1 |= TR1_WIS;
			if (randint1(2) == 1) return;
		}
		break;

	case BIAS_RANGER:
		if (!(o_ptr->art_flags1 & TR1_CON))
		{
			o_ptr->art_flags1 |= TR1_CON;
			if (randint1(2) == 1) return; /* 50% chance of being a "free" power */
		}
		if (!(o_ptr->art_flags1 & TR1_DEX))
		{
			o_ptr->art_flags1 |= TR1_DEX;
			if (randint1(2) == 1) return;
		}
		if (!(o_ptr->art_flags1 & TR1_STR))
		{
			o_ptr->art_flags1 |= TR1_STR;
			if (randint1(2) == 1) return;
		}
		break;

	case BIAS_ROGUE:
		if (!(o_ptr->art_flags1 & TR1_STEALTH))
		{
			o_ptr->art_flags1 |= TR1_STEALTH;
			if (randint1(2) == 1) return;
		}
		if (!(o_ptr->art_flags1 & TR1_SEARCH))
		{
			o_ptr->art_flags1 |= TR1_SEARCH;
			if (randint1(2) == 1) return;
		}
		break;

	case BIAS_STR:
		if (!(o_ptr->art_flags1 & TR1_STR))
		{
			o_ptr->art_flags1 |= TR1_STR;
			if (randint1(2) == 1) return;
		}
		break;

	case BIAS_WIS:
		if (!(o_ptr->art_flags1 & TR1_WIS))
		{
			o_ptr->art_flags1 |= TR1_WIS;
			if (randint1(2) == 1) return;
		}
		break;

	case BIAS_INT:
		if (!(o_ptr->art_flags1 & TR1_INT))
		{
			o_ptr->art_flags1 |= TR1_INT;
			if (randint1(2) == 1) return;
		}
		break;

	case BIAS_DEX:
		if (!(o_ptr->art_flags1 & TR1_DEX))
		{
			o_ptr->art_flags1 |= TR1_DEX;
			if (randint1(2) == 1) return;
		}
		break;

	case BIAS_CON:
		if (!(o_ptr->art_flags1 & TR1_CON))
		{
			o_ptr->art_flags1 |= TR1_CON;
			if (randint1(2) == 1) return;
		}
		break;

	case BIAS_CHR:
		if (!(o_ptr->art_flags1 & TR1_CHR))
		{
			o_ptr->art_flags1 |= TR1_CHR;
			if (randint1(2) == 1) return;
		}
		break;
	}

	if ((artifact_bias == BIAS_MAGE || artifact_bias == BIAS_PRIESTLY) && (o_ptr->tval == TV_SOFT_ARMOR) && (o_ptr->sval == SV_ROBE))
	{
		if (!(o_ptr->art_flags3 & TR3_DEC_MANA) && one_in_(3))
		{
			o_ptr->art_flags3 |= TR3_DEC_MANA;
			if (one_in_(2)) return;
		}
	}

	switch (randint1(this_type))
	{
	case 1: case 2:
		o_ptr->art_flags1 |= TR1_STR;
		if (!artifact_bias && randint1(13) != 1)
			artifact_bias = BIAS_STR;
		else if (!artifact_bias && randint1(7) == 1)
			artifact_bias = BIAS_WARRIOR;
		break;
	case 3: case 4:
		o_ptr->art_flags1 |= TR1_INT;
		if (!artifact_bias && randint1(13) != 1)
			artifact_bias = BIAS_INT;
		else if (!artifact_bias && randint1(7) == 1)
			artifact_bias = BIAS_MAGE;
		break;
	case 5: case 6:
		o_ptr->art_flags1 |= TR1_WIS;
		if (!artifact_bias && randint1(13) != 1)
			artifact_bias = BIAS_WIS;
		else if (!artifact_bias && randint1(7) == 1)
			artifact_bias = BIAS_PRIESTLY;
		break;
	case 7: case 8:
		o_ptr->art_flags1 |= TR1_DEX;
		if (!artifact_bias && randint1(13) != 1)
			artifact_bias = BIAS_DEX;
		else if (!artifact_bias && randint1(7) == 1)
			artifact_bias = BIAS_ROGUE;
		break;
	case 9: case 10:
		o_ptr->art_flags1 |= TR1_CON;
		if (!artifact_bias && randint1(13) != 1)
			artifact_bias = BIAS_CON;
		else if (!artifact_bias && randint1(9) == 1)
			artifact_bias = BIAS_RANGER;
		break;
	case 11: case 12:
		o_ptr->art_flags1 |= TR1_CHR;
		if (!artifact_bias && randint1(13) != 1)
			artifact_bias = BIAS_CHR;
		break;
	case 13: case 14:
		o_ptr->art_flags1 |= TR1_STEALTH;
		if (!artifact_bias && randint1(3) == 1)
			artifact_bias = BIAS_ROGUE;
		break;
	case 15: case 16:
		o_ptr->art_flags1 |= TR1_SEARCH;
		if (!artifact_bias && randint1(9) == 1)
			artifact_bias = BIAS_RANGER;
		break;
	case 17: case 18:
		o_ptr->art_flags1 |= TR1_INFRA;
		break;
	case 19:
		o_ptr->art_flags1 |= TR1_SPEED;
		if (!artifact_bias && randint1(11) == 1)
			artifact_bias = BIAS_ROGUE;
		break;
	case 20:
		o_ptr->art_flags1 |= TR1_MAGIC_MASTERY;
		if (!artifact_bias && randint1(11) == 1)
			artifact_bias = BIAS_MAGE;
		break;
	case 21: case 22:
		o_ptr->art_flags1 |= TR1_TUNNEL;
		break;
	case 23: case 24:
		if (o_ptr->tval == TV_BOW) random_plus(o_ptr);
		else
		{
			o_ptr->art_flags1 |= TR1_BLOWS;
			if (!artifact_bias && randint1(11) == 1)
				artifact_bias = BIAS_WARRIOR;
		}
		break;
	}
}

void one_plus(object_type *o_ptr)
{
	switch (randint0(9))
	{
	case 0: o_ptr->art_flags1 |= TR1_STR; break;
	case 1: o_ptr->art_flags1 |= TR1_INT; break;
	case 2: o_ptr->art_flags1 |= TR1_WIS; break;
	case 3: o_ptr->art_flags1 |= TR1_DEX; break;
	case 4: o_ptr->art_flags1 |= TR1_CON; break;
	case 5: o_ptr->art_flags1 |= TR1_CHR; break;
	case 6: o_ptr->art_flags1 |= TR1_SEARCH; break;
	case 7: o_ptr->art_flags1 |= TR1_INFRA; break;
	case 8: o_ptr->art_flags1 |= TR1_TUNNEL; break;
	}
}

static void random_resistance(object_type * o_ptr, int specific)
{
	if (!specific) /* To avoid a number of possible bugs */
	{
		if (artifact_bias == BIAS_ACID)
		{
			if (!(o_ptr->art_flags2 & TR2_RES_ACID))
			{
				o_ptr->art_flags2 |= TR2_RES_ACID;
				if (randint1(2) == 1) return;
			}
			if ((randint1(BIAS_LUCK) == 1) && !(o_ptr->art_flags2 & TR2_IM_ACID))
			{
				o_ptr->art_flags2 |= TR2_IM_ACID;
				if (randint1(2) == 1) return;
			}
		}
		else if (artifact_bias == BIAS_ELEC)
		{
			if (!(o_ptr->art_flags2 & TR2_RES_ELEC))
			{
				o_ptr->art_flags2 |= TR2_RES_ELEC;
				if (randint1(2) == 1) return;
			}
			if ((o_ptr->tval >= TV_CLOAK) && (o_ptr->tval <= TV_HARD_ARMOR) &&
			   !(o_ptr->art_flags3 & TR3_SH_ELEC))
			{
				o_ptr->art_flags3 |= TR3_SH_ELEC;
				if (randint1(2) == 1) return;
			}
			if (randint1(BIAS_LUCK) == 1 && !(o_ptr->art_flags2 & TR2_IM_ELEC))
			{
				o_ptr->art_flags2 |= TR2_IM_ELEC;
				if (randint1(2) == 1) return;
			}
		}
		else if (artifact_bias == BIAS_FIRE)
		{
			if (!(o_ptr->art_flags2 & TR2_RES_FIRE))
			{
				o_ptr->art_flags2 |= TR2_RES_FIRE;
				if (randint1(2) == 1) return;
			}
			if ((o_ptr->tval >= TV_CLOAK) &&
			    (o_ptr->tval <= TV_HARD_ARMOR) &&
			    !(o_ptr->art_flags3 & TR3_SH_FIRE))
			{
				o_ptr->art_flags3 |= TR3_SH_FIRE;
				if (randint1(2) == 1) return;
			}
			if ((randint1(BIAS_LUCK) == 1) &&
			    !(o_ptr->art_flags2 & TR2_IM_FIRE))
			{
				o_ptr->art_flags2 |= TR2_IM_FIRE;
				if (randint1(2) == 1) return;
			}
		}
		else if (artifact_bias == BIAS_COLD)
		{
			if (!(o_ptr->art_flags2 & TR2_RES_COLD))
			{
				o_ptr->art_flags2 |= TR2_RES_COLD;
				if (randint1(2) == 1) return;
			}
			if ((o_ptr->tval >= TV_CLOAK) &&
			    (o_ptr->tval <= TV_HARD_ARMOR) &&
			    !(o_ptr->art_flags3 & TR3_SH_COLD))
			{
				o_ptr->art_flags3 |= TR3_SH_COLD;
				if (randint1(2) == 1) return;
			}
			if (randint1(BIAS_LUCK) == 1 && !(o_ptr->art_flags2 & TR2_IM_COLD))
			{
				o_ptr->art_flags2 |= TR2_IM_COLD;
				if (randint1(2) == 1) return;
			}
		}
		else if (artifact_bias == BIAS_POIS)
		{
			if (!(o_ptr->art_flags2 & TR2_RES_POIS))
			{
				o_ptr->art_flags2 |= TR2_RES_POIS;
				if (randint1(2) == 1) return;
			}
		}
		else if (artifact_bias == BIAS_WARRIOR)
		{
			if (randint1(3) != 1 && (!(o_ptr->art_flags2 & TR2_RES_FEAR)))
			{
				o_ptr->art_flags2 |= TR2_RES_FEAR;
				if (randint1(2) == 1) return;
			}
			if ((randint1(3) == 1) && (!(o_ptr->art_flags3 & TR3_NO_MAGIC)))
			{
				o_ptr->art_flags3 |= TR3_NO_MAGIC;
				if (randint1(2) == 1) return;
			}
		}
		else if (artifact_bias == BIAS_NECROMANTIC)
		{
			if (!(o_ptr->art_flags2 & TR2_RES_NETHER))
			{
				o_ptr->art_flags2 |= TR2_RES_NETHER;
				if (randint1(2) == 1) return;
			}
			if (!(o_ptr->art_flags2 & TR2_RES_POIS))
			{
				o_ptr->art_flags2 |= TR2_RES_POIS;
				if (randint1(2) == 1) return;
			}
			if (!(o_ptr->art_flags2 & TR2_RES_DARK))
			{
				o_ptr->art_flags2 |= TR2_RES_DARK;
				if (randint1(2) == 1) return;
			}
		}
	}

	switch (specific ? specific : randint1(35))
	{
		case 1:
			if (randint1(WEIRD_LUCK) != 1)
				random_resistance(o_ptr, specific);
			else
			{
				o_ptr->art_flags2 |= TR2_IM_ACID;
				if (!artifact_bias)
					artifact_bias = BIAS_ACID;
			}
			break;
		case 2:
			if (randint1(WEIRD_LUCK) != 1)
				random_resistance(o_ptr, specific);
			else
			{
				o_ptr->art_flags2 |= TR2_IM_ELEC;
				if (!artifact_bias)
					artifact_bias = BIAS_ELEC;
			}
			break;
		case 3:
			if (randint1(WEIRD_LUCK) != 1)
				random_resistance(o_ptr, specific);
			else
			{
				o_ptr->art_flags2 |= TR2_IM_COLD;
				if (!artifact_bias)
					artifact_bias = BIAS_COLD;
			}
			break;
		case 4:
			if (randint1(WEIRD_LUCK) != 1)
				random_resistance(o_ptr, specific);
			else
			{
				o_ptr->art_flags2 |= TR2_IM_FIRE;
				if (!artifact_bias)
					artifact_bias = BIAS_FIRE;
			}
			break;
		case 5:
		case 6:
		case 13:
			o_ptr->art_flags2 |= TR2_RES_ACID;
			if (!artifact_bias)
				artifact_bias = BIAS_ACID;
			break;
		case 7:
		case 8:
		case 14:
			o_ptr->art_flags2 |= TR2_RES_ELEC;
			if (!artifact_bias)
				artifact_bias = BIAS_ELEC;
			break;
		case 9:
		case 10:
		case 15:
			o_ptr->art_flags2 |= TR2_RES_FIRE;
			if (!artifact_bias)
				artifact_bias = BIAS_FIRE;
			break;
		case 11:
		case 12:
		case 16:
			o_ptr->art_flags2 |= TR2_RES_COLD;
			if (!artifact_bias)
				artifact_bias = BIAS_COLD;
			break;
		case 17:
		case 18:
			o_ptr->art_flags2 |= TR2_RES_POIS;
			if (!artifact_bias && randint1(4) != 1)
				artifact_bias = BIAS_POIS;
			else if (!artifact_bias && randint1(2) == 1)
				artifact_bias = BIAS_NECROMANTIC;
			else if (!artifact_bias && randint1(2) == 1)
				artifact_bias = BIAS_ROGUE;
			break;
		case 19:
		case 20:
			o_ptr->art_flags2 |= TR2_RES_FEAR;
			if (!artifact_bias && randint1(3) == 1)
				artifact_bias = BIAS_WARRIOR;
			break;
		case 21:
		case 22:
			o_ptr->art_flags2 |= TR2_RES_DARK;
			break;
		case 23:
		case 24:
			o_ptr->art_flags2 |= TR2_RES_BLIND;
			break;
		case 25:
		case 26:
			o_ptr->art_flags2 |= TR2_RES_CONF;
			break;
		case 27:
		case 28:
			o_ptr->art_flags2 |= TR2_RES_SOUND;
			break;
		case 29:
		case 30:
			o_ptr->art_flags2 |= TR2_RES_NETHER;
			if (!artifact_bias && randint1(3) == 1)
				artifact_bias = BIAS_NECROMANTIC;
			break;
		case 31:
			if (o_ptr->tval >= TV_CLOAK && o_ptr->tval <= TV_HARD_ARMOR)
				o_ptr->art_flags3 |= TR3_SH_ELEC;
			else
				random_resistance(o_ptr, specific);
			if (!artifact_bias)
				artifact_bias = BIAS_ELEC;
			break;
		case 32:
			if (o_ptr->tval >= TV_CLOAK && o_ptr->tval <= TV_HARD_ARMOR)
				o_ptr->art_flags3 |= TR3_SH_FIRE;
			else
				random_resistance(o_ptr, specific);
			if (!artifact_bias)
				artifact_bias = BIAS_FIRE;
			break;
		case 33:
			if (o_ptr->tval == TV_SHIELD || o_ptr->tval == TV_CLOAK ||
			    o_ptr->tval == TV_HELM || o_ptr->tval == TV_HARD_ARMOR)
				o_ptr->art_flags2 |= TR2_REFLECT;
			else
				random_resistance(o_ptr, specific);
			break;
		case 34:
			if (o_ptr->tval >= TV_CLOAK && o_ptr->tval <= TV_HARD_ARMOR)
				o_ptr->art_flags3 |= TR3_SH_COLD;
			else
				random_resistance(o_ptr, specific);
			if (!artifact_bias)
				artifact_bias = BIAS_COLD;
			break;
	}
}



static void random_misc(object_type * o_ptr)
{
	if (artifact_bias == BIAS_RANGER)
	{
		if (!(o_ptr->art_flags2 & TR2_SUST_CON))
		{
			o_ptr->art_flags2 |= TR2_SUST_CON;
			if (randint1(2) == 1) return;
		}
	}
	else if (artifact_bias == BIAS_STR)
	{
		if (!(o_ptr->art_flags2 & TR2_SUST_STR))
		{
			o_ptr->art_flags2 |= TR2_SUST_STR;
			if (randint1(2) == 1) return;
		}
	}
	else if (artifact_bias == BIAS_WIS)
	{
		if (!(o_ptr->art_flags2 & TR2_SUST_WIS))
		{
			o_ptr->art_flags2 |= TR2_SUST_WIS;
			if (randint1(2) == 1) return;
		}
	}
	else if (artifact_bias == BIAS_INT)
	{
		if (!(o_ptr->art_flags2 & TR2_SUST_INT))
		{
			o_ptr->art_flags2 |= TR2_SUST_INT;
			if (randint1(2) == 1) return;
		}
	}
	else if (artifact_bias == BIAS_DEX)
	{
		if (!(o_ptr->art_flags2 & TR2_SUST_DEX))
		{
			o_ptr->art_flags2 |= TR2_SUST_DEX;
			if (randint1(2) == 1) return;
		}
	}
	else if (artifact_bias == BIAS_CON)
	{
		if (!(o_ptr->art_flags2 & TR2_SUST_CON))
		{
			o_ptr->art_flags2 |= TR2_SUST_CON;
			if (randint1(2) == 1) return;
		}
	}
	else if (artifact_bias == BIAS_CHR)
	{
		if (!(o_ptr->art_flags2 & TR2_SUST_CHR))
		{
			o_ptr->art_flags2 |= TR2_SUST_CHR;
			if (randint1(2) == 1) return;
		}
	}
	else if (artifact_bias == BIAS_FIRE)
	{
		if (!(o_ptr->art_flags3 & TR3_LITE))
		{
			o_ptr->art_flags3 |= TR3_LITE; /* Freebie */
		}
	}

	switch (randint1(31))
	{
		case 1:
			o_ptr->art_flags2 |= TR2_SUST_STR;
			if (!artifact_bias)
				artifact_bias = BIAS_STR;
			break;
		case 2:
			o_ptr->art_flags2 |= TR2_SUST_INT;
			if (!artifact_bias)
				artifact_bias = BIAS_INT;
			break;
		case 3:
			o_ptr->art_flags2 |= TR2_SUST_WIS;
			if (!artifact_bias)
				artifact_bias = BIAS_WIS;
			break;
		case 4:
			o_ptr->art_flags2 |= TR2_SUST_DEX;
			if (!artifact_bias)
				artifact_bias = BIAS_DEX;
			break;
		case 5:
			o_ptr->art_flags2 |= TR2_SUST_CON;
			if (!artifact_bias)
				artifact_bias = BIAS_CON;
			break;
		case 6:
			o_ptr->art_flags2 |= TR2_SUST_CHR;
			if (!artifact_bias)
				artifact_bias = BIAS_CHR;
			break;
		case 7:
		case 8:
		case 14:
			o_ptr->art_flags2 |= TR2_FREE_ACT;
			break;
		case 9:
			o_ptr->art_flags2 |= TR2_HOLD_LIFE;
			if (!artifact_bias && (randint1(5) == 1))
				artifact_bias = BIAS_PRIESTLY;
			else if (!artifact_bias && (randint1(6) == 1))
				artifact_bias = BIAS_NECROMANTIC;
			break;
		case 10:
		case 11:
			o_ptr->art_flags3 |= TR3_LITE;
			break;
		case 12:
		case 13:
			o_ptr->art_flags3 |= TR3_FEATHER;
			break;
		case 15:
		case 16:
		case 17:
			o_ptr->art_flags3 |= TR3_SEE_INVIS;
			break;
		case 18:
			o_ptr->art_flags3 |= TR3_TELEPATHY;
			if (!artifact_bias && (randint1(9) == 1))
				artifact_bias = BIAS_MAGE;
			break;
		case 19:
		case 20:
			o_ptr->art_flags3 |= TR3_SLOW_DIGEST;
			break;
		case 21:
		case 22:
			o_ptr->art_flags3 |= TR3_REGEN;
			break;
		case 23:
			o_ptr->art_flags3 |= TR3_TELEPORT;
			break;
		case 24:
		case 25:
		case 26:
			if (o_ptr->tval >= TV_BOOTS && o_ptr->tval <= TV_DRAG_ARMOR)
				random_misc(o_ptr);
			else
				o_ptr->to_a = 4 + (s16b)randint1(11);
			break;
		case 27:
			o_ptr->art_flags3 |= TR3_SHOW_MODS;
			o_ptr->to_h += 5 + damroll(2, 6);
			break;
		case 28:
			o_ptr->art_flags3 |= TR3_SHOW_MODS;
			o_ptr->to_d += 5 + damroll(2, 6);
			break;
		case 29:
			o_ptr->art_flags3 |= TR3_SHOW_MODS;
			o_ptr->to_h += 2 + damroll(2, 4);
			o_ptr->to_d += 2 + damroll(2, 4);
			break;
		case 30:
			o_ptr->art_flags3 |= TR3_NO_MAGIC;
			break;
		case 31:
			o_ptr->art_flags3 |= TR3_NO_TELE;
			break;
	}
}


static void random_slay(object_type *o_ptr)
{
	if (artifact_bias == BIAS_PRIESTLY &&
	   (o_ptr->tval == TV_SWORD || o_ptr->tval == TV_POLEARM) &&
	  !(o_ptr->art_flags3 & TR3_BLESSED))
	{
		/* A free power for "priestly" random artifacts */
		o_ptr->art_flags3 |= TR3_BLESSED;
	}

	else if (artifact_bias == BIAS_NECROMANTIC)
	{
		if (!(o_ptr->art_flags1 & TR1_VAMPIRIC) && (o_ptr->tval != TV_BOW))
		{
			o_ptr->art_flags1 |= TR1_VAMPIRIC;
			if (randint1(2) == 1) return;
		}
		if (!(o_ptr->art_flags1 & TR1_BRAND_POIS) && (randint1(2) == 1))
		{
			o_ptr->art_flags1 |= TR1_BRAND_POIS;
			if (randint1(2) == 1) return;
		}
	}

	else if (artifact_bias == BIAS_RANGER && (o_ptr->tval != TV_BOW))
	{
		if (!(o_ptr->art_flags1 & TR1_SLAY_ANIMAL))
		{
			o_ptr->art_flags1 |= TR1_SLAY_ANIMAL;
			if (randint1(2) == 1) return;
		}
	}

	else if (artifact_bias == BIAS_ROGUE)
	{
		if ((((o_ptr->tval == TV_SWORD) && (o_ptr->sval == SV_DAGGER)) ||
		     ((o_ptr->tval == TV_POLEARM) && (o_ptr->sval == SV_SPEAR)))
			 && (o_ptr->tval != TV_BOW) && !(o_ptr->art_flags2 & TR2_THROW))
		{
			/* Free power for rogues... */
			o_ptr->art_flags2 |= TR2_THROW;
		}
		if ((!(o_ptr->art_flags1 & TR1_BRAND_POIS)) && (randint1(2) == 1))
		{
			o_ptr->art_flags1 |= TR1_BRAND_POIS;
			if (randint1(2) == 1) return;
		}
	}

	else if (artifact_bias == BIAS_POIS)
	{
		if (!(o_ptr->art_flags1 & TR1_BRAND_POIS))
		{
			o_ptr->art_flags1 |= TR1_BRAND_POIS;
			if (randint1(2) == 1) return;
		}
	}

	else if (artifact_bias == BIAS_FIRE)
	{
		if (!(o_ptr->art_flags1 & TR1_BRAND_FIRE))
		{
			o_ptr->art_flags1 |= TR1_BRAND_FIRE;
			if (randint1(2) == 1) return;
		}
	}

	else if (artifact_bias == BIAS_COLD)
	{
		if (!(o_ptr->art_flags1 & TR1_BRAND_COLD))
		{
			o_ptr->art_flags1 |= TR1_BRAND_COLD;
			if (randint1(2) == 1) return;
		}
	}

	else if (artifact_bias == BIAS_ELEC)
	{
		if (!(o_ptr->art_flags1 & TR1_BRAND_ELEC))
		{
			o_ptr->art_flags1 |= TR1_BRAND_ELEC;
			if (randint1(2) == 1) return;
		}
	}

	else if (artifact_bias == BIAS_ACID)
	{
		if (!(o_ptr->art_flags1 & TR1_BRAND_ACID))
		{
			o_ptr->art_flags1 |= TR1_BRAND_ACID;
			if (randint1(2) == 1) return;
		}
	}

	else if (artifact_bias == BIAS_LAW && (o_ptr->tval != TV_BOW))
	{
		if (!(o_ptr->art_flags1 & TR1_SLAY_EVIL))
		{
			o_ptr->art_flags1 |= TR1_SLAY_EVIL;
			if (randint1(2) == 1) return;
		}
		if (!(o_ptr->art_flags1 & TR1_SLAY_UNDEAD))
		{
			o_ptr->art_flags1 |= TR1_SLAY_UNDEAD;
			if (randint1(2) == 1) return;
		}
		if (!(o_ptr->art_flags1 & TR1_SLAY_DEMON))
		{
			o_ptr->art_flags1 |= TR1_SLAY_DEMON;
			if (randint1(2) == 1) return;
		}
	}

	if (o_ptr->tval != TV_BOW)
	{
		switch (randint1(36))
		{
		case 1:
		case 2:
			o_ptr->art_flags1 |= TR1_SLAY_ANIMAL;
			break;
		case 3:
		case 4:
			o_ptr->art_flags1 |= TR1_SLAY_EVIL;
			if (!artifact_bias && (randint1(2) == 1))
				artifact_bias = BIAS_LAW;
			else if (!artifact_bias && (randint1(9) == 1))
				artifact_bias = BIAS_PRIESTLY;
			break;
		case 5:
		case 6:
			o_ptr->art_flags1 |= TR1_SLAY_UNDEAD;
			if (!artifact_bias && (randint1(9) == 1))
				artifact_bias = BIAS_PRIESTLY;
			break;
		case 7:
		case 8:
			o_ptr->art_flags1 |= TR1_SLAY_DEMON;
			if (!artifact_bias && (randint1(9) == 1))
				artifact_bias = BIAS_PRIESTLY;
			break;
		case 9:
		case 10:
			o_ptr->art_flags1 |= TR1_SLAY_ORC;
			break;
		case 11:
		case 12:
			o_ptr->art_flags1 |= TR1_SLAY_TROLL;
			break;
		case 13:
		case 14:
			o_ptr->art_flags1 |= TR1_SLAY_GIANT;
			break;
		case 15:
		case 16:
			o_ptr->art_flags1 |= TR1_SLAY_DRAGON;
			break;
		case 17:
			o_ptr->art_flags1 |= TR1_KILL_DRAGON;
			break;
		case 18:
		case 19:
			if (o_ptr->tval == TV_SWORD)
			{
				o_ptr->art_flags1 |= TR1_VORPAL;
				if (!artifact_bias && (randint1(9) == 1))
					artifact_bias = BIAS_WARRIOR;
			}
			else
				random_slay(o_ptr);
			break;
		case 20:
			o_ptr->art_flags1 |= TR1_IMPACT;
			break;
		case 21:
		case 22:
			o_ptr->art_flags1 |= TR1_BRAND_FIRE;
			if (!artifact_bias)
				artifact_bias = BIAS_FIRE;
			break;
		case 23:
		case 24:
			o_ptr->art_flags1 |= TR1_BRAND_COLD;
			if (!artifact_bias)
				artifact_bias = BIAS_COLD;
			break;
		case 25:
		case 26:
			o_ptr->art_flags1 |= TR1_BRAND_ELEC;
			if (!artifact_bias)
				artifact_bias = BIAS_ELEC;
			break;
		case 27:
		case 28:
			o_ptr->art_flags1 |= TR1_BRAND_ACID;
			if (!artifact_bias)
				artifact_bias = BIAS_ACID;
			break;
		case 29:
		case 30:
			o_ptr->art_flags1 |= TR1_BRAND_POIS;
			if (!artifact_bias && (randint1(3) != 1))
				artifact_bias = BIAS_POIS;
			else if (!artifact_bias && randint1(6) == 1)
				artifact_bias = BIAS_NECROMANTIC;
			else if (!artifact_bias)
				artifact_bias = BIAS_ROGUE;
			break;
		case 31:
		case 32:
			o_ptr->art_flags1 |= TR1_VAMPIRIC;
			if (!artifact_bias)
				artifact_bias = BIAS_NECROMANTIC;
			break;
		case 33:
		case 34:
			o_ptr->art_flags1 |= TR1_SLAY_HUMAN;
		default:
			o_ptr->art_flags1 |= TR1_CHAOTIC;
			break;
		}
	}
	else
	{
		int rnd = (artifact_bias > 0) ? 6 : 9;
		switch (randint1(rnd))
		{
		case 1:
		case 2:
		case 3:
			o_ptr->art_flags3 |= TR3_XTRA_MIGHT;
			if (!artifact_bias && randint1(9) == 1)
				artifact_bias = BIAS_RANGER;
			break;
		case 4:
		case 5:
		case 6:
			o_ptr->art_flags3 |= TR3_XTRA_SHOTS;
			if (!artifact_bias && randint1(9) == 1)
				artifact_bias = BIAS_RANGER;
			break;
		default:
			switch(randint1(10)){
			case 1:
			case 2:
				o_ptr->art_flags1 |= TR1_BRAND_FIRE;
				if (!artifact_bias)
					artifact_bias = BIAS_FIRE;
				break;
			case 3:
			case 4:
				o_ptr->art_flags1 |= TR1_BRAND_COLD;
				if (!artifact_bias)
					artifact_bias = BIAS_COLD;
				break;
			case 5:
			case 6:
				o_ptr->art_flags1 |= TR1_BRAND_ELEC;
				if (!artifact_bias)
					artifact_bias = BIAS_ELEC;
				break;
			case 7:
			case 8:
				o_ptr->art_flags1 |= TR1_BRAND_ACID;
				if (!artifact_bias)
					artifact_bias = BIAS_ACID;
				break;
			default:
				o_ptr->art_flags1 |= TR1_BRAND_POIS;
				if (!artifact_bias && (randint1(3) != 1))
					artifact_bias = BIAS_POIS;
				else if (!artifact_bias && randint1(6) == 1)
					artifact_bias = BIAS_NECROMANTIC;
				else if (!artifact_bias)
					artifact_bias = BIAS_ROGUE;
				break;
			}
		}
	}
}


void one_brand(object_type *o_ptr)
{
	switch(randint0(6))
	{
	case 0: o_ptr->art_flags1 |= TR1_BRAND_ACID; break;
	case 1: o_ptr->art_flags1 |= TR1_BRAND_ELEC; break;
	case 2: o_ptr->art_flags1 |= TR1_BRAND_COLD; break;
	case 3: o_ptr->art_flags1 |= TR1_BRAND_FIRE; break;
	case 4: o_ptr->art_flags1 |= TR1_BRAND_POIS; break;
	default:
		switch(randint0(4))
		{
		case 0: o_ptr->art_flags1 |= TR1_CHAOTIC; break;
		case 1: o_ptr->art_flags1 |= TR1_IMPACT; break;
		case 2: o_ptr->art_flags1 |= TR1_VORPAL; break;
		default: o_ptr->art_flags1 |= TR1_VAMPIRIC; break;
		}
	}
}


void one_slay(object_type *o_ptr)
{
	switch(randint0(8))
	{
	case 0: o_ptr->art_flags1 |= TR1_SLAY_HUMAN; break;
	case 1: o_ptr->art_flags1 |= TR1_SLAY_ANIMAL; break;
	case 2: o_ptr->art_flags1 |= TR1_SLAY_UNDEAD; break;
	case 3: o_ptr->art_flags1 |= TR1_SLAY_DEMON; break;
	case 4: o_ptr->art_flags1 |= TR1_SLAY_ORC; break;
	case 5: o_ptr->art_flags1 |= TR1_SLAY_TROLL; break;
	case 6: o_ptr->art_flags1 |= TR1_SLAY_GIANT; break;
	case 7: o_ptr->art_flags1 |= ((one_in_(7)) ? TR1_KILL_DRAGON : TR1_SLAY_DRAGON); break;
	default: /* Evil slaying is not branded */
		o_ptr->art_flags1 |= TR1_SLAY_EVIL;
	}
}


void give_activation_power(object_type *o_ptr)
{
	int type = 0, chance = 0;
#if 0
	if (artifact_bias)
	{
		if (artifact_bias == BIAS_ELEC)
		{
			if (randint1(3) != 1)
			{
				type = ACT_BO_ELEC_1;
			}
			else if (randint1(5) != 1)
			{
				type = ACT_BA_ELEC_2;
			}
			else
			{
				type = ACT_BA_ELEC_3;
			}
			chance = 101;
		}
		else if (artifact_bias == BIAS_POIS)
		{
			type = ACT_BA_POIS_1;
			chance = 101;
		}
		else if (artifact_bias == BIAS_FIRE)
		{
			if (randint1(3) != 1)
			{
				type = ACT_BO_FIRE_1;
			}
			else if (randint1(5) != 1)
			{
				type = ACT_BA_FIRE_1;
			}
			else
			{
				type = ACT_BA_FIRE_2;
			}
			chance = 101;
		}
		else if (artifact_bias == BIAS_COLD)
		{
			chance = 101;
			if (randint1(3) != 1)
				type = ACT_BO_COLD_1;
			else if (randint1(3) != 1)
				type = ACT_BA_COLD_1;
			else if (randint1(3) != 1)
				type = ACT_BA_COLD_2;
			else
				type = ACT_BA_COLD_3;
		}
		else if (artifact_bias == BIAS_PRIESTLY)
		{
			chance = 101;

			if (randint1(13) == 1)
				type = ACT_CHARM_UNDEAD;
			else if (randint1(12) == 1)
				type = ACT_BANISH_EVIL;
			else if (randint1(11) == 1)
				type = ACT_DISP_EVIL;
			else if (randint1(10) == 1)
				type = ACT_PROT_EVIL;
			else if (randint1(9) == 1)
				type = ACT_CURE_1000;
			else if (randint1(8) == 1)
				type = ACT_CURE_700;
			else if (randint1(7) == 1)
				type = ACT_REST_ALL;
			else if (randint1(6) == 1)
				type = ACT_REST_LIFE;
			else if (randint1(5) == 1)
				type = ACT_CURING;
			else
				type = ACT_CURE_MW;
		}
		else if (artifact_bias == BIAS_NECROMANTIC)
		{
			chance = 101;
			if (randint1(66) == 1)
				type = ACT_WRAITH;
			else if (randint1(13) == 1)
				type = ACT_DISP_GOOD;
			else if (randint1(9) == 1)
				type = ACT_MASS_GENO;
			else if (randint1(8) == 1)
				type = ACT_GENOCIDE;
			else if (randint1(13) == 1)
				type = ACT_SUMMON_UNDEAD;
			else if (randint1(9) == 1)
				type = ACT_VAMPIRE_2;
			else if (randint1(6) == 1)
				type = ACT_CHARM_UNDEAD;
			else
				type = ACT_VAMPIRE_1;
		}
		else if (artifact_bias == BIAS_LAW)
		{
			chance = 101;
			if (randint1(8) == 1)
				type = ACT_BANISH_EVIL;
			else if (randint1(4) == 1)
				type = ACT_DISP_EVIL;
			else
				type = ACT_PROT_EVIL;
		}
		else if (artifact_bias == BIAS_ROGUE)
		{
			chance = 101;
			if (randint1(50) == 1)
				type = ACT_SPEED;
			else if (randint1(4) == 1)
				type = ACT_SLEEP;
			else if (randint1(3) == 1)
				type = ACT_DETECT_ALL;
			else if (randint1(8) == 1)
				type = ACT_ID_FULL;
			else
				type = ACT_ID_PLAIN;
		}
		else if (artifact_bias == BIAS_MAGE)
		{
			chance = 66;
			if (randint1(20) == 1)
				type = ACT_SUMMON_ELEMENTAL;
			else if (randint1(10) == 1)
				type = ACT_SUMMON_PHANTOM;
			else if (randint1(5) == 1)
				type = ACT_RUNE_EXPLO;
			else
				type = ACT_ESP;
		}
		else if (artifact_bias == BIAS_WARRIOR)
		{
			chance = 80;
			if (randint1(100) == 1)
				type = ACT_INVULN;
			else
				type = ACT_BERSERK;
		}
		else if (artifact_bias == BIAS_RANGER)
		{
			chance = 101;
			if (randint1(20) == 1)
				type = ACT_CHARM_ANIMALS;
			else if (randint1(7) == 1)
				type = ACT_SUMMON_ANIMAL;
			else if (randint1(6) == 1)
				type = ACT_CHARM_ANIMAL;
			else if (randint1(4) == 1)
				type = ACT_RESIST_ALL;
			else if (randint1(3) == 1)
				type = ACT_SATIATE;
			else
				type = ACT_CURE_POISON;
		}
	}
#endif
	while (!type || (randint1(100) >= chance))
	{
		int i;
		type = randint1(255);
		chance = 0;
		for (i = 0; activation_info[i].flag != NULL; i++)
		{
			if (activation_info[i].index == type)
			{
				chance = activation_info[i].chance;
				break;
			}
		}
	}

	/* A type was chosen... */
	o_ptr->xtra2 = type;
	o_ptr->art_flags3 |= TR3_ACTIVATE;
	o_ptr->timeout = 0;
}


static void get_random_name(char *return_name, byte tval, int power)
{
	if ((tval == TV_AMULET) || (tval == TV_RING) || (tval == TV_LITE))
	{
		get_table_name(return_name);
	}
	else
	{
		get_table_sindarin(return_name);
	}
}


bool create_artifact(object_type *o_ptr, bool a_scroll)
{
	char    new_name[1024];
	int     has_pval = 0;
	int     powers = randint1(5) + 1;
	int     max_type = (o_ptr->tval < TV_BOOTS ? 7 : 5);
	int     power_level;
	s32b    total_flags;
	bool    a_cursed = FALSE;
	int     warrior_artifact_bias = 0;


	artifact_bias = 0;

	/* Nuke enchantments */
	o_ptr->name1 = 0;
	o_ptr->name2 = 0;

	/* Include obvious flags */
	o_ptr->art_flags1 |= k_info[o_ptr->k_idx].flags1;
	o_ptr->art_flags2 |= k_info[o_ptr->k_idx].flags2;
	o_ptr->art_flags3 |= k_info[o_ptr->k_idx].flags3;

	/* base item have pval? */
	if (o_ptr->pval) has_pval = TRUE;

	if (a_scroll && one_in_(4))
	{
		switch (p_ptr->pclass)
		{
			case CLASS_WARRIOR:
				artifact_bias = BIAS_WARRIOR;
				break;
			case CLASS_MAGE:
				artifact_bias = BIAS_MAGE;
				break;
			case CLASS_PRIEST:
				artifact_bias = BIAS_PRIESTLY;
				break;
			case CLASS_ARCHER:
				artifact_bias = BIAS_RANGER;
				warrior_artifact_bias = 30;
				break;
			case CLASS_PALADIN:
				artifact_bias = BIAS_PRIESTLY;
				warrior_artifact_bias = 40;
				break;
			case CLASS_WARRIOR_MAGE:
				artifact_bias = BIAS_MAGE;
				warrior_artifact_bias = 40;
				break;
		}
	}

	if (a_scroll && (randint1(100) <= warrior_artifact_bias))
		artifact_bias = BIAS_WARRIOR;

	strcpy(new_name, "");

	if (!a_scroll && one_in_(A_CURSED))
		a_cursed = TRUE;

	while (one_in_(powers) || one_in_(7) || one_in_(10))
		powers++;

	if (!a_cursed && one_in_(WEIRD_LUCK))
		powers *= 2;

	if (a_cursed) powers /= 2;

	/* Main loop */
	while (powers--)
	{
		switch (randint1(max_type))
		{
			case 1: case 2:
				random_plus(o_ptr);
				has_pval = TRUE;
				break;
			case 3: case 4:
				random_resistance(o_ptr, FALSE);
				break;
			case 5:
				random_misc(o_ptr);
				break;
			case 6: case 7:
				random_slay(o_ptr);
				break;
			default:
#ifdef JP
				if (wizard) msg_print("Switch error in create_artifact!");
#else
				if (wizard) msg_print("Switch error in create_artifact!");
#endif
				powers++;
		}
	};

	/* Some Artifact Weapon gets extra dice */
	if (o_ptr->tval < TV_BOOTS)
	{
		if (!a_cursed) {
			if (one_in_(2 + (o_ptr->dd / 2))) o_ptr->dd++;
		}

		/* Hack -- Super-charge the damage dice */
		while (one_in_(10L * o_ptr->dd)) o_ptr->dd++;

		/* Hack -- Lower the damage dice */
		if (o_ptr->dd > 9) o_ptr->dd = 9;
	}

	if (has_pval)
	{
		if (o_ptr->art_flags1 & (TR1_BLOWS))
		{
			o_ptr->pval = randint1(2);
			if (one_in_(3) && ((o_ptr->dd * (o_ptr->ds + 1)) < 15)) o_ptr->pval++;
		}
		else
		{
			do
			{
				o_ptr->pval++;
			}
			while (o_ptr->pval < randint1(5) || randint1(o_ptr->pval) == 1);
		}

		if ((o_ptr->pval > 4) && (randint1(WEIRD_LUCK) != 1))
			o_ptr->pval = 4;
	}

	/* give it some plusses... ( except Amulets and Rings ) */
	if (o_ptr->tval >= TV_BOOTS && o_ptr->tval <= TV_DRAG_ARMOR)
		o_ptr->to_a += (s16b)randint1(o_ptr->to_a > 19 ? 1 : 20 - o_ptr->to_a);
	else if (o_ptr->tval <= TV_SWORD)
	{
		o_ptr->to_h += (s16b)randint1(o_ptr->to_h > 19 ? 1 : 20 - o_ptr->to_h);
		o_ptr->to_d += (s16b)randint1(o_ptr->to_d > 19 ? 1 : 20 - o_ptr->to_d);
	}

	/* Just to be sure */
	o_ptr->art_flags3 |= (TR3_IGNORE_ACID | TR3_IGNORE_ELEC |
			      TR3_IGNORE_FIRE | TR3_IGNORE_COLD);

	total_flags = flag_cost(o_ptr, o_ptr->pval);
	if (cheat_peek) msg_format("%ld", total_flags);

	if (a_cursed) curse_artifact(o_ptr);

	if (o_ptr->tval == TV_LITE && o_ptr->sval == SV_LITE_FEANOR)
	{
		artifact_bias = 0;
		o_ptr->xtra2 = 0;
		give_activation_power(o_ptr);
	}
	else if (!a_cursed &&
	    (randint1((o_ptr->tval >= TV_BOOTS)
	    ? ACTIVATION_CHANCE * 2 : ACTIVATION_CHANCE) == 1))
	{
		o_ptr->xtra2 = 0;
		give_activation_power(o_ptr);
	}

	if (o_ptr->tval >= TV_BOOTS)
	{
		if (a_cursed) power_level = 0;
		else if (total_flags < 10000) power_level = 1;
		else if (total_flags < 20000) power_level = 2;
		else power_level = 3;
	}

	else
	{
		if (a_cursed) power_level = 0;
		else if (total_flags < 15000) power_level = 1;
		else if (total_flags < 30000) power_level = 2;
		else power_level = 3;
	}

	if (a_scroll)
	{
		char dummy_name[80];
		strcpy(dummy_name, "");
		(void)identify_fully_aux(o_ptr, TRUE);
		o_ptr->ident |= IDENT_STORE; /* This will be used later on... */
#ifdef JP
		if (!(get_string("このアーティファクトを何と名付けますか？", dummy_name, 80)))
#else
		if (!(get_string("What do you want to call the artifact? ", dummy_name, 80)))
#endif
		{
			get_random_name(new_name, o_ptr->tval, power_level);
		}
		else
		{
#ifdef JP
			strcpy(new_name, "《");
#else
			strcpy(new_name, "'");
#endif
			strcat(new_name, dummy_name);
#ifdef JP
			strcat(new_name, "》という名の");
#else
			strcat(new_name, "'");
#endif
		}
		/* Identify it fully */
		object_aware(o_ptr);
		object_known(o_ptr);

		/* Mark the item as fully known */
		o_ptr->ident |= (IDENT_MENTAL);
	}
	else
	{
		get_random_name(new_name, o_ptr->tval, power_level);
	}

	if (cheat_xtra)
	{
		if (artifact_bias)
#ifdef JP
			msg_format("運の偏ったアーティファクト: %d。", artifact_bias);
#else
			msg_format("Biased artifact: %d.", artifact_bias);
#endif
		else
#ifdef JP
			msg_print("アーティファクトに運の偏りなし。");
#else
			msg_print("No bias in artifact.");
#endif
	}

	/* Save the inscription */
	o_ptr->art_name = quark_add(new_name);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP);

	return TRUE;
}


int activation_index(const object_type *o_ptr)
{
	if (artifact_p(o_ptr))
	{
		if (a_info[o_ptr->name1].flags3 & TR3_ACTIVATE)
		{
			return a_info[o_ptr->name1].activate;
		}
	}
	if (ego_item_p(o_ptr))
	{
		if (e_info[o_ptr->name2].flags3 & TR3_ACTIVATE)
		{
			return e_info[o_ptr->name2].activate;
		}
	}
	if (!(o_ptr->art_name))
	{
		if (k_info[o_ptr->k_idx].flags3 & TR3_ACTIVATE)
		{
			return k_info[o_ptr->k_idx].activate;
		}
	}
	return o_ptr->xtra2;
}

const activation_type* find_activation_info(const object_type *o_ptr)
{
	const int index = activation_index(o_ptr);
	const activation_type* p;

	for (p = activation_info; p->flag != NULL; ++ p)
	{
		if (p->index == index)
		{
			return p;
		}
	}

	return NULL;
}

bool activate_random_artifact(object_type * o_ptr)
{
	int plev = p_ptr->lev;
	int k, dir;
	const activation_type* const act_ptr = find_activation_info(o_ptr);

	/* Paranoia */
	if (!act_ptr) {
		/* Maybe forgot adding information to activation_info table ? */
		msg_print("Activation information is not found.");
		return FALSE;
	}

	/* Activate for attack */
	switch (act_ptr->index)
	{
		/* Bolts/Beams */
		case ACT_SUNLIGHT:
		{
			if (!get_aim_dir(&dir)) return FALSE;
			(void)lite_line(dir);
			break;
		}
		case ACT_BO_MISSILE:
		{
			if (!get_aim_dir(&dir)) return FALSE;
			fire_bolt(GF_MISSILE, dir, damroll(2, 6));
			break;
		}
		case ACT_BO_ELEC:
		{
			if (!get_aim_dir(&dir)) return FALSE;
			fire_bolt(GF_ELEC, dir, damroll(4, 8));
			break;
		}
		case ACT_BO_ACID:
		{
			if (!get_aim_dir(&dir)) return FALSE;
			fire_bolt(GF_ACID, dir, damroll(5, 8));
			break;
		}
		case ACT_BO_COLD:
		{
			if (!get_aim_dir(&dir)) return FALSE;
			fire_bolt(GF_COLD, dir, damroll(6, 8));
			break;
		}
		case ACT_BO_FIRE:
		{
			if (!get_aim_dir(&dir)) return FALSE;
			fire_bolt(GF_FIRE, dir, damroll(9, 8));
			break;
		}
		case ACT_BO_DRAIN:
		{
			if (!get_aim_dir(&dir)) return FALSE;
			if (drain_life(dir, 120))
			break;
		}
		case ACT_BO_MANA:
		{
			if (!get_aim_dir(&dir)) return FALSE;
			fire_bolt(GF_MANA, dir, 150);
			break;
		}

		case ACT_BA_POIS:
		{
			if (!get_aim_dir(&dir)) return FALSE;
			fire_ball(GF_POIS, dir, 12, 3);
			break;
		}
		case ACT_BA_ELEC_72:
		{
			if (!get_aim_dir(&dir)) return FALSE;
			fire_ball(GF_ELEC, dir, 72, 3);
			break;
		}
		case ACT_BA_ACID_72:
		{
			if (!get_aim_dir(&dir)) return FALSE;
			fire_ball(GF_ACID, dir, 72, 2);
			break;
		}
		case ACT_BA_FIRE_72:
		{
			if (!get_aim_dir(&dir)) return FALSE;
			fire_ball(GF_FIRE, dir, 72, 2);
			break;
		}
		case ACT_BA_COLD_72:
		{
			if (!get_aim_dir(&dir)) return FALSE;
			fire_ball(GF_COLD, dir, 72, 2);
			break;
		}
		case ACT_BA_ELEC_300:
		{
			if (!get_aim_dir(&dir)) return FALSE;
			fire_ball(GF_ELEC, dir, 300, 4);
			break;
		}
		case ACT_BA_ACID_300:
		{
			if (!get_aim_dir(&dir)) return FALSE;
			fire_ball(GF_ACID, dir, 300, 4);
			break;
		}
		case ACT_BA_FIRE_300:
		{
			if (!get_aim_dir(&dir)) return FALSE;
			fire_ball(GF_FIRE, dir, 300, 4);
			break;
		}
		case ACT_BA_COLD_300:
		{
			if (!get_aim_dir(&dir)) return FALSE;
			fire_ball(GF_COLD, dir, 300, 4);
			break;
		}
		case ACT_BA_STAR:
		{
			if (!get_aim_dir(&dir)) return FALSE;
			fire_ball(GF_LITE, dir, 150, 4);
			break;
		}
		case ACT_BA_DARK:
		{
			if (!get_aim_dir(&dir)) return FALSE;
			fire_ball(GF_DARK, dir, 250, 4);
			break;
		}
		case ACT_BA_MANA:
		{
			if (!get_aim_dir(&dir)) return FALSE;
			fire_ball(GF_MANA, dir, 250, 4);
			break;
		}
		case ACT_BIZARRE:
		{
			if (!get_aim_dir(&dir)) return FALSE;
			ring_of_power(dir);
			break;
		}
		case ACT_STARLIGHT:
		{
			int num = damroll(5, 3);
			int y, x;
			int attempts;

			for (k = 0; k < num; k++)
			{
				attempts = 1000;

				while (attempts--)
				{
					scatter(&y, &x, py, px, 4, 0);

					if (!cave_floor_bold(y, x)) continue;

					if ((y != py) || (x != px)) break;
				}

				project(0, 0, y, x, damroll(6 + p_ptr->lev / 8, 10), GF_LITE_WEAK,
						  (PROJECT_BEAM | PROJECT_THRU | PROJECT_GRID | PROJECT_KILL));
			}
			break;
		}

		/* Dispells */
		case ACT_DISP_EVIL:
		{
			dispel_evil(p_ptr->lev * 5);
			break;
		}
		case ACT_PESTICIDE:
		{
			(void)dispel_monsters(4);
			break;
		}
		case ACT_BLINDING_LIGHT:
		{
			fire_ball(GF_LITE, 0, 300, 6);
			confuse_monsters(3 * p_ptr->lev / 2);
			break;
		}

		/* Offensive action */
		case ACT_AGGRAVATE:
		{
			aggravate_monsters(0);
			break;
		}
		case ACT_TERROR:
		{
			msg_print(_("おどろおどろしい音が鳴り渡った...",
				"You blow a mighty blast; your enemies tremble!"));
			turn_monsters(40 + p_ptr->lev);
			break;
		}
		case ACT_CONFUSE:
		{
			confuse_monsters(p_ptr->lev);
			break;
		}
		case ACT_SLEEP:
		{
			sleep_monsters_touch();
			break;
		}
		case ACT_TELE_AWAY:
		{
			if (!get_aim_dir(&dir)) return FALSE;
			(void)fire_beam(GF_AWAY_ALL, dir, plev);
			break;
		}
		case ACT_BANISH_EVIL:
		{
			(void)banish_evil(100);
			break;
		}
		case ACT_GENOCIDE:
		{
			(void)genocide(66);
			break;
		}
		case ACT_MASS_GENO:
		{
			(void)mass_genocide(66);
			break;
		}

		/* Heals */
		case ACT_CURE_LW:
		{
			(void)set_afraid(0);
			(void)hp_player(30);
			break;
		}
		case ACT_CURE_MW:
		{
			hp_player(100);
			(void)set_poisoned(0);
			(void)set_cut(0);
			(void)set_confused(0);
			(void)set_image(0);
			(void)set_blind(0);
			(void)set_stun(0);
			break;
		}
		case ACT_REST_ALL:
		{
			(void)do_res_stat(A_STR);
			(void)do_res_stat(A_INT);
			(void)do_res_stat(A_WIS);
			(void)do_res_stat(A_DEX);
			(void)do_res_stat(A_CON);
			(void)do_res_stat(A_CHR);
			(void)restore_level();
			break;
		}
		case ACT_CURE_700:
		{
			(void)hp_player(700);
			(void)set_cut(0);
			break;
		}
		case ACT_CURE_MANA:
		{
			if (p_ptr->csp < p_ptr->msp)
			{
				p_ptr->csp = p_ptr->msp;
				p_ptr->csp_frac = 0;
				msg_print(_("頭がハッキリとした。", "You feel your head clear."));
				p_ptr->redraw |= (PR_MANA);
				p_ptr->window |= (PW_PLAYER);
				p_ptr->window |= (PW_SPELL);
			}
			break;
		}

		/* Activate for timed effect */
		case ACT_HELO:
		{
			(void)set_hero(p_ptr->hero + randint1(20) + 20);
			break;
		}
		case ACT_HELO_SPEED:
		{
			int t = randint1(50) + 50;
			(void)set_hero(p_ptr->hero + t);
			(void)set_fast(p_ptr->fast + t);
			break;
		}
		case ACT_BERSERK:
		{
			(void)set_shero(p_ptr->shero + randint1(50) + 50);
			(void)set_blessed(p_ptr->blessed + randint1(50) + 50);
			break;
		}
		case ACT_SPEED:
		{
			(void)set_fast(randint1(20) + 20);
			break;
		}
		case ACT_PROT_EVIL:
		{
			k = 3 * p_ptr->lev;
			(void)set_protevil(p_ptr->protevil + randint1(25) + k);
			break;
		}
		case ACT_WRAITH:
		{
			set_wraith_form(p_ptr->tim_wraith + randint1(plev / 2) + (plev / 2));
			break;
		}

		case ACT_INVULN:
		{
			(void)set_invuln(p_ptr->invuln + randint1(8) + 8);
			break;
		}
		case ACT_RESIST_ELEC:
		{
			(void)set_oppose_elec(p_ptr->oppose_elec + randint1(40) + 40);
			break;
		}
		case ACT_RESIST_ACID:
		{
			(void)set_oppose_acid(p_ptr->oppose_acid + randint1(40) + 40);
			break;
		}
		case ACT_RESIST_FIRE:
		{
			(void)set_oppose_fire(p_ptr->oppose_fire + randint1(40) + 40);
			break;
		}
		case ACT_RESIST_COLD:
		{
			(void)set_oppose_cold(p_ptr->oppose_cold + randint1(40) + 40);
			break;
		}
		case ACT_RESIST_POIS:
		{
			(void)set_oppose_pois(p_ptr->oppose_pois + randint1(40) + 40);
			break;
		}
		case ACT_RESIST_ALL:
		{
			(void)set_oppose_acid(p_ptr->oppose_acid + randint1(40) + 40);
			(void)set_oppose_elec(p_ptr->oppose_elec + randint1(40) + 40);
			(void)set_oppose_fire(p_ptr->oppose_fire + randint1(40) + 40);
			(void)set_oppose_cold(p_ptr->oppose_cold + randint1(40) + 40);
			(void)set_oppose_pois(p_ptr->oppose_pois + randint1(40) + 40);
			break;
		}
		case ACT_ESP:
		{
			(void)set_tim_esp(p_ptr->tim_esp + randint1(30) + 25);
			o_ptr->timeout = 200;
			break;
		}

		/* General purposes */
		case ACT_LIGHT:
		{
			lite_area(damroll(2, 15), 3);
			break;
		}
		case ACT_MAP_LIGHT:
		{
			map_area(DETECT_RAD_MAP);
			lite_area(damroll(2, 15), 3);
			break;
		}
		case ACT_DETECT_ALL:
		{
			detect_all(DETECT_RAD_DEFAULT);
			break;
		}
		case ACT_DETECT_UNIQUE:
		{
			monster_type *m_ptr;
			monster_race *r_ptr;
			int i;

			msg_print(_("奇妙な場所が頭の中に浮かんだ．．．",
				"Some strange places show up in your mind. And you see ..."));

			/* Process the monsters (backwards) */
			for (i = m_max - 1; i >= 1; i--)
			{
				/* Access the monster */
				m_ptr = &m_list[i];
					
				/* Ignore "dead" monsters */
				if (!m_ptr->r_idx) continue;
					
				r_ptr = &r_info[m_ptr->r_idx];
					
				if(r_ptr->flags1 & RF1_UNIQUE)
				{
					msg_format(_("%s． ", "%s. "), r_name + r_ptr->name);
				}
			}
			break;
		}
		case ACT_DETECT_XTRA:
		{
			detect_all(DETECT_RAD_DEFAULT);
			probing();
			(void)identify_fully();
			break;
		}

		case ACT_ID_FULL:
		{
			(void)identify_fully();
			break;
		}

		case ACT_ID_PLAIN:
		{
			if (!ident_spell()) return FALSE;
			break;
		}
		case ACT_RUNE_EXPLO:
		{
			explosive_rune();
			break;
		}
		case ACT_RUNE_PROT:
		{
			warding_glyph();
			break;
		}
		case ACT_SATIATE:
		{
			(void)set_food(PY_FOOD_MAX - 1);
			break;
		}
		case ACT_DEST_DOOR:
		{
			destroy_doors_touch();
			break;
		}
		case ACT_STONE_MUD:
		{
			if (!get_aim_dir(&dir)) return FALSE;
			wall_to_mud(dir);
			break;
		}
		case ACT_RECHARGE:
		{
			recharge(130);
			break;
		}
		case ACT_ALCHEMY:
		{
			(void)alchemy();
			break;
		}
		case ACT_PHASE_DOOR:
		{
			teleport_player(10);
			break;
		}
		case ACT_TELEPORT:
		{
			teleport_player(100);
			break;
		}
		case ACT_DIM_DOOR:
		{
			msg_print(_("次元の扉が開いた。目的地を選んで下さい。",
				"You open a dimensional gate. Choose a destination."));
			if (!dimension_door()) return FALSE;
			break;
		}
		case ACT_RECALL:
		{
			if (ironman_downward)
			{
				msg_print(_("それは一瞬輝いたがすぐ元に戻った。", "It glows and then fades."));
				return FALSE;
			}
			else
			{
				(void)recall_player(randint0(21) + 15);
			}
			break;
		}
		case ACT_ARKENSTONE:
		{
#ifdef JP
			msg_print("アーケン石はあなたの体力を奪った...");
			take_hit(damroll(3,8), "スラインのアーケン石");
			msg_print("アーケン石は赤く明るく光った！");
#else
			msg_print("The Arkenstone drains your vitality...");
			take_hit(damroll(3, 8), "the Arkenstone of Thrain");
			msg_print("The Arkenstone flashes bright red!");
#endif
			wiz_lite();
			if (get_check(_("帰還の力を使いますか？", "Activate recall? ")))
			{
				word_of_recall();
			}
			break;
		}
		case ACT_TELEKINESIS:
		{
			if (!get_aim_dir(&dir)) return FALSE;
			fetch(dir, 500, TRUE);
			break;
		}
		case ACT_BRAND_FIRE_BOLTS:
		{
			(void)brand_bolts();
			break;
		}
		case ACT_QUAKE:
		{
			earthquake(py, px, 10);
			break;
		}
		case ACT_DETECT_MONSTERS:
		{
			detect_monsters_invis(DETECT_RAD_DEFAULT);
			detect_monsters_normal(DETECT_RAD_DEFAULT);
			break;
		}

		default:
		{
#ifdef JP
			msg_format("Unknown activation effect: %d.", o_ptr->xtra2);
#else
			msg_format("Unknown activation effect: %d.", o_ptr->xtra2);
#endif
			return FALSE;
		}
	}

	/* Set activation timeout */
	if (act_ptr->timeout.constant >= 0)
	{
		o_ptr->timeout = act_ptr->timeout.constant;
		if (act_ptr->timeout.dice > 0)
		{
			o_ptr->timeout += randint1(act_ptr->timeout.dice);
		}
	}

	return TRUE;
}


void random_artifact_resistance(object_type * o_ptr, artifact_type *a_ptr)
{
	bool give_resistance = FALSE, give_power = FALSE;

	if (a_ptr->gen_flags & (TRG_XTRA_POWER)) give_power = TRUE;
	if (a_ptr->gen_flags & (TRG_XTRA_H_RES)) give_resistance = TRUE;
	if (a_ptr->gen_flags & (TRG_XTRA_RES_OR_POWER))
	{
		/* Give a resistance OR a power */
		if (one_in_(2)) give_resistance = TRUE;
		else give_power = TRUE;
	}

	if (give_power) one_ability(o_ptr);
	if (give_resistance) one_high_resistance(o_ptr);
}


/*
 * Create the artifact of the specified number
 */
void create_named_art(int a_idx, int y, int x)
{
	object_type forge;
	object_type *q_ptr;
	int i;

	artifact_type *a_ptr = &a_info[a_idx];

	/* Get local object */
	q_ptr = &forge;

	/* Ignore "empty" artifacts */
	if (!a_ptr->name) return;

	/* Acquire the "kind" index */
	i = lookup_kind(a_ptr->tval, a_ptr->sval);

	/* Oops */
	if (!i) return;

	/* Create the artifact */
	object_prep(q_ptr, i);

	/* Save the name */
	q_ptr->name1 = a_idx;

	/* Extract the fields */
	q_ptr->pval = a_ptr->pval;
	q_ptr->ac = a_ptr->ac;
	q_ptr->dd = a_ptr->dd;
	q_ptr->ds = a_ptr->ds;
	q_ptr->to_a = a_ptr->to_a;
	q_ptr->to_h = a_ptr->to_h;
	q_ptr->to_d = a_ptr->to_d;
	q_ptr->weight = a_ptr->weight;

	/* Hack -- acquire "cursed" flag */
	if (a_ptr->flags3 & TR3_CURSED) q_ptr->ident |= (IDENT_CURSED);

	random_artifact_resistance(q_ptr, a_ptr);

	/* Drop the artifact from heaven */
	(void)drop_near(q_ptr, -1, y, x);
}

/*
 *  named ego
 */
void create_named_ego(object_type *o_ptr)
{
	int max_type = (o_ptr->tval < TV_BOOTS ? 7 : 5);
	char buf[80];
	bool got_random_plus;
	bool has_blows;
	bool has_act;
	u32b o1, o2, o3;
	u32b f1, f2, f3;

	/* Paranoia */
	if (o_ptr->tval <= TV_CORPSE || o_ptr->tval >= TV_LITE) return;

	/* Prevent Ammo */
	if (o_ptr->tval <= TV_BOLT && o_ptr->tval >= TV_SHOT) return;

	/* Save old flags */
	object_flags(o_ptr, &o1, &o2, &o3);

	/* Does it has extra blows */
	has_blows = (o1 & (TR1_BLOWS)) ? TRUE : FALSE;

	/* Does it has activation */
	has_act = (o3 & (TR3_ACTIVATE)) ? TRUE : FALSE;

	/* Add one extra power */
	do
	{
		/* Do not use bias */
		artifact_bias = 0;

		got_random_plus = FALSE;

		/* Get one random power */
		switch (randint1(max_type))
		{
			case 1: case 2:
				random_plus(o_ptr);
				got_random_plus = TRUE;
				break;
			case 3: case 4:
				random_resistance(o_ptr, FALSE);
				break;	
			case 5:
				random_misc(o_ptr);
				break;
			case 6: case 7:
				random_slay(o_ptr);
				break;
			default:
#ifdef JP
				if (wizard) msg_print("銘付エゴの能力追加で不正が発生しました！");
#else
				if (wizard) msg_print("Switch error in create_named_ego!");
#endif
		}

		object_flags(o_ptr, &f1, &f2, &f3);
	}
	/* Check extra power is added truely */
	while ((o1 == f1) && (o2 == f2) && (o3 == f3));

	/* Tweak pval */
	if (got_random_plus)
	{
		/* If don't have pval, gain some pval */
		if (o_ptr->pval == 0)
		{
			o_ptr->pval = 2;
			while (o_ptr->pval < randint1(5) || one_in_(o_ptr->pval)) o_ptr->pval++;
		}

		/* Limit Extra Attacks */
		if (!has_blows && (o_ptr->art_flags1 & (TR1_BLOWS)))
		{
			o_ptr->pval = randint1(2);
			if (one_in_(3) && ((o_ptr->dd * (o_ptr->ds + 1)) < 15)) o_ptr->pval++;
		}

		if ((o_ptr->pval > 4) && (randint1(WEIRD_LUCK) != 1))
			o_ptr->pval = 4;
	}

	/* Add some bonuses to hit and dam (Weapons only)*/
	if (o_ptr->tval < TV_BOOTS)
	{
		o_ptr->to_h += (u16b)randint0(5);
		o_ptr->to_d += (u16b)randint0(5);
	}

	/* Add some bonuses to AC (Armors only) */
	if (o_ptr->tval >= TV_BOOTS)
	{
		o_ptr->to_a += (u16b)randint0(5);
	}

	/* Do not use bias */
	artifact_bias = 0;

	/* Get random activation */
	if (!has_act && one_in_(7))
	{
		o_ptr->xtra2 = 0;
		give_activation_power(o_ptr);
	}

	/* get random name */
	get_table_name(buf);

	/* Set name */
	o_ptr->ego_name = quark_add(buf);
}


bool create_nazgul_ring(object_type *o_ptr)
{
	char    new_name[1024];
	int     powers = randint1(3) + 1;

	/* paranoia */
	if ((o_ptr->tval != TV_RING) || (o_ptr->sval != SV_RING_WRAITH))
	{
		return FALSE;
	}

	switch(randint1(3))
	{
		case 1:
			artifact_bias = BIAS_NECROMANTIC;
			break;
		case 2:
			artifact_bias = BIAS_MAGE;
			break;
		default:
			artifact_bias = 0;
	}

	/* Nuke enchantments */
	o_ptr->name1 = 0;
	o_ptr->name2 = 0;

	/* Include obvious flags */
	o_ptr->art_flags1 |= k_info[o_ptr->k_idx].flags1;
	o_ptr->art_flags2 |= k_info[o_ptr->k_idx].flags2;
	o_ptr->art_flags3 |= k_info[o_ptr->k_idx].flags3;

	strcpy(new_name, "");

	while (one_in_(powers) || one_in_(7) || one_in_(10))
		powers++;

	if (one_in_(WEIRD_LUCK)) powers *= 2;

	/* Main loop */
	while (powers--)
	{
		switch (randint1(3))
		{
			case 1:
				random_plus(o_ptr);
				break;
			case 2:
				random_resistance(o_ptr, FALSE);
				break;
			case 3:
				random_misc(o_ptr);
				break;
			default:
#ifdef JP
				if (wizard) msg_print("Switch error in create_nazgul_ring!");
#else
				if (wizard) msg_print("Switch error in create_nazgul_ring!");
#endif
				powers++;
		}
	};

	/* Paranoia */
	o_ptr->art_flags1 &= ~(TR1_BLOWS);

	/* Just to be sure */
	o_ptr->art_flags1 |= (TR1_STR | TR1_INT | TR1_WIS | TR1_DEX | TR1_CON | TR1_CHR);
	o_ptr->art_flags3 |= (TR3_HIDE_TYPE | TR3_SHOW_MODS | TR3_SEE_INVIS);

	/* Get some pluses */
	if (one_in_(WEIRD_LUCK)) o_ptr->art_flags1 |= (TR1_SPEED);

	/* give it some plusses... */
	o_ptr->pval = (one_in_(5)) ? 2 : 1;
	o_ptr->to_a = 0;
	o_ptr->to_h = (s16b)randint0(3) + 10;
	o_ptr->to_d = o_ptr->to_h;

	/* Just to be sure */
	o_ptr->art_flags3 |= (TR3_IGNORE_ACID | TR3_IGNORE_ELEC | TR3_IGNORE_FIRE | TR3_IGNORE_COLD);

	/* Give one activation */
	o_ptr->xtra2 = 0;
	give_activation_power(o_ptr);

	/* Nazgul rings are always cursed */
	if (one_in_(3)) o_ptr->art_flags3 |= TR3_PERMA_CURSE;
	else o_ptr->art_flags3 |= TR3_HEAVY_CURSE;
	o_ptr->ident |= IDENT_CURSED;

	/* Give some cursed effects */
	o_ptr->art_flags3 |= TR3_DRAIN_EXP;
	if (one_in_(3)) o_ptr->art_flags3 |= TR3_AGGRAVATE;
	if (one_in_(5)) o_ptr->art_flags3 |= TR3_TY_CURSE;
	if (one_in_(10)) o_ptr->art_flags3 |= TR3_NO_TELE;

	/* Get wraith form */
	if (one_in_(666)) o_ptr->art_flags3 |= TR3_WRAITH;

	/* Save the inscription */
	get_table_bad_sindarin(new_name);
	o_ptr->art_name = quark_add(new_name);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP);

	return TRUE;
}
