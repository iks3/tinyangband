﻿/* File: racial.c */

/* Purpose: Racial powers (and mutations) */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"


typedef struct power_desc_type power_desc_type;

struct power_desc_type
{
	char name[40];
	int  level;
	int  cost;
	int  stat;
	int  fail;
	int  number;
	bool ignore_conf;
};


/*
 * Returns the chance to activate a racial power/mutation
 */
static int racial_chance(power_desc_type *pd_ptr)
{
	s16b min_level  = pd_ptr->level;
	int  difficulty = pd_ptr->fail;

	int i;
	int val;
	int sum = 0;
	int stat = p_ptr->stat_cur[pd_ptr->stat];

	/* No chance for success */
	if ((p_ptr->lev < min_level) || (p_ptr->confused && !(pd_ptr->ignore_conf)))
	{
		return (0);
	}

	if (difficulty == 0) return (100);

	/* Calculate difficulty */
	if (p_ptr->stun)
	{
		difficulty += p_ptr->stun;
	}
	else if (p_ptr->lev > min_level)
	{
		int lev_adj = ((p_ptr->lev - min_level) / 3);
		if (lev_adj > 10) lev_adj = 10;
		difficulty -= lev_adj;
	}

	if (difficulty < 5) difficulty = 5;

	/* We only need halfs of the difficulty */
	difficulty = difficulty / 2;

	for (i = 1; i <= stat; i++)
	{
		val = i - difficulty;
		if (val > 0)
			sum += (val <= difficulty) ? val : difficulty;
	}

	if (difficulty == 0)
		return (100);
	else
		return (((sum * 100) / difficulty) / stat);
}


static int racial_cost;

/*
 * Note: return value indicates that we have succesfully used the power
 * 1: Succeeded, 0: Cancelled, -1: Failed
 */
static int racial_aux(power_desc_type *pd_ptr)
{
	s16b min_level  = pd_ptr->level;
	int  use_stat   = pd_ptr->stat;
	int  difficulty = pd_ptr->fail;
	int  use_hp = 0;

	racial_cost = pd_ptr->cost;

	/* Not enough mana - use hp */
	if (p_ptr->csp < racial_cost) use_hp = racial_cost - p_ptr->csp;

	/* Power is not available yet */
	if (p_ptr->lev < min_level)
	{
		msg_format(_("この能力を使用するにはレベル %d に達していなければなりません。",
			"You need to attain level %d to use this power."),
			min_level);
		energy_use = 0;
		return 0;
	}

	/* Too confused */
	else if (p_ptr->confused && !(pd_ptr->ignore_conf))
	{
		msg_print(_("混乱していてその能力は使えません。", "You are too confused to use this power."));
		energy_use = 0;
		return 0;
	}

	/* Risk death? */
	else if (p_ptr->chp < use_hp)
	{
		if (!get_check(_("本当に今の衰弱した状態でこの能力を使いますか？",
			"Really use the power in your weakened state? ")))
		{
			energy_use = 0;
			return 0;
		}
	}

	/* Else attempt to do it! */
	if (difficulty)
	{
		if (p_ptr->stun)
		{
			difficulty += p_ptr->stun;
		}
		else if (p_ptr->lev > min_level)
		{
			int lev_adj = ((p_ptr->lev - min_level) / 3);
			if (lev_adj > 10) lev_adj = 10;
			difficulty -= lev_adj;
		}

		if (difficulty < 5) difficulty = 5;
	}

	/* take time */
	energy_use = 100;

	/* Success? */
	if (randint1(p_ptr->stat_cur[use_stat]) >=
	    ((difficulty / 2) + randint1(difficulty / 2)))
	{
		return 1;
	}

	if (flush_failure) flush();
	msg_print(_("充分に集中できなかった。", "You've failed to concentrate hard enough."));

	return -1;
}

/**** The monster bashing code. -LM- ****/
static bool do_cmd_shield_bash(void)
{
	int bash_chance, bash_quality, bash_dam;
	int y, x, dir = 0;
	bool fear;
	char m_name[80];
	cave_type *c_ptr;
	object_type *o_ptr;
	monster_type *m_ptr;
	monster_race *r_ptr;

	o_ptr = &inventory[INVEN_ARM];

	/* No shield on arm, no bash.  */
	if (!o_ptr->k_idx || is_two_handed())
	{
#ifdef JP
		msg_print("盾を装備していない！");
#else
		msg_print("You weild no shields !");
#endif
		return (FALSE);
	}

	/* Only works on adjacent monsters */
	if (!get_rep_dir(&dir)) return (FALSE);   /* was get_aim_dir */
	y = py + ddy[dir];
	x = px + ddx[dir];
	c_ptr = &cave[y][x];

	if (!c_ptr->m_idx)
	{
#ifdef JP
		msg_print("そこにはなにもいません。");
#else
		msg_print("Nobody is there.");
#endif
		return (FALSE);
	}

	m_ptr = &m_list[c_ptr->m_idx];
	r_ptr = &r_info[m_ptr->r_idx];

	/* Extract monster name (or "it") */
	monster_desc(m_name, m_ptr, 0);

	/* Auto-Recall if possible and visible */
	if (m_ptr->ml) monster_race_track(m_ptr->r_idx);

	/* Track a new monster */
	if (m_ptr->ml) health_track(c_ptr->m_idx);

	/* Bashing chance depends on melee Skill, Dex, and a class level bonus. */
	bash_chance = p_ptr->skill_thn + (p_ptr->to_h * BTH_PLUS_ADJ);

	/* Try to get in a shield bash. */
	if (randint0(bash_chance) >= (r_ptr->ac * 3 / 4))
	{
#ifdef JP
		msg_print("シールドでモンスターに体当りした！");
#else
		msg_print("You get in a shield bash!");
#endif

		/* Calculate attack quality, a mix of momentum and accuracy. */
		bash_quality = p_ptr->skill_thn + (p_ptr->wt / 8) +
			(p_ptr->total_weight / 80) + (o_ptr->weight / 3);

		/* Calculate damage.  Big shields are deadly. */
		bash_dam = damroll(o_ptr->dd, o_ptr->ds);

		/* Multiply by quality and experience factors */
		bash_dam *= bash_quality / 30;

		/* Add damage bonus */
		bash_dam += p_ptr->to_d;

		/* Encourage the player to keep wearing that heavy shield. */
		if (randint1(bash_dam) > 30 + randint1(bash_dam / 2))
#ifdef JP
			msg_print("バーン！");
#else
			msg_print("WHAMM!");
#endif

		/* Complex message */
		if (wizard)
		{
#ifdef JP
			msg_format("%d/%d のダメージを与えた。", bash_dam, m_ptr->hp);
#else
			msg_format("You do %d (out of %d) damage.", bash_dam, m_ptr->hp);
#endif
		}

		/* Damage, check for fear and death. */
		if (mon_take_hit(c_ptr->m_idx, bash_dam, &fear, NULL))
		{
			/* Fight's over. */
			return (TRUE);
		}

		if (!m_ptr->stunned && !(r_ptr->flags3 & RF3_NO_STUN))
		{
			/* Stunning. Must be stunned. */
#ifdef JP
			msg_format("%^sはフラフラになった。", m_name);
#else
			msg_format("%^s is stunned.", m_name);
#endif
			m_ptr->stunned += (u16b)(1 + randint1(bash_quality / 40));
		}

		/* Confusion. */
		if (bash_quality + p_ptr->lev > randint1(200 + r_ptr->level * 4) &&
			!(r_ptr->flags3 & (RF3_NO_CONF)))
		{
#ifdef JP
			msg_format("%^sは混乱したようだ。", m_name);
#else
			msg_format("%^s appears confused.", m_name);
#endif
			m_ptr->confused += (byte)(randint0(p_ptr->lev / 5) + 4);
		}

		/* Hack -- delay fear messages */
		if (fear && m_ptr->ml)
		{
			/* Sound */
			sound(SOUND_FLEE);

			/* Message */
#ifdef JP
			msg_format("%^sは恐怖して逃げ出した！", m_name);
#else
			msg_format("%^s flees in terror!", m_name);
#endif
		}
	}
	/* Player misses */
	else
	{
		/* Sound */
		sound(SOUND_MISS);

		/* Message */
#ifdef JP
		msg_format("ミス！ %sにかわされた。", m_name);
#else
		msg_format("You miss %s.", m_name);
#endif
	}

	/* Monster is not dead */
	return (TRUE);
}


/* Mei-kyo-Shi-Sui */
static bool do_cmd_restore_mana(void)
{
	if (total_friends)
	{
#ifdef JP
		msg_print("今はペットを操ることに集中していないと。");
#else
		msg_print("You need concentration on the pets now.");
#endif
		return FALSE;
	}

#ifdef JP
	msg_print("少し頭がハッキリした。");
#else
	msg_print("You feel your head clear a little.");
#endif
	p_ptr->csp += (3 + p_ptr->lev/15);
	if (p_ptr->csp >= p_ptr->msp)
	{
		p_ptr->csp = p_ptr->msp;
		p_ptr->csp_frac = 0;
	}

	return TRUE;
}

static bool item_tester_hook_bows_and_ammos(const object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
		case TV_BOW:
		{
			return (bool) (!object_known_p(o_ptr) ||
				(!(o_ptr->ident & IDENT_MENTAL) && (p_ptr->lev >= 30)));
		}
	}
	return (FALSE);
}

static bool do_cmd_identify_bows_and_ammos(void)
{
	int item;
	cptr q, s;
	object_type *o_ptr;
	char o_name[MAX_NLEN];
	bool old_known;

	item_tester_hook = item_tester_hook_bows_and_ammos;

	/* Get an item */
#ifdef JP
	q = "どのアイテムを鑑定しますか? ";
	s = "鑑定できるアイテムがない。";
#else
	q = "Identify which item? ";
	s = "You have nothing to identify.";
#endif

	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return (FALSE);

	/* Access the item (if in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Identify it */
	old_known = identify_item(o_ptr);

	/* Mark the item as fully known */
	if (p_ptr->lev >= 30) o_ptr->ident |= (IDENT_MENTAL);

	/* Handle stuff */
	handle_stuff();

	/* Description */
	object_desc(o_name, o_ptr, 0);

	/* Possibly play a sound depending on object quality. */
    if (o_ptr->pval < 0)
    {
                /* This is a bad item. */
                sound(SOUND_IDENT_BAD);
    }
    else if (o_ptr->name1 != 0)
    {
                /* We have a good artifact. */
                sound(SOUND_IDENT_ART);
    }
    else if (o_ptr->name2 != 0)
    {
                /* We have a good ego item. */
                sound(SOUND_IDENT_EGO);
    }

	/* Describe */
	if (item >= INVEN_WIELD)
	{
#ifdef JP
		msg_format("%^s: %s(%c)。",
#else
		msg_format("%^s: %s (%c).",
#endif
			   describe_use(item), o_name, index_to_label(item));
	}
	else if (item >= 0)
	{
#ifdef JP
		msg_format("ザック中: %s(%c)。",
#else
		msg_format("In your pack: %s (%c).",
#endif
			   o_name, index_to_label(item));
	}
	else
	{
#ifdef JP
		msg_format("床上: %s。",
#else
		msg_format("On the ground: %s.",
#endif
			   o_name);
	}

	if (p_ptr->lev >= 30) identify_fully_aux(o_ptr, TRUE);
	else identify_item(o_ptr);

	/* Auto-inscription/destroy */
	autopick_alter_item(item, (bool)(destroy_identify && !old_known));

	return (TRUE);
}

static bool item_tester_hook_arrow(const object_type *o_ptr)
{
	if (o_ptr->tval == TV_ARROW) return (TRUE);

	return (FALSE);
}

static bool do_cmd_arrow_rain(void)
{
	int x, y, tx, ty;
	int ny, nx;
	int tdam, tmul;
	int i, dir = 0;
	int item;
	int n = 5 + randint1(3);
	cptr q, s;
	object_type *o_ptr, *q_ptr, *j_ptr;
	object_type forge;
	cave_type *c_ptr;

	o_ptr = &inventory[INVEN_BOW];
	item_tester_hook = item_tester_hook_arrow;

	if (!((o_ptr->tval == TV_BOW) &&
		((o_ptr->sval == SV_LONG_BOW) || (o_ptr->sval == SV_SHORT_BOW))))
	{
#ifdef JP
		msg_print("弓を装備していない！");
#else
		msg_print("You wield no bows!");
#endif
		return (FALSE);
	}

	/* Get an item */
#ifdef JP
	q = "どれを撃ちますか? ";
	s = "発射されるアイテムがありません。";
#else
	q = "Fire which item? ";
	s = "You have nothing to fire.";
#endif

	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return (FALSE);

	/* Access the item (if in the pack) */
	if (item >= 0)
	{
		q_ptr = &inventory[item];
	}
	else
	{
		q_ptr = &o_list[0 - item];
	}

	if (!get_aim_dir(&dir)) return FALSE;

	tdam = damroll(q_ptr->dd, q_ptr->ds) + o_ptr->to_d + q_ptr->to_d;
	tmul = bow_tmul(o_ptr->sval);

	/* Get extra "power" from "extra might" */
	if (p_ptr->xtra_might) tmul++;

	tmul = tmul * (100 + (int)(adj_str_td[p_ptr->stat_ind[A_STR]]) - 128);

	/* Boost the damage */
	tdam *= tmul;
	tdam /= 100;

	/* Use the given direction */
	tx = px + 99 * ddx[dir];
	ty = py + 99 * ddy[dir];

	/* Hack -- Use an actual "target" */
	if ((dir == 5) && target_okay())
	{
		tx = target_col;
		ty = target_row;
	}

	x = px;
	y = py;

	while (1)
	{
		/* Hack -- Stop at the target */
		if ((y == ty) && (x == tx)) break;

		ny = y;
		nx = x;
		mmove2(&ny, &nx, py, px, ty, tx);

		/* Stop at maximum range */
		if (MAX_RANGE <= distance(py, px, ny, nx)) break;

		/* Stopped by walls/doors */
		c_ptr = &cave[ny][nx];
		if (!cave_floor_grid(c_ptr) && !c_ptr->m_idx) break;

		/* Save the new location */
		x = nx;
		y = ny;

		/* Stopped by monsters */
		if ((dir != 5) && cave[ny][nx].m_idx != 0) break;
	}

	tx = x;
	ty = y;

	j_ptr = &forge;
	object_copy(j_ptr, q_ptr);
	j_ptr->number = 1;
	
	for (i = 0; i < n; i++)
	{
		if (q_ptr->number < 1) break;

		/* Costs an arrow */
		q_ptr->number--;

		project(0, 0, y, x, tdam, GF_ARROW, (PROJECT_JUMP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL));
		drop_near(j_ptr, 20, y, x);

		do 
		{
			x = tx - 1 + randint0(3);
			y = ty - 1 + randint0(3);
		}
		while (!in_bounds(y, x));
	}

	return (TRUE);
}


static bool cmd_racial_power_aux(s32b command)
{
	s16b        plev = p_ptr->lev;
	int         dir = 0;

	if (command < -9)
	{
		switch (p_ptr->valar_patron)
		{
		case VAR_MANWE:
			{
				int n = 10 + randint1(10);
				(void)set_tim_sh_elec(n);
				(void)set_tim_sh_cold(n);
				(void)set_tim_brand(n, (TR1_BRAND_ELEC | TR1_BRAND_COLD));
			}
			break;
		case VAR_ULMO:
			(void)set_oppose_acid(15 + randint1(15));
			break;
		case VAR_AULE:
			if (command == -10)
			{
				(void)set_tim_might(30 + randint1(30));
			}
			else if (command == -11)
			{
				return coat_equip();
			}
			break;
		case VAR_OROME:
			(void)set_tim_radar(40 + randint1(40));
			break;
		case VAR_MANDOS:
			(void)dispel_undead(plev * 2);
			break;
		case VAR_IRMO:
			(void)sleep_monsters();
			break;
		case VAR_TULKAS:
			if (command == -10)
			{
				int y, x;
				cave_type       *c_ptr;
				monster_type    *m_ptr;

				for (dir = 1; dir <= 9; dir++)
				{
					y = py + ddy[dir];
					x = px + ddx[dir];
					c_ptr = &cave[y][x];

					/* Get the monster */
					m_ptr = &m_list[c_ptr->m_idx];

					/* Hack -- attack monsters */
					if (c_ptr->m_idx && (m_ptr->ml || cave_floor_bold(y, x)))
						py_attack(y, x);
				}
			}
			else if (command == -11)
			{
				(void)set_afraid(0);
				(void)set_hero(15 + randint1(15));
			}
			break;
		case VAR_VARDA:
#ifdef JP
			msg_print("ア エルベレス ギルソニエル！");
#else
			msg_print("A Elbereth Gilthoniel!!");
#endif
			(void)set_afraid(0);
			if (p_ptr->protevil)
			{
#ifdef JP
				msg_print("あなたはすでに守られている。");
#else
				msg_print("You are already protected.");
#endif
			}
			else
			{
				(void)set_protevil(8 + randint1(8));
			}
			break;
		case VAR_YAVANNA:
			if (command == -10)
			{
				for (dir = 1; dir <= 9; dir++)
				{
					int y = py + ddy[dir];
					int x = px + ddx[dir];

					if (cave_floor_bold(y, x) && !cave_perma_bold(y, x))
					{
						cave[y][x].feat = FEAT_GRASS;
						cave[y][x].info |= (CAVE_GLOW);
						note_spot(y, x);
						lite_spot(y, x);
					}
				}
			}
			else if (command == -11)
			{
				(void)project_hack(GF_LITE_WEAK, plev * 2);
				(void)lite_area(0, 0);
			}
			break;
		case VAR_NIENNA:
			(void)project_hack(GF_DISP_ALL, plev);
			break;
		case VAR_ESTE:
			(void)set_poisoned(0);
			(void)set_cut(0);
			(void)set_blind(0);
			(void)set_confused(0);
			(void)set_image(0);
			(void)set_stun(0);
			break;
		case VAR_VAIRE:
			if (!get_aim_dir(&dir)) return FALSE;
			(void)charm_monster(dir, plev * 3 / 2);
			break;
		case VAR_VANA:
			(void)restore_level();
			break;
		case VAR_NESSA:
			project_length = 5;
			if (!get_aim_dir(&dir)) return FALSE;
			project_hook(GF_JUMP, dir, 0, PROJECT_STOP);
			project_length = 0;
			break;
		default:
			break;
		}
	}
	else if (command <= -3)
	{
		switch (p_ptr->pclass)
		{
			case CLASS_WARRIOR:
				if (!do_cmd_shield_bash()) return FALSE;
				break;
			case CLASS_PALADIN:
				psychometry();
				break;
			case CLASS_WARRIOR_MAGE:
				project_length = 2;
				if (!get_aim_dir(&dir)) return FALSE;
				project_hook(GF_DIST_ATTACK, dir, 0, (PROJECT_STOP | PROJECT_KILL));
				project_length = 0;
				break;
			case CLASS_MAGE:
			case CLASS_PRIEST:
				if (!do_cmd_restore_mana()) return FALSE;
				break;
			case CLASS_ARCHER:
				if (command == -3)
				{
					if (!do_cmd_identify_bows_and_ammos()) return FALSE;
				}
				else if (command == -4)
				{
					if (!do_cmd_arrow_rain()) return FALSE;
				}
				break;
		}
	}
	else switch (p_ptr->prace)
	{
		case RACE_DWARF:
#ifdef JP
			msg_print("周囲を調べた。");
#else
			msg_print("You examine your surroundings.");
#endif
			(void)detect_traps(DETECT_RAD_DEFAULT);
			(void)detect_doors(DETECT_RAD_DEFAULT);
			(void)detect_stairs(DETECT_RAD_DEFAULT);
			break;

		case RACE_HOBBIT:
			{
				object_type *q_ptr;
				object_type forge;

				/* Get local object */
				q_ptr = &forge;

				/* Create the food ration */
				object_prep(q_ptr, lookup_kind(TV_FOOD, SV_FOOD_RATION));

				/* Drop the object from heaven */
				(void)drop_near(q_ptr, -1, py, px);
#ifdef JP
				msg_print("食事を料理して作った。");
#else
				msg_print("You cook some food.");
#endif
			}
			break;

		case RACE_HALF_ORC:
#ifdef JP
			msg_print("勇気を出した。");
#else
			msg_print("You play tough.");
#endif
			(void)set_afraid(0);
			break;

		case RACE_BARBARIAN:
#ifdef JP
			msg_print("うぉぉおお！");
#else
			msg_print("Raaagh!");
#endif
			(void)set_afraid(0);
			(void)set_shero(p_ptr->shero + 10 + randint1(plev));
			(void)hp_player(30);
			break;
#if 0
		case RACE_VAMPIRE:
			{
				int y, x, dummy;
				cave_type *c_ptr;

				/* Only works on adjacent monsters */
				if (!get_rep_dir(&dir)) return FALSE;   /* was get_aim_dir */
				y = py + ddy[dir];
				x = px + ddx[dir];
				c_ptr = &cave[y][x];

				if (!c_ptr->m_idx)
				{
#ifdef JP
					msg_print("何もない場所に噛みついた！");
#else
					msg_print("You bite into thin air!");
#endif
					break;
				}

#ifdef JP
				msg_print("あなたはニヤリとして牙をむいた...");
#else
				msg_print("You grin and bare your fangs...");
#endif
				dummy = plev + randint1(plev) * MAX(1, plev / 10);   /* Dmg */
				if (drain_life(dir, dummy))
				{
					if (p_ptr->food < PY_FOOD_FULL)
						/* No heal if we are "full" */
						(void)hp_player(dummy);
					else
#ifdef JP
						msg_print("あなたは空腹ではありません。");
#else
						msg_print("You were not hungry.");
#endif

						/* Gain nutritional sustenance: 150/hp drained */
						/* A Food ration gives 5000 food points (by contrast) */
						/* Don't ever get more than "Full" this way */
						/* But if we ARE Gorged,  it won't cure us */
						dummy = p_ptr->food + MIN(5000, 100 * dummy);
					if (p_ptr->food < PY_FOOD_MAX)   /* Not gorged already */
						(void)set_food(dummy >= PY_FOOD_MAX ? PY_FOOD_MAX - 1 : dummy);
				}
				else
#ifdef JP
					msg_print("げぇ。ひどい味だ。");
#else
					msg_print("Yechh. That tastes foul.");
#endif
			}
			break;
#endif

		default:
#ifdef JP
			msg_print("この種族は特殊な能力を持っていません。");
#else
			msg_print("This race has no bonus power.");
#endif
			energy_use = 0;
	}

	/* Redraw mana and hp */
	p_ptr->redraw |= (PR_HP | PR_MANA);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER | PW_SPELL);

	return TRUE;
}


static power_desc_type power_desc_entry(cptr name, int level, int cost, int stat, int fail, int number)
{
	power_desc_type power_desc;
	strcpy(power_desc.name, name);
	power_desc.level = level;
	power_desc.cost = cost;
	power_desc.stat = stat;
	power_desc.fail = fail;
	power_desc.number = number;
	power_desc.ignore_conf = FALSE;

	return (power_desc);
}


/*
 * Allow user to choose a power (racial / mutation) to activate
 */
void do_cmd_racial_power(void)
{
	power_desc_type power_desc[36];
	int  num, ask, i = 0;
	bool flag, redraw, cast = FALSE;
	bool warrior = ((p_ptr->pclass == CLASS_WARRIOR) ? TRUE : FALSE);
	char choice;
	char out_val[160];

	for (num = 0; num < 36; num++)
	{
		strcpy(power_desc[num].name, "");
		power_desc[num].number = 0;
	}

	num = 0;

	/* Can use Este's power when confusion */
	if (p_ptr->confused && (p_ptr->valar_patron != VAR_ESTE))
	{
		msg_print(_("混乱していて特殊能力を使えません！", "You are too confused to use any powers!"));
		return;
	}

	switch (p_ptr->pclass)
	{
		case CLASS_WARRIOR:
			power_desc[num++] = power_desc_entry(
				_("シールド・バッシュ", "Shield bashing"),
				1, 15, A_DEX, 0, -3);
			break;
		case CLASS_PALADIN:
			power_desc[num++] = power_desc_entry(
				_("呪い識別", "Identify curse"),
				5, 10, A_WIS, 9, -3);
			break;
		case CLASS_WARRIOR_MAGE:
			power_desc[num++] = power_desc_entry(
				_("遠方攻撃", "Distance attack"),
				1, 15, A_DEX, 0, -3);
			break;
		case CLASS_MAGE:
		case CLASS_PRIEST:
			power_desc[num++] = power_desc_entry(
				_("明鏡止水", "Clear mind"),
				15, 0, m_info[p_ptr->pclass].spell_stat, 10, -3);
			break;
		case CLASS_ARCHER:
			power_desc[num++] = power_desc_entry(
				_("弓/矢の鑑定", "Identify bows/ammos"),
				10, 15, A_INT, 15, -3);
			power_desc[num++] = power_desc_entry(
				_("矢の雨", "Arrow rain"),
				5, 15, A_DEX, 15, -4);
			break;
		default:
			strcpy(power_desc[num].name, _("(なし)", "(none)"));
			break;
	}

	switch (p_ptr->prace)
	{
		case RACE_DWARF:
			power_desc[num++] = power_desc_entry(
				_("ドアと罠 感知", "Detect doors+traps"),
				5, 5, A_WIS, 12, -1);
			break;
		case RACE_HOBBIT:
			power_desc[num++] = power_desc_entry(
				_("食糧生成", "Create food"),
				15, 10, A_INT, 10, -1);
			break;
		case RACE_HALF_ORC:
			power_desc[num++] = power_desc_entry(
				_("恐怖除去", "Remove fear"),
				3, 5, A_WIS, (warrior ? 5 : 10), -1);
			break;
		case RACE_BARBARIAN:
			power_desc[num++] = power_desc_entry(
				_("肉体野獣化", "Berserk"),
				8, 10, A_WIS, (warrior ? 6 : 12), -1);
			break;
#if 0
		case RACE_VAMPIRE:
			power_desc[num++] = power_desc_entry(
				_("生命力吸収", "Drain life"),
				2, 1 + (lvl / 3), A_CON, 9, -1);
			break;
#endif
		default:
			break;
	}

	switch(p_ptr->valar_patron)
	{
	case VAR_MANWE:
			power_desc[num++] = power_desc_entry(
				_("風のオーラ", "Cloak of Hurricane"),
				7, 10, A_WIS, 15, -10);
			break;
	case VAR_ULMO:
			power_desc[num++] = power_desc_entry(
				_("酸への耐性", "Acid resistance"),
				10, 15, A_INT, 15, -10);
			break;
	case VAR_AULE:
			power_desc[num++] = power_desc_entry(
				_("腕力強化", "Extra might"),
				5, 10, A_STR, (warrior ? 6 : 12), -10);
			power_desc[num++] = power_desc_entry(
				_("腐食防止", "Rust proofing"),
				20, 30, A_STR, (warrior ? 12 : 24), -11);
			break;
	case VAR_OROME:
			power_desc[num++] = power_desc_entry(
				_("エルフの目", "Elven eye"),
				3, 5, A_DEX, 15, -10);
			break;
	case VAR_MANDOS:
			power_desc[num++] = power_desc_entry(
				_("死者退散", "Dispel undead"),
				10, 10, A_WIS, 10, -10);
			break;
	case VAR_IRMO:
			power_desc[num++] = power_desc_entry(
				_("周辺スリープ", "Sleep monsters"),
				8, 10, A_INT, 15, -10);
			break;
	case VAR_TULKAS:
			power_desc[num++] = power_desc_entry(
				_("全方位攻撃", "Whirlwind Attack"),
				1, 15, A_STR, (warrior ? 9 : 15), -10);
			power_desc[num++] = power_desc_entry(
				_("士気高揚", "Heroism"),
				7, 10, A_WIS, 12, -11);
			break;
	case VAR_VARDA:
			power_desc[num++] = power_desc_entry(
				_("対邪悪結界", "Protection from evil"),
				7, 15, A_WIS, 15, -10);
			break;
	case VAR_YAVANNA:
			power_desc[num++] = power_desc_entry(
				_("草地生成", "Create grass"),
				1, 2, A_DEX, 6, -10);
			power_desc[num++] = power_desc_entry(
				_("フラッシュ・ライト", "Flash Light"),
				5, 5, A_CHR, 6, -1);
			break;
	case VAR_NIENNA:
			power_desc[num++] = power_desc_entry(
				_("哀しみの歌", "Sorrowful song"),
				7, 5, A_CHR, 9, -10);
			break;
	case VAR_ESTE:
			power_desc[num] = power_desc_entry(
				_("癒しの手", "Curing"),
				1, 5, A_WIS, 6, -10);
			power_desc[num++].ignore_conf = TRUE;
			break;
	case VAR_VAIRE:
			power_desc[num++] = power_desc_entry(
				_("魅惑の目", "Charming"),
				10, 10, A_CHR, 12, -10);
			break;
	case VAR_VANA:
			power_desc[num++] = power_desc_entry(
				_("経験値復活", "Restore level"),
				15, 10, A_WIS, 9, -10);
			break;
	case VAR_NESSA:
			power_desc[num++] = power_desc_entry(
				_("跳躍", "Stepping"),
				1, 3, A_DEX, 6, -10);
			break;
	default:
		break;
	}

	if (num == 0)
	{
		msg_print(_("使える特殊能力が何もありません。", "You have no powers to activate."));
		energy_use = 0;
		return;
	}

	/* Nothing chosen yet */
	flag = FALSE;

	/* No redraw yet */
	redraw = FALSE;

	/* Build a prompt */
#ifdef JP
	(void)strnfmt(out_val, 78, "(特殊能力 %c-%c, *'で一覧, ESCで中断) どの特殊能力を使いますか？",
#else
	(void)strnfmt(out_val, 78, "(Powers %c-%c, *=List, ESC=exit) Use which power? ",
#endif
		I2A(0), (num <= 26) ? I2A(num - 1) : '0' + num - 27);

	/* Repeat previous command or get new choice */
	if (!repeat_pull(&i) ||  i< 0 || i >= num)
	{
		/* Get a spell from the user */
		choice = always_show_list ? ESCAPE : 1;
		while (!flag)
		{
			if( choice==ESCAPE ) choice = ' '; 
			else if( !get_com(out_val, &choice) )break; 

			/* Request redraw */
			if ((choice == ' ') || (choice == '*') || (choice == '?'))
			{
				/* Show the list */
				if (!redraw)
				{
					byte y = 1, x = 0;
					int ctr = 0;
					char dummy[80];
					char letter;
					int x1, y1;

					strcpy(dummy, "");

						/* Show list */
						redraw = TRUE;

						/* Save the screen */
						screen_save();

						/* Print header(s) */
						if (num < 17)
#ifdef JP
							prt("                            Lv   MP 失率", y++, x);
#else
							prt("                            Lv Cost Fail", y++, x);
#endif
						else
#ifdef JP
							prt("                            Lv   MP 失率                            Lv   MP 失率", y++, x);
#else
							prt("                            Lv Cost Fail                            Lv Cost Fail", y++, x);
#endif

						/* Print list */
						while (ctr < num)
						{
							/* letter/number for power selection */
							if (ctr < 26)
								letter = I2A(ctr);
							else
								letter = '0' + ctr - 26;
							x1 = ((ctr < 17) ? x : x + 40);
							y1 = ((ctr < 17) ? y + ctr : y + ctr - 17);

							sprintf(dummy, " %c) %-23.23s %2d %4d %3d%%", letter, power_desc[ctr].name, power_desc[ctr].level, power_desc[ctr].cost, 100 - racial_chance(&power_desc[ctr]));
							prt(dummy, y1, x1);
							ctr++;
						}
					}

					/* Hide the list */
					else
					{
						/* Hide list */
						redraw = FALSE;

						/* Restore the screen */
						screen_load();
					}

					/* Redo asking */
					continue;
				}

				if (choice == '\r' && num == 1)
				{
					choice = 'a';
				}

				if (isalpha(choice))
				{
					/* Note verify */
					ask = (isupper(choice));

					/* Lowercase */
					if (ask) choice = tolower(choice);

					/* Extract request */
					i = (islower(choice) ? A2I(choice) : -1);
				}
				else
				{
					ask = FALSE; /* Can't uppercase digits */

					i = choice - '0' + 26;
				}

				/* Totally Illegal */
				if ((i < 0) || (i >= num))
				{
					bell();
					continue;
				}

				/* Verify it */
				if (ask)
				{
					char tmp_val[160];

					/* Prompt */
#ifdef JP
					(void)strnfmt(tmp_val, 78, "%sを使いますか？ ", power_desc[i].name);
#else
					(void)strnfmt(tmp_val, 78, "Use %s? ", power_desc[i].name);
#endif

					/* Belay that order */
					if (!get_check(tmp_val)) continue;
				}

				/* Stop the loop */
				flag = TRUE;
			}

			/* Restore the screen */
			if (redraw) screen_load();

			/* Abort if needed */
			if (!flag)
			{
				energy_use = 0;
				return;
			}

		/* Remember the command for repeating */
		repeat_push(i);
	}

	switch (racial_aux(&power_desc[i]))
	{
	case 1:
		if (power_desc[i].number < 0)
			cast = cmd_racial_power_aux(power_desc[i].number);
		else
			cast = mutation_power_aux(power_desc[i].number);
		break;
	case 0:
		cast = FALSE;
		break;
	case -1:
		cast = TRUE;
		break;
	}

	if (cast)
	{
		if (racial_cost)
		{
			int actual_racial_cost = racial_cost / 2 + randint1(racial_cost / 2);

			/* If mana is not enough, player consumes hit point! */
			if (p_ptr->csp < actual_racial_cost)
			{
				actual_racial_cost -= p_ptr->csp;
				p_ptr->csp = 0;
#ifdef JP
				take_hit(actual_racial_cost, "過度の集中");
#else
				take_hit(actual_racial_cost, "concentrating too hard");
#endif
			}
			else p_ptr->csp -= actual_racial_cost;

			/* Redraw mana and hp */
			p_ptr->redraw |= (PR_HP | PR_MANA);

			/* Window stuff */
			p_ptr->window |= (PW_PLAYER | PW_SPELL);
		}
	}
	else energy_use = 0;

	/* Success */
	return;
}
