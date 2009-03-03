/* File: melee1.c */

/* Purpose: Monster attacks */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"



#ifdef JP
/* XTRA HACK SILLY_ATTACK */

#define MAX_SILLY_ATTACK 41

static cptr silly_attacks1[MAX_SILLY_ATTACK] =
{
  "�˾��ؤ򤫤���줿��",
  "�����ʤ��β���3���äƥ��ȸ��ä���",
  "�ˤ��Ф��ޤ蘆�줿��",
  "�˷���ʤᤵ����줿��",
  "�˥ϥʥ��������Ф��줿��",
  "�˥������ǹ��⤵�줿��",
  "�����ʤ����ˤ�«�Ǥ��Ф�����",
  "�����ʤ������ǥݡ����󥰤򤷤���",
  "�˥�����١����줿��",
  "�ˡֿ��ι��ȯ����ű������줿��",
  "�˥�á��������ޤ��줿��",
  "�ˤĤä��ߤ������줿��",
  "�Ϥ��ʤ����٤ä���",
  "�˴�ˤ餯�񤭤򤵤줿��",
  "�˼ڶ���ֺѤ򤻤ޤ�줿��",
  "�˥������Ȥ�᤯��줿��",
  "�Ϥ��ʤ��μ������ä���",
  "����������夬��줿��",
  "���鰦�ι���򤵤줿��",
  "�Ϥ��ʤ������500�ߤǸۤä���",
  "�Ϥ��ʤ���100����̩�ˤĤ���Ǯ����ä���",
  "���˥㡼���Ĥ�����",
  "�Ϥ��ʤ��˵���Ĥ�����",
  "�Ϥ��ʤ���ݥꥴ�󲽤�������",
  "�˾���������줿��",
  "�ϥ���ƥޤμ�ʸ�򾧤�����",
  "�Ϥ��ʤ��Υ��ѥ�����֥�å�������",
  "�ϥ��饤�ɰ�ư������",
  "�Ͼ�ζ�����ޥ�ɤ����Ϥ˼��Ԥ�����",
  "�ϳȻ���ưˤ��ȯ�ͤ�����",
  "�ϥǥ��顼��ˡ�򤷤�������",
  "�˥饤�������å��ǹ��⤵�줿��",
  "���󽵴ְ���ǥӥǥ���ͤ˸����ʤ��Ȼ�̼����򤫤���줿��",
  "�ϥѥ�ץ�Ƥ򾧤�����",
  "�ϥ����ѡ�����ȥ饮��饯�ƥ����ޥ��ʥ�����ä���",
  "�ˤ��㤬�߾����å��ǥϥ��줿��",
  "�˥����åȥ��ȥ꡼�ॢ���å��򤫤���줿��",
  "�Ϥ��ʤ����ĸǤ�򤫤��ơ�1��2��3�������á��פȶ������",
  "�ϡ֤������ʤ����Ф��Ф��Ф����פȤ��äƶ�Ф�����",
  "���֤���󡢥롼�٥󥹤γ�����פȸ��ä��Ť����ܤ��Ĥ�����",
  "�ϸ��ä������Ѷ����ܡ��仿�����桪�ס�",
};
#endif
/*
 * Critical blow.  All hits that do 95% of total possible damage,
 * and which also do at least 20 damage, or, sometimes, N damage.
 * This is used only to determine "cuts" and "stuns".
 */
static int monster_critical(int dice, int sides, int dam)
{
	int max = 0;
	int total = dice * sides;

	/* Must do at least 95% of perfect */
	if (dam < total * 19 / 20) return (0);

	/* Weak blows rarely work */
	if ((dam < 20) && (randint0(100) >= dam)) return (0);

	/* Perfect damage */
	if (dam == total) max++;

	/* Super-charge */
	if (dam >= 20)
	{
		while (randint0(100) < 2) max++;
	}

	/* Critical damage */
	if (dam > 160) return (6 + max);
	if (dam > 80) return (5 + max);
	if (dam > 40) return (4 + max);
	if (dam > 20) return (3 + max);
	if (dam > 10) return (2 + max);
	return (1 + max);
}





/*
 * Determine if a monster attack against the player succeeds.
 * Always miss 5% of the time, Always hit 5% of the time.
 * Otherwise, match monster power against player armor.
 */
static int check_hit(int power, int level)
{
	int i, k, ac;

	/* Percentile dice */
	k = randint0(100);

	/* Hack -- Always miss or hit */
	if (k < 10) return (k < 5);

	/* Calculate the "attack quality" */
#ifdef TINYANGBAND
	i = (power + (level * 5));
#else
	i = (power + (level * 3));
#endif
	/* Total armor */
	ac = p_ptr->ac + p_ptr->to_a;

	/* Power and Level compete against Armor */
	if ((i > 0) && (randint1(i) > ((ac * 3) / 4))) return (TRUE);

	/* Assume miss */
	return (FALSE);
}



/*
 * Hack -- possible "insult" messages
 */
static cptr desc_insult[] =
{
#ifdef JP
	"�����ʤ����������",
	"�����ʤ�������������",
	"�����ʤ�����Τ�����",
	"�����ʤ��򿫤᤿��",
	"�����ʤ����������",
	"�����ʤ��β����٤ä���",
	"�������ʿȤ֤�򤷤���",
	"�����ʤ���ܤ���ȸ���������"
#else
	"insults you!",
	"insults your mother!",
	"gives you the finger!",
	"humiliates you!",
	"defiles you!",
	"dances around you!",
	"makes obscene gestures!",
	"moons you!!!"
#endif

};



/*
 * Hack -- possible "insult" messages
 */
static cptr desc_moan[] =
{
#ifdef JP
	"�ϲ������ᤷ��Ǥ���褦����",
	"����λ������򸫤ʤ��ä����ȿҤͤƤ��롣",
	"����ĥ�꤫��ФƹԤ��ȸ��äƤ��롣",
	"�ϥ��Υ����ɤ��Ȥ��줤�Ƥ��롣"
#else
	"seems sad about something.",
	"asks if you have seen his dogs.",
	"tells you to get off his land.",
	"mumbles something about mushrooms."
#endif

};


/*
 * Attack the player via physical attacks.
 */
bool make_attack_normal(int m_idx)
{
	monster_type *m_ptr = &m_list[m_idx];

	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	int ap_cnt;

	int i, k, tmp, ac, rlev;
	int do_cut, do_stun;

	s32b gold;

	object_type *o_ptr;

	char o_name[MAX_NLEN];

	char m_name[80];

	char ddesc[80];

	bool blinked;
	bool touched = FALSE, fear = FALSE, alive = TRUE;
	bool explode = FALSE;
	bool resist_drain = FALSE;

#ifdef JP
	/* 0:�̾�  1:����ά  -1:ǽư��ɽ���Τ����ά�Բ�ǽ */
	int omit_mname = 0;
#endif

	int get_damage = 0;

	/* Not allowed to attack */
	if (r_ptr->flags1 & (RF1_NEVER_BLOW)) return (FALSE);

	/* ...nor if friendly */
	if (!is_hostile(m_ptr)) return FALSE;

	/* Total armor */
	ac = p_ptr->ac + p_ptr->to_a;

	/* Extract the effective monster level */
	rlev = ((r_ptr->level >= 1) ? r_ptr->level : 1);


	/* Get the monster name (or "it") */
	monster_desc(m_name, m_ptr, 0);

	/* Get the "died from" information (i.e. "a kobold") */
	monster_desc(ddesc, m_ptr, 0x88);

	/* Assume no blink */
	blinked = FALSE;

	/* Scan through all four blows */
	for (ap_cnt = 0; ap_cnt < 4; ap_cnt++)
	{
		bool visible = FALSE;
		bool obvious = FALSE;

		int power;
		int damage = 0;

		cptr act = NULL;

		/* Extract the attack infomation */
		int effect = r_ptr->blow[ap_cnt].effect;
		int method = r_ptr->blow[ap_cnt].method;
		int d_dice = r_ptr->blow[ap_cnt].d_dice;
		int d_side = r_ptr->blow[ap_cnt].d_side;


		/* Hack -- no more attacks */
		if (!method) break;


		/* Stop if player is dead or gone */
		if (!alive || death) break;

		/* Handle "leaving" */
		if (p_ptr->leaving) break;

		/* Extract visibility (before blink) */
		if (m_ptr->ml) visible = TRUE;

		/* Extract the attack "power" */
		power = mbe_info[effect].power;

		/* Monster hits player */
		if (!effect || check_hit(power, rlev))
		{
			/* Always disturbing */
			disturb(1, 0);


			/* Hack -- Apply "protection from evil" */
			if ((p_ptr->protevil > 0) &&
			    (r_ptr->flags3 & RF3_EVIL) &&
			    ((5 + randint1(p_ptr->lev)) > rlev))
			{
				/* Remember the Evil-ness */
				if (m_ptr->ml)
				{
					r_ptr->r_flags3 |= RF3_EVIL;
				}

				/* Message */
#ifdef JP
				if (omit_mname)
					msg_format("���ष����");
				else
					msg_format("%^s�Ϸ��व�줿��", m_name);
				omit_mname = 1;/*�����ܰʹߤϾ�ά*/
#else
				msg_format("%^s is repelled.", m_name);
#endif


				/* Hack -- Next attack */
				continue;
			}


			/* Assume no cut or stun */
			do_cut = do_stun = 0;

			/* Describe the attack method */
			switch (method)
			{
				case RBM_HIT:
				{
#ifdef JP
					act = "����줿��";
#else
					act = "hits you.";
#endif

					do_cut = do_stun = 1;
					touched = TRUE;
					sound(SOUND_HIT);
					break;
				}

				case RBM_TOUCH:
				{
#ifdef JP
					act = "����줿��";
#else
					act = "touches you.";
#endif

					touched = TRUE;
					sound(SOUND_TOUCH);
					break;
				}

				case RBM_PUNCH:
				{
#ifdef JP
					act = "�ѥ�����줿��";
#else
					act = "punches you.";
#endif

					touched = TRUE;
					do_stun = 1;
					sound(SOUND_HIT);
					break;
				}

				case RBM_KICK:
				{
#ifdef JP
					act = "����줿��";
#else
					act = "kicks you.";
#endif

					touched = TRUE;
					do_stun = 1;
					sound(SOUND_HIT);
					break;
				}

				case RBM_CLAW:
				{
#ifdef JP
					act = "�Ҥä����줿��";
#else
					act = "claws you.";
#endif

					touched = TRUE;
					do_cut = 1;
					sound(SOUND_CLAW);
					break;
				}

				case RBM_BITE:
				{
#ifdef JP
					act = "���ޤ줿��";
#else
					act = "bites you.";
#endif

					do_cut = 1;
					touched = TRUE;
					sound(SOUND_BITE);
					break;
				}

				case RBM_STING:
				{
#ifdef JP
					act = "�ɤ��줿��";
#else
					act = "stings you.";
#endif

					touched = TRUE;
					sound(SOUND_STING);
					break;
				}

				case RBM_SLASH:
				{
#ifdef JP
					act = "�¤�줿��";
#else
					act = "slashs you.";
#endif

					do_cut = 1;
					touched = TRUE;
					sound(SOUND_HIT);
					break;
				}

				case RBM_BUTT:
				{
#ifdef JP
					act = "�Ѥ��ͤ��줿��";
#else
					act = "butts you.";
#endif

					do_stun = 1;
					touched = TRUE;
					sound(SOUND_HIT);
					break;
				}

				case RBM_CRUSH:
				{
#ifdef JP
					act = "�������ꤵ�줿��";
#else
					act = "crushes you.";
#endif

					do_stun = 1;
					touched = TRUE;
					sound(SOUND_CRUSH);
					break;
				}

				case RBM_ENGULF:
				{
#ifdef JP
					act = "���߹��ޤ줿��";
#else
					act = "engulfs you.";
#endif

					touched = TRUE;
					sound(SOUND_CRUSH);
					break;
				}

				case RBM_CHARGE:
				{
#ifdef JP
				omit_mname = -1;
					act = "��������褳������";
#else
					act = "charges you.";
#endif

					touched = TRUE;
					sound(SOUND_BUY); /* Note! This is "charges", not "charges at". */
					break;
				}

				case RBM_CRAWL:
				{
#ifdef JP
				omit_mname = -1;
					act = "���Τξ���礤��ä���";
#else
					act = "crawls on you.";
#endif

					touched = TRUE;
					sound(SOUND_SLIME);
					break;
				}

				case RBM_DROOL:
				{
#ifdef JP
					act = "�����򤿤餵�줿��";
#else
					act = "drools on you.";
#endif

					sound(SOUND_SLIME);
					break;
				}

				case RBM_SPIT:
				{
#ifdef JP
					act = "�ä��Ǥ��줿��";
#else
					act = "spits on you.";
#endif

					sound(SOUND_SLIME);
					break;
				}

				case RBM_EXPLODE:
				{
#ifdef JP
				omit_mname = -1;
					act = "����ȯ������";
#else
					act = "explodes.";
#endif

					explode = TRUE;
					break;
				}

				case RBM_GAZE:
				{
#ifdef JP
					act = "�ˤ�ޤ줿��";
#else
					act = "gazes at you.";
#endif

					break;
				}

				case RBM_WAIL:
				{
#ifdef JP
					act = "�㤭���Ф줿��";
#else
					act = "wails at you.";
#endif

					sound(SOUND_WAIL);
					break;
				}

				case RBM_SPORE:
				{
#ifdef JP
					act = "˦�Ҥ����Ф��줿��";
#else
					act = "releases spores at you.";
#endif

					sound(SOUND_SLIME);
					break;
				}

				case RBM_XXX4:
				{
#ifdef JP
				omit_mname = -1;
					act = "�� XXX4 ��ȯ�ͤ�����";
#else
					act = "projects XXX4's at you.";
#endif

					break;
				}

				case RBM_BEG:
				{
#ifdef JP
					act = "��򤻤��ޤ줿��";
#else
					act = "begs you for money.";
#endif

					sound(SOUND_MOAN);
					break;
				}

				case RBM_INSULT:
				{
#ifdef JP
				omit_mname = -1;
#endif
					act = desc_insult[randint0(8)];
					sound(SOUND_MOAN);
					break;
				}

				case RBM_MOAN:
				{
#ifdef JP
				omit_mname = -1;
#endif
					act = desc_moan[randint0(4)];
					sound(SOUND_MOAN);
					break;
				}

				case RBM_SHOW:
				{
					if (randint1(3) == 1)
#ifdef JP
						act = "�Ϣ��ͤ�ϳڤ�����²���ȲΤäƤ��롣";
					else
						act = "�Ϣ����� ��� �桼���桼 ��� �ߡ����ȲΤäƤ��롣";
					omit_mname = -1;
#else
						act = "sings 'We are a happy family.'";
					else
						act = "sings 'I love you, you love me.'";
#endif
					sound(SOUND_SHOW);
					break;
				}
			}

			/* Message */
			/* Message */
			if (act)
			{
#ifdef JP
				if ((p_ptr->image) && (randint1(3) != 1))
#else
				if ((p_ptr->image) && (randint1(3) == 1))
#endif

				{
#ifdef JP
					msg_format("%^s%s", m_name,
						   silly_attacks1[randint1(MAX_SILLY_ATTACK) - 1]);
#else
					msg_format("%^s %s you.", m_name,
						   silly_attacks[randint1(MAX_SILLY_ATTACK) - 1]);
#endif

				}
				else
#ifdef JP
				switch (omit_mname){
				/* �̾� */
				case 0: msg_format("%^s��%s", m_name, act); 
						break;
				/* ����ά */
				case 1: msg_format("%s", act); 
						break;
				/* �����ȤǤʤ��������ά�Բ�ǽ */
				case -1: msg_format("%^s%s", m_name, act); 
						break;
				}
				omit_mname = 1;
#else
				msg_format("%^s %s", m_name, act);
#endif
			}


			/* Hack -- assume all attacks are obvious */
			obvious = TRUE;

			/* Roll out the damage */
			damage = damroll(d_dice, d_side);

			/*
			 * Skip the effect when exploding, since the explosion
			 * already causes the effect.
			 */
			if (!explode)
			{
				/* Apply appropriate damage */
				switch (effect)
				{
					case 0:
					{
						/* Hack -- Assume obvious */
						obvious = TRUE;

						/* Hack -- No damage */
						damage = 0;

						break;
					}

					case RBE_HURT:
					{
						/* Obvious */
						obvious = TRUE;

						/* Hack -- Player armor reduces total damage */
						damage -= (damage * ((ac < 150) ? ac : 150) / 250);

						/* Take damage */
						get_damage += take_hit(damage, ddesc);

						break;
					}

					case RBE_POISON:
					{
						/* Take some damage */
						get_damage += take_hit(damage, ddesc);

						/* Take "poison" effect */
						if (!(p_ptr->resist_pois || p_ptr->oppose_pois))
						{
							if (set_poisoned(p_ptr->poisoned + randint1(rlev) + 5))
							{
								obvious = TRUE;
							}
						}

						/* Learn about the player */
						update_smart_learn(m_idx, DRS_POIS);

						break;
					}

					case RBE_UN_BONUS:
					{
						/* Take some damage */
						get_damage += take_hit(damage, ddesc);

						/* Allow complete resist */
						if (!p_ptr->resist_disen)
						{
							/* Apply disenchantment */
							if (apply_disenchant(0)) obvious = TRUE;
						}

						/* Learn about the player */
						update_smart_learn(m_idx, DRS_DISEN);

						break;
					}

					case RBE_UN_POWER:
					{
						/* Take some damage */
						get_damage += take_hit(damage, ddesc);

						/* Find an item */
						for (k = 0; k < 10; k++)
						{
							/* Pick an item */
							i = randint0(INVEN_PACK);

							/* Obtain the item */
							o_ptr = &inventory[i];

							/* Skip non-objects */
							if (!o_ptr->k_idx) continue;

							/* Drain charged wands/staffs */
							if (((o_ptr->tval == TV_STAFF) ||
								 (o_ptr->tval == TV_WAND)) &&
								(o_ptr->pval))
							{
								/* Calculate healed hitpoints */
								int heal = rlev * o_ptr->pval;

								if( o_ptr->tval == TV_STAFF)
									heal *=  o_ptr->number;

								/* Don't heal more than max hp */
								heal = MIN(heal, m_ptr->maxhp - m_ptr->hp);

								/* Message */
#ifdef JP
								msg_print("���å����饨�ͥ륮�����ۤ����줿��");
#else
								msg_print("Energy drains from your pack!");
#endif


								/* Obvious */
								obvious = TRUE;

								/* Heal the monster */
								m_ptr->hp += heal;

								/* Redraw (later) if needed */
								if (p_ptr->health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

								/* Uncharge */
								o_ptr->pval = 0;

								/* Combine / Reorder the pack */
								p_ptr->notice |= (PN_COMBINE | PN_REORDER);

								/* Window stuff */
								p_ptr->window |= (PW_INVEN);

								/* Done */
								break;
							}
						}

						break;
					}

					case RBE_EAT_GOLD:
					{
						/* Take some damage */
						get_damage += take_hit(damage, ddesc);

						/* Confused monsters cannot steal successfully. -LM-*/
						if (m_ptr->confused) break;

						/* Obvious */
						obvious = TRUE;

						/* Saving throw (unless paralyzed) based on dex and level */
						if (!p_ptr->paralyzed &&
							(randint0(100) < (adj_dex_safe[p_ptr->stat_ind[A_DEX]] +
											  p_ptr->lev)))
						{
							/* Saving throw message */
#ifdef JP
						msg_print("���������᤯���ۤ��ä���");
#else
							msg_print("You quickly protect your money pouch!");
#endif


							/* Occasional blink anyway */
							if (randint0(3)) blinked = TRUE;
						}

						/* Eat gold */
						else
						{
							gold = (p_ptr->au / 10) + randint1(25);
							if (gold < 2) gold = 2;
							if (gold > 5000) gold = (p_ptr->au / 20) + randint1(3000);
							if (gold > p_ptr->au) gold = p_ptr->au;
							p_ptr->au -= gold;
							if (gold <= 0)
							{
#ifdef JP
							msg_print("������������ޤ�ʤ��ä���");
#else
								msg_print("Nothing was stolen.");
#endif

							}
							else if (p_ptr->au)
							{
#ifdef JP
							msg_print("���ۤ��ڤ��ʤä��������롣");
							msg_format("$%ld �Τ��⤬��ޤ줿��", (long)gold);
#else
								msg_print("Your purse feels lighter.");
								msg_format("%ld coins were stolen!", (long)gold);
#endif
							}
							else
							{
#ifdef JP
							msg_print("���ۤ��ڤ��ʤä��������롣");
							msg_print("���⤬������ޤ줿��");
#else
								msg_print("Your purse feels lighter.");
								msg_print("All of your coins were stolen!");
#endif
							}

							/* Redraw gold */
							p_ptr->redraw |= (PR_GOLD);

							/* Window stuff */
							p_ptr->window |= (PW_PLAYER);

							/* Blink away */
							blinked = TRUE;
						}

						break;
					}

					case RBE_EAT_ITEM:
					{
						/* Take some damage */
						get_damage += take_hit(damage, ddesc);

						/* Confused monsters cannot steal successfully. -LM-*/
						if (m_ptr->confused) break;

						/* Saving throw (unless paralyzed) based on dex and level */
						if (!p_ptr->paralyzed &&
							(randint0(100) < (adj_dex_safe[p_ptr->stat_ind[A_DEX]] +
											  p_ptr->lev)))
						{
							/* Saving throw message */
#ifdef JP
						msg_print("����������Ƥƥ��å������֤�����");
#else
							msg_print("You grab hold of your backpack!");
#endif


							/* Occasional "blink" anyway */
							blinked = TRUE;

							/* Obvious */
							obvious = TRUE;

							/* Done */
							break;
						}

						/* Find an item */
						for (k = 0; k < 10; k++)
						{
							/* Pick an item */
							i = randint0(INVEN_PACK);

							/* Obtain the item */
							o_ptr = &inventory[i];

							/* Skip non-objects */
							if (!o_ptr->k_idx) continue;

							/* Skip artifacts */
							if (artifact_p(o_ptr) || o_ptr->art_name) continue;

							/* Get a description */
							object_desc(o_name, o_ptr, OD_OMIT_PREFIX);

							/* Message */
#ifdef JP
						msg_format("%s(%c)��%s��ޤ줿��",
							   o_name, index_to_label(i),
							   ((o_ptr->number > 1) ? "���" : ""));
#else
							msg_format("%sour %s (%c) was stolen!",
									   ((o_ptr->number > 1) ? "One of y" : "Y"),
									   o_name, index_to_label(i));
#endif

							{
								s16b o_idx;

								/* Make an object */
								o_idx = o_pop();

								/* Success */
								if (o_idx)
								{
									object_type *j_ptr;

									/* Get new object */
									j_ptr = &o_list[o_idx];

									/* Copy object */
									object_copy(j_ptr, o_ptr);

									/* Modify number */
									j_ptr->number = 1;

									/* Hack -- If a rod or wand, allocate total
									 * maximum timeouts or charges between those
									 * stolen and those missed. -LM-
									 */
									if ((o_ptr->tval == TV_ROD) || (o_ptr->tval == TV_WAND))
									{
										j_ptr->pval = o_ptr->pval / o_ptr->number;
										o_ptr->pval -= j_ptr->pval;
									}

									/* Forget mark */
									j_ptr->marked = OM_TOUCHED;

									/* Memorize monster */
									j_ptr->held_m_idx = m_idx;

									/* Build stack */
									j_ptr->next_o_idx = m_ptr->hold_o_idx;

									/* Build stack */
									m_ptr->hold_o_idx = o_idx;
								}
							}

							/* Steal the items */
							inven_item_increase(i, -1);
							inven_item_optimize(i);

							/* Obvious */
							obvious = TRUE;

							/* Blink away */
							blinked = TRUE;

							/* Done */
							break;
						}

						break;
					}

					case RBE_EAT_FOOD:
					{
						/* Take some damage */
						get_damage += take_hit(damage, ddesc);

						/* Steal some food */
						for (k = 0; k < 10; k++)
						{
							/* Pick an item from the pack */
							i = randint0(INVEN_PACK);

							/* Get the item */
							o_ptr = &inventory[i];

							/* Skip non-objects */
							if (!o_ptr->k_idx) continue;

							/* Skip non-food objects */
							if (o_ptr->tval != TV_FOOD) continue;

							/* Get a description */
							object_desc(o_name, o_ptr, OD_OMIT_PREFIX | OD_NAME_ONLY);

							/* Message */
#ifdef JP
						msg_format("%s(%c)��%s���٤��Ƥ��ޤä���",
							  o_name, index_to_label(i),
							  ((o_ptr->number > 1) ? "���" : ""));
#else
							msg_format("%sour %s (%c) was eaten!",
									   ((o_ptr->number > 1) ? "One of y" : "Y"),
									   o_name, index_to_label(i));
#endif


							/* Steal the items */
							inven_item_increase(i, -1);
							inven_item_optimize(i);

							/* Obvious */
							obvious = TRUE;

							/* Done */
							break;
						}

						break;
					}

					case RBE_EAT_LITE:
					{
						/* Take some damage */
						get_damage += take_hit(damage, ddesc);

						/* Access the lite */
						o_ptr = &inventory[INVEN_LITE];

						/* Drain fuel */
						if ((o_ptr->xtra3 > 0) && (!artifact_p(o_ptr)))
						{
							/* Reduce fuel */
							o_ptr->xtra3 -= (250 + randint1(250));
							if (o_ptr->xtra3 < 1) o_ptr->xtra3 = 1;

							/* Notice */
							if (!p_ptr->blind)
							{
#ifdef JP
							msg_print("�����꤬�Ť��ʤäƤ��ޤä���");
#else
								msg_print("Your light dims.");
#endif

								obvious = TRUE;
							}

							/* Window stuff */
							p_ptr->window |= (PW_EQUIP);
						}

						break;
					}

					case RBE_ACID:
					{
						/* Obvious */
						obvious = TRUE;

						/* Message */
#ifdef JP
					msg_print("������Ӥ���줿��");
#else
						msg_print("You are covered in acid!");
#endif


						/* Special damage */
						get_damage += acid_dam(damage, ddesc);

						/* Learn about the player */
						update_smart_learn(m_idx, DRS_ACID);

						break;
					}

					case RBE_ELEC:
					{
						/* Obvious */
						obvious = TRUE;

						/* Message */
#ifdef JP
					msg_print("�ŷ����Ӥ���줿��");
#else
						msg_print("You are struck by electricity!");
#endif


						/* Special damage */
						get_damage += elec_dam(damage, ddesc);

						/* Learn about the player */
						update_smart_learn(m_idx, DRS_ELEC);

						break;
					}

					case RBE_FIRE:
					{
						/* Obvious */
						obvious = TRUE;

						/* Message */
#ifdef JP
					msg_print("���Ȥ������ޤ줿��");
#else
						msg_print("You are enveloped in flames!");
#endif


						/* Special damage */
						get_damage += fire_dam(damage, ddesc);

						/* Learn about the player */
						update_smart_learn(m_idx, DRS_FIRE);

						break;
					}

					case RBE_COLD:
					{
						/* Obvious */
						obvious = TRUE;

						/* Message */
#ifdef JP
					msg_print("���Ȥ��䵤��ʤ��줿��");
#else
						msg_print("You are covered with frost!");
#endif


						/* Special damage */
						get_damage += cold_dam(damage, ddesc);

						/* Learn about the player */
						update_smart_learn(m_idx, DRS_COLD);

						break;
					}

					case RBE_BLIND:
					{
						/* Take damage */
						get_damage += take_hit(damage, ddesc);

						/* Increase "blind" */
						if (!p_ptr->resist_blind)
						{
							if (set_blind(p_ptr->blind + 10 + randint1(rlev)))
							{
								obvious = TRUE;
							}
						}

						/* Learn about the player */
						update_smart_learn(m_idx, DRS_BLIND);

						break;
					}

					case RBE_CONFUSE:
					{
						/* Take damage */
						get_damage += take_hit(damage, ddesc);

						/* Increase "confused" */
						if (!p_ptr->resist_conf)
						{
							if (set_confused(p_ptr->confused + 3 + randint1(rlev)))
							{
								obvious = TRUE;
							}
						}

						/* Learn about the player */
						update_smart_learn(m_idx, DRS_CONF);

						break;
					}

					case RBE_TERRIFY:
					{
						/* Take damage */
						get_damage += take_hit(damage, ddesc);

						/* Increase "afraid" */
						if (p_ptr->resist_fear)
						{
#ifdef JP
						msg_print("���������ݤ˿�����ʤ��ä���");
#else
							msg_print("You stand your ground!");
#endif

							obvious = TRUE;
						}
						else if (randint0(100) < p_ptr->skill_sav)
						{
#ifdef JP
						msg_print("���������ݤ˿�����ʤ��ä���");
#else
							msg_print("You stand your ground!");
#endif

							obvious = TRUE;
						}
						else
						{
							if (set_afraid(p_ptr->afraid + 3 + randint1(rlev)))
							{
								obvious = TRUE;
							}
						}

						/* Learn about the player */
						update_smart_learn(m_idx, DRS_FEAR);

						break;
					}

					case RBE_PARALYZE:
					{
						/* Hack -- Prevent perma-paralysis via damage */
						if (p_ptr->paralyzed && (damage < 1)) damage = 1;

						/* Take damage */
						get_damage += take_hit(damage, ddesc);

						/* Increase "paralyzed" */
						if (p_ptr->free_act)
						{
#ifdef JP
						msg_print("���������̤��ʤ��ä���");
#else
							msg_print("You are unaffected!");
#endif

							obvious = TRUE;
						}
						else if (randint0(100) < p_ptr->skill_sav)
						{
#ifdef JP
						msg_print("���������Ϥ�ķ���֤�����");
#else
							msg_print("You resist the effects!");
#endif

							obvious = TRUE;
						}
						else
						{
							if (set_paralyzed(p_ptr->paralyzed + 3 + randint1(rlev)))
							{
								obvious = TRUE;
							}
						}

						/* Learn about the player */
						update_smart_learn(m_idx, DRS_FREE);

						break;
					}

					case RBE_LOSE_STR:
					{
						/* Damage (physical) */
						get_damage += take_hit(damage, ddesc);

						/* Damage (stat) */
						if (do_dec_stat(A_STR)) obvious = TRUE;

						break;
					}

					case RBE_LOSE_INT:
					{
						/* Damage (physical) */
						get_damage += take_hit(damage, ddesc);

						/* Damage (stat) */
						if (do_dec_stat(A_INT)) obvious = TRUE;

						break;
					}

					case RBE_LOSE_WIS:
					{
						/* Damage (physical) */
						get_damage += take_hit(damage, ddesc);

						/* Damage (stat) */
						if (do_dec_stat(A_WIS)) obvious = TRUE;

						break;
					}

					case RBE_LOSE_DEX:
					{
						/* Damage (physical) */
						get_damage += take_hit(damage, ddesc);

						/* Damage (stat) */
						if (do_dec_stat(A_DEX)) obvious = TRUE;

						break;
					}

					case RBE_LOSE_CON:
					{
						/* Damage (physical) */
						get_damage += take_hit(damage, ddesc);

						/* Damage (stat) */
						if (do_dec_stat(A_CON)) obvious = TRUE;

						break;
					}

					case RBE_LOSE_CHR:
					{
						/* Damage (physical) */
						get_damage += take_hit(damage, ddesc);

						/* Damage (stat) */
						if (do_dec_stat(A_CHR)) obvious = TRUE;

						break;
					}

					case RBE_LOSE_ALL:
					{
						/* Damage (physical) */
						get_damage += take_hit(damage, ddesc);

						/* Damage (stats) */
						if (do_dec_stat(A_STR)) obvious = TRUE;
						if (do_dec_stat(A_DEX)) obvious = TRUE;
						if (do_dec_stat(A_CON)) obvious = TRUE;
						if (do_dec_stat(A_INT)) obvious = TRUE;
						if (do_dec_stat(A_WIS)) obvious = TRUE;
						if (do_dec_stat(A_CHR)) obvious = TRUE;

						break;
					}

					case RBE_SHATTER:
					{
						/* Obvious */
						obvious = TRUE;

						/* Hack -- Reduce damage based on the player armor class */
						damage -= (damage * ((ac < 150) ? ac : 150) / 250);

						/* Take damage */
						get_damage += take_hit(damage, ddesc);

						/* Radius 8 earthquake centered at the monster */
						if (damage > 23)
						{
							earthquake_aux(m_ptr->fy, m_ptr->fx, 8, m_idx);
						}

						break;
					}

					case RBE_EXP_10:
					{
						/* Obvious */
						obvious = TRUE;

						/* Take damage */
						get_damage += take_hit(damage, ddesc);

						if (p_ptr->hold_life && (randint0(100) < 95))
						{
#ifdef JP
						msg_print("���������ʤ���̿�Ϥ��꤭�ä���");
#else
							msg_print("You keep hold of your life force!");
#endif

						}
						else
						{
							s32b d = damroll(10, 6) + (p_ptr->exp/100) * MON_DRAIN_LIFE;
							if (p_ptr->hold_life)
							{
#ifdef JP
							msg_print("��̿�Ϥ򾯤��ۤ����줿�������롪");
#else
								msg_print("You feel your life slipping away!");
#endif

								lose_exp(d/10);
							}
							else
							{
#ifdef JP
							msg_print("��̿�Ϥ��Τ���ۤ����줿�������롪");
#else
								msg_print("You feel your life draining away!");
#endif

								lose_exp(d);
							}
						}
						break;
					}

					case RBE_EXP_20:
					{
						/* Obvious */
						obvious = TRUE;

						/* Take damage */
						get_damage += take_hit(damage, ddesc);

						if (p_ptr->hold_life && (randint0(100) < 90))
						{
#ifdef JP
						msg_print("���������ʤ���̿�Ϥ��꤭�ä���");
#else
							msg_print("You keep hold of your life force!");
#endif

						}
						else
						{
							s32b d = damroll(20, 6) + (p_ptr->exp/100) * MON_DRAIN_LIFE;
							if (p_ptr->hold_life)
							{
#ifdef JP
							msg_print("��̿�Ϥ򾯤��ۤ����줿�������롪");
#else
								msg_print("You feel your life slipping away!");
#endif

								lose_exp(d/10);
							}
							else
							{
#ifdef JP
							msg_print("��̿�Ϥ��Τ���ۤ����줿�������롪");
#else
								msg_print("You feel your life draining away!");
#endif

								lose_exp(d);
							}
						}
						break;
					}

					case RBE_EXP_40:
					{
						/* Obvious */
						obvious = TRUE;

						/* Take damage */
						get_damage += take_hit(damage, ddesc);

						if (p_ptr->hold_life && (randint0(100) < 75))
						{
#ifdef JP
						msg_print("���������ʤ���̿�Ϥ��꤭�ä���");
#else
							msg_print("You keep hold of your life force!");
#endif

						}
						else
						{
							s32b d = damroll(40, 6) + (p_ptr->exp/100) * MON_DRAIN_LIFE;
							if (p_ptr->hold_life)
							{
#ifdef JP
							msg_print("��̿�Ϥ򾯤��ۤ����줿�������롪");
#else
								msg_print("You feel your life slipping away!");
#endif

								lose_exp(d/10);
							}
							else
							{
#ifdef JP
							msg_print("��̿�Ϥ��Τ���ۤ����줿�������롪");
#else
								msg_print("You feel your life draining away!");
#endif

								lose_exp(d);
							}
						}
						break;
					}

					case RBE_EXP_80:
					{
						/* Obvious */
						obvious = TRUE;

						/* Take damage */
						get_damage += take_hit(damage, ddesc);

						if (p_ptr->hold_life && (randint0(100) < 50))
						{
#ifdef JP
						msg_print("���������ʤ���̿�Ϥ��꤭�ä���");
#else
							msg_print("You keep hold of your life force!");
#endif

						}
						else
						{
							s32b d = damroll(80, 6) + (p_ptr->exp/100) * MON_DRAIN_LIFE;
							if (p_ptr->hold_life)
							{
#ifdef JP
							msg_print("��̿�Ϥ򾯤��ۤ����줿�������롪");
#else
								msg_print("You feel your life slipping away!");
#endif

								lose_exp(d/10);
							}
							else
							{
#ifdef JP
							msg_print("��̿�Ϥ��Τ���ۤ����줿�������롪");
#else
								msg_print("You feel your life draining away!");
#endif

								lose_exp(d);
							}
						}
						break;
					}

					case RBE_DISEASE:
					{
						/* Take some damage */
						get_damage += take_hit(damage, ddesc);

						/* Take "poison" effect */
						if (!(p_ptr->resist_pois || p_ptr->oppose_pois))
						{
							if (set_poisoned(p_ptr->poisoned + randint1(rlev) + 5))
							{
								obvious = TRUE;
							}
						}

						/* Damage CON (10% chance) */
						if (randint1(100) < 11)
						{
							/* 1% chance for perm. damage */
							bool perm = (randint1(10) == 1);
							if (dec_stat(A_CON, randint1(10), perm))
							{
#ifdef JP
								msg_print("�¤����ʤ��򿪤�Ǥ��뵤�����롣");
#else
								msg_print("You feel strange sickness.");
#endif
								obvious = TRUE;
							}
						}

						break;
					}
					case RBE_TIME:
					{
						switch (randint1(10))
						{
							case 1: case 2: case 3: case 4: case 5:
							{
#ifdef JP
							msg_print("����������ꤷ���������롣");
#else
								msg_print("You feel life has clocked back.");
#endif

								lose_exp(100 + (p_ptr->exp / 100) * MON_DRAIN_LIFE);
								break;
							}

							case 6: case 7: case 8: case 9:
							{
								int stat = randint0(6);

								switch (stat)
								{
#ifdef JP
								case A_STR: act = "����"; break;
								case A_INT: act = "������"; break;
								case A_WIS: act = "������"; break;
								case A_DEX: act = "���Ѥ�"; break;
								case A_CON: act = "�򹯤�"; break;
								case A_CHR: act = "������"; break;
#else
									case A_STR: act = "strong"; break;
									case A_INT: act = "bright"; break;
									case A_WIS: act = "wise"; break;
									case A_DEX: act = "agile"; break;
									case A_CON: act = "hale"; break;
									case A_CHR: act = "beautiful"; break;
#endif

								}

#ifdef JP
							msg_format("���ʤ��ϰ����ۤ�%s�ʤ��ʤäƤ��ޤä�...��", act);
#else
								msg_format("You're not as %s as you used to be...", act);
#endif


								p_ptr->stat_cur[stat] = (p_ptr->stat_cur[stat] * 3) / 4;
								if (p_ptr->stat_cur[stat] < 3) p_ptr->stat_cur[stat] = 3;
								p_ptr->update |= (PU_BONUS);
								break;
							}

							case 10:
							{
#ifdef JP
					msg_print("���ʤ��ϰ����ۤ��϶����ʤ��ʤäƤ��ޤä�...��");
#else
								msg_print("You're not as powerful as you used to be...");
#endif


								for (k = 0; k < 6; k++)
								{
									p_ptr->stat_cur[k] = (p_ptr->stat_cur[k] * 3) / 4;
									if (p_ptr->stat_cur[k] < 3) p_ptr->stat_cur[k] = 3;
								}
								p_ptr->update |= (PU_BONUS);
								break;
							}
						}
						get_damage += take_hit(damage, ddesc);

						break;
					}
					case RBE_EXP_VAMP:
					{
						/* Obvious */
						obvious = TRUE;

						/* Take damage */
						get_damage += take_hit(damage, ddesc);

						if (p_ptr->hold_life && (randint0(100) < 50))
						{
#ifdef JP
msg_print("���������ʤ���̿�Ϥ��꤭�ä���");
#else
							msg_print("You keep hold of your life force!");
#endif

							resist_drain = TRUE;
						}
						else
						{
							s32b d = damroll(60, 6) + (p_ptr->exp / 100) * MON_DRAIN_LIFE;
							if (p_ptr->hold_life)
							{
#ifdef JP
msg_print("��̿�Ϥ������Τ���ȴ��������������롪");
#else
								msg_print("You feel your life slipping away!");
#endif

								lose_exp(d / 10);
							}
							else
							{
#ifdef JP
msg_print("��̿�Ϥ��Τ���ۤ����줿�������롪");
#else
								msg_print("You feel your life draining away!");
#endif

								lose_exp(d);
							}
						}

						/* Heal the attacker? */
#if 0
						if (!(p_ptr->prace == RACE_VAMPIRE) &&
							(damage > 2) && !(resist_drain))
#else
						if ((damage > 2) && !(resist_drain))
#endif
						{
							bool did_heal = FALSE;

							if (m_ptr->hp < m_ptr->maxhp) did_heal = TRUE;

							/* Heal */
							m_ptr->hp += damroll(4, damage / 6);
							if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

							/* Redraw (later) if needed */
							if (p_ptr->health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

							/* Special message */
							if ((m_ptr->ml) && (did_heal))
							{
#ifdef JP
								msg_format("%s�����Ϥ���������褦����", m_name);
#else
								msg_format("%^s appears healthier.", m_name);
#endif
							}
						}
					}
				}
			}

			/* Hack -- only one of cut or stun */
			if (do_cut && do_stun)
			{
				/* Cancel cut */
				if (randint0(100) < 50)
				{
					do_cut = 0;
				}

				/* Cancel stun */
				else
				{
					do_stun = 0;
				}
			}

			/* Handle cut */
			if (do_cut)
			{
				int k;

				/* Critical hit (zero if non-critical) */
				tmp = monster_critical(d_dice, d_side, damage);

				/* Roll for damage */
				switch (tmp)
				{
					case 0: k = 0; break;
					case 1: k = randint1(5); break;
					case 2: k = randint1(5) + 5; break;
					case 3: k = randint1(20) + 20; break;
					case 4: k = randint1(50) + 50; break;
					case 5: k = randint1(100) + 100; break;
					case 6: k = 300; break;
					default: k = 500; break;
				}

				/* Apply the cut */
				if (k) (void)set_cut(p_ptr->cut + k);
			}

			/* Handle stun */
			if (do_stun)
			{
				int k;

				/* Critical hit (zero if non-critical) */
				tmp = monster_critical(d_dice, d_side, damage);

				/* Roll for damage */
				switch (tmp)
				{
					case 0: k = 0; break;
					case 1: k = randint1(5); break;
					case 2: k = randint1(10) + 10; break;
					case 3: k = randint1(20) + 20; break;
					case 4: k = randint1(30) + 30; break;
					case 5: k = randint1(40) + 40; break;
					case 6: k = 100; break;
					default: k = 200; break;
				}

				/* Apply the stun */
				if (k) (void)set_stun(p_ptr->stun + k);
			}

			if (explode)
			{
				sound(SOUND_EXPLODE);

				if (mon_take_hit(m_idx, m_ptr->hp + 1, &fear, NULL))
				{
					blinked = FALSE;
					alive = FALSE;
				}
			}

			if (touched)
			{
				if (p_ptr->sh_fire && alive)
				{
					if (!(r_ptr->flags3 & RF3_IM_FIRE))
					{
						int dam = damroll(2, 6);

						/* Modify the damage */
						dam = mon_damage_mod(m_ptr, dam, 0);

#ifdef JP
						msg_format("%^s������Ǯ���ʤä���", m_name);
						if (mon_take_hit(m_idx, dam, &fear,
						    "�ϳ��λ��ˤʤä���"))
#else
						msg_format("%^s is suddenly very hot!", m_name);

						if (mon_take_hit(m_idx, dam, &fear,
						    " turns into a pile of ash."))
#endif

						{
							blinked = FALSE;
							alive = FALSE;
						}
					}
					else
					{
						if (m_ptr->ml)
							r_ptr->r_flags3 |= RF3_IM_FIRE;
					}
				}

				if (p_ptr->sh_elec && alive)
				{
					if (!(r_ptr->flags3 & RF3_IM_ELEC))
					{
						int dam = damroll(2, 6);

						/* Modify the damage */
						dam = mon_damage_mod(m_ptr, dam, 0);

#ifdef JP
						msg_format("%^s���ŷ�򤯤�ä���", m_name);
						if (mon_take_hit(m_idx, dam, &fear,
						    "��ǳ���̤λ��ˤʤä���"))
#else
						msg_format("%^s gets zapped!", m_name);

						if (mon_take_hit(m_idx, dam, &fear,
						    " turns into a pile of cinder."))
#endif

						{
							blinked = FALSE;
							alive = FALSE;
						}
					}
					else
					{
						if (m_ptr->ml)
							r_ptr->r_flags3 |= RF3_IM_ELEC;
					}
				}

				if (p_ptr->sh_cold && alive)
				{
					if (!(r_ptr->flags3 & RF3_IM_COLD))
					{
						int dam = damroll(2, 6);

						/* Modify the damage */
						dam = mon_damage_mod(m_ptr, dam, 0);

#ifdef JP
						msg_format("%^s�������䤿���ʤä���", m_name);
						if (mon_take_hit(m_idx, dam, &fear,
						    "�����Ĥ�����"))
#else
						msg_format("%^s is suddenly very cold!", m_name);

						if (mon_take_hit(m_idx, dam, &fear,
						    " freezes."))
#endif

						{
							blinked = FALSE;
							alive = FALSE;
						}
					}
					else
					{
						if (m_ptr->ml)
							r_ptr->r_flags3 |= RF3_IM_COLD;
					}
				}
				touched = FALSE;
			}
		}

		/* Monster missed player */
		else
		{
			/* Analyze failed attacks */
			switch (method)
			{
				case RBM_HIT:
				case RBM_TOUCH:
				case RBM_PUNCH:
				case RBM_KICK:
				case RBM_CLAW:
				case RBM_BITE:
				case RBM_STING:
				case RBM_SLASH:
				case RBM_BUTT:
				case RBM_CRUSH:
				case RBM_ENGULF:
				case RBM_CHARGE:

				/* Visible monsters */
				if (m_ptr->ml)
				{
					/* Disturbing */
					disturb(1, 0);

					/* Message */
#ifdef JP
					if (omit_mname)
					    msg_print("���路����");
					else
					    msg_format("%^s�ι���򤫤路����", m_name);
					omit_mname = 1;/*�����ܰʹߤϾ�ά*/
#else
					msg_format("%^s misses you.", m_name);
#endif

				}

				break;
			}
		}


		/* Analyze "visible" monsters only */
		if (visible)
		{
			/* Count "obvious" attacks (and ones that cause damage) */
			if (obvious || damage || (r_ptr->r_blows[ap_cnt] > 10))
			{
				/* Count attacks of this type */
				if (r_ptr->r_blows[ap_cnt] < MAX_UCHAR)
				{
					r_ptr->r_blows[ap_cnt]++;
				}
			}
		}
	}

	/* Blink away */
	if (blinked)
	{
#ifdef JP
		msg_print("ť���ϾФä�ƨ������");
#else
		msg_print("The thief flees laughing!");
#endif
		teleport_away(m_idx, MAX_SIGHT * 2 + 5);
	}


	/* Always notice cause of death */
	if (death && (r_ptr->r_deaths < MAX_SHORT))
	{
		r_ptr->r_deaths++;
	}

	if (m_ptr->ml && fear)
	{
		sound(SOUND_FLEE);
#ifdef JP
		msg_format("%^s�϶��ݤ�ƨ���Ф�����", m_name);
#else
		msg_format("%^s flees in terror!", m_name);
#endif

	}

	/* Assume we attacked */
	return (TRUE);
}


