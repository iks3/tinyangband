/* File: spells3.c */

/* Purpose: Spell code (part 3) */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"

/* Maximum number of tries for teleporting */
#define MAX_TRIES 100

/* 1/x chance of reducing stats (for elemental attacks) */
#define HURT_CHANCE 16


/*
 * Teleport a monster, normally up to "dis" grids away.
 *
 * Attempt to move the monster at least "dis/2" grids away.
 *
 * But allow variation to prevent infinite loops.
 */
bool teleport_away(int m_idx, int dis)
{
	int ny=0, nx=0, oy, ox, d, i, min;
	int tries = 0;

	bool look = TRUE;

	monster_type *m_ptr = &m_list[m_idx];


	/* Paranoia */
	if (!m_ptr->r_idx) return (FALSE);

	/* Save the old location */
	oy = m_ptr->fy;
	ox = m_ptr->fx;

	/* Minimum distance */
	min = dis / 2;

	/* Look until done */
	while (look)
	{
		tries++;

		/* Verify max distance */
		if (dis > 200) dis = 200;

		/* Try several locations */
		for (i = 0; i < 500; i++)
		{
			/* Pick a (possibly illegal) location */
			while (1)
			{
				ny = rand_spread(oy, dis);
				nx = rand_spread(ox, dis);
				d = distance(oy, ox, ny, nx);
				if ((d >= min) && (d <= dis)) break;
			}

			/* Ignore illegal locations */
			if (!in_bounds(ny, nx)) continue;

			/* Require "empty" floor space */
			if (!cave_empty_bold(ny, nx)) continue;

			/* Hack -- no teleport onto glyph of warding */
			if (cave[ny][nx].feat == FEAT_GLYPH) continue;
			if (cave[ny][nx].feat == FEAT_MINOR_GLYPH) continue;

			/* ...nor onto the Pattern */
			if ((cave[ny][nx].feat >= FEAT_PATTERN_START) &&
			    (cave[ny][nx].feat <= FEAT_PATTERN_XTRA2)) continue;

			/* No teleporting into vaults and such */
			if (!(p_ptr->inside_quest || p_ptr->inside_arena))
				if (cave[ny][nx].info & CAVE_ICKY) continue;

			/* This grid looks good */
			look = FALSE;

			/* Stop looking */
			break;
		}

		/* Increase the maximum distance */
		dis = dis * 2;

		/* Decrease the minimum distance */
		min = min / 2;

		/* Stop after MAX_TRIES tries */
		if (tries > MAX_TRIES) return (FALSE);
	}

	/* Sound */
	sound(SOUND_TPOTHER);

	/* Update the new location */
	cave[ny][nx].m_idx = m_idx;

	/* Update the old location */
	cave[oy][ox].m_idx = 0;

	/* Move the monster */
	m_ptr->fy = ny;
	m_ptr->fx = nx;

	/* Update the monster (new location) */
	update_mon(m_idx, TRUE);

	/* Redraw the old grid */
	lite_spot(oy, ox);

	/* Redraw the new grid */
	lite_spot(ny, nx);

	if (r_info[m_ptr->r_idx].flags7 & (RF7_HAS_LITE_1 | RF7_SELF_LITE_1 | RF7_HAS_LITE_2 | RF7_SELF_LITE_2))
		p_ptr->update |= (PU_MON_LITE);

	return (TRUE);
}



/*
 * Teleport monster next to the player
 */
void teleport_to_player(int m_idx, int power)
{
	int ny, nx, oy, ox, d, i, min;
	int attempts = 500;
	int dis = 2;
	bool look = TRUE;
	monster_type *m_ptr = &m_list[m_idx];


	/* Paranoia */
	if (!m_ptr->r_idx) return;

	/* "Skill" test */
	/* if (randint1(100) > r_info[m_ptr->r_idx].level) return; */
	if (randint1(100) > power) return;

	/* Initialize */
	ny = m_ptr->fy;
	nx = m_ptr->fx;

	/* Save the old location */
	oy = m_ptr->fy;
	ox = m_ptr->fx;

	/* Minimum distance */
	min = dis / 2;

	/* Look until done */
	while (look && --attempts)
	{
		/* Verify max distance */
		if (dis > 200) dis = 200;

		/* Try several locations */
		for (i = 0; i < 500; i++)
		{
			/* Pick a (possibly illegal) location */
			while (1)
			{
				ny = rand_spread(py, dis);
				nx = rand_spread(px, dis);
				d = distance(py, px, ny, nx);
				if ((d >= min) && (d <= dis)) break;
			}

			/* Ignore illegal locations */
			if (!in_bounds(ny, nx)) continue;

			/* Require "empty" floor space */
			if (!cave_empty_bold(ny, nx)) continue;

			/* Hack -- no teleport onto glyph of warding */
			if (cave[ny][nx].feat == FEAT_GLYPH) continue;
			if (cave[ny][nx].feat == FEAT_MINOR_GLYPH) continue;

			/* ...nor onto the Pattern */
			if ((cave[ny][nx].feat >= FEAT_PATTERN_START) &&
			    (cave[ny][nx].feat <= FEAT_PATTERN_XTRA2)) continue;

			/* No teleporting into vaults and such */
			/* if (cave[ny][nx].info & (CAVE_ICKY)) continue; */

			/* This grid looks good */
			look = FALSE;

			/* Stop looking */
			break;
		}

		/* Increase the maximum distance */
		dis = dis * 2;

		/* Decrease the minimum distance */
		min = min / 2;
	}

	if (attempts < 1) return;

	/* Sound */
	sound(SOUND_TPOTHER);

	/* Update the new location */
	cave[ny][nx].m_idx = m_idx;

	/* Update the old location */
	cave[oy][ox].m_idx = 0;

	/* Move the monster */
	m_ptr->fy = ny;
	m_ptr->fx = nx;

	/* Update the monster (new location) */
	update_mon(m_idx, TRUE);

	/* Redraw the old grid */
	lite_spot(oy, ox);

	/* Redraw the new grid */
	lite_spot(ny, nx);

	if (r_info[m_ptr->r_idx].flags7 & (RF7_HAS_LITE_1 | RF7_SELF_LITE_1 | RF7_HAS_LITE_2 | RF7_SELF_LITE_2))
		p_ptr->update |= (PU_MON_LITE);
}


/*
 * Teleport the player to a location up to "dis" grids away.
 *
 * If no such spaces are readily available, the distance may increase.
 * Try very hard to move the player at least a quarter that distance.
 *
 * When long-range teleport effects are considered, there is a nasty
 * tendency to "bounce" the player between two or three different spots
 * because these are the only spots that are "far enough" way to satisfy
 * the algorithm.  Therefore, if the teleport distance is more than 50,
 * we decrease the minimum acceptable distance to try to increase randomness.
 * -GJW
 */
void teleport_player(int dis)
{
	int d, i, min, ox, oy;
	int tries = 0;

	int xx = -1, yy;

	/* Initialize */
	int y = py;
	int x = px;

	bool look = TRUE;

	if (p_ptr->anti_tele)
	{
#ifdef JP
msg_print("�Ի׵Ĥ��Ϥ��ƥ�ݡ��Ȥ��ɤ�����");
#else
		msg_print("A mysterious force prevents you from teleporting!");
#endif

		return;
	}

	if (dis > 200) dis = 200; /* To be on the safe side... */

	/* Minimum distance */
	min = dis / (dis > 50 ? 3 : 2);

	/* Look until done */
	while (look)
	{
		tries++;

		/* Verify max distance */
		if (dis > 200) dis = 200;

		/* Try several locations */
		for (i = 0; i < 500; i++)
		{
			/* Pick a (possibly illegal) location */
			while (1)
			{
				y = rand_spread(py, dis);
				x = rand_spread(px, dis);
				d = distance(py, px, y, x);
				if ((d >= min) && (d <= dis)) break;
			}

			/* Ignore illegal locations */
			if (!in_bounds(y, x)) continue;

			/* Require "naked" floor space or trees */
			if (!(cave_naked_bold(y, x) ||
			    (cave[y][x].feat == FEAT_TREES))) continue;

			/* No teleporting into vaults and such */
			if (cave[y][x].info & CAVE_ICKY) continue;

			/* This grid looks good */
			look = FALSE;

			/* Stop looking */
			break;
		}

		/* Increase the maximum distance */
		dis = dis * 2;

		/* Decrease the minimum distance */
		min = min / 2;

		/* Stop after MAX_TRIES tries */
		if (tries > MAX_TRIES) return;
	}

	/* Sound */
	sound(SOUND_TELEPORT);

	/* Save the old location */
	oy = py;
	ox = px;

	/* Move the player */
	py = y;
	px = x;

	/* Redraw the old spot */
	lite_spot(oy, ox);

	/* Monsters with teleport ability may follow the player */
	while (xx < 2)
	{
		yy = -1;

		while (yy < 2)
		{
			if (xx == 0 && yy == 0)
			{
				/* Do nothing */
			}
			else
			{
				if (cave[oy+yy][ox+xx].m_idx)
				{
					if ((r_info[m_list[cave[oy+yy][ox+xx].m_idx].r_idx].flags6 & RF6_TPORT) &&
					    !(r_info[m_list[cave[oy+yy][ox+xx].m_idx].r_idx].flags3 & RF3_RES_TELE))
						/*
						 * The latter limitation is to avoid
						 * totally unkillable suckers...
						 */
					{
						if (!(m_list[cave[oy+yy][ox+xx].m_idx].csleep))
							teleport_to_player(cave[oy+yy][ox+xx].m_idx,
								r_info[m_list[cave[oy+yy][ox+xx].m_idx].r_idx].level);
					}
				}
			}
			yy++;
		}
		xx++;
	}

	/* Forget the flow */
	forget_flow();

	/* Redraw the new spot */
	lite_spot(py, px);

	/* Check for new panel (redraw map) */
	verify_panel();

	/* Update stuff */
	p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MON_LITE);

	/* Update the monsters */
	p_ptr->update |= (PU_DISTANCE);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

	/* Handle stuff XXX XXX XXX */
	handle_stuff();
}



/*
 * Teleport player to a grid near the given location
 *
 * This function is slightly obsessive about correctness.
 * This function allows teleporting into vaults (!)
 */
void teleport_player_to(int ny, int nx)
{
	int y, x, oy, ox, dis = 0, ctr = 0;

	if (p_ptr->anti_tele)
	{
#ifdef JP
msg_print("�Ի׵Ĥ��Ϥ��ƥ�ݡ��Ȥ��ɤ�����");
#else
		msg_print("A mysterious force prevents you from teleporting!");
#endif

		return;
	}

	/* Find a usable location */
	while (1)
	{
		/* Pick a nearby legal location */
		while (1)
		{
			y = rand_spread(ny, dis);
			x = rand_spread(nx, dis);
			if (in_bounds(y, x)) break;
		}

		/* Accept "naked" floor grids */
		if (cave_naked_bold(y, x)) break;

		/* Occasionally advance the distance */
		if (++ctr > (4 * dis * dis + 4 * dis + 1))
		{
			ctr = 0;
			dis++;
		}
	}

	/* Sound */
	sound(SOUND_TELEPORT);

	/* Save the old location */
	oy = py;
	ox = px;

	/* Move the player */
	py = y;
	px = x;

	/* Forget the flow */
	forget_flow();

	/* Redraw the old spot */
	lite_spot(oy, ox);

	/* Redraw the new spot */
	lite_spot(py, px);

	/* Check for new panel (redraw map) */
	verify_panel();

	/* Update stuff */
	p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MON_LITE);

	/* Update the monsters */
	p_ptr->update |= (PU_DISTANCE);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

	/* Handle stuff XXX XXX XXX */
	handle_stuff();
}



/*
 * Teleport the player one level up or down (random when legal)
 */
void teleport_player_level(void)
{
	/* No effect in arena or quest */
	if (p_ptr->inside_arena || (p_ptr->inside_quest && (quest[p_ptr->inside_quest].type != QUEST_TYPE_RANDOM)) ||
	    (quest_number(dun_level) && (dun_level > 1) && ironman_downward))
	{
#ifdef JP
		msg_print("���̤��ʤ��ä���");
#else
		msg_print("There is no effect.");
#endif
		return;
	}

	if (p_ptr->anti_tele)
	{
#ifdef JP
		msg_print("�Ի׵Ĥ��Ϥ��ƥ�ݡ��Ȥ��ɤ�����");
#else
		msg_print("A mysterious force prevents you from teleporting!");
#endif
		return;
	}

	if (!dun_level || ironman_downward)
	{
#ifdef JP
		msg_print("���ʤ��Ͼ����ͤ��ˤä�����Ǥ�����");
#else
		msg_print("You sink through the floor.");
#endif

		if (autosave_l) do_cmd_save_game(TRUE);

		dun_level++;

		/* Leaving */
		p_ptr->leaving = TRUE;
	}
	else if (quest_number(dun_level) || (dun_level >= TINY_MAX_DEPTH - 1))
	{
#ifdef JP
		msg_print("���ʤ���ŷ����ͤ��ˤä�����⤤�Ƥ�����");
#else
		msg_print("You rise up through the ceiling.");
#endif

		if (autosave_l) do_cmd_save_game(TRUE);

		dun_level--;

		/* Leaving */
		leave_quest_check();
		p_ptr->inside_quest = 0;
		p_ptr->leaving = TRUE;
	}
	else if (randint0(100) < 50)
	{
#ifdef JP
		msg_print("���ʤ���ŷ����ͤ��ˤä�����⤤�Ƥ�����");
#else
		msg_print("You rise up through the ceiling.");
#endif

		if (autosave_l) do_cmd_save_game(TRUE);

		dun_level--;

		/* Leaving */
		p_ptr->leaving = TRUE;
	}
	else
	{
#ifdef JP
		msg_print("���ʤ��Ͼ����ͤ��ˤä�����Ǥ�����");
#else
		msg_print("You sink through the floor.");
#endif

		if (autosave_l) do_cmd_save_game(TRUE);

		dun_level++;

		/* Leaving */
		p_ptr->leaving = TRUE;
	}

	/* Sound */
	sound(SOUND_TPLEVEL);
}



/*
 * Recall the player to town or dungeon
 */
bool recall_player(int turns)
{
	/*
	 * TODO: Recall the player to the last
	 * visited town when in the wilderness
	 */

	/* Ironman option */
	if (ironman_downward)
	{
#ifdef JP
msg_print("���ⵯ����ʤ��ä���");
#else
		msg_print("Nothing happens.");
#endif

		return TRUE;
	}

	if (dun_level && (p_ptr->max_dlv > dun_level) && !p_ptr->inside_quest && (!p_ptr->word_recall))
	{
#ifdef JP
if (get_check("�����Ϻǿ���ã������������Ǥ������γ�����ä���ޤ����� "))
#else
		if (get_check("Reset recall depth? "))
#endif

			p_ptr->max_dlv = dun_level;

	}
	if (!p_ptr->word_recall)
	{
		p_ptr->word_recall = turns;
#ifdef JP
msg_print("�����絤��ĥ��Ĥ�Ƥ���...");
#else
		msg_print("The air about you becomes charged...");
#endif

		p_ptr->redraw |= (PR_STATUS);
	}
	else
	{
		p_ptr->word_recall = 0;
#ifdef JP
msg_print("ĥ��Ĥ᤿�絤��ή���ä�...");
#else
		msg_print("A tension leaves the air around you...");
#endif

		p_ptr->redraw |= (PR_STATUS);
	}
	return TRUE;
}


void word_of_recall(void)
{
	recall_player(randint0(21) + 15);
}


/*
 * Apply disenchantment to the player's stuff
 *
 * XXX XXX XXX This function is also called from the "melee" code
 *
 * The "mode" is currently unused.
 *
 * Return "TRUE" if the player notices anything
 */
bool apply_disenchant(int mode)
{
	int             t = 0;
	object_type     *o_ptr;
	char            o_name[MAX_NLEN];

	/* Unused */
	(void)mode;

	/* Pick a random slot */
	switch (randint1(8))
	{
		case 1: t = INVEN_WIELD; break;
		case 2: t = INVEN_BOW; break;
		case 3: t = INVEN_BODY; break;
		case 4: t = INVEN_OUTER; break;
		case 5: t = INVEN_ARM; break;
		case 6: t = INVEN_HEAD; break;
		case 7: t = INVEN_HANDS; break;
		case 8: t = INVEN_FEET; break;
	}

	/* Get the item */
	o_ptr = &inventory[t];

	/* No item, nothing happens */
	if (!o_ptr->k_idx) return (FALSE);


	/* Nothing to disenchant */
	if ((o_ptr->to_h <= 0) && (o_ptr->to_d <= 0) && (o_ptr->to_a <= 0))
	{
		/* Nothing to notice */
		return (FALSE);
	}


	/* Describe the object */
	object_desc(o_name, o_ptr, OD_OMIT_PREFIX | OD_NAME_ONLY);


	/* Artifacts have 71% chance to resist */
	if ((artifact_p(o_ptr) || o_ptr->art_name) && (randint0(100) < 71))
	{
		/* Message */
#ifdef JP
msg_format("%s(%c)��������ķ���֤�����",o_name, index_to_label(t) );
#else
		msg_format("Your %s (%c) resist%s disenchantment!",
			   o_name, index_to_label(t),
			   ((o_ptr->number != 1) ? "" : "s"));
#endif


		/* Notice */
		return (TRUE);
	}


	/* Disenchant tohit */
	if (o_ptr->to_h > 0) o_ptr->to_h--;
	if ((o_ptr->to_h > 5) && (randint0(100) < 20)) o_ptr->to_h--;

	/* Disenchant todam */
	if (o_ptr->to_d > 0) o_ptr->to_d--;
	if ((o_ptr->to_d > 5) && (randint0(100) < 20)) o_ptr->to_d--;

	/* Disenchant toac */
	if (o_ptr->to_a > 0) o_ptr->to_a--;
	if ((o_ptr->to_a > 5) && (randint0(100) < 20)) o_ptr->to_a--;

	/* Message */
#ifdef JP
msg_format("%s(%c)���������Ƥ��ޤä���",
		   o_name, index_to_label(t) );
#else
	msg_format("Your %s (%c) %s disenchanted!",
		   o_name, index_to_label(t),
		   ((o_ptr->number != 1) ? "were" : "was"));
#endif

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Window stuff */
	p_ptr->window |= (PW_EQUIP | PW_PLAYER);

	/* Notice */
	return (TRUE);
}


void mutate_player(void)
{
	int max1, cur1, max2, cur2, ii, jj;

	/* Pick a pair of stats */
	ii = randint0(6);
	for (jj = ii; jj == ii; jj = randint0(6)) /* loop */;

	max1 = p_ptr->stat_max[ii];
	cur1 = p_ptr->stat_cur[ii];
	max2 = p_ptr->stat_max[jj];
	cur2 = p_ptr->stat_cur[jj];

	p_ptr->stat_max[ii] = max2;
	p_ptr->stat_cur[ii] = cur2;
	p_ptr->stat_max[jj] = max1;
	p_ptr->stat_cur[jj] = cur1;

	p_ptr->update |= (PU_BONUS);
}


/*
 * Apply Nexus
 */
void apply_nexus(monster_type *m_ptr)
{
	switch (randint1(7))
	{
		case 1: case 2: case 3:
		{
			teleport_player(200);
			break;
		}

		case 4: case 5:
		{
			teleport_player_to(m_ptr->fy, m_ptr->fx);
			break;
		}

		case 6:
		{
			if (randint0(100) < p_ptr->skill_sav)
			{
#ifdef JP
msg_print("���������Ϥ�ķ���֤�����");
#else
				msg_print("You resist the effects!");
#endif

				break;
			}

			/* Teleport Level */
			teleport_player_level();
			break;
		}

		case 7:
		{
			if (randint0(100) < p_ptr->skill_sav)
			{
#ifdef JP
msg_print("���������Ϥ�ķ���֤�����");
#else
				msg_print("You resist the effects!");
#endif

				break;
			}

#ifdef JP
msg_print("�Τ��ͤ���Ϥ᤿...");
#else
			msg_print("Your body starts to scramble...");
#endif

			mutate_player();
			break;
		}
	}
}


/*
 * Charge a lite (torch or latern)
 */
void phlogiston(void)
{
	int max_flog;
	object_type *o_ptr = &inventory[INVEN_LITE];

	/* It's a lamp */
	if ((o_ptr->tval == TV_LITE) && (o_ptr->sval == SV_LITE_LANTERN))
	{
		max_flog = FUEL_LAMP;
	}

	/* It's a torch */
	else if ((o_ptr->tval == TV_LITE) && (o_ptr->sval == SV_LITE_TORCH))
	{
		max_flog = FUEL_TORCH;
	}

	/* No torch to refill */
	else
	{
#ifdef JP
msg_print("ǳ�Ǥ���񤹤륢���ƥ���������Ƥ��ޤ���");
#else
		msg_print("You are not wielding anything which uses phlogiston.");
#endif

		return;
	}

	if (o_ptr->xtra3 >= max_flog)
	{
#ifdef JP
msg_print("���Υ����ƥ�ˤϤ���ʾ�ǳ�Ǥ��佼�Ǥ��ޤ���");
#else
		msg_print("No more phlogiston can be put in this item.");
#endif

		return;
	}

	/* Refuel */
	o_ptr->xtra3 += (max_flog / 2);

	/* Message */
#ifdef JP
msg_print("�����ѥ����ƥ��ǳ�Ǥ��佼������");
#else
	msg_print("You add phlogiston to your light item.");
#endif


	/* Comment */
	if (o_ptr->xtra3 >= max_flog)
	{
		o_ptr->xtra3 = max_flog;
#ifdef JP
msg_print("�����ѥ����ƥ��������ˤʤä���");
#else
		msg_print("Your light item is full.");
#endif

	}

	/* Recalculate torch */
	p_ptr->update |= (PU_TORCH);
}


#if 0
/*
 * Brand the current weapon
 */
void brand_weapon(int brand_type)
{
	object_type *o_ptr;

	o_ptr = &inventory[INVEN_WIELD];

	/* you can never modify artifacts / ego-items */
	/* you can never modify cursed items */
	/* TY: You _can_ modify broken items (if you're silly enough) */
	if (o_ptr->k_idx && !artifact_p(o_ptr) && !ego_item_p(o_ptr) &&
	    !o_ptr->art_name && !cursed_p(o_ptr))
	{
		cptr act;

		/* Let's get the name before it is changed... */
		char o_name[MAX_NLEN];
		object_desc(o_name, o_ptr, OD_OMIT_PREFIX | OD_NAME_ONLY);

		switch (brand_type)
		{
		case 5:
#ifdef JP
			act = "�����ʤ륪�������ޤ줿��";
#else
			act = "is surrounded by holy light.";
#endif
			o_ptr->name2 = EGO_SLAY_EVIL;
			break;
		case 4:
#ifdef JP
			act = "�������԰���ˤʤä��褦����";
#else
			act = "seems very unstable now.";
#endif
			o_ptr->name2 = EGO_TRUMP;
			o_ptr->pval = randint1(2);
			break;
		case 3:
#ifdef JP
			act = "�Ϸ����Ƥ��롪";
#else
			act = "thirsts for blood!";
#endif
			o_ptr->name2 = EGO_VAMPIRIC;
			break;
		case 2:
#ifdef JP
			act = "���Ǥ�ʤ��줿��";
#else
			act = "is coated with poison.";
#endif
			o_ptr->name2 = EGO_BRAND_POIS;
			break;
		case 1:
#ifdef JP
			act = "�ϥ������˰��߹��ޤ줿��";
#else
			act = "is engulfed in chaos!";
#endif
			o_ptr->name2 = EGO_CHAOTIC;
			break;
		default:
			if (randint0(100) < 25)
			{
#ifdef JP
				act = "�ϱ�Υ�����ɤ�ʤ��줿��";
#else
				act = "is covered in a fiery shield!";
#endif
				o_ptr->name2 = EGO_BRAND_FIRE;
			}
			else
			{
#ifdef JP
				act = "�Ͽ����䤿���֥롼�˵�������";
#else
				act = "glows deep, icy blue!";
#endif
				o_ptr->name2 = EGO_BRAND_COLD;
			}
		}
#ifdef JP
		msg_format("���ʤ���%s%s", o_name, act);
#else
		msg_format("Your %s %s", o_name, act);
#endif
		enchant(o_ptr, randint0(3) + 4, ENCH_TOHIT | ENCH_TODAM);
	}
	else
	{
		if (flush_failure) flush();
#ifdef JP
		msg_print("°���ղä˼��Ԥ�����");
#else
		msg_print("The Branding failed.");
#endif
	}
}
#endif


void call_the_(void)
{
	int i;

	if (cave_floor_bold(py - 1, px - 1) &&
	    cave_floor_bold(py - 1, px    ) &&
	    cave_floor_bold(py - 1, px + 1) &&
	    cave_floor_bold(py    , px - 1) &&
	    cave_floor_bold(py    , px + 1) &&
	    cave_floor_bold(py + 1, px - 1) &&
	    cave_floor_bold(py + 1, px    ) &&
	    cave_floor_bold(py + 1, px + 1))
	{
		for (i = 1; i < 10; i++)
		{
			if (i-5) fire_ball(GF_ROCKET, i, 175, 2);
		}

		for (i = 1; i < 10; i++)
		{
			if (i-5) fire_ball(GF_MANA, i, 175, 3);
		}

		for (i = 1; i < 10; i++)
		{
			if (i-5) fire_ball(GF_NUKE, i, 175, 4);
		}
	}
	else
	{
#ifdef JP
		msg_format("���ʤ���%s���ɤ˶᤹������Ǿ����Ƥ��ޤä���",
			((mp_ptr->spell_type == ST_PRAYER) ? "����" : "��ʸ"));
		msg_print("�礭����ȯ�������ä���");
#else
		msg_format("You %s the %s too close to a wall!",
			((mp_ptr->spell_type == ST_PRAYER) ? "recite" : "cast"),
			((mp_ptr->spell_type == ST_PRAYER) ? "prayer" : "spell"));
		msg_print("There is a loud explosion!");
#endif


		if (destroy_area(py, px, 20 + p_ptr->lev, TRUE))
#ifdef JP
msg_print("���󥸥����������...");
#else
			msg_print("The dungeon collapses...");
#endif

		else
#ifdef JP
msg_print("���󥸥����礭���ɤ줿��");
#else
			msg_print("The dungeon trembles.");
#endif


#ifdef JP
take_hit(100 + randint1(150), "����Ū�ʵ�̵����");
#else
		take_hit(100 + randint1(150), "a suicidal Call the Void");
#endif

	}
}


/*
 * Fetch an item (teleport it right underneath the caster)
 */
void fetch(int dir, int wgt, bool require_los)
{
	int             ty, tx, i;
	cave_type       *c_ptr;
	object_type     *o_ptr;
	char            o_name[MAX_NLEN];

	/* Check to see if an object is already there */
	if (cave[py][px].o_idx)
	{
#ifdef JP
msg_print("��ʬ��­�β��ˤ���ʪ�ϼ��ޤ���");
#else
		msg_print("You can't fetch when you're already standing on something.");
#endif

		return;
	}

	/* Use a target */
	if (dir == 5 && target_okay())
	{
		tx = target_col;
		ty = target_row;

		if (distance(py, px, ty, tx) > MAX_RANGE)
		{
#ifdef JP
msg_print("����ʤ˱󤯤ˤ���ʪ�ϼ��ޤ���");
#else
			msg_print("You can't fetch something that far away!");
#endif

			return;
		}

		c_ptr = &cave[ty][tx];

		/* We need an item to fetch */
		if (!c_ptr->o_idx)
		{
#ifdef JP
msg_print("�����ˤϲ��⤢��ޤ���");
#else
			msg_print("There is no object at this place.");
#endif

			return;
		}

		/* No fetching from vault */
		if (c_ptr->info & CAVE_ICKY)
		{
#ifdef JP
msg_print("�����ƥब����ȥ���򳰤���������");
#else
			msg_print("The item slips from your control.");
#endif

			return;
		}

		/* We need to see the item */
		if (require_los && !player_has_los_bold(ty, tx))
		{
#ifdef JP
msg_print("�����Ϥ��ʤ��λ볦�����äƤ��ޤ���");
#else
			msg_print("You have no direct line of sight to that location.");
#endif

			return;
		}
	}
	else
	{
		/* Use a direction */
		ty = py; /* Where to drop the item */
		tx = px;

		do
		{
			ty += ddy[dir];
			tx += ddx[dir];
			c_ptr = &cave[ty][tx];

			if ((distance(py, px, ty, tx) > MAX_RANGE) ||
			    !cave_floor_bold(ty, tx)) return;
		}
		while (!c_ptr->o_idx);
	}

	o_ptr = &o_list[c_ptr->o_idx];

	if (o_ptr->weight > wgt)
	{
		/* Too heavy to 'fetch' */
#ifdef JP
msg_print("���Υ����ƥ�ϽŲ᤮�ޤ���");
#else
		msg_print("The object is too heavy.");
#endif

		return;
	}

	i = c_ptr->o_idx;
	c_ptr->o_idx = o_ptr->next_o_idx;
	cave[py][px].o_idx = i; /* 'move' it */
	o_ptr->next_o_idx = 0;
	o_ptr->iy = (byte)py;
	o_ptr->ix = (byte)px;

	object_desc(o_name, o_ptr, OD_NAME_ONLY);
#ifdef JP
msg_format("%^s�����ʤ���­��������Ǥ�����", o_name);
#else
	msg_format("%^s flies through the air to your feet.", o_name);
#endif


	note_spot(py, px);
	p_ptr->redraw |= PR_MAP;
}


void alter_reality(void)
{
	if (!quest_number(dun_level) && dun_level)
	{
#ifdef JP
msg_print("�������Ѥ�ä���");
#else
		msg_print("The world changes!");
#endif


		if (autosave_l) do_cmd_save_game(TRUE);

		/* Leaving */
		p_ptr->leaving = TRUE;
	}
	else
	{
#ifdef JP
msg_print("�����������δ��Ѳ������褦����");
#else
		msg_print("The world seems to change for a moment!");
#endif

	}
}


/*
 * Leave a "glyph of warding" which prevents monster movement
 */
bool warding_glyph(void)
{
	/* XXX XXX XXX */
	if (!cave_clean_bold(py, px))
	{
#ifdef JP
msg_print("����Υ����ƥब��ʸ��ķ���֤�����");
#else
		msg_print("The object resists the spell.");
#endif

		return FALSE;
	}

	/* Create a glyph */
	cave_set_feat(py, px, FEAT_GLYPH);

	return TRUE;
}


/*
 * Leave an "explosive rune" which prevents monster movement
 */
bool explosive_rune(void)
{
	/* XXX XXX XXX */
	if (!cave_clean_bold(py, px))
	{
#ifdef JP
msg_print("����Υ����ƥब��ʸ��ķ���֤�����");
#else
		msg_print("The object resists the spell.");
#endif

		return FALSE;
	}

	/* Create a glyph */
	cave_set_feat(py, px, FEAT_MINOR_GLYPH);

	return TRUE;
}


/*
 * Identify everything being carried.
 * Done by a potion of "self knowledge".
 */
void identify_pack(void)
{
	int i;

	/* Simply identify and know every item */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Identify it */
		identify_item(o_ptr);

		/* Auto-inscription */
		autopick_alter_item(i, FALSE);
	}
}


/*
 * Used by the "enchant" function (chance of failure)
 * (modified for Zangband, we need better stuff there...) -- TY
 */
static int enchant_table[16] =
{
	0, 10,  50, 100, 200,
	300, 400, 500, 650, 800,
	950, 987, 993, 995, 998,
	1000
};


/*
 * Removes curses from items in inventory
 *
 * Note that Items which are "Perma-Cursed" (The One Ring,
 * The Crown of Morgoth) can NEVER be uncursed.
 *
 * Note that if "all" is FALSE, then Items which are
 * "Heavy-Cursed" (Mormegil, Calris, and Weapons of Morgul)
 * will not be uncursed.
 */
static int remove_curse_aux(int all)
{
	int i, cnt = 0;

	/* Attempt to uncurse items being worn */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		u32b f1, f2, f3;

		object_type *o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Uncursed already */
		if (!cursed_p(o_ptr)) continue;

		/* Extract the flags */
		object_flags(o_ptr, &f1, &f2, &f3);

		/* Heavily Cursed Items need a special spell */
		if (!all && (f3 & TR3_HEAVY_CURSE)) continue;

		/* Perma-Cursed Items can NEVER be uncursed */
		if (f3 & TR3_PERMA_CURSE) continue;

		/* Uncurse it */
		o_ptr->ident &= ~(IDENT_CURSED);

		/* Hack -- Assume felt */
		o_ptr->ident |= (IDENT_SENSE);

		if (o_ptr->art_flags3 & TR3_CURSED)
			o_ptr->art_flags3 &= ~(TR3_CURSED);

		if (o_ptr->art_flags3 & TR3_HEAVY_CURSE)
			o_ptr->art_flags3 &= ~(TR3_HEAVY_CURSE);

		/* Take note */
#ifdef JP
		o_ptr->feeling = FEEL_NONE;
#else
		o_ptr->feeling = FEEL_UNCURSED;
#endif


		/* Recalculate the bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Window stuff */
		p_ptr->window |= (PW_EQUIP);

		/* Count the uncursings */
		cnt++;
	}

	/* Return "something uncursed" */
	return (cnt);
}


/*
 * Remove most curses
 */
bool remove_curse(void)
{
	return (remove_curse_aux(FALSE));
}

/*
 * Remove all curses
 */
bool remove_all_curse(void)
{
	return (remove_curse_aux(TRUE));
}


/*
 * Turns an object into gold, gain some of its value in a shop
 */
bool alchemy(void)
{
	int item, amt = 1;
	int old_number;
	long price;
	bool force = FALSE;
	object_type *o_ptr;
	char o_name[MAX_NLEN];
	char out_val[160];

	cptr q, s;

	/* Hack -- force destruction */
	if (command_arg > 0) force = TRUE;

	/* Get an item */
#ifdef JP
q = "�ɤΥ����ƥ�����Ѥ��ޤ�����";
s = "����Ѥ�����ʪ������ޤ���";
#else
	q = "Turn which item to gold? ";
	s = "You have nothing to turn to gold.";
#endif

	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return (FALSE);

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}


	/* See how many items */
	if (o_ptr->number > 1)
	{
		/* Get a quantity */
		amt = get_quantity(NULL, o_ptr->number);

		/* Allow user abort */
		if (amt <= 0) return FALSE;
	}


	/* Describe the object */
	old_number = o_ptr->number;
	o_ptr->number = amt;
	object_desc(o_name, o_ptr, 0);
	o_ptr->number = old_number;

	/* Verify unless quantity given */
	if (!force)
	{
		if (!(auto_destroy && (object_value(o_ptr) < 1)))
		{
			/* Make a verification */
#ifdef JP
sprintf(out_val, "������%s�����Ѥ��ޤ�����", o_name);
#else
			sprintf(out_val, "Really turn %s to gold? ", o_name);
#endif

			if (!get_check(out_val)) return FALSE;
		}
	}

	/* Check for artifacts */
	if (!can_player_destroy_object(o_ptr))
	{
		/* Message */
#ifdef JP
msg_format("%s�����Ѥ��뤳�Ȥ˼��Ԥ�����", o_name);
#else
		msg_format("You fail to turn %s to gold!", o_name);
#endif


		/* Done */
		return FALSE;
	}

	price = object_value_real(o_ptr);

	if (price <= 0)
	{
		/* Message */
#ifdef JP
msg_format("%s��˥��ζ���Ѥ�����", o_name);
#else
		msg_format("You turn %s to fool's gold.", o_name);
#endif

	}
	else
	{
		price /= 3;

		if (amt > 1) price *= amt;

		if (price > 30000) price = 30000;
#ifdef JP
msg_format("%s���%d �ζ���Ѥ�����", o_name, price);
#else
		msg_format("You turn %s to %ld coins worth of gold.", o_name, price);
#endif

		p_ptr->au += price;

		/* Redraw gold */
		p_ptr->redraw |= (PR_GOLD);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);

	}

	/* Eliminate the item (from the pack) */
	if (item >= 0)
	{
		inven_item_increase(item, -amt);
		inven_item_describe(item);
		inven_item_optimize(item);
	}

	/* Eliminate the item (from the floor) */
	else
	{
		floor_item_increase(0 - item, -amt);
		floor_item_describe(0 - item);
		floor_item_optimize(0 - item);
	}

	return TRUE;
}


/*
 * Create stairs at the player location
 */
void stair_creation(void)
{
	/* XXX XXX XXX */
	if (!cave_valid_bold(py, px))
	{
#ifdef JP
msg_print("����Υ����ƥब��ʸ��ķ���֤�����");
#else
		msg_print("The object resists the spell.");
#endif

		return;
	}

	/* XXX XXX XXX */
	delete_object(py, px);

	/* Create a staircase */
	if (p_ptr->inside_arena || p_ptr->inside_quest)
	{
		/* arena or quest */
#ifdef JP
msg_print("���̤�����ޤ���");
#else
		msg_print("There is no effect!");
#endif

	}
	else if (!dun_level || ironman_downward)
	{
		/* Town/wilderness or Ironman */
		cave_set_feat(py, px, FEAT_MORE);
	}
	else if (quest_number(dun_level) || (dun_level >= TINY_MAX_DEPTH - 1))
	{
		/* Quest level */
		cave_set_feat(py, px, FEAT_LESS);
	}
	else if (randint0(100) < 50)
	{
		cave_set_feat(py, px, FEAT_MORE);
	}
	else
	{
		cave_set_feat(py, px, FEAT_LESS);
	}
}


/*
 * Hook to specify "weapon"
 */
bool item_tester_hook_weapon(const object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
		case TV_SWORD:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_DIGGING:
		case TV_BOW:
		case TV_BOLT:
		case TV_ARROW:
		case TV_SHOT:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Hook to specify "weapon" except ammo
 */
static bool item_tester_hook_weapon_except_ammo(const object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
		case TV_SWORD:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_DIGGING:
		case TV_BOW:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Hook to specify "armour"
 */
bool item_tester_hook_armour(const object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
		case TV_DRAG_ARMOR:
		case TV_HARD_ARMOR:
		case TV_SOFT_ARMOR:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_CROWN:
		case TV_HELM:
		case TV_BOOTS:
		case TV_GLOVES:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Check if an object is weapon or armour (but not arrow, bolt, or shot)
 */
bool item_tester_hook_weapon_armour(const object_type *o_ptr)
{
	return (item_tester_hook_weapon(o_ptr) ||
		item_tester_hook_armour(o_ptr));
}


/*
 * Break the curse of an item
 */
static void break_curse(object_type *o_ptr)
{
	u32b    f1, f2, f3;

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3);

	if (cursed_p(o_ptr) && !(f3 & TR3_PERMA_CURSE) && (randint0(100) < 25))
	{
#ifdef JP
msg_print("�������Ƥ����������Ǥ��ˤ�줿��");
#else
		msg_print("The curse is broken!");
#endif


		o_ptr->ident &= ~(IDENT_CURSED);
		o_ptr->ident |= (IDENT_SENSE);

		if (o_ptr->art_flags3 & TR3_CURSED)
			o_ptr->art_flags3 &= ~(TR3_CURSED);
		if (o_ptr->art_flags3 & TR3_HEAVY_CURSE)
			o_ptr->art_flags3 &= ~(TR3_HEAVY_CURSE);

#ifdef JP
		o_ptr->feeling = FEEL_NONE;
#else
		o_ptr->feeling = FEEL_UNCURSED;
#endif

	}
}


/*
 * Enchants a plus onto an item. -RAK-
 *
 * Revamped!  Now takes item pointer, number of times to try enchanting,
 * and a flag of what to try enchanting.  Artifacts resist enchantment
 * some of the time, and successful enchantment to at least +0 might
 * break a curse on the item. -CFT-
 *
 * Note that an item can technically be enchanted all the way to +15 if
 * you wait a very, very, long time.  Going from +9 to +10 only works
 * about 5% of the time, and from +10 to +11 only about 1% of the time.
 *
 * Note that this function can now be used on "piles" of items, and
 * the larger the pile, the lower the chance of success.
 */
bool enchant(object_type *o_ptr, int n, int eflag)
{
	int     i, chance, prob;
	bool    res = FALSE;
	bool    a = (artifact_p(o_ptr) || o_ptr->art_name);
	bool    force = (eflag & ENCH_FORCE);


	/* Large piles resist enchantment */
	prob = o_ptr->number * 100;

	/* Missiles are easy to enchant */
	if ((o_ptr->tval == TV_BOLT) ||
	    (o_ptr->tval == TV_ARROW) ||
	    (o_ptr->tval == TV_SHOT))
	{
		prob = prob / 20;
	}

	/* Try "n" times */
	for (i = 0; i < n; i++)
	{
		/* Hack -- Roll for pile resistance */
		if (!force && randint0(prob) >= 100) continue;

		/* Enchant to hit */
		if (eflag & ENCH_TOHIT)
		{
			if (o_ptr->to_h < 0) chance = 0;
			else if (o_ptr->to_h > 15) chance = 1000;
			else chance = enchant_table[o_ptr->to_h];

			if (force || ((randint1(1000) > chance) && (!a || (randint0(100) < 50))))
			{
				o_ptr->to_h++;
				res = TRUE;

				/* only when you get it above -1 -CFT */
				if (o_ptr->to_h >= 0)
					break_curse(o_ptr);
			}
		}

		/* Enchant to damage */
		if (eflag & ENCH_TODAM)
		{
			if (o_ptr->to_d < 0) chance = 0;
			else if (o_ptr->to_d > 15) chance = 1000;
			else chance = enchant_table[o_ptr->to_d];

			if (force || ((randint1(1000) > chance) && (!a || (randint0(100) < 50))))
			{
				o_ptr->to_d++;
				res = TRUE;

				/* only when you get it above -1 -CFT */
				if (o_ptr->to_d >= 0)
					break_curse(o_ptr);
			}
		}

		/* Enchant to armor class */
		if (eflag & ENCH_TOAC)
		{
			if (o_ptr->to_a < 0) chance = 0;
			else if (o_ptr->to_a > 15) chance = 1000;
			else chance = enchant_table[o_ptr->to_a];

			if (force || ((randint1(1000) > chance) && (!a || (randint0(100) < 50))))
			{
				o_ptr->to_a++;
				res = TRUE;

				/* only when you get it above -1 -CFT */
				if (o_ptr->to_a >= 0)
					break_curse(o_ptr);
			}
		}
	}

	/* Failure */
	if (!res) return (FALSE);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	/* Success */
	return (TRUE);
}



/*
 * Enchant an item (in the inventory or on the floor)
 * Note that "num_ac" requires armour, else weapon
 * Returns TRUE if attempted, FALSE if cancelled
 */
bool enchant_spell(int num_hit, int num_dam, int num_ac)
{
	int         item;
	bool        okay = FALSE;
	object_type *o_ptr;
	char        o_name[MAX_NLEN];
	cptr        q, s;


	/* Assume enchant weapon */
	item_tester_hook = item_tester_hook_weapon;

	/* Enchant armor if requested */
	if (num_ac) item_tester_hook = item_tester_hook_armour;

	/* Get an item */
#ifdef JP
q = "�ɤΥ����ƥ�򶯲����ޤ���? ";
s = "�����Ǥ��륢���ƥब�ʤ���";
#else
	q = "Enchant which item? ";
	s = "You have nothing to enchant.";
#endif

	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return (FALSE);

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}


	/* Description */
	object_desc(o_name, o_ptr, OD_OMIT_PREFIX | OD_NAME_ONLY);

	/* Describe */
#ifdef JP
msg_format("%s �����뤯��������",
    o_name);
#else
	msg_format("%s %s glow%s brightly!",
		   ((item >= 0) ? "Your" : "The"), o_name,
		   ((o_ptr->number > 1) ? "" : "s"));
#endif


	/* Enchant */
	if (enchant(o_ptr, num_hit, ENCH_TOHIT)) okay = TRUE;
	if (enchant(o_ptr, num_dam, ENCH_TODAM)) okay = TRUE;
	if (enchant(o_ptr, num_ac, ENCH_TOAC)) okay = TRUE;

	/* Failure */
	if (!okay)
	{
		/* Flush */
		if (flush_failure) flush();

		/* Message */
#ifdef JP
msg_print("�����˼��Ԥ�����");
#else
		msg_print("The enchantment failed.");
#endif
	}

	/* Something happened */
	return (TRUE);
}


/*
 * Enchant an item (in the inventory or on the floor)
 * Note that "num_ac" requires armour, else weapon
 * Returns TRUE if attempted, FALSE if cancelled
 */
bool coat_equip(void)
{
	int         item;
	object_type *o_ptr;
	char        o_name[MAX_NLEN];
	cptr        q, s;


	/* coat weapon or armor */
	item_tester_hook = item_tester_hook_weapon_armour;

	/* Get an item */
#ifdef JP
q = "�ɤΥ����ƥ���忩�ɻߤ��ޤ���? ";
s = "�忩�ɻߤǤ��륢���ƥब�ʤ���";
#else
	q = "Enchant which item? ";
	s = "You have nothing to enchant.";
#endif

	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return (FALSE);

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}


	/* Description */
	object_desc(o_name, o_ptr, OD_OMIT_PREFIX | OD_NAME_ONLY);

	/* Describe */
#ifdef JP
msg_format("%s �����뤯��������",
    o_name);
#else
	msg_format("%s %s glow%s brightly!",
		   ((item >= 0) ? "Your" : "The"), o_name,
		   ((o_ptr->number > 1) ? "" : "s"));
#endif


	/* Coat */
	o_ptr->art_flags3 |= (TR3_IGNORE_ACID | TR3_IGNORE_ELEC |
			      TR3_IGNORE_FIRE | TR3_IGNORE_COLD);

	/* Something happened */
	return (TRUE);
}


bool artifact_scroll(void)
{
	int             item;
	bool            okay;
	object_type     *o_ptr;
	char            o_name[MAX_NLEN];
	cptr            q, s;


	/* Enchant weapon/armour */
	item_tester_hook = item_tester_hook_weapon_armour;

	/* Get an item */
#ifdef JP
q = "�ɤΥ����ƥ�򶯲����ޤ���? ";
s = "�����Ǥ��륢���ƥब�ʤ���";
#else
	q = "Enchant which item? ";
	s = "You have nothing to enchant.";
#endif

	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return (FALSE);

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}


	/* Description */
	object_desc(o_name, o_ptr, OD_OMIT_PREFIX | OD_NAME_ONLY);

	/* Describe */
#ifdef JP
msg_format("%s ����������ȯ������",o_name);
#else
	msg_format("%s %s radiate%s a blinding light!",
		  ((item >= 0) ? "Your" : "The"), o_name,
		  ((o_ptr->number > 1) ? "" : "s"));
#endif


#if 0
	/* No artifact creation of Dragon Scale Mail */
	if (o_ptr->tval == TV_DRAG_ARMOR)
	{
		/* ToDo: Maybe allow some of the DSMs to be enchanted */
#ifdef JP
msg_format("%s �ˤϴ�����ˡ�������äƤ��ޤ���",
    o_name);
#else
		msg_format("The %s %s already magical!",
		    o_name, ((o_ptr->number > 1) ? "are" : "is"));
#endif


		okay = FALSE;
	}

	else if (o_ptr->name1 || o_ptr->art_name)
#endif
	if (o_ptr->name1 || o_ptr->art_name)
	{
#ifdef JP
msg_format("%s�ϴ�������Υ����ƥ�Ǥ���",
    o_name  );
#else
		msg_format("The %s %s already %s!",
		    o_name, ((o_ptr->number > 1) ? "are" : "is"),
		    ((o_ptr->number > 1) ? "artifacts" : "an artifact"));
#endif

		return (FALSE);
	}

	else if (o_ptr->name2)
	{
#ifdef JP
msg_format("%s�ϴ���̾�Τ��륢���ƥ�Ǥ���",
    o_name );
#else
		msg_format("The %s %s already %s!",
		    o_name, ((o_ptr->number > 1) ? "are" : "is"),
		    ((o_ptr->number > 1) ? "ego items" : "an ego item"));
#endif

		return (FALSE);
	}

	else
	{
		if (o_ptr->number > 1)
		{
#ifdef JP
			msg_print("ʣ���Υ����ƥ����ˡ�򤫤���������ϤϤ���ޤ���");
			msg_format("%s��%d�Ĳ��줿��", o_name, (o_ptr->number) - 1);
#else
			msg_print("Not enough enough energy to enchant more than one object!");
			msg_format("%d of your %s %s destroyed!", (o_ptr->number) - 1,
				   o_name, ((o_ptr->number > 2) ? "were" : "was"));
#endif

			o_ptr->number = 1;
		}
		okay = create_artifact(o_ptr, TRUE);
	}

	/* Failure */
	if (!okay)
	{
		/* Flush */
		if (flush_failure) flush();

		/* Message */
#ifdef JP
msg_print("�����˼��Ԥ�����");
#else
		msg_print("The enchantment failed.");
#endif
	}

	/* Something happened */
	return (TRUE);
}


/*
 * Identify an object
 */
bool identify_item(object_type *o_ptr)
{
	bool old_known = FALSE;
	char o_name[MAX_NLEN];

	if (o_ptr->ident & IDENT_KNOWN)
		old_known = TRUE;

	/* Identify it fully */
	object_aware(o_ptr);
	object_known(o_ptr);

	/* Player touches it */
	o_ptr->marked |= OM_TOUCHED;

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	/*
	 * If the item was an artifact, and if the
	 * auto-note is selected, write a message (if not written before).
	 */
	object_desc(o_name, o_ptr, OD_NAME_ONLY);
	
	if (take_notes && record_artifact)
	{
		if(artifact_p(o_ptr) && a_info[o_ptr->name1].cur_num != 2)
		{
			add_note(o_name, 'A');

			/* Mark item as found */
			a_info[o_ptr->name1].cur_num = 2;
		}
		else if(record_randart && o_ptr->art_name && !old_known)
		{
			add_note(o_name, 'A');
		}
	}

	return old_known;
}


static bool item_tester_hook_identify(const object_type *o_ptr)
{
	return (bool)!object_known_p(o_ptr);
}

/*
 * Identify an object in the inventory (or on the floor)
 * This routine does *not* automatically combine objects.
 * Returns TRUE if something was identified, else FALSE.
 */
bool ident_spell(void)
{
	int             item;
	object_type     *o_ptr;
	char            o_name[MAX_NLEN];
	cptr            q, s;
	bool old_known;

	item_tester_hook = item_tester_hook_identify;

	/* Get an item */
	if (can_get_item())
	{
#ifdef JP
		q = "�ɤΥ����ƥ����ꤷ�ޤ���? ";
#else
		q = "Identify which item? ";
#endif
	}
	else
	{
		item_tester_hook = NULL;

#ifdef JP
		q = "���٤ƴ���ѤߤǤ��� ";
#else
		q = "All items are identified. ";
#endif
	}

	/* Get an item */
#ifdef JP
	s = "���ꤹ��٤������ƥब�ʤ���";
#else
	s = "You have nothing to identify.";
#endif

	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return (FALSE);

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}


	/* Identify it */
	old_known = identify_item(o_ptr);

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
		msg_format("%^s: %s(%c)��",
#else
		msg_format("%^s: %s (%c).",
#endif
				describe_use(item), o_name, index_to_label(item));
	}
	else if (item >= 0)
	{
#ifdef JP
		msg_format("���å���: %s(%c)��",
#else
		msg_format("In your pack: %s (%c).",
#endif
				o_name, index_to_label(item));
	}
	else
	{
#ifdef JP
		msg_format("����: %s��",
#else
		msg_format("On the ground: %s.",
#endif
				o_name);
	}

	/* Auto-inscription/destroy */
	autopick_alter_item(item, (bool)(destroy_identify && !old_known));

	/* Something happened */
	return (TRUE);
}


/*
 * Mundanify an object in the inventory (or on the floor)
 * This routine does *not* automatically combine objects.
 * Returns TRUE if something was mundanified, else FALSE.
 */
bool mundane_spell(bool equip)
{
	int             item;
	object_type     *o_ptr;
	object_kind     *k_ptr;
	cptr            q, s;


	if (equip) item_tester_hook = item_tester_hook_wear;

	/* Get an item */
#ifdef JP
	q = "�ɤ��Ȥ��ޤ�����";
	s = "�Ȥ����Τ�����ޤ���";
#else
	q = "Use which item? ";
	s = "You have nothing you can use.";
#endif

	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return (FALSE);

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	k_ptr = &k_info[o_ptr->k_idx];

	/* Oops */
#ifdef JP
	msg_print("�ޤФ椤���������ä���");
#else
	msg_print("There is a bright flash of light!");
#endif

	/* No discount */
	o_ptr->discount = 0;

	/* No extra info */
	o_ptr->xtra1 = 0;
	o_ptr->xtra2 = 0;

	/* No artifact name (random artifacts) */
	o_ptr->art_name = 0;

	/* No ego name (named ego) */
	o_ptr->ego_name = 0;

	/* Not identified yet */
	o_ptr->ident = 0;

	/* Erase the inscription */
	o_ptr->inscription = 0;

	/* Erase the "feeling" */
	o_ptr->feeling = FEEL_NONE;

	/* Default "pval" */
	o_ptr->pval = k_ptr->pval;

	/* Default weight */
	o_ptr->weight = k_ptr->weight;

	/* Default magic */
	o_ptr->to_h = k_ptr->to_h;
	o_ptr->to_d = k_ptr->to_d;
	o_ptr->to_a = k_ptr->to_a;

	/* No longer artifact / ego item */
	o_ptr->name1 = 0;
	o_ptr->name2 = 0;

	/* Default power */
	o_ptr->ac = k_ptr->ac;
	o_ptr->dd = k_ptr->dd;
	o_ptr->ds = k_ptr->ds;

	/* No artifact powers */
	o_ptr->art_flags1 = 0;
	o_ptr->art_flags2 = 0;
	o_ptr->art_flags3 = 0;

	/* For rod-stacking */
	if (o_ptr->tval == TV_ROD)
	{
		o_ptr->timeout = o_ptr->pval * o_ptr->number;
		o_ptr->pval = k_ptr->pval * o_ptr->number;
	}

	/* Hack -- worthless items are always "broken" */
	if (get_object_cost(o_ptr) <= 0) o_ptr->ident |= (IDENT_BROKEN);

	/* Hack -- cursed items are always "cursed" */
	if (k_ptr->flags3 & (TR3_CURSED)) o_ptr->ident |= (IDENT_CURSED);

	/* Update Stuff */
	p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA);

	/* Something happened */
	return (TRUE);
}


static bool item_tester_hook_identify_fully(const object_type *o_ptr)
{
	return (bool)(!object_known_p(o_ptr) || !(o_ptr->ident & IDENT_MENTAL));
}

/*
 * Fully "identify" an object in the inventory  -BEN-
 * This routine returns TRUE if an item was identified.
 */
bool identify_fully(void)
{
	int             item;
	object_type     *o_ptr;
	char            o_name[MAX_NLEN];
	cptr            q, s;
	bool old_known;

	item_tester_hook = item_tester_hook_identify_fully;

	if (can_get_item())
	{
#ifdef JP
		q = "�ɤΥ����ƥ��*����*���ޤ���? ";
#else
		q = "*Identify* which item? ";
#endif
	}
	else
	{
		item_tester_hook = NULL;

#ifdef JP
		q = "���٤�*����*�ѤߤǤ��� ";
#else
		q = "All items are *identified*. ";
#endif
	}

	/* Get an item */
#ifdef JP
	s = "*����*����٤������ƥब�ʤ���";
#else
	s = "You have nothing to *identify*.";
#endif

	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return (FALSE);

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Identify it */
	old_known = identify_item(o_ptr);

	/* Mark the item as fully known */
	o_ptr->ident |= (IDENT_MENTAL);

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
		msg_format("%^s: %s(%c)��",
#else
		msg_format("%^s: %s (%c).",
#endif
			   describe_use(item), o_name, index_to_label(item));
	}
	else if (item >= 0)
	{
#ifdef JP
		msg_format("���å���: %s(%c)��",
#else
		msg_format("In your pack: %s (%c).",
#endif
			   o_name, index_to_label(item));
	}
	else
	{
#ifdef JP
		msg_format("����: %s��",
#else
		msg_format("On the ground: %s.",
#endif
			   o_name);
	}

	/* Describe it fully */
	(void)identify_fully_aux(o_ptr, TRUE);

	/* Auto-inscription/destroy */
	autopick_alter_item(item, (bool)(destroy_identify && !old_known));

	/* Success */
	return (TRUE);
}


/*
 * Hook for "get_item()".  Determine if something is rechargable.
 */
bool item_tester_hook_recharge(const object_type *o_ptr)
{
	/* Can not recharge a staff of wishing */
	if ((o_ptr->tval == TV_STAFF) && (o_ptr->sval == SV_STAFF_WISHING)) return (FALSE);

	/* Recharge staffs */
	if (o_ptr->tval == TV_STAFF) return (TRUE);

	/* Recharge wands */
	if (o_ptr->tval == TV_WAND) return (TRUE);

	/* Hack -- Recharge rods */
	if (o_ptr->tval == TV_ROD) return (TRUE);

	/* Nope */
	return (FALSE);
}


static void fail_recharge(int item)
{
	object_type *o_ptr = &inventory[item];
	object_kind *k_ptr = &k_info[o_ptr->k_idx];
	byte fail_type = 1;
	char o_name[MAX_NLEN];

	/* Artifacts are never destroyed. */
	if (artifact_p(o_ptr))
	{
		object_desc(o_name, o_ptr, OD_NAME_ONLY);
#ifdef JP
		msg_format("���Ϥ���ή������%s�ϴ��������Ϥ򼺤ä���", o_name);
#else
		msg_format("The recharging backfires - %s is completely drained!", o_name);
#endif
		/* Artifact rods. */
		if ((o_ptr->tval == TV_ROD) && (o_ptr->timeout < 10000))
			o_ptr->timeout = (o_ptr->timeout + 100) * 2;

		/* Artifact wands and staffs. */
		else if ((o_ptr->tval == TV_WAND) || (o_ptr->tval == TV_STAFF))
			o_ptr->pval = 0;
	}
	else
	{
		/* Get the object description */
		object_desc(o_name, o_ptr, OD_OMIT_PREFIX | OD_NAME_ONLY);

		/*** Determine Seriousness of Failure ***/

		/* Mages and Deice-users recharge objects more safely. */
		if (p_ptr->pclass == CLASS_MAGE)
		{
			/* 10% chance to blow up one rod, otherwise draining. */
			if (o_ptr->tval == TV_ROD)
			{
				if (randint1(10) == 1) fail_type = 2;
				else fail_type = 1;
			}
			/* 75% chance to blow up one wand, otherwise draining. */
			else if (o_ptr->tval == TV_WAND)
			{
				if (randint1(3) != 1) fail_type = 2;
				else fail_type = 1;
			}
			/* 50% chance to blow up one staff, otherwise no effect. */
			else if (o_ptr->tval == TV_STAFF)
			{
				if (randint1(2) == 1) fail_type = 2;
				else fail_type = 0;
			}
		}

		/* All other classes get no special favors. */
		else
		{
			/* 33% chance to blow up one rod, otherwise draining. */
			if (o_ptr->tval == TV_ROD)
			{
				if (randint1(3) == 1) fail_type = 2;
				else fail_type = 1;
			}
			/* 20% chance of the entire stack, else destroy one wand. */
			else if (o_ptr->tval == TV_WAND)
			{
				if (randint1(5) == 1) fail_type = 3;
				else fail_type = 2;
			}
			/* Blow up one staff. */
			else if (o_ptr->tval == TV_STAFF)
			{
				fail_type = 2;
			}
		}

		/*** Apply draining and destruction. ***/

		/* Drain object or stack of objects. */
		if (fail_type == 1)
		{
			if (o_ptr->tval == TV_ROD)
			{
#ifdef JP
				msg_print("���Ϥ���ʮ�ͤ��ơ���åɤ��餵������Ϥ�ۤ���äƤ��ޤä���");
#else
				msg_print("The recharge backfires, draining the rod further!");
#endif
				if (o_ptr->timeout < 10000)
					o_ptr->timeout = (o_ptr->timeout + 100) * 2;
			}
			else if (o_ptr->tval == TV_WAND)
			{
#ifdef JP
				msg_format("%s����»���Ȥ줿�������Ϥ����Ƽ���줿��", o_name);
#else
				msg_format("You save your %s from destruction, but all charges are lost.", o_name);
#endif
				o_ptr->pval = 0;
			}
			/* Staffs aren't drained. */
		}

		/* Destroy an object or one in a stack of objects. */
		if (fail_type == 2)
		{
			if (o_ptr->number > 1)
#ifdef JP
			msg_format("��˽����ˡ�Τ����%s�����ܲ��줿��", o_name);
#else
				msg_format("Wild magic consumes one of your %s!", o_name);
#endif
			else
#ifdef JP
				msg_format("��˽����ˡ�Τ����%s�����줿��", o_name);
#else
				msg_format("Wild magic consumes your %s!", o_name);
#endif

			/* Reduce rod stack maximum timeout, drain wands. */
			if (o_ptr->tval == TV_ROD) o_ptr->pval -= k_ptr->pval;
			if (o_ptr->tval == TV_WAND) o_ptr->pval = 0;

			/* Reduce and describe inventory */
			if (item >= 0)
			{
				inven_item_increase(item, -1);
				inven_item_describe(item);
				inven_item_optimize(item);
			}

			/* Reduce and describe floor item */
			else
			{
				floor_item_increase(0 - item, -1);
				floor_item_describe(0 - item);
				floor_item_optimize(0 - item);
			}
		}

		/* Destroy all members of a stack of objects. */
		if (fail_type == 3)
		{
			if (o_ptr->number > 1)
#ifdef JP
				msg_format("��˽����ˡ�Τ����%s�����Ʋ��줿��", o_name);
#else
				msg_format("Wild magic consumes all your %s!", o_name);
#endif
			else
#ifdef JP
				msg_format("��˽����ˡ�Τ����%s�����줿��", o_name);
#else
				msg_format("Wild magic consumes your %s!", o_name);
#endif

			/* Reduce and describe inventory */
			if (item >= 0)
			{
				inven_item_increase(item, -999);
				inven_item_describe(item);
				inven_item_optimize(item);
			}

			/* Reduce and describe floor item */
			else
			{
				floor_item_increase(0 - item, -999);
				floor_item_describe(0 - item);
				floor_item_optimize(0 - item);
			}
		}
	}
}

/*
 * Recharge a wand/staff/rod from the pack or on the floor.
 * This function has been rewritten in Oangband and ZAngband.
 *
 * Sorcery/Arcane -- Recharge  --> recharge(plev * 4)
 * Chaos -- Arcane Binding     --> recharge(90)
 *
 * Scroll of recharging        --> recharge(130)
 * Artifact activation/Thingol --> recharge(130)
 *
 * It is harder to recharge high level, and highly charged wands,
 * staffs, and rods.  The more wands in a stack, the more easily and
 * strongly they recharge.  Staffs, however, each get fewer charges if
 * stacked.
 *
 * XXX XXX XXX Beware of "sliding index errors".
 */
bool recharge(int power)
{
	int item, lev;
	int recharge_strength, recharge_amount;

	object_type *o_ptr;
	object_kind *k_ptr;

	bool fail = FALSE;
	cptr q, s;


	/* Only accept legal items */
	item_tester_hook = item_tester_hook_recharge;

	/* Get an item */
#ifdef JP
q = "�ɤΥ����ƥ�����Ϥ�Ŷ���ޤ���? ";
s = "���Ϥ�Ŷ���٤������ƥब�ʤ���";
#else
	q = "Recharge which item? ";
	s = "You have nothing to recharge.";
#endif

	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return (FALSE);

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Get the object kind. */
	k_ptr = &k_info[o_ptr->k_idx];

	/* Extract the object "level" */
	lev = k_info[o_ptr->k_idx].level;

	/* Recharge a rod */
	if (o_ptr->tval == TV_ROD)
	{
		/* Extract a recharge strength by comparing object level to power. */
		recharge_strength = ((power > lev) ? (power - lev) : 0) / 5;


		/* Back-fire */
		if (randint0(recharge_strength) == 0)
		{
			/* Activate the failure code. */
			fail = TRUE;
		}

		/* Recharge */
		else
		{
			/* Recharge amount */
			recharge_amount = (power * damroll(3, 2));

			/* Recharge by that amount */
			if (o_ptr->timeout > recharge_amount)
				o_ptr->timeout -= recharge_amount;
			else
				o_ptr->timeout = 0;
		}
	}


	/* Recharge wand/staff */
	else
	{
		/* Extract a recharge strength by comparing object level to power.
		 * Divide up a stack of wands' charges to calculate charge penalty.
		 */
		if ((o_ptr->tval == TV_WAND) && (o_ptr->number > 1))
			recharge_strength = (100 + power - lev -
			(8 * o_ptr->pval / o_ptr->number)) / 15;

		/* All staffs, unstacked wands. */
		else recharge_strength = (100 + power - lev -
			(8 * o_ptr->pval)) / 15;

		/* Paranoia */
		if (recharge_strength < 0) recharge_strength = 0;

		/* Back-fire */
		if (randint0(recharge_strength) == 0)
		{
			/* Activate the failure code. */
			fail = TRUE;
		}

		/* If the spell didn't backfire, recharge the wand or staff. */
		else
		{
			/* Recharge based on the standard number of charges. */
			recharge_amount = randint1(1 + k_ptr->pval / 2);

			/* Multiple wands in a stack increase recharging somewhat. */
			if ((o_ptr->tval == TV_WAND) && (o_ptr->number > 1))
			{
				recharge_amount +=
					(randint1(recharge_amount * (o_ptr->number - 1))) / 2;
				if (recharge_amount < 1) recharge_amount = 1;
				if (recharge_amount > 12) recharge_amount = 12;
			}

			/* But each staff in a stack gets fewer additional charges,
			 * although always at least one.
			 */
			if ((o_ptr->tval == TV_STAFF) && (o_ptr->number > 1))
			{
				recharge_amount /= o_ptr->number;
				if (recharge_amount < 1) recharge_amount = 1;
			}

			/* Recharge the wand or staff. */
			o_ptr->pval += recharge_amount;

			/* Hack -- we no longer "know" the item */
			o_ptr->ident &= ~(IDENT_KNOWN);

			/* Hack -- we no longer think the item is empty */
			o_ptr->ident &= ~(IDENT_EMPTY);
		}
	}


	/* Inflict the penalties for failing a recharge. */
	if (fail) fail_recharge(item);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN);

	/* Something was done */
	return (TRUE);
}


/*
 * Bless a weapon
 */
bool bless_weapon(void)
{
	int             item;
	object_type     *o_ptr;
	u32b            f1, f2, f3;
	char            o_name[MAX_NLEN];
	cptr            q, s;

	/* Assume enchant weapon */
	item_tester_hook = item_tester_hook_weapon_except_ammo;

	/* Get an item */
#ifdef JP
	q = "�ɤΥ����ƥ���ʡ���ޤ�����";
	s = "��ʡ�Ǥ�����郎����ޤ���";
#else
	q = "Bless which weapon? ";
	s = "You have weapon to bless.";
#endif

	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR)))
		return FALSE;

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}


	/* Description */
	object_desc(o_name, o_ptr, OD_OMIT_PREFIX | OD_NAME_ONLY);

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3);

	if (o_ptr->ident & IDENT_CURSED)
	{
		if (((f3 & TR3_HEAVY_CURSE) && (randint1(100) < 33)) ||
		    (f3 & TR3_PERMA_CURSE))
		{
#ifdef JP
			msg_format("%s��ʤ������������Ͻ�ʡ��ķ���֤�����",
				o_name);
#else
			msg_format("The black aura on %s %s disrupts the blessing!",
			    ((item >= 0) ? "your" : "the"), o_name);
#endif
			return TRUE;
		}

#ifdef JP
		msg_format("%s ����ٰ��ʥ����餬�ä�����",
			o_name);
#else
		msg_format("A malignant aura leaves %s %s.",
		    ((item >= 0) ? "your" : "the"), o_name);
#endif

		/* Uncurse it */
		o_ptr->ident &= ~(IDENT_CURSED);

		/* Hack -- Assume felt */
		o_ptr->ident |= (IDENT_SENSE);

		/* Take note */
#ifdef JP
		o_ptr->feeling = FEEL_NONE;
#else
		o_ptr->feeling = FEEL_UNCURSED;
#endif

		/* Recalculate the bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Window stuff */
		p_ptr->window |= (PW_EQUIP);
	}

	/*
	 * Next, we try to bless it. Artifacts have a 1/3 chance of
	 * being blessed, otherwise, the operation simply disenchants
	 * them, godly power negating the magic. Ok, the explanation
	 * is silly, but otherwise priests would always bless every
	 * artifact weapon they find. Ego weapons and normal weapons
	 * can be blessed automatically.
	 */
	if (f3 & TR3_BLESSED)
	{
#ifdef JP
		msg_format("%s �ϴ��˽�ʡ����Ƥ��롣",
			o_name    );
#else
		msg_format("%s %s %s blessed already.",
		    ((item >= 0) ? "Your" : "The"), o_name,
		    ((o_ptr->number > 1) ? "were" : "was"));
#endif
		return TRUE;
	}

	if (!(o_ptr->art_name || o_ptr->name1) || (randint1(3) == 1))
	{
		/* Describe */
#ifdef JP
		msg_format("%s �ϵ�������",
			 o_name);
#else
		msg_format("%s %s shine%s!",
		    ((item >= 0) ? "Your" : "The"), o_name,
		    ((o_ptr->number > 1) ? "" : "s"));
#endif
		o_ptr->art_flags3 |= TR3_BLESSED;
	}
	else
	{
		bool dis_happened = FALSE;

#ifdef JP
		msg_print("���Υ����ƥ��ե����ȤϽ�ʡ����äƤ��롪");
#else
		msg_print("The artifact resists your blessing!");
#endif

		/* Disenchant tohit */
		if (o_ptr->to_h > 0)
		{
			o_ptr->to_h--;
			dis_happened = TRUE;
		}

		if ((o_ptr->to_h > 5) && (randint0(100) < 33))
		{
			o_ptr->to_h--;
			dis_happened = TRUE;
		}

		/* Disenchant todam */
		if (o_ptr->to_d > 0)
		{
			o_ptr->to_d--;
			dis_happened = TRUE;
		}

		if ((o_ptr->to_d > 5) && (randint0(100) < 33))
		{
			o_ptr->to_d--;
			dis_happened = TRUE;
		}

		/* Disenchant toac */
		if (o_ptr->to_a > 0)
		{
			o_ptr->to_a--;
			dis_happened = TRUE;
		}

		if ((o_ptr->to_a > 5) && (randint0(100) < 33))
		{
			o_ptr->to_a--;
			dis_happened = TRUE;
		}

		if (dis_happened)
		{
#ifdef JP
			msg_print("���Ϥ����Ǥ�ʷ�ϵ���������...");
			msg_format("%s ������������",
				 o_name    );
#else
			msg_print("There is a static feeling in the air...");
			msg_format("%s %s %s disenchanted!",
			    ((item >= 0) ? "Your" : "The"), o_name,
			    ((o_ptr->number > 1) ? "were" : "was"));
#endif
		}
	}

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Window stuff */
	p_ptr->window |= (PW_EQUIP | PW_PLAYER);

	return TRUE;
}


/*
 * Potions "smash open" and cause an area effect when
 * (1) they are shattered while in the player's inventory,
 * due to cold (etc) attacks;
 * (2) they are thrown at a monster, or obstacle;
 * (3) they are shattered by a "cold ball" or other such spell
 * while lying on the floor.
 *
 * Arguments:
 *    who   ---  who caused the potion to shatter (0=player)
 *          potions that smash on the floor are assumed to
 *          be caused by no-one (who = 1), as are those that
 *          shatter inside the player inventory.
 *          (Not anymore -- I changed this; TY)
 *    y, x  --- coordinates of the potion (or player if
 *          the potion was in her inventory);
 *    o_ptr --- pointer to the potion object.
 */
bool potion_smash_effect(int who, int y, int x, int k_idx)
{
	int     radius = 2;
	int     dt = 0;
	int     dam = 0;
	bool    ident = FALSE;
	bool    angry = FALSE;

	object_kind *k_ptr = &k_info[k_idx];

	switch (k_ptr->sval)
	{
		case SV_POTION_WATER:   /* perhaps a 'water' attack? */
			return TRUE;

		case SV_POTION_INFRAVISION:
		case SV_POTION_DETECT_INVIS:
		case SV_POTION_CURE_POISON:
		case SV_POTION_RESIST_HEAT:
		case SV_POTION_RESIST_COLD:
		case SV_POTION_HEROISM:
		case SV_POTION_BESERK_STRENGTH:
		case SV_POTION_RESTORE_EXP:
		case SV_POTION_RESTORING:
		case SV_POTION_RES_STR:
		case SV_POTION_RES_INT:
		case SV_POTION_RES_WIS:
		case SV_POTION_RES_DEX:
		case SV_POTION_RES_CON:
		case SV_POTION_RES_CHR:
		case SV_POTION_ENLIGHTENMENT:
		case SV_POTION_SELF_KNOWLEDGE:
		case SV_POTION_EXPERIENCE:
		case SV_POTION_RESISTANCE:
		case SV_POTION_INVULNERABILITY:
			/* All of the above potions have no effect when shattered */
			return FALSE;
		case SV_POTION_DETONATIONS:
			dt = GF_SHARDS;
			dam = damroll(25, 25);
			angry = TRUE;
			ident = TRUE;
			break;
		case SV_POTION_SPEED:
			dt = GF_OLD_SPEED;
			ident = TRUE;
			break;
		case SV_POTION_CURE_LIGHT:
			dt = GF_OLD_HEAL;
			dam = damroll(2, 3);
			ident = TRUE;
			break;
		case SV_POTION_CURE_SERIOUS:
			dt = GF_OLD_HEAL;
			dam = damroll(4, 3);
			ident = TRUE;
			break;
		case SV_POTION_CURE_CRITICAL:
		case SV_POTION_CURING:
			dt = GF_OLD_HEAL;
			dam = damroll(6, 3);
			ident = TRUE;
			break;
		case SV_POTION_HEALING:
			dt = GF_OLD_HEAL;
			dam = damroll(10, 10);
			ident = TRUE;
			break;
		case SV_POTION_STAR_HEALING:
			dt = GF_OLD_HEAL;
			dam = damroll(50, 50);
			radius = 1;
			ident = TRUE;
			break;
		case SV_POTION_RESTORE_MANA:   /* MANA */
			dt = GF_MANA;
			dam = damroll(10, 10);
			radius = 1;
			ident = TRUE;
			break;
		default:
			/* Do nothing */  ;
	}

	(void)project(who, radius, y, x, dam, dt,
	    (PROJECT_JUMP | PROJECT_ITEM | PROJECT_KILL));

	/* An identification was made */
	if (ident && !(k_ptr->aware))
	{
		k_ptr->aware = TRUE;
		gain_exp((k_ptr->level + (p_ptr->lev >> 1)) / p_ptr->lev);
	}

	return angry;
}


s16b get_spell_chance(int fail, int min_lev, int mana_cost)
{
	int chance, minfail;
	bool dc = p_ptr->dec_mana;

	/* Get the failure rate */
	chance = fail;

	/* Reduce failure rate by "effective" level adjustment */
	chance -= 3 * (p_ptr->lev - min_lev);

	/* Reduce failure rate by INT/WIS adjustment */
	chance -= 3 * (adj_mag_stat[p_ptr->stat_ind[mp_ptr->spell_stat]] - 1);

	/* Not enough mana to cast */
	if (mana_cost > p_ptr->csp)
	{
		chance += 5 * (mana_cost - p_ptr->csp);
	}

	/* Reduce failure rate by dec_mana */
	if (dc) chance -= 2;

	/* Extract the minimum failure rate */
	minfail = adj_mag_fail[p_ptr->stat_ind[mp_ptr->spell_stat]];

	/*
	 * Non mage/priest characters never get too good
	 * (added high mage, mindcrafter)
	 */
	if ((p_ptr->pclass != CLASS_PRIEST) &&
	    (p_ptr->pclass != CLASS_MAGE))
	{
		if (minfail < 5) minfail = 5;
	}

	/* Hack -- Priest prayer penalty for "edged" weapons  -DGK */
	if ((p_ptr->pclass == CLASS_PRIEST) && p_ptr->icky_wield) chance += 25;

	/* Minimum failure rate */
	if (chance < minfail) chance = minfail;

	/* Stunning makes spells harder */
	if (p_ptr->stun > 50) chance += 25;
	else if (p_ptr->stun) chance += 15;

	/* Always a 5 percent chance of working */
	if (chance > 95) chance = 95;

	if (dc) chance--;

	chance = MAX(chance, 0);

	return (chance);
}


/*
 * Hack -- Display all known spells in a window
 *
 * XXX XXX XXX Need to analyze size of the window.
 *
 * XXX XXX XXX Need more color coding.
 */
void display_spell_list(void)
{
	int             i, j;
	int             y, x;
	int             use_realm1 = p_ptr->realm1 - 1;
	int             use_realm2 = p_ptr->realm2 - 1;
	int             m[9];
	magic_type      *s_ptr;
	char            name[80];
	char            out_val[160];


	/* Erase window */
	clear_from(0);

	/* Warriors are illiterate */
	if (!mp_ptr->spell_type) return;

	/* Normal spellcaster with books */

	/* Scan books */
	for (j = 0; j < ((use_realm2 > -1) ? 2 : 1); j++)
	{
		int n = 0;
		int bn, obn, sn;

		/* Reset numbers */
		bn = 0;
		obn = 0;
		sn = 0;

		/* Reset vertical */
		m[j] = 0;

		/* Vertical location */
		y = (j < 3) ? 0 : (m[j - 3] + 2);

		/* Horizontal location */
		x = 27 * (j % 3);

		/* Scan spells */
		for (i = 0; i < 32; i++)
		{
			byte a = TERM_WHITE;

			/* search a book */
			for (bn = 0; bn < 4; bn++)
			{
				if ((1L << i) & fake_spell_flags[bn]) break;
			}

			if (bn != obn) sn = 0;

			/* Access the spell */
			s_ptr = &mp_ptr->info[(j < 1) ? use_realm1 : use_realm2][i % 32];

			strcpy(name, spell_names[(j < 1) ? use_realm1 : use_realm2][i % 32]);

			/* Illegible */
			if (s_ptr->slevel >= 99)
			{
				/* Illegible */
#ifdef JP
strcpy(name, "(Ƚ����ǽ)");
#else
				strcpy(name, "(illegible)");
#endif


				/* Unusable */
				a = TERM_L_DARK;
			}

			/* Forgotten */
			else if ((j < 1) ?
				((spell_forgotten1 & (1L << i))) :
				((spell_forgotten2 & (1L << (i % 32)))))
			{
				/* Forgotten */
				a = TERM_ORANGE;
			}

			/* Unknown */
			else if (!((j < 1) ?
				(spell_learned1 & (1L << i)) :
				(spell_learned2 & (1L << (i % 32)))))
			{
				/* Unknown */
				a = TERM_RED;
			}

			/* Untried */
			else if (!((j < 1) ?
				(spell_worked1 & (1L << i)) :
				(spell_worked2 & (1L << (i % 32)))))
			{
				/* Untried */
				a = TERM_YELLOW;
			}

			/* Dump the spell --(-- */
			sprintf(out_val, "%c/%c) %-20.20s",
				I2A(bn), I2A(sn), name);

			/* Track maximum */
			m[j] = y + n;

			/* Dump onto the window */
			Term_putstr(x, m[j], -1, a, out_val);

			/* Next */
			obn = bn;
			sn++;
			n++;
		}
	}
}


/*
 * Returns spell chance of failure for spell -RAK-
 */
s16b spell_chance(int spell, int realm)
{
	int             chance;
	magic_type      *s_ptr;
	byte            use_mana;


	/* Paranoia -- must be literate */
	if (!mp_ptr->spell_type) return (100);

	/* Access the spell */
	s_ptr = &mp_ptr->info[realm][spell];
	use_mana = get_modified_smana(s_ptr);

	chance = get_spell_chance(s_ptr->sfail, s_ptr->slevel, use_mana);

	/* Return the chance */
	return (chance);
}



/*
 * Determine if a spell is "okay" for the player to cast or study
 * The spell must be legible, not forgotten, and also, to cast,
 * it must be known, and to study, it must not be known.
 */
bool spell_okay(int spell, bool known, int realm)
{
	magic_type *s_ptr;

	/* Access the spell */
	s_ptr = &mp_ptr->info[realm][spell];

	/* Spell is illegal */
	if (s_ptr->slevel > p_ptr->lev) return (FALSE);

	/* Spell is forgotten */
	if ((realm == p_ptr->realm2 - 1) ?
	    (spell_forgotten2 & (1L << spell)) :
	    (spell_forgotten1 & (1L << spell)))
	{
		/* Never okay */
		return (FALSE);
	}

	/* Spell is learned */
	if ((realm == p_ptr->realm2 - 1) ?
	    (spell_learned2 & (1L << spell)) :
	    (spell_learned1 & (1L << spell)))
	{
		/* Okay to cast, not to study */
		return (known);
	}

	/* Okay to study, not to cast */
	return (!known);
}



/*
 * Extra information on a spell -DRS-
 *
 * We can use up to 14 characters of the buffer 'p'
 *
 * The strings in this function were extracted from the code in the
 * functions "do_cmd_cast()" and "do_cmd_pray()" and may be dated.
 */
static void spell_info(char *p, int spell, int realm)
{
	/* Default */
	strcpy(p, "");

#ifdef DRS_SHOW_SPELL_INFO
	{
		int plev = p_ptr->lev;

		/* See below */
		int orb;

		/* Analyze the spell */
		switch (realm+1)
		{
			case REALM_LIFE: /* Life */
				orb = (plev / ((p_ptr->pclass == CLASS_PRIEST) ? 2 : 4));

				switch (spell)
				{
#ifdef JP
					case  1: strcpy (p, " ����:2d10"); break;
					case  2: strcpy (p, " ����:12+d12"); break;
					case  3: sprintf(p, " »��:2d%d", plev / 2); break;
					case  6: sprintf(p, " »��:3d6+%d", plev + orb); break;
					case  7: strcpy (p, " ����:20+d20"); break;

					case  8: strcpy (p, " ����:8d10"); break;
					case 11: sprintf(p, " ����:%d+d25", plev * 3); break;
					case 13: sprintf(p, " »��:%d", 3 * plev); break;
					case 14: strcpy (p, " ����:200"); break;
					case 15: sprintf(p, " »��:%d", 100 + plev * 4); break;
#else
					case  1: strcpy (p, " heal 2d10"); break;
					case  2: strcpy (p, " dur 12+d12"); break;
					case  3: sprintf(p, " dam 2d%d", plev / 2); break;
					case  6: sprintf(p, " dam 3d6+%d", plev + orb); break;
					case  7: strcpy (p, " dur 20+d20"); break;

					case  8: strcpy (p, " heal 8d10"); break;
					case 11: sprintf(p, " dur %d+d25", plev * 3); break;
					case 13: sprintf(p, " dam %d", 3 * plev); break;
					case 14: strcpy (p, " heal 200"); break;
					case 15: sprintf(p, " dam %d", 100 + plev * 4); break;
#endif
				}
				break;

			case REALM_SORCERY: /* Sorcery */
				orb = (plev / ((p_ptr->pclass == CLASS_MAGE) ? 2 : 4));

				switch (spell)
				{
#ifdef JP
					case  0: sprintf(p, " »��:%dd4", 3 + ((plev - 1) / 5)); break;
					case  1: strcpy (p, " ��Υ:10"); break;
					case  3: sprintf(p, " »��:2d%d", plev / 2); break;
					case  4: sprintf(p, " ��Υ:%d", plev * 5); break;
					case  5: sprintf(p, " »��:3d5+%d", plev + orb); break;

					case 11: sprintf(p, " »��:%d", 70 + plev * 2); break;
					case 12: strcpy (p, " ����:25+d30"); break;
					case 13: sprintf(p, " ����:%d+d%d", plev, plev + 20); break;
					case 15: sprintf(p, " »��:%d", 111 + 6 * plev); break;
#else
					case  0: sprintf(p, " dam %dd4", 3 + ((plev - 1) / 5)); break;
					case  1: strcpy (p, " range 10"); break;
					case  3: sprintf(p, " dam 2d%d", plev / 2); break;
					case  4: sprintf(p, " range %d", plev * 5); break;
					case  5: sprintf(p, " dam 3d5+%d", plev + orb); break;

					case 11: sprintf(p, " dam %d", 70 + plev * 2); break;
					case 12: strcpy (p, " dur 25+d30"); break;
					case 13: sprintf(p, " dur %d+d%d", plev, plev + 20); break;
					case 15: sprintf(p, " dam %d", 111 + 6 * plev); break;
#endif
				}
				break;

			default:
#ifdef JP
				sprintf(p, "̤�ΤΥ�����: %d", realm);
#else
				sprintf(p, "Unknown type: %d.", realm);
#endif
		}
	}
#endif /* DRS_SHOW_SPELL_INFO */
}


/*
 * Print a list of spells (for browsing or casting or viewing)
 */
void print_spells(byte *spells, int num, int y, int x, int realm)
{
	int             i, spell;
	magic_type      *s_ptr;
	cptr            comment;
	char            info[80];
	char            out_val[160];
	byte            line_attr;
	byte            use_mana;


	if (((realm < 0) || (realm > MAX_REALM - 1)) && wizard)
#ifdef JP
msg_print("�ٹ� print_spell ���ΰ�ʤ��˸ƤФ줿");
#else
		msg_print("Warning! print_spells called with null realm");
#endif


	/* Title the list */
	prt("", y, x);
#ifdef JP
put_str("̾��", y, x + 5);
put_str("Lv   MP ��Ψ ����", y, x + 35);
#else
	put_str("Name", y, x + 5);
	put_str("Lv Mana Fail Info", y, x + 35);
#endif



	/* Dump the spells */
	for (i = 0; i < num; i++)
	{
		/* Access the spell */
		spell = spells[i];

		/* Access the spell */
		s_ptr = &mp_ptr->info[realm][spell];
		use_mana = get_modified_smana(s_ptr);

		/* Skip illegible spells */
		if (s_ptr->slevel >= 99)
		{
#ifdef JP
			sprintf(out_val, "  %c) %-30s", I2A(i), "(Ƚ����ǽ)");
#else
			sprintf(out_val, "  %c) %-30s", I2A(i), "(illegible)");
#endif

			c_prt(TERM_L_DARK, out_val, y + i + 1, x);
			continue;
		}

		/* XXX XXX Could label spells above the players level */

		/* Get extra info */
		spell_info(info, spell, realm);

		/* Use that info */
		comment = info;

		/* Assume spell is known and tried */
		line_attr = TERM_WHITE;

		/* Analyze the spell */
		if ((realm + 1 == p_ptr->realm1) ?
		    ((spell_forgotten1 & (1L << spell))) :
		    ((spell_forgotten2 & (1L << spell))))
		{
#ifdef JP
			comment = " ˺��";
#else
			comment = " forgotten";
#endif

			line_attr = TERM_YELLOW;
		}
		else if (!((realm + 1 == p_ptr->realm1) ?
		    (spell_learned1 & (1L << spell)) :
		    (spell_learned2 & (1L << spell))))
		{
#ifdef JP
			comment = " ̤��";
#else
			comment = " unknown";
#endif

			line_attr = TERM_L_BLUE;
		}
		else if (!((realm + 1 == p_ptr->realm1) ?
		    (spell_worked1 & (1L << spell)) :
		    (spell_worked2 & (1L << spell))))
		{
#ifdef JP
			comment = " ̤�и�";
#else
			comment = " untried";
#endif

			line_attr = TERM_L_GREEN;
		}

		/* Dump the spell --(-- */
		sprintf(out_val, "  %c) %-30s%2d %4d %3d%%%s",
		    I2A(i), spell_names[realm][spell], /* realm, spell */
		    s_ptr->slevel, use_mana, spell_chance(spell, realm), comment);
		c_prt(line_attr, out_val, y + i + 1, x);
	}

	/* Clear the bottom line */
	prt("", y + i + 1, x);
}


/*
 * Note that amulets, rods, and high-level spell books are immune
 * to "inventory damage" of any kind.  Also sling ammo and shovels.
 */


/*
 * Does a given class of objects (usually) hate acid?
 * Note that acid can either melt or corrode something.
 */
bool hates_acid(const object_type *o_ptr)
{
	/* Analyze the type */
	switch (o_ptr->tval)
	{
		/* Wearable items */
		case TV_ARROW:
		case TV_BOLT:
		case TV_BOW:
		case TV_SWORD:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_HELM:
		case TV_CROWN:
		case TV_SHIELD:
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_CLOAK:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
		{
			return (TRUE);
		}

		/* Staffs/Scrolls are wood/paper */
		case TV_STAFF:
		case TV_SCROLL:
		{
			return (TRUE);
		}

		/* Ouch */
		case TV_CHEST:
		{
			return (TRUE);
		}

		/* Junk is useless */
		case TV_SKELETON:
		case TV_BOTTLE:
		case TV_JUNK:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Does a given object (usually) hate electricity?
 */
bool hates_elec(const object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
		case TV_RING:
		case TV_WAND:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Does a given object (usually) hate fire?
 * Hafted/Polearm weapons have wooden shafts.
 * Arrows/Bows are mostly wooden.
 */
bool hates_fire(const object_type *o_ptr)
{
	/* Analyze the type */
	switch (o_ptr->tval)
	{
		/* Wearable */
		case TV_LITE:
		case TV_ARROW:
		case TV_BOW:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_CLOAK:
		case TV_SOFT_ARMOR:
		{
			return (TRUE);
		}

		/* Books */
		case TV_LIFE_BOOK:
		case TV_SORCERY_BOOK:
		{
			return (TRUE);
		}

		/* Chests */
		case TV_CHEST:
		{
			return (TRUE);
		}

		/* Staffs/Scrolls burn */
		case TV_STAFF:
		case TV_SCROLL:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Does a given object (usually) hate cold?
 */
bool hates_cold(const object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
		case TV_POTION:
		case TV_FLASK:
		case TV_BOTTLE:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Melt something
 */
int set_acid_destroy(object_type *o_ptr)
{
	u32b f1, f2, f3;
	if (!hates_acid(o_ptr)) return (FALSE);
	object_flags(o_ptr, &f1, &f2, &f3);
	if (f3 & TR3_IGNORE_ACID) return (FALSE);
	return (TRUE);
}


/*
 * Electrical damage
 */
int set_elec_destroy(object_type *o_ptr)
{
	u32b f1, f2, f3;
	if (!hates_elec(o_ptr)) return (FALSE);
	object_flags(o_ptr, &f1, &f2, &f3);
	if (f3 & TR3_IGNORE_ELEC) return (FALSE);
	return (TRUE);
}


/*
 * Burn something
 */
int set_fire_destroy(object_type *o_ptr)
{
	u32b f1, f2, f3;
	if (!hates_fire(o_ptr)) return (FALSE);
	object_flags(o_ptr, &f1, &f2, &f3);
	if (f3 & TR3_IGNORE_FIRE) return (FALSE);
	return (TRUE);
}


/*
 * Freeze things
 */
int set_cold_destroy(object_type *o_ptr)
{
	u32b f1, f2, f3;
	if (!hates_cold(o_ptr)) return (FALSE);
	object_flags(o_ptr, &f1, &f2, &f3);
	if (f3 & TR3_IGNORE_COLD) return (FALSE);
	return (TRUE);
}


/*
 * Destroys a type of item on a given percent chance
 * Note that missiles are no longer necessarily all destroyed
 * Destruction taken from "melee.c" code for "stealing".
 * New-style wands and rods handled correctly. -LM-
 * Returns number of items destroyed.
 */
int inven_damage(inven_func typ, int perc)
{
	int         i, j, k, amt;
	object_type *o_ptr;
	char        o_name[MAX_NLEN];


	/* Count the casualties */
	k = 0;

	/* Scan through the slots backwards */
	for (i = 0; i < INVEN_PACK; i++)
	{
		o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Hack -- for now, skip artifacts */
		if (artifact_p(o_ptr) || o_ptr->art_name) continue;

		/* Give this item slot a shot at death */
		if ((*typ)(o_ptr))
		{
			/* Count the casualties */
			for (amt = j = 0; j < o_ptr->number; ++j)
			{
				if (randint0(100) < perc) amt++;
			}

			/* Some casualities */
			if (amt)
			{
				/* Get a description */
				object_desc(o_name, o_ptr, OD_OMIT_PREFIX);

				/* Message */
#ifdef JP
msg_format("%s(%c)��%s����Ƥ��ޤä���",
#else
				msg_format("%sour %s (%c) %s destroyed!",
#endif

#ifdef JP
o_name, index_to_label(i),
    ((o_ptr->number > 1) ?
    ((amt == o_ptr->number) ? "����" :
    (amt > 1 ? "���Ĥ�" : "���")) : "")    );
#else
				    ((o_ptr->number > 1) ?
				    ((amt == o_ptr->number) ? "All of y" :
				    (amt > 1 ? "Some of y" : "One of y")) : "Y"),
				    o_name, index_to_label(i),
				    ((amt > 1) ? "were" : "was"));
#endif


				/* Potions smash open */
				if (object_is_potion(o_ptr))
				{
					(void)potion_smash_effect(0, py, px, o_ptr->k_idx);
				}

				/* Reduce the charges of rods/wands */
				reduce_charges(o_ptr, amt);

				/* Destroy "amt" items */
				inven_item_increase(i, -amt);
				inven_item_optimize(i);

				/* Count the casualties */
				k += amt;
			}
		}
	}

	/* Return the casualty count */
	return (k);
}


/*
 * Acid has hit the player, attempt to affect some armor.
 *
 * Note that the "base armor" of an object never changes.
 *
 * If any armor is damaged (or resists), the player takes less damage.
 */
static int minus_ac(void)
{
	object_type *o_ptr = NULL;
	u32b        f1, f2, f3;
	char        o_name[MAX_NLEN];


	/* Pick a (possibly empty) inventory slot */
	switch (randint1((is_two_handed() ? 5 : 6)))
	{
		case 1: o_ptr = &inventory[INVEN_BODY]; break;
		case 2: o_ptr = &inventory[INVEN_OUTER]; break;
		case 3: o_ptr = &inventory[INVEN_HANDS]; break;
		case 4: o_ptr = &inventory[INVEN_HEAD]; break;
		case 5: o_ptr = &inventory[INVEN_FEET]; break;
		case 6: o_ptr = &inventory[INVEN_ARM]; break;
	}

	/* Nothing to damage */
	if (!o_ptr->k_idx) return (FALSE);

	/* No damage left to be done */
	if (o_ptr->ac + o_ptr->to_a <= 0) return (FALSE);


	/* Describe */
	object_desc(o_name, o_ptr, OD_OMIT_PREFIX | OD_NAME_ONLY);

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3);

	/* Object resists */
	if (f3 & TR3_IGNORE_ACID)
	{
#ifdef JP
msg_format("������%s�ˤϸ��̤��ʤ��ä���", o_name);
#else
		msg_format("Your %s is unaffected!", o_name);
#endif


		return (TRUE);
	}

	/* Message */
#ifdef JP
msg_format("%s�����᡼�����������", o_name);
#else
	msg_format("Your %s is damaged!", o_name);
#endif


	/* Damage the item */
	o_ptr->to_a--;

	/* Calculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Window stuff */
	p_ptr->window |= (PW_EQUIP | PW_PLAYER);

	/* Item was damaged */
	return (TRUE);
}


/*
 * Hurt the player with Acid
 */
int acid_dam(int dam, cptr kb_str)
{
	int inv = (dam < 30) ? 1 : (dam < 60) ? 2 : 3;

	/* Total Immunity */
	if (p_ptr->immune_acid || (dam <= 0)) return 0;

	/* Vulnerability (Ouch!) */
	if (p_ptr->muta & MUT_VULN_ELEM) dam *= 2;

	/* Resist the damage */
	if (p_ptr->resist_acid) dam = (dam + 2) / 3;
	if (p_ptr->oppose_acid) dam = (dam + 2) / 3;

	if ((!(p_ptr->oppose_acid || p_ptr->resist_acid)) &&
	    randint1(HURT_CHANCE) == 1)
		(void)do_dec_stat(A_CHR);

	/* If any armor gets hit, defend the player */
	if (minus_ac()) dam = (dam + 1) / 2;

	/* Take damage */
	dam = take_hit(dam, kb_str);

	/* Inventory damage */
	if (!(p_ptr->oppose_acid && p_ptr->resist_acid))
		inven_damage(set_acid_destroy, inv);

	return (dam);
}


/*
 * Hurt the player with electricity
 */
int elec_dam(int dam, cptr kb_str)
{
	int inv = (dam < 30) ? 1 : (dam < 60) ? 2 : 3;

	/* Total immunity */
	if (p_ptr->immune_elec || (dam <= 0)) return 0;

	/* Vulnerability (Ouch!) */
	if (p_ptr->muta & MUT_VULN_ELEM) dam *= 2;

	/* Resist the damage */
	if (p_ptr->oppose_elec) dam = (dam + 2) / 3;
	if (p_ptr->resist_elec) dam = (dam + 2) / 3;

	if ((!(p_ptr->oppose_elec || p_ptr->resist_elec)) &&
	    randint1(HURT_CHANCE) == 1)
		(void)do_dec_stat(A_DEX);

	/* Take damage */
	dam = take_hit(dam, kb_str);

	/* Inventory damage */
	if (!(p_ptr->oppose_elec && p_ptr->resist_elec))
		inven_damage(set_elec_destroy, inv);

	return (dam);
}


/*
 * Hurt the player with Fire
 */
int fire_dam(int dam, cptr kb_str)
{
	int inv = (dam < 30) ? 1 : (dam < 60) ? 2 : 3;

	/* Totally immune */
	if (p_ptr->immune_fire || (dam <= 0)) return 0;

	/* Vulnerability (Ouch!) */
	if (p_ptr->muta & MUT_VULN_ELEM)
		dam *= 2;

	/* Resist the damage */
	if (p_ptr->resist_fire) dam = (dam + 2) / 3;
	if (p_ptr->oppose_fire) dam = (dam + 2) / 3;

	if ((!(p_ptr->oppose_fire || p_ptr->resist_fire)) &&
	    randint1(HURT_CHANCE) == 1)
		(void)do_dec_stat(A_STR);

	/* Take damage */
	dam = take_hit(dam, kb_str);

	/* Inventory damage */
	if (!(p_ptr->resist_fire && p_ptr->oppose_fire))
		inven_damage(set_fire_destroy, inv);

	return (dam);
}


/*
 * Hurt the player with Cold
 */
int cold_dam(int dam, cptr kb_str)
{
	int inv = (dam < 30) ? 1 : (dam < 60) ? 2 : 3;

	/* Total immunity */
	if (p_ptr->immune_cold || (dam <= 0)) return 0;

	/* Vulnerability (Ouch!) */
	if (p_ptr->muta & MUT_VULN_ELEM)
		dam *= 2;

	/* Resist the damage */
	if (p_ptr->resist_cold) dam = (dam + 2) / 3;
	if (p_ptr->oppose_cold) dam = (dam + 2) / 3;

	if ((!(p_ptr->oppose_cold || p_ptr->resist_cold)) &&
	    randint1(HURT_CHANCE) == 1)
		(void)do_dec_stat(A_STR);

	/* Take damage */
	dam = take_hit(dam, kb_str);

	/* Inventory damage */
	if (!(p_ptr->resist_cold && p_ptr->oppose_cold))
		inven_damage(set_cold_destroy, inv);

	return (dam);
}


bool rustproof(void)
{
	int         item;
	object_type *o_ptr;
	char        o_name[MAX_NLEN];
	cptr        q, s;

	/* Select a piece of armour */
	item_tester_hook = item_tester_hook_armour;

	/* Get an item */
#ifdef JP
q = "�ɤ��ɶ�˻��ߤ�򤷤ޤ�����";
s = "���ߤ�Ǥ����Τ�����ޤ���";
#else
	q = "Rustproof which piece of armour? ";
	s = "You have nothing to rustproof.";
#endif

	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return FALSE;

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}


	/* Description */
	object_desc(o_name, o_ptr, OD_OMIT_PREFIX | OD_NAME_ONLY);

	o_ptr->art_flags3 |= TR3_IGNORE_ACID;

	if ((o_ptr->to_a < 0) && !(o_ptr->ident & IDENT_CURSED))
	{
#ifdef JP
msg_format("%s�Ͽ���Ʊ�ͤˤʤä���",o_name);
#else
		msg_format("%s %s look%s as good as new!",
			((item >= 0) ? "Your" : "The"), o_name,
			((o_ptr->number > 1) ? "" : "s"));
#endif

		o_ptr->to_a = 0;
	}

#ifdef JP
msg_format("%s���忩���ʤ��ʤä���", o_name);
#else
	msg_format("%s %s %s now protected against corrosion.",
		((item >= 0) ? "Your" : "The"), o_name,
		((o_ptr->number > 1) ? "are" : "is"));
#endif


	return TRUE;
}


/*
 * Curse the players armor
 */
bool curse_armor(void)
{
	object_type *o_ptr;

	char o_name[MAX_NLEN];


	/* Curse the body armor */
	o_ptr = &inventory[INVEN_BODY];

	/* Nothing to curse */
	if (!o_ptr->k_idx) return (FALSE);


	/* Describe */
	object_desc(o_name, o_ptr, OD_OMIT_PREFIX);

	/* Attempt a saving throw for artifacts */
	if ((o_ptr->art_name || artifact_p(o_ptr)) && (randint0(100) < 50))
	{
		/* Cool */
#ifdef JP
msg_format("%s��%s����߹��⤦�Ȥ�������%s�Ϥ����ķ���֤�����",
"���ݤΰŹ�������", "�ɶ�", o_name);
#else
		msg_format("A %s tries to %s, but your %s resists the effects!",
			   "terrible black aura", "surround your armor", o_name);
#endif

	}

	/* not artifact or failed save... */
	else
	{
		/* Oops */
#ifdef JP
msg_format("���ݤΰŹ������餬���ʤ���%s����߹������", o_name);
#else
		msg_format("A terrible black aura blasts your %s!", o_name);
#endif

		/* Blast the armor */
		o_ptr->name1 = 0;
		o_ptr->name2 = EGO_BLASTED;
		o_ptr->to_a = 0 - randint1(5) - randint1(5);
		o_ptr->to_h = 0;
		o_ptr->to_d = 0;
		o_ptr->ac = 0;
		o_ptr->dd = 1;
		o_ptr->ds = 1;
		o_ptr->art_flags1 = 0;
		o_ptr->art_flags2 = 0;
		o_ptr->art_flags3 = 0;

		/* Curse it */
		o_ptr->ident |= (IDENT_CURSED);

		/* Break it */
		o_ptr->ident |= (IDENT_BROKEN);

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Recalculate mana */
		p_ptr->update |= (PU_MANA);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);
	}

	return (TRUE);
}


/*
 * Curse the players weapon
 */
bool curse_weapon(void)
{
	object_type *o_ptr;

	char o_name[MAX_NLEN];


	/* Curse the weapon */
	o_ptr = &inventory[INVEN_WIELD];

	/* Nothing to curse */
	if (!o_ptr->k_idx) return (FALSE);


	/* Describe */
	object_desc(o_name, o_ptr, OD_OMIT_PREFIX);

	/* Attempt a saving throw */
	if ((artifact_p(o_ptr) || o_ptr->art_name) && (randint0(100) < 50))
	{
		/* Cool */
#ifdef JP
msg_format("%s��%s����߹��⤦�Ȥ�������%s�Ϥ����ķ���֤�����",
"���ݤΰŹ�������", "���", o_name);
#else
		msg_format("A %s tries to %s, but your %s resists the effects!",
					  "terrible black aura", "surround your weapon", o_name);
#endif

	}

	/* not artifact or failed save... */
	else
	{
		/* Oops */
#ifdef JP
msg_format("���ݤΰŹ������餬���ʤ���%s����߹������", o_name);
#else
		msg_format("A terrible black aura blasts your %s!", o_name);
#endif

		/* Shatter the weapon */
		o_ptr->name1 = 0;
		o_ptr->name2 = EGO_SHATTERED;
		o_ptr->to_h = 0 - randint1(5) - randint1(5);
		o_ptr->to_d = 0 - randint1(5) - randint1(5);
		o_ptr->to_a = 0;
		o_ptr->ac = 0;
		o_ptr->dd = 1;
		o_ptr->ds = 1;
		o_ptr->art_flags1 = 0;
		o_ptr->art_flags2 = 0;
		o_ptr->art_flags3 = 0;


		/* Curse it */
		o_ptr->ident |= (IDENT_CURSED);

		/* Break it */
		o_ptr->ident |= (IDENT_BROKEN);

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Recalculate mana */
		p_ptr->update |= (PU_MANA);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);
	}

	/* Notice */
	return (TRUE);
}


/*
 * Enchant some bolts
 */
bool brand_bolts(void)
{
	int i;

	/* Use the first acceptable bolts */
	for (i = 0; i < INVEN_PACK; i++)
	{
		object_type *o_ptr = &inventory[i];

		/* Skip non-bolts */
		if (o_ptr->tval != TV_BOLT) continue;

		/* Skip artifacts and ego-items */
		if (o_ptr->art_name || artifact_p(o_ptr) || ego_item_p(o_ptr))
			continue;

		/* Skip cursed/broken items */
		if (cursed_p(o_ptr) || broken_p(o_ptr)) continue;

		/* Randomize */
		if (randint0(100) < 75) continue;

		/* Message */
#ifdef JP
msg_print("�����ܥ����𤬱�Υ��������ޤ줿��");
#else
		msg_print("Your bolts are covered in a fiery aura!");
#endif


		/* Ego-item */
		o_ptr->name2 = EGO_FLAME;

		/* Enchant */
		enchant(o_ptr, randint0(3) + 4, ENCH_TOHIT | ENCH_TODAM);

		/* Notice */
		return (TRUE);
	}

	/* Flush */
	if (flush_failure) flush();

	/* Fail */
#ifdef JP
msg_print("��Ƕ�������Τ˼��Ԥ�����");
#else
	msg_print("The fiery enchantment failed.");
#endif


	/* Notice */
	return (TRUE);
}


/*
 * Helper function -- return a "nearby" race for polymorphing
 *
 * Note that this function is one of the more "dangerous" ones...
 */
static s16b poly_r_idx(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	int i, r, lev1, lev2;

	/* Hack -- Uniques/Questors never polymorph */
	if ((r_ptr->flags1 & RF1_UNIQUE) ||
	    (r_ptr->flags1 & RF1_QUESTOR))
		return (r_idx);

	/* Allowable range of "levels" for resulting monster */
	lev1 = r_ptr->level - ((randint1(20) / randint1(9)) + 1);
	lev2 = r_ptr->level + ((randint1(20) / randint1(9)) + 1);

	/* Pick a (possibly new) non-unique race */
	for (i = 0; i < 1000; i++)
	{
		/* Pick a new race, using a level calculation */
		r = get_mon_num((dun_level + r_ptr->level) / 2 + 5);

		/* Handle failure */
		if (!r) break;

		/* Obtain race */
		r_ptr = &r_info[r];

		/* Ignore unique monsters */
		if (r_ptr->flags1 & RF1_UNIQUE) continue;

		/* Ignore monsters with incompatible levels */
		if ((r_ptr->level < lev1) || (r_ptr->level > lev2)) continue;

		/* Use that index */
		r_idx = r;

		/* Done */
		break;
	}

	/* Result */
	return (r_idx);
}


bool polymorph_monster(int y, int x)
{
	cave_type *c_ptr = &cave[y][x];
	monster_type *m_ptr = &m_list[c_ptr->m_idx];
	bool friendly, pet;
	bool polymorphed = FALSE;
	int new_r_idx;
	int old_r_idx = m_ptr->r_idx;


	/* Get the monsters attitude */
	friendly = is_friendly(m_ptr);
	pet = is_pet(m_ptr);

	/* Pick a "new" monster race */
	new_r_idx = poly_r_idx(old_r_idx);

	/* Handle polymorph */
	if (new_r_idx != old_r_idx)
	{
		/* "Kill" the "old" monster */
		delete_monster_idx(c_ptr->m_idx);

		/* Create a new monster (no groups) */
		if (place_monster_aux(y, x, new_r_idx, FALSE, FALSE, friendly, pet))
		{
			/* Success */
			polymorphed = TRUE;
		}
		else
		{
			monster_terrain_sensitive = FALSE;

			/* Placing the new monster failed */
			place_monster_aux(y, x, old_r_idx, FALSE, FALSE, friendly, pet);

			monster_terrain_sensitive = TRUE;
		}
	}

	return polymorphed;
}


/*
 * Dimension Door
 */
bool dimension_door(void)
{
	int	plev = p_ptr->lev;
	int	x = 0, y = 0;

	if (!tgt_pt(&x, &y)) return FALSE;

	if (ironman_hengband)
		p_ptr->energy_need += (s16b)((s32b)(60 - plev) * ENERGY_NEED() / 100L);
	else
		p_ptr->energy_need += (60 - plev);

	if (!cave_empty_bold(y, x) || (cave[y][x].info & CAVE_ICKY) ||
		(distance(y, x, py, px) > plev + 2) ||
		(!randint0(plev * plev / 2)))
	{
#ifdef JP
msg_print("�������ʪ�������������ޤ������ʤ��ä���");
#else
		msg_print("You fail to exit the astral plane correctly!");
#endif

		if (ironman_hengband)
			p_ptr->energy_need += ENERGY_NEED();
		else
			p_ptr->energy_need += 100;
		teleport_player(10);
	}
	else teleport_player_to(y, x);

	return (TRUE);
}
