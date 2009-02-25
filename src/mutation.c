/* File: mutation.c */

/* Purpose: Mutation effects (and racial powers) */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"


bool gain_random_mutation(int choose_mut)
{
	int     attempts_left = 20;
	cptr    muta_desc = "";
	bool    muta_chosen = FALSE;
	u32b    muta_which = 0;
	u32b    *muta_class = NULL;

	if (choose_mut) attempts_left = 1;

	while (attempts_left--)
	{
		switch (choose_mut ? choose_mut : randint1(65))
		{
		case 1: case 2:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_BERS_RAGE;
#ifdef JP
			muta_desc = "���ʤ��϶�˽����ȯ��򵯤����褦�ˤʤä���";
#else
			muta_desc = "You become subject to fits of berserk rage!";
#endif
			break;
		case 3: case 4:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_COWARDICE;
#ifdef JP
			muta_desc = "�������ʤ����餤���¤ˤʤä���";
#else
			muta_desc = "You become an incredible coward!";
#endif
			break;
		case 5: case 6:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_HALLU;
#ifdef JP
			muta_desc = "���ʤ��ϸ��Ф������������������˿�����Ƥ��롣";
#else
			muta_desc = "You are afflicted by a hallucinatory insanity!";
#endif
			break;
		case 7:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_NORMALITY;
#ifdef JP
			muta_desc = "���ʤ��ϴ�̯�ʤۤ����̤ˤʤä��������롣";
#else
			muta_desc = "You feel strangely normal.";
#endif
			break;
		case 8: case 9: case 10:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_WASTING;
#ifdef JP
			muta_desc = "���ʤ������������ޤ�������¤ˤ����ä���";
#else
			muta_desc = "You suddenly contract a horrible wasting disease.";
#endif
			break;
		case 11:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_RES_TIME;
#ifdef JP
			muta_desc = "�Ի�ˤʤä���ʬ�����롣";
#else
			muta_desc = "You feel immortal.";
#endif
			break;
		case 12: case 13:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_WARNING;
#ifdef JP
			muta_desc = "���ʤ��������ѥ�Υ����ˤʤä��������롣";
#else
			muta_desc = "You suddenly feel paranoid.";
#endif
			break;
		case 14: case 15: case 16:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_HYPER_STR;
#ifdef JP
			muta_desc = "Ķ��Ū�˶����ʤä���";
#else
			muta_desc = "You turn into a superhuman he-man!";
#endif
			break;
		case 17: case 18: case 19:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_PUNY;
#ifdef JP
			muta_desc = "��������äƤ��ޤä�...";
#else
			muta_desc = "Your muscles wither away...";
#endif
			break;
		case 20: case 21:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_HYPER_INT;
#ifdef JP
			muta_desc = "���ʤ���Ǿ�����Υ���ԥ塼���˿ʲ�������";
#else
			muta_desc = "Your brain evolves into a living computer!";
#endif
			break;
		case 22: case 23:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_MORONIC;
#ifdef JP
			muta_desc = "Ǿ����̤��Ƥ��ޤä�...";
#else
			muta_desc = "Your brain withers away...";
#endif
			break;
		case 24: case 25:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_RESILIENT;
#ifdef JP
			muta_desc = "�³���ƥ��դˤʤä���";
#else
			muta_desc = "You become extraordinarily resilient.";
#endif
			break;
		case 26: case 27: case 28:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_XTRA_FAT;
#ifdef JP
			muta_desc = "���ʤ��ϵ������������餤���ä���";
#else
			muta_desc = "You become sickeningly fat!";
#endif
			break;
		case 29: case 30: case 31:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_ALBINO;
#ifdef JP
			muta_desc = "����ӥΤˤʤä����夯�ʤä���������...";
#else
			muta_desc = "You turn into an albino! You feel frail...";
#endif
			break;
		case 32: case 33: case 34:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_FLESH_ROT;
#ifdef JP
			muta_desc = "���ʤ������Τ����Ԥ����µ��˿����줿��";
#else
			muta_desc = "Your flesh is afflicted by a rotting disease!";
#endif
			break;
		case 35: case 36:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_SILLY_VOI;
#ifdef JP
			muta_desc = "������ȴ���ʥ����������ˤʤä���";
#else
			muta_desc = "Your voice turns into a ridiculous squeak!";
#endif
			break;
		case 37: case 38:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_ILL_NORM;
#ifdef JP
			muta_desc = "���ΰ¤餰���Ƥ�Ǥ��Ф��褦�ˤʤä���";
#else
			muta_desc = "You start projecting a reassuring image.";
#endif
			break;
		case 39: case 40: case 41:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_XTRA_EYES;
#ifdef JP
			muta_desc = "��������Ĥ��ܤ����褿��";
#else
			muta_desc = "You grow an extra pair of eyes!";
#endif
			break;
		case 42: case 43:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_XTRA_LEGS;
#ifdef JP
			muta_desc = "���������ܤ�­�������Ƥ�����";
#else
			muta_desc = "You grow an extra pair of legs!";
#endif
			break;
		case 44: case 45: case 46:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_SHORT_LEG;
#ifdef JP
			muta_desc = "­��û���͵��ˤʤäƤ��ޤä���";
#else
			muta_desc = "Your legs turn into short stubs!";
#endif
			break;
		case 47:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_ELEC_TOUC;
#ifdef JP
			muta_desc = "��ɤ���ή��ή��Ϥ᤿��";
#else
			muta_desc = "Electricity starts running through you!";
#endif
			break;
		case 48:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_FIRE_BODY;
#ifdef JP
			muta_desc = "���ʤ����Τϱ�ˤĤĤޤ�Ƥ��롣";
#else
			muta_desc = "Your body is enveloped in flames!";
#endif
			break;
		case 49: case 50: case 51:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_SCALES;
#ifdef JP
			muta_desc = "ȩ�������ڤ��Ѥ�ä���";
#else
			muta_desc = "Your skin turns into black scales!";
#endif
			break;
		case 52:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_IRON_SKIN;
#ifdef JP
			muta_desc = "���ʤ���ȩ��Ŵ�ˤʤä���";
#else
			muta_desc = "Your skin turns to steel!";
#endif
			break;
		case 53: case 54:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_WINGS;
#ifdef JP
			muta_desc = "����˱�����������";
#else
			muta_desc = "You grow a pair of wings.";
#endif
			break;
		case 55:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_FEARLESS;
#ifdef JP
			muta_desc = "�������ݤ��Τ餺�ˤʤä���";
#else
			muta_desc = "You become completely fearless.";
#endif
			break;
		case 56:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_REGEN;
#ifdef JP
			muta_desc = "��®�˲������Ϥ᤿��";
#else
			muta_desc = "You start regenerating.";
#endif
			break;
		case 57:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_ESP;
#ifdef JP
			muta_desc = "�ƥ�ѥ�����ǽ�Ϥ�������";
#else
			muta_desc = "You develop a telepathic ability!";
#endif
			break;
		case 58: case 59:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_LIMBER;
#ifdef JP
			muta_desc = "���������ʤ䤫�ˤʤä���";
#else
			muta_desc = "Your muscles become limber.";
#endif
			break;
		case 60: case 61: case 62:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_ARTHRITIS;
#ifdef JP
			muta_desc = "���᤬�����ˤ߽Ф�����";
#else
			muta_desc = "Your joints suddenly hurt.";
#endif
			break;
		case 63:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_VULN_ELEM;
#ifdef JP
			muta_desc = "̯��̵�����ˤʤä��������롣";
#else
			muta_desc = "You feel strangely exposed.";
#endif
			break;
		case 64: case 65:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_MOTION;
#ifdef JP
			muta_desc = "�Τ�ư�������Τˤʤä���";
#else
			muta_desc = "You move with new assurance.";
#endif
			break;
		default:
			muta_class = NULL;
			muta_which = 0;
		}

		if (muta_class && muta_which)
		{
			if (!(*muta_class & muta_which))
			{
				muta_chosen = TRUE;
			}
		}
		if (muta_chosen) break;
	}

	if (!muta_chosen)
	{
#ifdef JP
		msg_print("���̤ˤʤä��������롣");
#else
		msg_print("You feel normal.");
#endif
		return FALSE;
	}
	else
	{
#ifdef JP
		msg_print("�����Ѱۤ�����");
#else
		msg_print("You mutate!");
#endif
		msg_print(muta_desc);
		*muta_class |= muta_which;

		if (muta_class == &(p_ptr->muta))
		{
			if (muta_which == MUT_PUNY)
			{
				if (p_ptr->muta & MUT_HYPER_STR)
				{
#ifdef JP
					msg_print("���ʤ��Ϥ⤦Ķ��Ū�˶����Ϥʤ���");
#else
					msg_print("You no longer feel super-strong!");
#endif
					p_ptr->muta &= ~(MUT_HYPER_STR);
				}
			}
			else if (muta_which == MUT_HYPER_STR)
			{
				if (p_ptr->muta & MUT_PUNY)
				{
#ifdef JP
					msg_print("���ʤ��Ϥ⤦����ǤϤʤ���");
#else
					msg_print("You no longer feel puny!");
#endif
					p_ptr->muta &= ~(MUT_PUNY);
				}
			}
			else if (muta_which == MUT_MORONIC)
			{
				if (p_ptr->muta & MUT_HYPER_INT)
				{
#ifdef JP
					msg_print("���ʤ���Ǿ�Ϥ⤦���Υ���ԥ塼���ǤϤʤ���");
#else
					msg_print("Your brain is no longer a living computer.");
#endif
					p_ptr->muta &= ~(MUT_HYPER_INT);
				}
			}
			else if (muta_which == MUT_HYPER_INT)
			{
				if (p_ptr->muta & MUT_MORONIC)
				{
#ifdef JP
					msg_print("���ʤ��Ϥ⤦��������ǤϤʤ���");
#else
					msg_print("You are no longer moronic.");
#endif
					p_ptr->muta &= ~(MUT_MORONIC);
				}
			}
			else if (muta_which == MUT_IRON_SKIN)
			{
				if (p_ptr->muta & MUT_SCALES)
				{
#ifdef JP
					msg_print("�ڤ��ʤ��ʤä���");
#else
					msg_print("You lose your scales.");
#endif
					p_ptr->muta &= ~(MUT_SCALES);
				}
				if (p_ptr->muta & MUT_FLESH_ROT)
				{
#ifdef JP
					msg_print("���Τ����𤷤ʤ��ʤä���");
#else
					msg_print("Your flesh rots no longer.");
#endif
					p_ptr->muta &= ~(MUT_FLESH_ROT);
				}
			}
			else if ((muta_which == MUT_SCALES) ||
				 (muta_which == MUT_FLESH_ROT))
			{
				if (p_ptr->muta & MUT_IRON_SKIN)
				{
#ifdef JP
					msg_print("���ʤ���ȩ�Ϥ⤦Ŵ�ǤϤʤ���");
#else
					msg_print("Your skin is no longer made of steel.");
#endif
					p_ptr->muta &= ~(MUT_IRON_SKIN);
				}
			}
			else if (muta_which == MUT_FEARLESS)
			{
				if (p_ptr->muta & MUT_COWARDICE)
				{
#ifdef JP
					msg_print("���¤Ǥʤ��ʤä���");
#else
					msg_print("You are no longer cowardly.");
#endif
					p_ptr->muta &= ~(MUT_COWARDICE);
				}
			}
			else if (muta_which == MUT_FLESH_ROT)
			{
				if (p_ptr->muta & MUT_REGEN)
				{
#ifdef JP
					msg_print("��®�˲������ʤ��ʤä���");
#else
					msg_print("You stop regenerating.");
#endif
					p_ptr->muta &= ~(MUT_REGEN);
				}
			}
			else if (muta_which == MUT_REGEN)
			{
				if (p_ptr->muta & MUT_FLESH_ROT)
				{
#ifdef JP
					msg_print("���Τ����𤷤ʤ��ʤä���");
#else
					msg_print("Your flesh stops rotting.");
#endif
					p_ptr->muta &= ~(MUT_FLESH_ROT);
				}
			}
			else if (muta_which == MUT_LIMBER)
			{
				if (p_ptr->muta & MUT_ARTHRITIS)
				{
#ifdef JP
					msg_print("���᤬�ˤ��ʤ��ʤä���");
#else
					msg_print("Your joints stop hurting.");
#endif
					p_ptr->muta &= ~(MUT_ARTHRITIS);
				}
			}
			else if (muta_which == MUT_ARTHRITIS)
			{
				if (p_ptr->muta & MUT_LIMBER)
				{
#ifdef JP
					msg_print("���ʤ��Ϥ��ʤ䤫�Ǥʤ��ʤä���");
#else
					msg_print("You no longer feel limber.");
#endif
					p_ptr->muta &= ~(MUT_LIMBER);
				}
			}
			else if (muta_which == MUT_COWARDICE)
			{
				if (p_ptr->muta & MUT_FEARLESS)
				{
#ifdef JP
					msg_print("�����Τ餺�Ǥʤ��ʤä���");
#else
					msg_print("You no longer feel fearless.");
#endif
					p_ptr->muta &= ~(MUT_FEARLESS);
				}
			}
		}

		mutant_regenerate_mod = calc_mutant_regenerate_mod();
		p_ptr->update |= PU_BONUS;
		handle_stuff();
		return TRUE;
	}
}


bool lose_mutation(int choose_mut)
{
	int attempts_left = 20;
	cptr muta_desc = "";
	bool muta_chosen = FALSE;
	u32b muta_which = 0;
	u32b *muta_class = NULL;

	if (choose_mut) attempts_left = 1;

	while (attempts_left--)
	{
		switch (choose_mut ? choose_mut : randint1(75))
		{
		case 1: case 2:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_BERS_RAGE;
#ifdef JP
			muta_desc = "��˽����ȯ��ˤ��餵��ʤ��ʤä���";
#else
			muta_desc = "You are no longer subject to fits of berserk rage!";
#endif
			break;
		case 3: case 4:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_COWARDICE;
#ifdef JP
			muta_desc = "�⤦�����������ۤɲ��¤ǤϤʤ��ʤä���";
#else
			muta_desc = "You are no longer an incredible coward!";
#endif
			break;
		case 5: case 6:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_HALLU;
#ifdef JP
			muta_desc = "���Ф�Ҥ������������㳲�򵯤����ʤ��ʤä���";
#else
			muta_desc = "You are no longer afflicted by a hallucinatory insanity!";
#endif
			break;
		case 7:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_NORMALITY;
#ifdef JP
			muta_desc = "���̤˴�̯�ʴ��������롣";
#else
			muta_desc = "You feel normally strange.";
#endif
			break;
		case 8: case 9:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_WASTING;
#ifdef JP
			muta_desc = "�����ޤ�������¤����ä���";
#else
			muta_desc = "You are cured of the horrible wasting disease!";
#endif
			break;
		case 10: case 11: case 12:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_RES_TIME;
#ifdef JP
			muta_desc = "�Ի�Ǥʤ��ʤä��������롣";
#else
			muta_desc = "You feel all too mortal.";
#endif
			break;
		case 13: case 14:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_WARNING;
#ifdef JP
			muta_desc = "�ѥ�Υ����Ǥʤ��ʤä���";
#else
			muta_desc = "You no longer feel paranoid.";
#endif
			break;
		case 15: case 16: case 17:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_HYPER_STR;
#ifdef JP
			muta_desc = "���������̤���ä���";
#else
			muta_desc = "Your muscles revert to normal.";
#endif
			break;
		case 18: case 19:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_PUNY;
#ifdef JP
			muta_desc = "���������̤���ä���";
#else
			muta_desc = "Your muscles revert to normal.";
#endif
			break;
		case 20: case 21: case 22: case 23:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_HYPER_INT;
#ifdef JP
			muta_desc = "Ǿ�����̤���ä���";
#else
			muta_desc = "Your brain reverts to normal.";
#endif
			break;
		case 24: case 25:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_MORONIC;
#ifdef JP
			muta_desc = "Ǿ�����̤���ä���";
#else
			muta_desc = "Your brain reverts to normal.";
#endif
			break;
		case 26: case 27: case 28:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_RESILIENT;
#ifdef JP
			muta_desc = "���̤ξ��פ�����ä���";
#else
			muta_desc = "You become ordinarily resilient again.";
#endif
			break;
		case 29:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_XTRA_FAT;
#ifdef JP
			muta_desc = "����Ū�ʥ������åȤ�����������";
#else
			muta_desc = "You benefit from a miracle diet!";
#endif
			break;
		case 30: case 31:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_ALBINO;
#ifdef JP
			muta_desc = "����ӥΤǤʤ��ʤä���";
#else
			muta_desc = "You are no longer an albino!";
#endif
			break;
		case 32:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_FLESH_ROT;
#ifdef JP
			muta_desc = "���Τ����Ԥ������µ������ä���";
#else
			muta_desc = "Your flesh is no longer afflicted by a rotting disease!";
#endif
			break;
		case 33:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_SILLY_VOI;
#ifdef JP
			muta_desc = "���������̤���ä���";
#else
			muta_desc = "Your voice returns to normal.";
#endif
			break;
		case 34: case 35: case 36:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_ILL_NORM;
#ifdef JP
			muta_desc = "�����¤餰���Ƥ�Ǥ��Ф��ʤ��ʤä���";
#else
			muta_desc = "You stop projecting a reassuring image.";
#endif
			break;
		case 37: case 38:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_XTRA_EYES;
#ifdef JP
			muta_desc = ";ʬ���ܤ��ä��Ƥ��ޤä���";
#else
			muta_desc = "Your extra eyes vanish!";
#endif
			break;
		case 39: case 40: case 41: case 42:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_XTRA_LEGS;
#ifdef JP
			muta_desc = ";ʬ�ʵӤ��ä��Ƥ��ޤä���";
#else
			muta_desc = "Your extra legs disappear!";
#endif
			break;
		case 43: case 44:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_SHORT_LEG;
#ifdef JP
			muta_desc = "�Ӥ�Ĺ�������̤���ä���";
#else
			muta_desc = "Your legs lengthen to normal.";
#endif
			break;
		case 45: case 46: case 47:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_ELEC_TOUC;
#ifdef JP
			muta_desc = "�Τ���ή��ή��ʤ��ʤä���";
#else
			muta_desc = "Electricity stops running through you.";
#endif
			break;
		case 48: case 49:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_FIRE_BODY;
#ifdef JP
			muta_desc = "�Τ������ޤ�ʤ��ʤä���";
#else
			muta_desc = "Your body is no longer enveloped in flames.";
#endif
			break;
		case 50: case 51:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_SCALES;
#ifdef JP
			muta_desc = "�ڤ��ä�����";
#else
			muta_desc = "Your scales vanish!";
#endif
			break;
		case 52: case 53: case 54:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_IRON_SKIN;
#ifdef JP
			muta_desc = "ȩ�����ˤ�ɤä���";
#else
			muta_desc = "Your skin reverts to flesh!";
#endif
			break;
		case 55: case 56:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_WINGS;
#ifdef JP
			muta_desc = "����α���������������";
#else
			muta_desc = "Your wings fall off.";
#endif
			break;
		case 57: case 58: case 59:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_FEARLESS;
#ifdef JP
			muta_desc = "�ƤӶ��ݤ򴶤���褦�ˤʤä���";
#else
			muta_desc = "You begin to feel fear again.";
#endif
			break;
		case 60: case 61: case 62:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_REGEN;
#ifdef JP
			muta_desc = "��®�������ʤ��ʤä���";
#else
			muta_desc = "You stop regenerating.";
#endif
			break;
		case 63: case 64: case 65: case 66:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_ESP;
#ifdef JP
			muta_desc = "�ƥ�ѥ�����ǽ�Ϥ򼺤ä���";
#else
			muta_desc = "You lose your telepathic ability!";
#endif
			break;
		case 67: case 68: case 69:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_LIMBER;
#ifdef JP
			muta_desc = "�������Ť��ʤä���";
#else
			muta_desc = "Your muscles stiffen.";
#endif
			break;
		case 70: case 71:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_ARTHRITIS;
#ifdef JP
			muta_desc = "���᤬�ˤ��ʤ��ʤä���";
#else
			muta_desc = "Your joints stop hurting.";
#endif
			break;
		case 72: case 73:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_VULN_ELEM;
#ifdef JP
			muta_desc = "̵�����ʴ����Ϥʤ��ʤä���";
#else
			muta_desc = "You feel less exposed.";
#endif
			break;
		case 74: case 75:
			muta_class = &(p_ptr->muta);
			muta_which = MUT_MOTION;
#ifdef JP
			muta_desc = "ư������Τ����ʤ��ʤä���";
#else
			muta_desc = "You move with less assurance.";
#endif
			break;
		default:
			muta_class = NULL;
			muta_which = 0;
		}

		if (muta_class && muta_which)
		{
			if (*(muta_class) & muta_which)
			{
				muta_chosen = TRUE;
			}
		}
		if (muta_chosen) break;
	}

	if (!muta_chosen)
	{
		return FALSE;
	}
	else
	{
		msg_print(muta_desc);
		*(muta_class) &= ~(muta_which);

		p_ptr->update |= PU_BONUS;
		handle_stuff();
		mutant_regenerate_mod = calc_mutant_regenerate_mod();
		return TRUE;
	}
}


void dump_mutations(FILE *OutFile)
{
	if (!OutFile) return;

	if (p_ptr->muta)
	{
		if (p_ptr->muta & MUT_BERS_RAGE)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ���������ò���ȯ��򵯤�����\n");
#else
			fprintf(OutFile, " You are subject to berserker fits.\n");
#endif
		}
		if (p_ptr->muta & MUT_COWARDICE)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ��ϻ������¤ˤʤ롣\n");
#else
			fprintf(OutFile, " You are subject to cowardice.\n");
#endif
		}
		if (p_ptr->muta & MUT_HALLU)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ��ϸ��Ф������������������˿�����Ƥ��롣\n");
#else
			fprintf(OutFile, " You have a hallucinatory insanity.\n");
#endif
		}
		if (p_ptr->muta & MUT_NORMALITY)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ����Ѱۤ��Ƥ��������������Ƥ��Ƥ��롣\n");
#else
			fprintf(OutFile, " You may be mutated, but you're recovering.\n");
#endif
		}
		if (p_ptr->muta & MUT_WASTING)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ��Ͽ�夹�붲�����µ��ˤ����äƤ��롣\n");
#else
			fprintf(OutFile, " You have a horrible wasting disease.\n");
#endif
		}
		if (p_ptr->muta & MUT_WARNING)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ���Ũ�˴ؤ���ٹ�򴶤��롣\n");
#else
			fprintf(OutFile, " You receive warnings about your foes.\n");
#endif
		}
		if (p_ptr->muta & MUT_HYPER_STR)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ���Ķ��Ū�˶�����(����+4)\n");
#else
			fprintf(OutFile, " You are superhumanly strong (+4 STR).\n");
#endif
		}
		if (p_ptr->muta & MUT_PUNY)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ��ϵ������(����-4)\n");
#else
			fprintf(OutFile, " You are puny (-4 STR).\n");
#endif
		}
		if (p_ptr->muta & MUT_HYPER_INT)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ���Ǿ�����Υ���ԥ塼������(��ǽ������+4)\n");
#else
			fprintf(OutFile, " Your brain is a living computer (+4 INT/WIS).\n");
#endif
		}
		if (p_ptr->muta & MUT_MORONIC)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ��������������(��ǽ������-4)\n");
#else
			fprintf(OutFile, " You are moronic (-4 INT/WIS).\n");
#endif
		}
		if (p_ptr->muta & MUT_RESILIENT)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ����Τ����������٤�Ǥ��롣(�ѵ�+4)\n");
#else
			fprintf(OutFile, " You are very resilient (+4 CON).\n");
#endif
		}
		if (p_ptr->muta & MUT_XTRA_FAT)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ��϶�ü�����äƤ��롣(�ѵ�+2,���ԡ���-2)\n");
#else
			fprintf(OutFile, " You are extremely fat (+2 CON, -2 speed).\n");
#endif
		}
		if (p_ptr->muta & MUT_ALBINO)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ��ϥ���ӥΤ���(�ѵ�-4)\n");
#else
			fprintf(OutFile, " You are albino (-4 CON).\n");
#endif
		}
		if (p_ptr->muta & MUT_FLESH_ROT)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ������Τ����Ԥ��Ƥ��롣(�ѵ�-2,̥��-1)\n");
#else
			fprintf(OutFile, " Your flesh is rotting (-2 CON, -1 CHR).\n");
#endif
		}
		if (p_ptr->muta & MUT_SILLY_VOI)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ������ϴ�ȴ���ʥ�������������(̥��-4)\n");
#else
			fprintf(OutFile, " Your voice is a silly squeak (-4 CHR).\n");
#endif
		}
		if (p_ptr->muta & MUT_ILL_NORM)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ��ϸ��Ƥ�ʤ���Ƥ��롣\n");
#else
			fprintf(OutFile, " Your appearance is masked with illusion.\n");
#endif
		}
		if (p_ptr->muta & MUT_XTRA_EYES)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ���;ʬ����Ĥ��ܤ���äƤ��롣(õ��+15)\n");
#else
			fprintf(OutFile, " You have an extra pair of eyes (+15 search).\n");
#endif
		}
		if (p_ptr->muta & MUT_XTRA_LEGS)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ���;ʬ�����ܤ�­�������Ƥ��롣(��®+3)\n");
#else
			fprintf(OutFile, " You have an extra pair of legs (+3 speed).\n");
#endif
		}
		if (p_ptr->muta & MUT_SHORT_LEG)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ���­��û���͵�����(��®-3)\n");
#else
			fprintf(OutFile, " Your legs are short stubs (-3 speed).\n");
#endif
		}
		if (p_ptr->muta & MUT_ELEC_TOUC)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ��η�ɤˤ���ή��ή��Ƥ��롣\n");
#else
			fprintf(OutFile, " Electricity is running through your veins.\n");
#endif
		}
		if (p_ptr->muta & MUT_FIRE_BODY)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ����Τϱ�ˤĤĤޤ�Ƥ��롣\n");
#else
			fprintf(OutFile, " Your body is enveloped in flames.\n");
#endif
		}
		if (p_ptr->muta & MUT_SCALES)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ���ȩ���ڤˤʤäƤ��롣(̥��-1, AC+10)\n");
#else
			fprintf(OutFile, " Your skin has turned into scales (-1 CHR, +10 AC).\n");
#endif
		}
		if (p_ptr->muta & MUT_IRON_SKIN)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ���ȩ��Ŵ�ǤǤ��Ƥ��롣(����-1, AC+25)\n");
#else
			fprintf(OutFile, " Your skin is made of steel (-1 DEX, +25 AC).\n");
#endif
		}
		if (p_ptr->muta & MUT_WINGS)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ��ϱ�����äƤ��롣\n");
#else
			fprintf(OutFile, " You have wings.\n");
#endif
		}
		if (p_ptr->muta & MUT_FEARLESS)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ����������ݤ򴶤��ʤ���\n");
#else
			fprintf(OutFile, " You are completely fearless.\n");
#endif
		}
		if (p_ptr->muta & MUT_REGEN)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ��ϵ�®�˲������롣\n");
#else
			fprintf(OutFile, " You are regenerating.\n");
#endif
		}
		if (p_ptr->muta & MUT_ESP)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ��ϥƥ�ѥ�������äƤ��롣\n");
#else
			fprintf(OutFile, " You are telepathic.\n");
#endif
		}
		if (p_ptr->muta & MUT_LIMBER)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ����Τ����ˤ��ʤ䤫����(����+3)\n");
#else
			fprintf(OutFile, " Your body is very limber (+3 DEX).\n");
#endif
		}
		if (p_ptr->muta & MUT_ARTHRITIS)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ��Ϥ��Ĥ������ˤߤ򴶤��Ƥ��롣(����-3)\n");
#else
			fprintf(OutFile, " Your joints ache constantly (-3 DEX).\n");
#endif
		}
		if (p_ptr->muta & MUT_RES_TIME)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ��ϻ��ֵ�ž���⤫�����Ƥ��롣\n");
#else
			fprintf(OutFile, " You are protected from the ravages of time.\n");
#endif
		}
		if (p_ptr->muta & MUT_VULN_ELEM)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ��ϸ��Ǥι���˼夤��\n");
#else
			fprintf(OutFile, " You are susceptible to damage from the elements.\n");
#endif
		}
		if (p_ptr->muta & MUT_MOTION)
		{
#ifdef JP
			fprintf(OutFile, " ���ʤ���ư������Τ��϶�����(��̩+1)\n");
#else
			fprintf(OutFile, " Your movements are precise and forceful (+1 STL).\n");
#endif
		}
	}
}


/*
 * List mutations we have...
 */
void do_cmd_knowledge_mutations(void)
{
	FILE *fff;
	char file_name[1024];

	/* Open a new file */
	fff = my_fopen_temp(file_name, 1024);

	/* Dump the mutations to file */
	if (fff) dump_mutations(fff);

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
#ifdef JP
show_file(file_name, "�����Ѱ�", 0, 0);
#else
	show_file(file_name, "Mutations", 0, 0);
#endif


	/* Remove the file */
	fd_kill(file_name);
}


static int count_bits(u32b x)
{
	int n = 0;

	if (x) do
	{
		n++;
	}
	while (0 != (x = x & (x - 1)));

	return (n);
}


static int count_mutations(void)
{
	return (count_bits(p_ptr->muta));
}


/*
 * Return the modifier to the regeneration rate
 * (in percent)
 */
int calc_mutant_regenerate_mod(void)
{
	int regen;
	int mod = 10;
	int count = count_mutations();

	/* No negative modifier */
	if (count <= 0) return 100;

	regen = 100 - count * mod;

	/* Max. 90% decrease in regeneration speed */
	if (regen < 10) regen = 10;

	return (regen);
}


bool mutation_power_aux(u32b power)
{
	energy_use = 0;
#ifdef JP
	msg_format("ǽ�� %s �ϼ�������Ƥ��ޤ���", power);
#else
	msg_format("Power %s not implemented. Oops.", power);
#endif
	return FALSE;
}
