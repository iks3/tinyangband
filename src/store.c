/* File: store.c */

/* Purpose: Store commands */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"

#define MIN_STOCK 12

/* 
 * Store current values to allow all functions in this file to access them.
 */
static int cur_store_num = 0; /* current "store number" */
static int store_top = 0; /* current "store page" */
static int store_bottom = 0;
static int xtra_stock = 0;
static store_type *st_ptr = NULL; /* current "store pointer" */
static owner_type *ot_ptr = NULL; /* current "owner type" */
static s16b old_town_num = 0;
static s16b inner_town_num = 0;
#define RUMOR_CHANCE 8

#define MAX_COMMENT_1	6 /* Comments on agreeing price */

static cptr comment_1[MAX_COMMENT_1] =
{
#ifdef JP
	"オーケーだ。",
	"結構だ。",
	"そうしよう！",
	"賛成だ！",
	"よし！",
	"わかった！"
#else
	"Okay.",
	"Fine.",
	"Accepted!",
	"Agreed!",
	"Done!",
	"Taken!"
#endif

};

/* ブラックマーケット追加メッセージ（承諾） */
/* Add additional comments for black market (agreement) */
static cptr comment_1_B[MAX_COMMENT_1] = {
#ifdef JP
	"まあ、それでいいや。",
	"今日はそれで勘弁してやる。",
	"分かったよ。",
	"しょうがない。",
	"それで我慢するよ。",
	"こんなもんだろう。"
#else
	"Well, 'spose that will do.",
	"I'll let you off at that today.",
	"OK, OK!",
	"Got me over a barrel here.",
	"It's that or nothing.",
	"Close enough."
#endif
};



/*
 * Successful haggle.
 */
static void say_comment_1(void)
{
	char rumour[1024];

	/* Give separate message for black market */
	if ( cur_store_num == STORE_BLACK )
	{
		msg_print(comment_1_B[randint0(MAX_COMMENT_1)]);
	}
	else
	{
		msg_print(comment_1[randint0(MAX_COMMENT_1)]);
	}

	if ((randint1(RUMOR_CHANCE) == 1) && speak_unique)
	{
#ifdef JP
		msg_print("店主は耳うちした:");
		if (!get_rnd_line_jonly("rumors_j.txt", 0, rumour, 10))
#else
		msg_print("The shopkeeper whispers something into your ear:");
		if (!get_rnd_line("rumors.txt", 0, rumour))
#endif
			msg_print(rumour);
	}
}


/*
 * Messages for reacting to purchase prices.
 */

#define MAX_COMMENT_2A	4

static cptr comment_2a[MAX_COMMENT_2A] =
{
#ifdef JP
	"うわああぁぁ！",
	"なんてこった！",
	"誰かがむせび泣く声が聞こえる...。",
	"店主が悔しげにわめいている！"
#else
	"Arrgghh!",
	"You bastard!",
	"You hear someone sobbing...",
	"The shopkeeper howls in agony!"
#endif
};

#define MAX_COMMENT_2B	4

static cptr comment_2b[MAX_COMMENT_2B] =
{
#ifdef JP
	"くそう！",
	"この悪魔め！",
	"店主が恨めしそうに見ている。",
	"店主が睨んでいる。"
#else
	"Damn!",
	"You fiend!",
	"The shopkeeper curses at you.",
	"The shopkeeper glares at you."
#endif
};

#define MAX_COMMENT_2C	4

static cptr comment_2c[MAX_COMMENT_2C] =
{
#ifdef JP
	"すばらしい！",
	"君が天使に見えるよ！",
	"店主がクスクス笑っている。",
	"店主が大声で笑っている。"
#else
	"Cool!",
	"You've made my day!",
	"The shopkeeper giggles.",
	"The shopkeeper laughs loudly."
#endif
};

#define MAX_COMMENT_2D	4

static cptr comment_2d[MAX_COMMENT_2D] =
{
#ifdef JP
	"やっほぅ！",
	"こんなおいしい思いをしたら、真面目に働けなくなるなぁ。",
	"店主は嬉しくて跳ね回っている。",
	"店主は満面に笑みをたたえている。"
#else
	"Yipee!",
	"I think I'll retire!",
	"The shopkeeper jumps for joy.",
	"The shopkeeper smiles gleefully."
#endif
};


/*
 * Let a shop-keeper React to a purchase
 *
 * We paid "price", it was worth "value", and we thought it was worth "guess"
 */
static void purchase_analyze(s32b price, s32b value, s32b guess)
{
	/* Item was worthless, but we bought it */
	if ((value <= 0) && (price > value))
	{
		/* Comment */
		msg_print(comment_2a[randint0(MAX_COMMENT_2A)]);

		/* Sound */
		sound(SOUND_STORE1);
	}

	/* Item was cheaper than we thought, and we paid more than necessary */
	else if ((value < guess) && (price > value))
	{
		/* Comment */
		msg_print(comment_2b[randint0(MAX_COMMENT_2B)]);

		/* Sound */
		sound(SOUND_STORE2);
	}

	/* Item was a good bargain, and we got away with it */
	else if ((value > guess) && (value < (4 * guess)) && (price < value))
	{
		/* Comment */
		msg_print(comment_2c[randint0(MAX_COMMENT_2C)]);

		/* Sound */
		sound(SOUND_STORE3);
	}

	/* Item was a great bargain, and we got away with it */
	else if ((value > guess) && (price < value))
	{
		/* Comment */
		msg_print(comment_2d[randint0(MAX_COMMENT_2D)]);

		/* Sound */
		sound(SOUND_STORE4);
	}
}


/*
 * Determine the price of an item (qty one) in a store.
 *
 * This function takes into account the player's charisma, and the
 * shop-keepers friendliness, and the shop-keeper's base greed, but
 * never lets a shop-keeper lose money in a transaction.
 *
 * The "greed" value should exceed 100 when the player is "buying" the
 * item, and should be less than 100 when the player is "selling" it.
 *
 * Hack -- the black market always charges twice as much as it should.
 *
 * Charisma adjustment runs from 80 to 130
 * Racial adjustment runs from 95 to 130
 *
 * Since greed/charisma/racial adjustments are centered at 100, we need
 * to adjust (by 200) to extract a usable multiplier.  Note that the
 * "greed" value is always something (?).
 */
static s32b price_item(object_type *o_ptr, int greed, bool flip)
{
	int 	factor = 100;
	int 	adjust;
	s32b	price;


	/* Get the value of one of the items */
	price = object_value(o_ptr);

	/* Worthless items */
	if (price <= 0) return (0L);

	/* Add in the charisma factor */
	factor += adj_chr_gold[p_ptr->stat_ind[A_CHR]];


	/* Shop is buying */
	if (flip)
	{
		/* Adjust for greed */
		adjust = 100 + (300 - (greed + factor));

		/* Never get "silly" */
		if (adjust > 100) adjust = 100;

		/* Mega-Hack -- Black market sucks */
		if (cur_store_num == STORE_BLACK)
			price = price / 2;

		/* Compute the final price (with rounding) */
		/* Hack -- prevent underflow */
		price = (price * adjust + 50L) / 100L;
	}

	/* Shop is selling */
	else
	{
		/* Adjust for greed */
		adjust = 100 + ((greed + factor) - 300);

		/* Never get "silly" */
		if (adjust < 100) adjust = 100;

		/* Mega-Hack -- Black market sucks */
		if (cur_store_num == STORE_BLACK)
			price = price * 2;

		/* Compute the final price (with rounding) */
		/* Hack -- prevent overflow */
		price = (s32b)(((u32b)price * (u32b)adjust + 50UL) / 100UL);
	}

	/* Note -- Never become "free" */
	if (price <= 0L) return (1L);

	/* Return the price */
	return (price);
}


/*
 * Certain "cheap" objects should be created in "piles"
 * Some objects can be sold at a "discount" (in small piles)
 */
static void mass_produce(object_type *o_ptr)
{
	int size = 1;
	int discount = 0;

	s32b cost = object_value(o_ptr);


	/* Analyze the type */
	switch (o_ptr->tval)
	{
		/* Food, Flasks, and Lites */
		case TV_FOOD:
		case TV_FLASK:
		case TV_LITE:
		{
			if (cost <= 5L) size += randint1(5);
			if (cost <= 20L) size += randint1(5);
			break;
		}

		case TV_POTION:
		case TV_SCROLL:
		{
			if (randint0(o_ptr->number) < 5)
			{
				if (cost <= 60L) size += randint1(2);
			}
			if (o_ptr->sval == SV_SCROLL_STAR_IDENTIFY) size += randint1(3);
			break;
		}

		case TV_LIFE_BOOK:
		case TV_SORCERY_BOOK:
		{
			if (cost <= 50L) size += damroll(2, 3);
			if (cost <= 500L) size += damroll(1, 3);
			break;
		}

		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_SHIELD:
		case TV_GLOVES:
		case TV_BOOTS:
		case TV_CLOAK:
		case TV_HELM:
		case TV_CROWN:
		case TV_SWORD:
		case TV_POLEARM:
		case TV_HAFTED:
		case TV_DIGGING:
		case TV_BOW:
		{
			break;
		}

		case TV_SPIKE:
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
		{
			if (cost <= 5L) size += damroll(3, 5);
			if (cost <= 50L) size += damroll(2, 5);
			if (cost <= 500L) size += damroll(2, 5);
			break;
		}

		case TV_FIGURINE:
		case TV_STATUE:
		{
			break;
		}

		/*
		 * Because many rods (and a few wands and staffs) are useful mainly
		 * in quantity, the Black Market will occasionally have a bunch of
		 * one kind. -LM-
		 */
		case TV_ROD:
		case TV_WAND:
		case TV_STAFF:
		{
			if ((cur_store_num == STORE_BLACK) && (randint1(3) == 1))
			{
				if (cost < 1601L) size += damroll(1, 5);
				else if (cost < 3201L) size += damroll(1, 3);
			}

			/* Ensure that mass-produced rods and wands get the correct pvals. */
			if ((o_ptr->tval == TV_ROD) || (o_ptr->tval == TV_WAND))
			{
				o_ptr->pval *= size;
			}
			break;
		}
	}


	/* Pick a discount */
	if (cost < 5)
	{
		discount = 0;
	}
	else if (randint0(25) == 0)
	{
		discount = 25;
	}
	else if (randint0(150) == 0)
	{
		discount = 50;
	}
	else if (randint0(300) == 0)
	{
		discount = 75;
	}
	else if (randint0(500) == 0)
	{
		discount = 90;
	}


	if (o_ptr->art_name)
	{
		if (cheat_peek && discount)
		{
			msg_print(_("ランダムアーティファクトは値引きなし。", "No discount on random artifacts."));
		}
		discount = 0;
	}

	/* Save the discount */
	o_ptr->discount = discount;

	/* Save the total pile size */
	o_ptr->number = size - (size * discount / 100);
}


/*
 * Determine if a store item can "absorb" another item
 *
 * See "object_similar()" for the same function for the "player"
 */
static bool store_object_similar(object_type *o_ptr, object_type *j_ptr)
{
	/* Hack -- Identical items cannot be stacked */
	if (o_ptr == j_ptr) return (0);

	/* Different objects cannot be stacked */
	if (o_ptr->k_idx != j_ptr->k_idx) return (0);

	/* Different charges (etc) cannot be stacked, unless wands or rods. */
	if ((o_ptr->pval != j_ptr->pval) && (o_ptr->tval != TV_WAND) && (o_ptr->tval != TV_ROD)) return (0);

	/* Require many identical values */
	if (o_ptr->to_h != j_ptr->to_h) return (0);
	if (o_ptr->to_d != j_ptr->to_d) return (0);
	if (o_ptr->to_a != j_ptr->to_a) return (0);

	/* Require identical "artifact" names */
	if (o_ptr->name1 != j_ptr->name1) return (0);

	/* Require identical "ego-item" names */
	if (o_ptr->name2 != j_ptr->name2) return (0);

	/* Random artifacts don't stack !*/
	if (o_ptr->art_name || j_ptr->art_name) return (0);

	/* Hack -- Identical art_flags! */
	if ((o_ptr->art_flags1 != j_ptr->art_flags1) ||
		(o_ptr->art_flags2 != j_ptr->art_flags2) ||
		(o_ptr->art_flags3 != j_ptr->art_flags3))
			return (0);

	/* Hack -- Never stack "powerful" items */
	if (o_ptr->xtra1 || j_ptr->xtra1) return (0);

	/* Hack -- Never stack different fuel lite */
	if (o_ptr->xtra3 != j_ptr->xtra3) return (0);

	/* Hack -- Never stack recharging items */
	if (o_ptr->timeout || j_ptr->timeout) return (0);

	/* Require many identical values */
	if (o_ptr->ac != j_ptr->ac)   return (0);
	if (o_ptr->dd != j_ptr->dd)   return (0);
	if (o_ptr->ds != j_ptr->ds)   return (0);

	/* Hack -- Never stack chests */
	if (o_ptr->tval == TV_CHEST) return (0);

	/* Require matching discounts */
	if (o_ptr->discount != j_ptr->discount) return (0);

	/* They match, so they must be similar */
	return (TRUE);
}


/*
 * Allow a store item to absorb another item
 */
static void store_object_absorb(object_type *o_ptr, object_type *j_ptr)
{
	int max_num = (o_ptr->tval == TV_ROD) ?
		MIN(99, MAX_SHORT / k_info[o_ptr->k_idx].pval) : 99;
	int total = o_ptr->number + j_ptr->number;
	int diff = (total > max_num) ? total - max_num : 0;

	/* Combine quantity, lose excess items */
	o_ptr->number = (total > max_num) ? max_num : total;

	/* Hack -- if rods are stacking, add the pvals (maximum timeouts) together. -LM- */
	if (o_ptr->tval == TV_ROD)
	{
		o_ptr->pval += j_ptr->pval * (j_ptr->number - diff) / j_ptr->number;
	}

	/* Hack -- if wands are stacking, combine the charges. -LM- */
	if (o_ptr->tval == TV_WAND)
	{
		o_ptr->pval += j_ptr->pval * (j_ptr->number - diff) / j_ptr->number;
	}
}


/*
 * Check to see if the shop will be carrying too many objects	-RAK-
 * Note that the shop, just like a player, will not accept things
 * it cannot hold.	Before, one could "nuke" potions this way.
 */
static bool store_check_num(object_type *o_ptr)
{
	int 	   i;
	object_type *j_ptr;

	/* Free space is always usable */
	if (st_ptr->stock_num < st_ptr->stock_size) {
		return TRUE;
	}

	/* The "home" acts like the player */
	if (cur_store_num == STORE_HOME)
	{
		/* Check all the items */
		for (i = 0; i < st_ptr->stock_num; i++)
		{
			/* Get the existing item */
			j_ptr = &st_ptr->stock[i];

			/* Can the new object be combined with the old one? */
			if (object_similar(j_ptr, o_ptr)) return (TRUE);
		}
	}

	/* Normal stores do special stuff */
	else
	{
		/* Check all the items */
		for (i = 0; i < st_ptr->stock_num; i++)
		{
			/* Get the existing item */
			j_ptr = &st_ptr->stock[i];

			/* Can the new object be combined with the old one? */
			if (store_object_similar(j_ptr, o_ptr)) return (TRUE);
		}
	}

	/* But there was no room at the inn... */
	return (FALSE);
}


static bool is_blessed(const object_type *o_ptr)
{
	u32b f1, f2, f3;
	object_flags(o_ptr, &f1, &f2, &f3);
	if (f3 & TR3_BLESSED) return (TRUE);
	else return (FALSE);
}


/*
 * Determine if the current store will purchase the given item
 *
 * Note that a shop-keeper must refuse to buy "worthless" items
 */
static bool store_will_buy(const object_type *o_ptr)
{
	/* Hack -- The Home is simple */
	if (cur_store_num == STORE_HOME) return (TRUE);

	/* Switch on the store */
	switch (cur_store_num)
	{
		/* General Store */
		case STORE_GENERAL:
		{
			/* Analyze the type */
			switch (o_ptr->tval)
			{
				case TV_FOOD:
				case TV_LITE:
				case TV_FLASK:
				case TV_SPIKE:
				case TV_SHOT:
				case TV_ARROW:
				case TV_BOLT:
				case TV_DIGGING:
				case TV_CLOAK:
				case TV_BOTTLE: /* 'Green', recycling Angband */
				case TV_FIGURINE:
				case TV_STATUE:
				break;
				default:
				return (FALSE);
			}
			break;
		}

		/* Armoury */
		case STORE_ARMOURY:
		{
			/* Analyze the type */
			switch (o_ptr->tval)
			{
				case TV_BOOTS:
				case TV_GLOVES:
				case TV_CROWN:
				case TV_HELM:
				case TV_SHIELD:
				case TV_CLOAK:
				case TV_SOFT_ARMOR:
				case TV_HARD_ARMOR:
				case TV_DRAG_ARMOR:
				break;
				default:
				return (FALSE);
			}
			break;
		}

		/* Weapon Shop */
		case STORE_WEAPON:
		{
			/* Analyze the type */
			switch (o_ptr->tval)
			{
				case TV_SHOT:
				case TV_BOLT:
				case TV_ARROW:
				case TV_BOW:
				case TV_DIGGING:
				case TV_POLEARM:
				case TV_SWORD:
				break;
				case TV_HAFTED:
				{
					if (o_ptr->sval == SV_WIZSTAFF) return (FALSE);
				}
				break;
				default:
				return (FALSE);
			}
			break;
		}

		/* Temple */
		case STORE_TEMPLE:
		{
			/* Analyze the type */
			switch (o_ptr->tval)
			{
				case TV_LIFE_BOOK:
				case TV_SCROLL:
				case TV_POTION:
				case TV_HAFTED:
				{
					break;
				}
				case TV_FIGURINE:
				case TV_STATUE:
				{
					monster_race *r_ptr = &r_info[o_ptr->pval];

					/* Decline evil */
					if (!(r_ptr->flags3 & RF3_EVIL))
					{
						/* Accept good */
						if (r_ptr->flags3 & RF3_GOOD) break;

						/* Accept animals */
						if (r_ptr->flags3 & RF3_ANIMAL) break;

						/* Accept mimics */
						if (my_strchr("?!", r_ptr->d_char)) break;
					}
				}
				case TV_POLEARM:
				case TV_SWORD:
				{
					if (is_blessed(o_ptr)) break;
				}
				default:
				return (FALSE);
			}
			break;
		}

		/* Alchemist */
		case STORE_ALCHEMIST:
		{
			/* Analyze the type */
			switch (o_ptr->tval)
			{
				case TV_SCROLL:
				case TV_POTION:
				break;
				default:
				return (FALSE);
			}
			break;
		}

		/* Magic Shop */
		case STORE_MAGIC:
		{
			/* Analyze the type */
			switch (o_ptr->tval)
			{
				case TV_SORCERY_BOOK:
				case TV_AMULET:
				case TV_RING:
				case TV_STAFF:
				case TV_WAND:
				case TV_ROD:
				case TV_SCROLL:
				case TV_POTION:
				case TV_FIGURINE:
				break;
				case TV_HAFTED:
				{
					if (o_ptr->sval == SV_WIZSTAFF) break;
					else return (FALSE);
				}
				default:
				return (FALSE);
			}
			break;
		}
		/* Bookstore Shop */
		case STORE_STATUE:
		{
			/* Analyze the type */
			switch (o_ptr->tval)
			{
				case TV_FIGURINE:
				case TV_STATUE:
				case TV_RING:
				case TV_AMULET:
					break;
				case TV_BOOTS:
				case TV_GLOVES:
				case TV_CROWN:
				case TV_HELM:
				case TV_SHIELD:
				case TV_CLOAK:
				case TV_SOFT_ARMOR:
				case TV_HARD_ARMOR:
				case TV_DRAG_ARMOR:
				case TV_ARROW:
				case TV_BOW:
				case TV_DIGGING:
				case TV_HAFTED:
				case TV_POLEARM:
				case TV_SWORD:
					if (!object_known_p(o_ptr) ||
						!(ego_item_p(o_ptr) || 
						artifact_p(o_ptr) || 
						o_ptr->art_name))
						return (FALSE);
					break;
				default:
					return (FALSE);
			}
			break;
		}
	}

	/* XXX XXX XXX Ignore "worthless" items */
	if (object_value(o_ptr) <= 0) return (FALSE);

	/* Assume okay */
	return (TRUE);
}


/*
 * Combine and reorder items in the home
 */
bool combine_and_reorder_home(void)
{
	int         i, j, k;
	s32b        o_value;
	object_type forge, *o_ptr, *j_ptr;
	bool        flag = FALSE, combined;
	store_type  *old_st_ptr = st_ptr;

	st_ptr = &town[1].store[STORE_HOME];

	do
	{
		combined = FALSE;

		/* Combine the items in the home (backwards) */
		for (i = st_ptr->stock_num - 1; i > 0; i--)
		{
			/* Get the item */
			o_ptr = &st_ptr->stock[i];

			/* Skip empty items */
			if (!o_ptr->k_idx) continue;

			/* Scan the items above that item */
			for (j = 0; j < i; j++)
			{
				int max_num;

				/* Get the item */
				j_ptr = &st_ptr->stock[j];

				/* Skip empty items */
				if (!j_ptr->k_idx) continue;

				/*
				 * Get maximum number of the stack if these
				 * are similar, get zero otherwise.
				 */
				max_num = object_similar_part(j_ptr, o_ptr);

				/* Can we (partialy) drop "o_ptr" onto "j_ptr"? */
				if (max_num && j_ptr->number < max_num)
				{
					if (o_ptr->number + j_ptr->number <= max_num)
					{
						/* Add together the item counts */
						object_absorb(j_ptr, o_ptr);

						/* One object is gone */
						st_ptr->stock_num--;

						/* Slide everything down */
						for (k = i; k < st_ptr->stock_num; k++)
						{
							/* Structure copy */
							st_ptr->stock[k] = st_ptr->stock[k + 1];
						}

						/* Erase the "final" slot */
						object_wipe(&st_ptr->stock[k]);
					}
					else
					{
						int old_num = o_ptr->number;
						int remain = j_ptr->number + o_ptr->number - max_num;

						/* Add together the item counts */
						object_absorb(j_ptr, o_ptr);

						o_ptr->number = remain;

						/* Hack -- if rods are stacking, add the pvals (maximum timeouts) and current timeouts together. -LM- */
						if (o_ptr->tval == TV_ROD)
						{
							o_ptr->pval =  o_ptr->pval * remain / old_num;
							o_ptr->timeout = o_ptr->timeout * remain / old_num;
						}

						/* Hack -- if wands are stacking, combine the charges. -LM- */
						else if (o_ptr->tval == TV_WAND)
						{
							o_ptr->pval = o_ptr->pval * remain / old_num;
						}
					}

					/* Take note */
					combined = TRUE;

					/* Done */
					break;
				}
			}
		}

		flag |= combined;
	}
	while (combined);

	/* Re-order the items in the home (forwards) */
	for (i = 0; i < st_ptr->stock_num; i++)
	{
		/* Get the item */
		o_ptr = &st_ptr->stock[i];

		/* Skip empty slots */
		if (!o_ptr->k_idx) continue;

		/* Get the "value" of the item */
		o_value = object_value(o_ptr);

		/* Scan every occupied slot */
		for (j = 0; j < st_ptr->stock_num; j++)
		{
			if (object_sort_comp(o_ptr, o_value, &st_ptr->stock[j])) break;
		}

		/* Never move down */
		if (j >= i) continue;

		/* Take note */
		flag = TRUE;

		/* Get local object */
		j_ptr = &forge;

		/* Save a copy of the moving item */
		object_copy(j_ptr, &st_ptr->stock[i]);

		/* Slide the objects */
		for (k = i; k > j; k--)
		{
			/* Slide the item */
			object_copy(&st_ptr->stock[k], &st_ptr->stock[k - 1]);
		}

		/* Insert the moving item */
		object_copy(&st_ptr->stock[j], j_ptr);
	}

	st_ptr = old_st_ptr;

	return flag;
}


/*
 * Add the item "o_ptr" to the inventory of the "Home"
 *
 * In all cases, return the slot (or -1) where the object was placed
 *
 * Note that this is a hacked up version of "inven_carry()".
 *
 * Also note that it may not correctly "adapt" to "knowledge" bacoming
 * known, the player may have to pick stuff up and drop it again.
 */
static int home_carry(object_type *o_ptr)
{
	int 				slot;
	s32b			   value;
	int 	i;
	object_type *j_ptr;


	/* Check each existing item (try to combine) */
	for (slot = 0; slot < st_ptr->stock_num; slot++)
	{
		/* Get the existing item */
		j_ptr = &st_ptr->stock[slot];

		/* The home acts just like the player */
		if (object_similar(j_ptr, o_ptr))
		{
			/* Save the new number of items */
			object_absorb(j_ptr, o_ptr);

			/* All done */
			return (slot);
		}
	}

	/* No space? */
	if (st_ptr->stock_num >= st_ptr->stock_size) return (-1);

	/* Determine the "value" of the item */
	value = object_value(o_ptr);

	/* Check existing slots to see if we must "slide" */
	for (slot = 0; slot < st_ptr->stock_num; slot++)
	{
		if (object_sort_comp(o_ptr, value, &st_ptr->stock[slot])) break;
	}

	/* Slide the others up */
	for (i = st_ptr->stock_num; i > slot; i--)
	{
		st_ptr->stock[i] = st_ptr->stock[i-1];
	}

	/* More stuff now */
	st_ptr->stock_num++;

	/* Insert the new item */
	st_ptr->stock[slot] = *o_ptr;

	(void)combine_and_reorder_home();

	/* Return the location */
	return (slot);
}


/*
 * Add the item "o_ptr" to a real stores inventory.
 *
 * If the item is "worthless", it is thrown away (except in the home).
 *
 * If the item cannot be combined with an object already in the inventory,
 * make a new slot for it, and calculate its "per item" price.	Note that
 * this price will be negative, since the price will not be "fixed" yet.
 * Adding an item to a "fixed" price stack will not change the fixed price.
 *
 * In all cases, return the slot (or -1) where the object was placed
 */
static int store_carry(object_type *o_ptr)
{
	int 	i, slot;
	s32b	value, j_value;
	object_type *j_ptr;


	/* Evaluate the object */
	value = object_value(o_ptr);

	/* Cursed/Worthless items "disappear" when sold */
	if (value <= 0) return (-1);

	/* All store items are fully *identified* */
	o_ptr->ident |= IDENT_MENTAL;

	/* Erase the inscription */
	o_ptr->inscription = 0;

	/* Erase the "feeling" */
	o_ptr->feeling = FEEL_NONE;

	/* Check each existing item (try to combine) */
	for (slot = 0; slot < st_ptr->stock_num; slot++)
	{
		/* Get the existing item */
		j_ptr = &st_ptr->stock[slot];

		/* Can the existing items be incremented? */
		if (store_object_similar(j_ptr, o_ptr))
		{
			/* Hack -- extra items disappear */
			store_object_absorb(j_ptr, o_ptr);

			/* All done */
			return (slot);
		}
	}

	/* No space? */
	if (st_ptr->stock_num >= st_ptr->stock_size) return (-1);


	/* Check existing slots to see if we must "slide" */
	for (slot = 0; slot < st_ptr->stock_num; slot++)
	{
		/* Get that item */
		j_ptr = &st_ptr->stock[slot];

		/* Objects sort by decreasing type */
		if (o_ptr->tval > j_ptr->tval) break;
		if (o_ptr->tval < j_ptr->tval) continue;

		/* Objects sort by increasing sval */
		if (o_ptr->sval < j_ptr->sval) break;
		if (o_ptr->sval > j_ptr->sval) continue;

		/*
		 * Hack:  otherwise identical rods sort by
		 * increasing recharge time --dsb
		 */
		if (o_ptr->tval == TV_ROD)
		{
			if (o_ptr->pval < j_ptr->pval) break;
			if (o_ptr->pval > j_ptr->pval) continue;
		}

		/* Evaluate that slot */
		j_value = object_value(j_ptr);

		/* Objects sort by decreasing value */
		if (value > j_value) break;
		if (value < j_value) continue;
	}

	/* Slide the others up */
	for (i = st_ptr->stock_num; i > slot; i--)
	{
		st_ptr->stock[i] = st_ptr->stock[i-1];
	}

	/* More stuff now */
	st_ptr->stock_num++;

	/* Insert the new item */
	st_ptr->stock[slot] = *o_ptr;

	/* Return the location */
	return (slot);
}


/*
 * Increase, by a given amount, the number of a certain item
 * in a certain store.	This can result in zero items.
 */
static void store_item_increase(int item, int num)
{
	int 		cnt;
	object_type *o_ptr;

	/* Get the item */
	o_ptr = &st_ptr->stock[item];

	/* Verify the number */
	cnt = o_ptr->number + num;
	if (cnt > 255) cnt = 255;
	else if (cnt < 0) cnt = 0;
	num = cnt - o_ptr->number;

	/* Save the new number */
	o_ptr->number += num;
}


/*
 * Remove a slot if it is empty
 */
static void store_item_optimize(int item)
{
	int 		j;
	object_type *o_ptr;

	/* Get the item */
	o_ptr = &st_ptr->stock[item];

	/* Must exist */
	if (!o_ptr->k_idx) return;

	/* Must have no items */
	if (o_ptr->number) return;

	/* One less item */
	st_ptr->stock_num--;

	/* Slide everyone */
	for (j = item; j < st_ptr->stock_num; j++)
	{
		st_ptr->stock[j] = st_ptr->stock[j + 1];
	}

	/* Nuke the final slot */
	object_wipe(&st_ptr->stock[j]);
}


/*
 * This function will keep 'crap' out of the black market.
 * Crap is defined as any item that is "available" elsewhere
 * Based on a suggestion by "Lee Vogt" <lvogt@cig.mcel.mot.com>
 */
static bool black_market_crap(object_type *o_ptr)
{
	int 	i, j;

	/* Ego items are never crap */
	if (o_ptr->name2) return (FALSE);

	/* Good items are never crap */
	if (o_ptr->to_a > 0) return (FALSE);
	if (o_ptr->to_h > 0) return (FALSE);
	if (o_ptr->to_d > 0) return (FALSE);

	/* Check all stores */
	for (i = 0; i < MAX_STORES; i++)
	{
		if (i == STORE_HOME) continue;

		/* Check every item in the store */
		for (j = 0; j < town[p_ptr->town_num].store[i].stock_num; j++)
		{
			object_type *j_ptr = &town[p_ptr->town_num].store[i].stock[j];

			/* Duplicate item "type", assume crappy */
			if (o_ptr->k_idx == j_ptr->k_idx) return (TRUE);
		}
	}

	/* Assume okay */
	return (FALSE);
}


/*
 * Attempt to delete (some of) a random item from the store
 * Hack -- we attempt to "maintain" piles of items when possible.
 */
static void store_delete(void)
{
	int what, num;

	/* Pick a random slot */
	what = randint0(st_ptr->stock_num);

	/* Determine how many items are here */
	num = st_ptr->stock[what].number;

	/* Hack -- sometimes, only destroy half the items */
	if (randint0(100) < 50) num = (num + 1) / 2;

	/* Hack -- sometimes, only destroy a single item */
	if (randint0(100) < 50) num = 1;

	/* Hack -- decrement the maximum timeouts and total charges of rods and wands. -LM- */
	if ((st_ptr->stock[what].tval == TV_ROD) || (st_ptr->stock[what].tval == TV_WAND))
	{
		st_ptr->stock[what].pval -= num * st_ptr->stock[what].pval / st_ptr->stock[what].number;
	}

	/* Actually destroy (part of) the item */
	store_item_increase(what, -num);
	store_item_optimize(what);
}


/*
 * Creates a random item and gives it to a store
 * This algorithm needs to be rethought.  A lot.
 * Currently, "normal" stores use a pre-built array.
 *
 * Note -- the "level" given to "obj_get_num()" is a "favored"
 * level, that is, there is a much higher chance of getting
 * items with a level approaching that of the given level...
 *
 * Should we check for "permission" to have the given item?
 */
static void store_create(void)
{
	int i, tries, level;

	object_type forge;
	object_type *q_ptr;


	/* Paranoia -- no room left */
	if (st_ptr->stock_num >= st_ptr->stock_size) return;


	/* Hack -- consider up to four items */
	for (tries = 0; tries < 4; tries++)
	{
		/* Black Market */
		if (cur_store_num == STORE_BLACK)
		{
			/* Pick a level for object/magic */
			level = 15 + randint0(7);

			/* Random item (usually of given level) */
			i = get_obj_num(level);

			/* Handle failure */
			if (!i) continue;
		}

		/* Normal Store */
		else
		{
			/* Hack -- Pick an item to sell */
			i = st_ptr->table[randint0(st_ptr->table_num)];

			/* Hack -- fake level for apply_magic() */
			level = rand_range(STORE_MIN_LEVEL, STORE_MAX_LEVEL);
		}


		/* Get local object */
		q_ptr = &forge;

		/* Create a new object of the chosen kind */
		object_prep(q_ptr, i);

		/* Apply some "low-level" magic (no artifacts) */
		apply_magic(q_ptr, level, FALSE, FALSE, FALSE, FALSE);

		/* Require valid object */
		if (!store_will_buy(q_ptr)) continue;

		/* Hack -- Charge lite's */
		if (q_ptr->tval == TV_LITE)
		{
			if (q_ptr->sval == SV_LITE_TORCH) q_ptr->xtra3 = FUEL_TORCH / 2;
			if (q_ptr->sval == SV_LITE_LANTERN) q_ptr->xtra3 = FUEL_LAMP / 2;
		}


		/* The item is "known" */
		object_known(q_ptr);

		/* Mark it storebought */
		q_ptr->ident |= IDENT_STORE;

		/* Mega-Hack -- no chests in stores */
		if (q_ptr->tval == TV_CHEST) continue;

		/* Prune the black market */
		if (cur_store_num == STORE_BLACK)
		{
			/* Hack -- No "crappy" items */
			if (black_market_crap(q_ptr)) continue;

			/* Hack -- No "cheap" items */
			if (object_value(q_ptr) < 10) continue;

			/* No "worthless" items */
			/* if (object_value(q_ptr) <= 0) continue; */
		}

		/* Prune normal stores */
		else
		{
			/* No "worthless" items */
			if (object_value(q_ptr) <= 0) continue;
		}


		/* Mass produce and/or Apply discount */
		mass_produce(q_ptr);

		/* Attempt to carry the (known) item */
		(void)store_carry(q_ptr);

		/* Definitely done */
		break;
	}
}


/*
 * Re-displays a single store entry
 */
static void display_entry(int pos)
{
	int 		i, cur_col;
	object_type 	*o_ptr;
	s32b		x;

	char		o_name[MAX_NLEN];
	char		out_val[160];


	int maxwid;

	/* Get the item */
	o_ptr = &st_ptr->stock[pos];

	/* Get the "offset" */
	i = (pos % store_bottom);

	/* Label it, clear the line --(-- */
	(void)sprintf(out_val, "%c) ", ((i > 25) ? toupper(I2A(i - 26)) : I2A(i)));
	prt(out_val, i+6, 0);

	cur_col = 3;
	if (show_store_graph)
	{
		byte a = object_attr(o_ptr);
		char c = object_char(o_ptr);

		Term_queue_bigchar(cur_col, i + 6, a, c, 0, 0);
		if (use_bigtile) cur_col++;

		cur_col += 2;
	}

	/* Describe an item in the home */
	if (cur_store_num == STORE_HOME)
	{
		maxwid = 65;

		/* Describe the object */
		object_desc(o_name, o_ptr, 0);
		o_name[maxwid] = '\0';
		c_put_str(tval_to_attr[o_ptr->tval], o_name, i+6, cur_col);

		/* Show weights */
		show_weight(o_ptr, TRUE, i + 6, 68);
	}

	/* Describe an item (fully) in a store */
	else
	{
		/* Must leave room for the "price" */
		maxwid = 58;

		/* Describe the object (fully) */
		object_desc(o_name, o_ptr, 0);
		o_name[maxwid] = '\0';
		c_put_str(tval_to_attr[o_ptr->tval], o_name, i+6, cur_col);

		/* Show weights */
		show_weight(o_ptr, FALSE, i + 6, 61);

		/* Display a cost */

		/* Extract the "minimum" price */
		x = price_item(o_ptr, ot_ptr->min_inflate, FALSE);

		/* Hack -- Apply Sales Tax */
		if (x >= 10L) x += x / 10;

		/* Actually draw the price (with tax) */
		(void)sprintf(out_val, "%9ld  ", (long)x);
		put_str(out_val, i+6, 68);
	}
}


/*
 * Displays a store's inventory 		-RAK-
 * All prices are listed as "per individual object".  -BEN-
 */
static void display_inventory(void)
{
	int i, k;

	/* Display the next 12 items */
	for (k = 0; k < store_bottom; k++)
	{
		/* Do not display "dead" items */
		if (store_top + k >= st_ptr->stock_num) break;

		/* Display that line */
		display_entry(store_top + k);
	}

	/* Erase the extra lines and the "more" prompt */
	for (i = k; i < store_bottom + 1; i++) prt("", i + 6, 0);

	/* Assume "no current page" */
	put_str("          ", 5, 20);

	/* Visual reminder of "more items" */
	if (st_ptr->stock_num > store_bottom)
	{
		/* Show "more" reminder (after the last item) */
		prt(_("-続く-", "-more-"), k + 6, 3);

		/* Indicate the "current page" */
		put_str(format(_("(%dページ)", "(Page %d)"), store_top / store_bottom + 1), 5, 20);
	}
}


/*
 * Displays players gold					-RAK-
 */
static void store_prt_gold(void)
{
	char out_val[64];

	prt(_("手持ちのお金: ", "Gold Remaining: "), 19 + xtra_stock, 53);

	sprintf(out_val, "%9ld", (long)p_ptr->au);
	prt(out_val, 19 + xtra_stock, 68);
}


/*
 * Displays store (after clearing screen)		-RAK-
 */
static void display_store(void)
{
	char buf[80];


	/* Clear screen */
	Term_clear();

	/* The "Home" is special */
	if (cur_store_num == STORE_HOME)
	{
		/* Put the owner name */
		put_str(_("我が家", "Your Home"), 3, _(31, 30));

		/* Label the item descriptions */
		put_str(_("アイテムの一覧", "Item Description"), 5, _(4, 3));

		/* Show weight label */
		put_str(_("重さ", "Weight"), 5, _(72, 70));
	}

	/* Normal stores */
	else
	{
		cptr store_name = (f_name + f_info[FEAT_SHOP_HEAD + cur_store_num].name);
		cptr owner_name = (ot_ptr->owner_name);

		/* Put the owner name */
		sprintf(buf, "%s", owner_name);
		put_str(buf, 3, 10);

		/* Show the max price in the store (above prices) */
		sprintf(buf, "%s (%ld)", store_name, (long)(ot_ptr->max_cost));
		prt(buf, 3, 50);

		/* Label the item descriptions */
		put_str(_("商品の一覧", "Item Description"), 5, _(7, 3));

		/* Show weight label */
		put_str(_("重さ", "Weight"), 5, _(62, 60));

		/* Label the asking price (in stores) */
		put_str(_("価格", "Price"), 5, _(73, 72));
	}

	/* Display the current gold */
	store_prt_gold();

	/* Draw in the inventory */
	display_inventory();
}


/*
 * Get the ID of a store item and return its value	-RAK-
 */
static int get_stock(int *com_val, cptr pmt, int i, int j)
{
	char	command;
	char	out_val[160];
	char	lo, hi;

	/* Repeat previous command */
	/* Get the item index */
	if (repeat_pull(com_val))
	{
		/* Verify the item */
		if ((*com_val >= i) && (*com_val <= j))
		{
			/* Success */
			return (TRUE);
		}
	}

	/* Paranoia XXX XXX XXX */
	msg_print(NULL);


	/* Assume failure */
	*com_val = (-1);

	/* Build the prompt */
	lo = I2A(i);
	hi = (j > 25) ? toupper(I2A(j - 26)) : I2A(j);
	(void)sprintf(out_val,
		_("(%s:%c-%c, ESCで中断) %s", "(%s %c-%c, ESC to exit) %s"),
		_(((cur_store_num == STORE_HOME) ? "アイテム" : "商品"), "Items"),
				  lo, hi, pmt);

	/* Ask until done */
	while (TRUE)
	{
		int k;

		/* Escape */
		if (!get_com(out_val, &command)) break;

		/* Convert */
		if (islower(command))
			k = A2I(command);
		else if (isupper(command))
			k = A2I(tolower(command)) + 26;
		else
			k = -1;

		/* Legal responses */
		if ((k >= i) && (k <= j))
		{
			*com_val = k;
			break;
		}

		/* Oops */
		bell();
	}

	/* Clear the prompt */
	prt("", 0, 0);

	/* Cancel */
	if (command == ESCAPE) return (FALSE);

	/* Remember repeat command */
	repeat_push(*com_val);

	/* Success */
	return (TRUE);
}


/*
 * Haggling routine 				-RAK-
 *
 * Return TRUE if purchase is NOT successful
 * Modified not to haggle by TinyAngband
 */
static bool purchase_haggle(object_type *o_ptr, s32b *price)
{
	s32b offer;

	/* Reset price */
	*price = 0;

	/* Get price of an item */
	offer = price_item(o_ptr, ot_ptr->min_inflate, FALSE);

	/* Message summary */
	msg_print(_("すんなりとこの金額にまとまった。", "You quickly agree upon the price."));
	msg_print(NULL);

	/* Apply Sales Tax */
	if (offer >= 10L) offer += offer / 10;

	/* Price of the whole pile */
	offer *= o_ptr->number;

	/* Confirm buying */
	if (confirm_store)
	{
		char i;
#ifdef JP
		prt(format("提示価格: %d", offer), 1, 0);
		prt("本当に買いますか？[Y/n] ", 0, 0);
#else
		prt(format("Offer: %d", offer), 1, 0);
		prt(format("Really buy %s? [Y/n] ", ((o_ptr->number > 1) ? "them" : "it")), 0, 0);
#endif
		i = inkey();
		prt("", 0, 0);

		/* Cancel */
		if (i == ESCAPE) return (TRUE);
		if (my_strchr("Nn", i)) return (TRUE);
	}

	/* Return price */
	*price = offer;

	/* Do not cancel */
	return (FALSE);
}


/*
 * Haggling routine 				-RAK-
 *
 * Return TRUE if purchase is NOT successful
 */
static bool sell_haggle(object_type *o_ptr, s32b *price)
{
	s32b    purse, offer;

	/* Reset price */
	*price = 0;

	/* Get price of an item */
	offer = price_item(o_ptr, ot_ptr->min_inflate, TRUE);

	/* Get the owner's payout limit */
	purse = (s32b)(ot_ptr->max_cost);

	/* More than owner's payout limit */
	if (offer >= purse)
	{
		/* Message */
		msg_print(_("即座にこの金額にまとまった。", "You instantly agree upon the price."));
		msg_print(NULL);

		/* Offer full purse */
		offer = purse;
	}

	/* Less than the limit */
	else
	{
		/* Message summary */
		msg_print(_("すんなりとこの金額にまとまった。", "You quickly agree upon the price."));
		msg_print(NULL);

		/* Apply Sales Tax */
		offer -= offer / 10;
	}

	/* Price of the whole pile */
	offer *= o_ptr->number;

	/* Confirm selling */
	if (confirm_store)
	{
		char i;
#ifdef JP
		prt(format("提示価格: %d", offer), 1, 0);
		prt("本当に売りますか？[Y/n] ", 0, 0);
#else
		prt(format("Offer: %d", offer), 1, 0);
		prt(format("Really sell %s? [Y/n] ", ((o_ptr->number > 1) ? "them" : "it")), 0, 0);
#endif
		i = inkey();
		prt("", 0, 0);

		/* Cancel */
		if (i == ESCAPE) return (TRUE);
		if (my_strchr("Nn", i)) return (TRUE);
	}

	/* Return price */
	*price = offer;

	/* Do not cancel */
	return (FALSE);
}


/*
 * Return the quantity of a given item in the pack.
 * Code taken from Angband 3.1.0 under Angband license
 */
static int find_inven(object_type *o_ptr)
{
	int j;
	int num = 0;

	/* Similar slot? */
	for (j = 0; j < INVEN_PACK; j++)
	{
		object_type *j_ptr = &inventory[j];

		/* Require identical object types */
		if (!j_ptr->k_idx || o_ptr->k_idx != j_ptr->k_idx) continue;

		/* Analyze the items */
		switch (o_ptr->tval)
		{
			/* Chests */
			case TV_CHEST:
			{
				/* Never okay */
				return 0;
			}

			/* Food and Potions and Scrolls */
			case TV_FOOD:
			case TV_POTION:
			case TV_SCROLL:
			{
				/* Assume okay */
				break;
			}

			/* Staves and Wands */
			case TV_STAFF:
			case TV_WAND:
			{
				/* Assume okay */
				break;
			}

			/* Rods */
			case TV_ROD:
			{
				/* Assume okay */
				break;
			}

			/* Weapons and Armor */
			case TV_BOW:
			case TV_DIGGING:
			case TV_HAFTED:
			case TV_POLEARM:
			case TV_SWORD:
			case TV_BOOTS:
			case TV_GLOVES:
			case TV_HELM:
			case TV_CROWN:
			case TV_SHIELD:
			case TV_CLOAK:
			case TV_SOFT_ARMOR:
			case TV_HARD_ARMOR:
			case TV_DRAG_ARMOR:
			{
				/* Fall through */
			}

			/* Rings, Amulets, Lites */
			case TV_RING:
			case TV_AMULET:
			case TV_LITE:
			{
				/* Require both items to be known */
				if (!object_known_p(o_ptr) || !object_known_p(j_ptr)) continue;

				/* Fall through */
			}

			/* Missiles */
			case TV_BOLT:
			case TV_ARROW:
			case TV_SHOT:
			{
				/* Require identical knowledge of both items */
				if (object_known_p(o_ptr) != object_known_p(j_ptr)) continue;

				/* Require identical "bonuses" */
				if (o_ptr->to_h != j_ptr->to_h) continue;
				if (o_ptr->to_d != j_ptr->to_d) continue;
				if (o_ptr->to_a != j_ptr->to_a) continue;

				/* Require identical "pval" code */
				if (o_ptr->pval != j_ptr->pval) continue;

				/* Require identical "artifact" names */
				if (o_ptr->name1 != j_ptr->name1) continue;

				/* Require identical "ego-item" names */
				if (o_ptr->name2 != j_ptr->name2) continue;

				/* Lites must have same amount of fuel */
				else if (o_ptr->timeout != j_ptr->timeout && o_ptr->tval == TV_LITE)
					continue;

				/* Require identical "values" */
				if (o_ptr->ac != j_ptr->ac) continue;
				if (o_ptr->dd != j_ptr->dd) continue;
				if (o_ptr->ds != j_ptr->ds) continue;

				/* Probably okay */
				break;
			}

			/* Various */
			default:
			{
				/* Require knowledge */
				if (!object_known_p(o_ptr) || !object_known_p(j_ptr)) continue;

				/* Probably okay */
				break;
			}
		}


		/* Different pseudo-ID statuses preclude combination */
		if (o_ptr->feeling != j_ptr->feeling) continue;


		/* Different flags */ 
		if (o_ptr->art_flags1 != j_ptr->art_flags1 ||
			o_ptr->art_flags2 != j_ptr->art_flags2 ||
			o_ptr->art_flags3 != j_ptr->art_flags3)
			continue;

		/* They match, so add up */
		num += j_ptr->number;
	}

	return num;
}


/*
 * Buy an item from a store 			-RAK-
 */
static void store_purchase(void)
{
	int i, amt, choice;
	int item, item_new;
	int num;

	s32b price, best;

	object_type forge;
	object_type *j_ptr;

	object_type *o_ptr;

	char o_name[MAX_NLEN];

	char out_val[160];


	/* Empty? */
	if (st_ptr->stock_num <= 0)
	{
		if (cur_store_num == STORE_HOME)
			msg_print(_("我が家には何も置いてありません。", "Your home is empty."));
		else
			msg_print(_("現在商品の在庫を切らしています。", "I am currently out of stock."));
		return;
	}


	/* Find the number of objects on this and following pages */
	i = (st_ptr->stock_num - store_top);

	/* And then restrict it to the current page */
	if (i > store_bottom) i = store_bottom;

	/* Prompt */
	/* Different message for black market and home */
	switch( cur_store_num )
	{
		case STORE_HOME:
			sprintf(out_val, _("どのアイテムを取りますか? ", "Which item do you want to take? "));
			break;
		case STORE_BLACK:
			sprintf(out_val, _("どれ? ", "Waddya want?"));
			break;
		default:
			sprintf(out_val, _("どの品物が欲しいんだい? ", "Which item are you interested in? "));
			break;
	}

	/* Get the item number to be bought */
	if (!get_stock(&item, out_val, 0, i - 1)) return;

	/* Get the actual index */
	item = item + store_top;

	/* Get the actual item */
	o_ptr = &st_ptr->stock[item];

	/* Assume the player wants just one of them */
	amt = 1;

	/* Get local object */
	j_ptr = &forge;

	/* Get a copy of the object */
	object_copy(j_ptr, o_ptr);

	/* Recalculate charges for a single wand/rod */
	reduce_charges(j_ptr, j_ptr->number - 1);

	/* Modify quantity */
	j_ptr->number = amt;

	/* Hack -- require room in pack */
	if (!inven_carry_okay(j_ptr))
	{
		msg_print(_("そんなにアイテムを持てない。", "You cannot carry that many different items."));
		return;
	}

	/* Determine the "best" price (per item) */
	best = price_item(j_ptr, ot_ptr->min_inflate, FALSE);
	if (best >= 10L) best += best / 10; /* Sales tax */ 


	/* Find out how many the player wants */
	if (o_ptr->number > 1)
	{
		/* Hack -- note cost of "fixed" items */ 
		if ((cur_store_num != STORE_HOME) &&
			(o_ptr->ident & IDENT_FIXED))
		{
			msg_format(_("一つにつき $%ldです。", "That costs %ld gold per item."), (long)(best));
		}

		if (cur_store_num == STORE_HOME)
		{
			amt = o_ptr->number;
		}
		else
		{
			/* Check if the player can afford any at all */
			if ((u32b)p_ptr->au < (u32b)best)
			{
				/* Tell the user */
				/* Even if haggling was on this money is not sufficient to buy one */
				msg_print(_("お金が足りません。", "You do not have enough gold for this item."));

				/* Abort now */
				return;
			}
	
			/* Work out how many the player can afford */
			if((p_ptr->au / best) > o_ptr->number)
			{
				amt = o_ptr->number;
			}
			else
			{
				amt = p_ptr->au / best;
			}
	       
		}

		/* Find the number of this item in the inventory */
		if (!object_aware_p(o_ptr))
			num = 0;
		else
			num = find_inven(o_ptr);

#ifdef JP
		strnfmt(o_name, sizeof(o_name), "いくつですか？(%s最大 %d個, 'a'全て) ",  
			(num ? format("現在 %d個，", num) : ""),
			amt); 
#else
		strnfmt(o_name, sizeof(o_name), "%s how many? (%smax %d, 'a' to all) ",
			(cur_store_num == STORE_HOME) ? "Take" : "Buy",
			(num ? format("you have %d, ", num) : ""),
			amt);
#endif

		/* Get a quantity */
		amt = get_quantity(o_name, amt);

		/* Allow user abort */
		if (amt <= 0) return;
	}

	/* Get local object */
	j_ptr = &forge;

	/* Get desired object */
	object_copy(j_ptr, o_ptr);

	/*
	 * If a rod or wand, allocate total maximum timeouts or charges
	 * between those purchased and left on the shelf.
	 */
	reduce_charges(j_ptr, j_ptr->number - amt);

	/* Modify quantity */
	j_ptr->number = amt;

	/* Hack -- require room in pack */
	if (!inven_carry_okay(j_ptr))
	{
		msg_print(_("ザックにそのアイテムを入れる隙間がない。", "You cannot carry that many items."));
		return;
	}

	/* Attempt to buy it */
	if (cur_store_num != STORE_HOME)
	{
		/* Fixed price, quick buy */
		if (o_ptr->ident & (IDENT_FIXED))
		{
			/* Assume accept */
			choice = 0;

			/* Go directly to the "best" deal */
			price = (best * j_ptr->number);
		}

		/* Haggle for it */
		else
		{
			/* Describe the object (fully) */
			object_desc(o_name, j_ptr, 0);

			/* Message */
			msg_format(_("%s(%c)を購入する。", "Buying %s (%c)."), o_name, I2A(item));
			msg_print(NULL);

			/* Haggle for a final price */
			choice = purchase_haggle(j_ptr, &price);

			/* Hack -- Got kicked out */
			if (st_ptr->store_open >= turn) return;
		}


		/* Player wants it */
		if (choice == 0)
		{
			/* Fix the item price (if "correctly" haggled) */
			if (price == (best * j_ptr->number)) o_ptr->ident |= (IDENT_FIXED);

			/* Player can afford it */
			if (p_ptr->au >= price)
			{
				/* Say "okay" */
				say_comment_1();

				/* Make a sound */
				sound(SOUND_STORE5);

				/* Spend the money */
				p_ptr->au -= price;

				/* Update the display */
				store_prt_gold();

				/* Hack -- buying an item makes you aware of it */
				object_aware(j_ptr);

				/* Hack -- clear the "fixed" flag from the item */
				j_ptr->ident &= ~(IDENT_FIXED);

				/* Describe the transaction */
				object_desc(o_name, j_ptr, 0);

				/* Message */
				msg_format(_("%sを $%ldで購入しました。", "You bought %s for %ld gold."),
					o_name, (long)price);

				/* Erase the inscription */
				j_ptr->inscription = 0;

				/* Erase the "feeling" */
				j_ptr->feeling = FEEL_NONE;
				j_ptr->ident &= ~(IDENT_STORE);

				/* Give it to the player */
				item_new = inven_carry(j_ptr);

				/* Describe the final result */
				object_desc(o_name, &inventory[item_new], 0);

				/* Message */
				msg_format(_("%s(%c)を手に入れた。", "You have %s (%c)."),
					o_name, index_to_label(item_new));

				/* Auto-inscription */
				autopick_alter_item(item_new, FALSE);

				/* Now, reduce the original stack's pval. */
				if ((o_ptr->tval == TV_ROD) || (o_ptr->tval == TV_WAND))
				{
					o_ptr->pval -= j_ptr->pval;
				}

				/* Handle stuff */
				handle_stuff();

				/* Note how many slots the store used to have */
				i = st_ptr->stock_num;

				/* Remove the bought items from the store */
				store_item_increase(item, -amt);
				store_item_optimize(item);

				/* Store is empty */
				if (st_ptr->stock_num == 0)
				{
					/* Shuffle */
					if (randint0(STORE_SHUFFLE) == 0)
					{
						/* Message */
						msg_print(_("店主は引退した。", "The shopkeeper retires."));

						/* Shuffle the store */
						store_shuffle(cur_store_num);
					}

					/* Maintain */
					else
					{
						/* Message */
						msg_print(_("店主は新たな在庫を取り出した。",
							"The shopkeeper brings out some new stock."));
					}

					/* New inventory */
					for (i = 0; i < 10; i++)
					{
						/* Maintain the store */
						store_maint(p_ptr->town_num, cur_store_num);
					}

					/* Start over */
					store_top = 0;

					/* Redraw everything */
					display_inventory();
				}

				/* The item is gone */
				else if (st_ptr->stock_num != i)
				{
					/* Pick the correct screen */
					if (store_top >= st_ptr->stock_num) store_top -= store_bottom;

					/* Redraw everything */
					display_inventory();
				}

				/* Item is still here */
				else
				{
					/* Redraw the item */
					display_entry(item);
				}
			}

			/* Player cannot afford it */
			else
			{
				/* Simple message */
				msg_print(_("お金が足りません。", "You do not have enough gold."));
			}
		}
	}

	/* Home is much easier */
	else
	{
		bool combined_or_reordered;

		/* Distribute charges of wands/rods */
		distribute_charges(o_ptr, j_ptr, amt);

		/* Give it to the player */
		item_new = inven_carry(j_ptr);

		/* Describe just the result */
		object_desc(o_name, &inventory[item_new], 0);

		/* Message */
		msg_format(_("%s(%c)を取った。", "You have %s (%c)."),
			 o_name, index_to_label(item_new));

		/* Handle stuff */
		handle_stuff();

		/* Take note if we take the last one */
		i = st_ptr->stock_num;

		/* Remove the items from the home */
		store_item_increase(item, -amt);
		store_item_optimize(item);

		combined_or_reordered = combine_and_reorder_home();

		/* Hack -- Item is still here */
		if (i == st_ptr->stock_num)
		{
			/* Redraw everything */
			if (combined_or_reordered) display_inventory();

			/* Redraw the item */
			else display_entry(item);
		}

		/* The item is gone */
		else
		{
			/* Nothing left */
			if (st_ptr->stock_num == 0) store_top = 0;

			/* Nothing left on that screen */
			else if (store_top >= st_ptr->stock_num) store_top -= store_bottom;

			/* Redraw everything */
			display_inventory();
		}
	}

	/* Not kicked out */
	return;
}


/*
 * Sell an item to the store (or home)
 */
static void store_sell(void)
{
	int choice;
	int item, item_pos;
	int amt;

	s32b price, value, dummy;

	object_type forge;
	object_type *q_ptr;

	object_type *o_ptr;

	cptr q, s;

	char o_name[MAX_NLEN];


	/* Prepare a prompt */
	if (cur_store_num == STORE_HOME)
		q = _("どのアイテムを置きますか? ", "Drop which item? ");
	else
		q = _("どのアイテムを売りますか? ", "Sell which item? ");

	/* Only allow items the store will buy */
	item_tester_hook = store_will_buy;

	/* Get an item */
	if (cur_store_num == STORE_HOME)
	{
		s = _("置けるアイテムを持っていません。", "You have nothing that you put in.");
	}
	else
	{
		s = _("欲しい物がないですねえ。", "You have nothing that I want.");
	}

	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return;

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


	/* Hack -- Cannot remove cursed items */
	if ((item >= INVEN_WIELD) && cursed_p(o_ptr))
	{
		/* Oops */
		msg_print(_("ふーむ、どうやらそれは呪われているようだね。", "Hmmm, it seems to be cursed."));

		/* Nope */
		return;
	}


	/* Assume one item */
	amt = 1;

	/* Find out how many the player wants (letter means "all") */
	if (o_ptr->number > 1)
	{
		/* Get a quantity */
		amt = get_quantity(NULL, o_ptr->number);

		/* Allow user abort */
		if (amt <= 0) return;
	}

	/* Get local object */
	q_ptr = &forge;

	/* Get a copy of the object */
	object_copy(q_ptr, o_ptr);

	/* Modify quantity */
	q_ptr->number = amt;

	/*
	 * Hack -- If a rod or wand, allocate total maximum
	 * timeouts or charges to those being sold. -LM-
	 */
	if ((o_ptr->tval == TV_ROD) || (o_ptr->tval == TV_WAND))
	{
		q_ptr->pval = o_ptr->pval * amt / o_ptr->number;
	}

	/* Get a full description */
	object_desc(o_name, q_ptr, 0);

	/* Remove any inscription, feeling for stores */
	if (cur_store_num != STORE_HOME)
	{
		q_ptr->inscription = 0;
		q_ptr->feeling = FEEL_NONE;
	}

	/* Is there room in the store (or the home?) */
	if (!store_check_num(q_ptr))
	{
		if (cur_store_num == STORE_HOME)
			msg_print(_("我が家にはもう置く場所がない。", "Your home is full."));
		else
			msg_print(_("すいませんが、店にはもう置く場所がありません。",
			"I have not the room in my store to keep it."));
		return;
	}


	/* Real store */
	if (cur_store_num != STORE_HOME)
	{
		/* Describe the transaction */
		msg_format(_("%s(%c)を売却する。", "Selling %s (%c)."),
			o_name, index_to_label(item));
		msg_print(NULL);

		/* Haggle for it */
		choice = sell_haggle(q_ptr, &price);

		/* Kicked out */
		if (st_ptr->store_open >= turn) return;

		/* Sold... */
		if (choice == 0)
		{
			/* Say "okay" */
			say_comment_1();

			/* Make a sound */ 
			sound(SOUND_STORE5);

			/* Get some money */
			p_ptr->au += price;

			/* Update the display */
			store_prt_gold();

			/* Get the "apparent" value */
			dummy = object_value(q_ptr) * q_ptr->number;

			/* Identify it */
			identify_item(o_ptr);

			/* Get local object */
			q_ptr = &forge;

			/* Get a copy of the object */
			object_copy(q_ptr, o_ptr);

			/* Modify quantity */
			q_ptr->number = amt;

			/* Make it look like to be known */
			q_ptr->ident |= IDENT_STORE;

			/*
			 * Hack -- If a rod or wand, let the shopkeeper know just
			 * how many charges he really paid for. -LM-
			 */
			if ((o_ptr->tval == TV_ROD) || (o_ptr->tval == TV_WAND))
			{
				q_ptr->pval = o_ptr->pval * amt / o_ptr->number;
			}

			/* Get the "actual" value */
			value = object_value(q_ptr) * q_ptr->number;

			/* Get the description all over again */
			object_desc(o_name, q_ptr, 0);

			/* Describe the result (in message buffer) */
			msg_format(_("%sを $%ldで売却しました。", "You sold %s for %ld gold."),
				o_name, (long)price);

			if (!((o_ptr->tval == TV_FIGURINE) && (value > 0)))
			{
				/* Analyze the prices (and comment verbally) unless a figurine*/
				purchase_analyze(price, value, dummy);
			}

			/*
			 * Hack -- Allocate charges between those wands or rods sold
			 * and retained, unless all are being sold. -LM-
			 */
			distribute_charges(o_ptr, q_ptr, amt);

			/* Reset timeouts of the sold items */
			q_ptr->timeout = 0;

			/* Take the item from the player, describe the result */
			inven_item_increase(item, -amt);
			inven_item_describe(item);

			/* If items remain, auto-inscribe before optimizing */
			if (o_ptr->number > 0) autopick_alter_item(item, FALSE);

			inven_item_optimize(item);

			/* Handle stuff */
			handle_stuff();

			/* The store gets that (known) item */
			item_pos = store_carry(q_ptr);

			/* Re-display if item is now in store */
			if (item_pos >= 0)
			{
				store_top = (item_pos / store_bottom) * store_bottom;
				display_inventory();
			}

			/* swap hands when two handed combat */
			if (item == INVEN_WIELD) swap_wielding_hands();
		}
	}

	/* Player is at home */
	else
	{
		/* Distribute charges of wands/rods */
		distribute_charges(o_ptr, q_ptr, amt);

		/* Describe */
		msg_format(_("%sを置いた。(%c)", "You drop %s (%c)."), o_name, index_to_label(item));

		/* Take it from the players inventory */
		inven_item_increase(item, -amt);
		inven_item_describe(item);
		inven_item_optimize(item);

		/* Handle stuff */
		handle_stuff();

		/* Let the home carry it */
		item_pos = home_carry(q_ptr);

		/* Update store display */
		if (item_pos >= 0)
		{
			store_top = (item_pos / store_bottom) * store_bottom;
			display_inventory();
		}

		/* swap hands when two handed combat */
		if (item == INVEN_WIELD) swap_wielding_hands();
	}
}


/*
 * Examine an item in a store			   -JDL-
 */
static void store_examine(void)
{
	int         i;
	int         item;
	object_type *o_ptr;
	char        o_name[MAX_NLEN];
	char        out_val[160];


	/* Empty? */
	if (st_ptr->stock_num <= 0)
	{
		if (cur_store_num == STORE_HOME)
			msg_print(_("我が家には何も置いてありません。", "Your home is empty."));
		else
			msg_print(_("現在商品の在庫を切らしています。", "I am currently out of stock."));
		return;
	}


	/* Find the number of objects on this and following pages */
	i = (st_ptr->stock_num - store_top);

	/* And then restrict it to the current page */
	if (i > store_bottom) i = store_bottom;

	/* Prompt */
	sprintf(out_val, _("どれを調べますか？", "Which item do you want to examine? "));

	/* Get the item number to be examined */
	if (!get_stock(&item, out_val, 0, i - 1)) return;

	/* Get the actual index */
	item = item + store_top;

	/* Get the actual item */
	o_ptr = &st_ptr->stock[item];

	/* Require full knowledge */
	if (!(o_ptr->ident & IDENT_MENTAL))
	{
		/* This can only happen in the home */
		msg_print(_("このアイテムについて特に知っていることはない。",
			"You have no special knowledge about that item."));
		return;
	}

	/* Description */
	object_desc(o_name, o_ptr, 0);

	/* Describe */
	msg_format(_("%sを調べている...", "Examining %s..."), o_name);

	/* Describe it fully */
	if (!identify_fully_aux(o_ptr, TRUE))
		msg_print(_("特に変わったところはないようだ。", "You see nothing special."));

	return;
}


/*
 * Hack -- set this to leave the store
 */
static bool leave_store = FALSE;


/*
 * Process a command in a store
 *
 * Note that we must allow the use of a few "special" commands
 * in the stores which are not allowed in the dungeon, and we
 * must disable some commands which are allowed in the dungeon
 * but not in the stores, to prevent chaos.
 */
static void store_process_command(void)
{

	/* Handle repeating the last command */
	repeat_check();

	if (rogue_like_commands && command_cmd == 'l')
	{
		command_cmd = 'x';	/* hack! */
	}

	/* Parse the command */
	switch (command_cmd)
	{
		/* Leave */
		case ESCAPE:
		{
			leave_store = TRUE;
			break;
		}

		/* 日本語版追加 */
		/* 1 ページ戻るコマンド: 我が家のページ数が多いので重宝するはず By BUG */
		case '-':
		{
			if (st_ptr->stock_num <= store_bottom) {
				msg_print(_("これで全部です。", "Entire inventory is shown."));
			}
			else{
				store_top -= store_bottom;
				if ( store_top < 0 )
					store_top = ((st_ptr->stock_num - 1) / store_bottom) * store_bottom;
				display_inventory();
			}
			break;
		}

		/* Browse */
		case ' ':
		{
			if (st_ptr->stock_num <= store_bottom)
			{
				msg_print(_("これで全部です。", "Entire inventory is shown."));
			}
			else
			{
				store_top += store_bottom;
				if (store_top >= st_ptr->stock_num) store_top = 0;

				display_inventory();
			}
			break;
		}

		/* Redraw */
		case KTRL('R'):
		{
			do_cmd_redraw();
			display_store();
			break;
		}

		/* Get (purchase) */
		case 'g':
		{
			store_purchase();
			break;
		}

		/* Drop (Sell) */
		case 'd':
		{
			store_sell();
			break;
		}

		/* Examine */
		case 'x':
		{
			store_examine();
			break;
		}

		/* Ignore return */
		case '\r':
		case '\n':
		{
			break;
		}

		/*** Inventory Commands ***/

		/* Wear/wield equipment */
		case 'w':
		{
			do_cmd_wield();
			break;
		}

		/* Take off equipment */
		case 't':
		{
			do_cmd_takeoff();
			break;
		}

		/* Destroy an item */
		case 'k':
		{
			do_cmd_destroy();
			break;
		}

		/* Equipment list */
		case 'e':
		{
			do_cmd_equip();
			break;
		}

		/* Inventory list */
		case 'i':
		{
			do_cmd_inven();
			break;
		}


		/*** Various commands ***/

		/* Identify an object */
		case 'I':
		{
			do_cmd_observe();
			break;
		}

		/* Hack -- toggle windows */
		case KTRL('I'):
		{
			toggle_inven_equip();
			break;
		}


		/*** Use various objects ***/

		/* Browse a book */
		case 'b':
		{
			do_cmd_browse();
			break;
		}

		/* Inscribe an object */
		case '{':
		{
			do_cmd_inscribe();
			break;
		}

		/* Uninscribe an object */
		case '}':
		{
			do_cmd_uninscribe();
			break;
		}


		/*** Help and Such ***/

		/* Help */
		case '?':
		{
			do_cmd_help();
			break;
		}

		/* Identify symbol */
		case '/':
		{
			do_cmd_query_symbol();
			break;
		}

		/* Character description */
		case 'C':
		{
			p_ptr->town_num = old_town_num;
			do_cmd_character();
			p_ptr->town_num = inner_town_num;
			display_store();
			break;
		}


		/*** System Commands ***/

		/* Hack -- User interface */
		case '!':
		{
			(void)Term_user(0);
			break;
		}

		/* Single line from a pref file */
		case '"':
		{
			p_ptr->town_num = old_town_num;
			do_cmd_pref();
			p_ptr->town_num = inner_town_num;
			break;
		}

		/* Interact with macros */
		case '@':
		{
			p_ptr->town_num = old_town_num;
			do_cmd_macros();
			p_ptr->town_num = inner_town_num;
			break;
		}

		/* Interact with visuals */
		case '%':
		{
			p_ptr->town_num = old_town_num;
			do_cmd_visuals();
			p_ptr->town_num = inner_town_num;
			break;
		}

		/* Interact with colors */
		case '&':
		{
			p_ptr->town_num = old_town_num;
			do_cmd_colors();
			p_ptr->town_num = inner_town_num;
			break;
		}

		/* Interact with options */
		case '=':
		{
			do_cmd_options();
			(void)combine_and_reorder_home();
			do_cmd_redraw();
			display_store();
			break;
		}

		/*** Misc Commands ***/

		/* Take notes */
		case ':':
		{
			do_cmd_note();
			break;
		}

		/* Version info */
		case 'V':
		{
			do_cmd_version();
			break;
		}

		/* Repeat level feeling */
		case KTRL('F'):
		{
			do_cmd_feeling();
			break;
		}

		/* Show previous message */
		case KTRL('O'):
		{
			do_cmd_message_one();
			break;
		}

		/* Show previous messages */
		case KTRL('P'):
		{
			do_cmd_messages(0);
			break;
		}

		/* Check artifacts, uniques etc. */
		case '~':
		case '|':
		{
			do_cmd_knowledge();
			break;
		}

		/* Load "screen dump" */
		case '(':
		{
			do_cmd_load_screen();
			break;
		}

		/* Save "screen dump" */
		case ')':
		{
			do_cmd_save_screen();
			break;
		}

		/* Hack -- Unknown command */
		default:
		{
			msg_print(_("そのコマンドは店の中では使えません。", "That command does not work in stores."));
			break;
		}
	}
}


/*
 * Enter a store, and interact with it.
 *
 * Note that we use the standard "request_command()" function
 * to get a command, allowing us to use "command_arg" and all
 * command macros and other nifty stuff, but we use the special
 * "shopping" argument, to force certain commands to be converted
 * into other commands, normally, we convert "p" (pray) and "m"
 * (cast magic) into "g" (get), and "s" (search) into "d" (drop).
 */
void do_cmd_store(void)
{
	int         which;
	int         maintain_num;
	int         tmp_chr;
	int         i;
	cave_type   *c_ptr;
	int w, h;


	/* Get term size */
	Term_get_size(&w, &h);

	/* Calculate stocks per 1 page */
	xtra_stock = MIN(14+26, ((h > 24) ? (h - 24) : 0));
	store_bottom = MIN_STOCK + xtra_stock;

	/* Access the player grid */
	c_ptr = &cave[py][px];

	/* Verify a store */
	if (!((c_ptr->feat >= FEAT_SHOP_HEAD) &&
		  (c_ptr->feat <= FEAT_SHOP_TAIL)))
	{
		msg_print(_("ここには店がありません。", "You see no store here."));
		return;
	}

	/* Extract the store code */
	which = (c_ptr->feat - FEAT_SHOP_HEAD);

	old_town_num = p_ptr->town_num;

	/* Hack -- My Home is same in all town */
	if (which == STORE_HOME) p_ptr->town_num = 1;

	/* Hack -- Extra shops in quest */
	if (p_ptr->inside_quest &&
		(quest[p_ptr->inside_quest].type != QUEST_TYPE_RANDOM))
		p_ptr->town_num = TOWN_DUNGEON;

	/* Hack -- Extra shops in dungeon */
	if (dun_level) p_ptr->town_num = TOWN_DUNGEON;

	inner_town_num = p_ptr->town_num;

	/* Hack -- Check the "locked doors" */
	if ((town[p_ptr->town_num].store[which].store_open >= turn) ||
	    (ironman_shops))
	{
		msg_print(_("ドアに鍵がかかっている。", "The doors are locked."));
		p_ptr->town_num = old_town_num;
		return;
	}

	/* Calculate the number of store maintainances since the last visit */
	maintain_num = (turn - town[p_ptr->town_num].store[which].last_visit) / (10L * STORE_TURNS);

	/* Maintain the store max. 10 times */
	if (maintain_num > 10) maintain_num = 10;

	if (maintain_num)
	{
		/* Maintain the store */
		for (i = 0; i < maintain_num; i++)
			store_maint(p_ptr->town_num, which);

		/* Save the visit */
		town[p_ptr->town_num].store[which].last_visit = turn;
	}

	/* Forget the lite */
	forget_lite();

	/* Forget the view */
	forget_view();


	/* Hack -- Character is in "icky" mode */
	character_icky = TRUE;


	/* No command argument */
	command_arg = 0;

	/* No repeated command */
	command_rep = 0;

	/* No automatic command */
	command_new = 0;


	/* Save the store number */
	cur_store_num = which;

	/* Save the store and owner pointers */
	st_ptr = &town[p_ptr->town_num].store[cur_store_num];
	ot_ptr = &owners[cur_store_num][st_ptr->owner];

	/* Start at the beginning */
	store_top = 0;

	/* Display the store */
	display_store();

	/* Do not leave */
	leave_store = FALSE;

	/* BGM */
	bgm(BGM_STORE);

	/* Interact with player */
	while (!leave_store)
	{
		/* Hack -- Clear line 1 */
		prt("", 1, 0);

		/* Hack -- Check the charisma */
		tmp_chr = p_ptr->stat_use[A_CHR];

		/* Clear */
		clear_from(20 + xtra_stock);


		/* Basic commands */
		prt(_(" ESC) 建物から出る", " ESC) Exit from Building."), 21 + xtra_stock, 0);

		/* Browse if necessary */
		if (st_ptr->stock_num > store_bottom)
		{
			prt(_(" -)前ページ", " -) Previous page"), 22 + xtra_stock, 0);
			prt(_(" スペース) 次ページ", " SPACE) Next page"), 23 + xtra_stock, 0);
		}

		/* Home commands */
		if (cur_store_num == STORE_HOME)
		{
			prt(_("g) アイテムを取る", "g) Get an item."), 21 + xtra_stock, 27);
			prt(_("d) アイテムを置く", "d) Drop an item."), 22 + xtra_stock, 27);
			prt(_("x) 家のアイテムを調べる", "x) eXamine an item in the home."), 23 + xtra_stock, 27);
		}

		/* Shop commands XXX XXX XXX */
		else
		{
			prt(_("p) 商品を買う", "p) Purchase an item."), 21 + xtra_stock, 30);
			prt(_("s) アイテムを売る", "s) Sell an item."), 22 + xtra_stock, 30);
			prt(_("x) 商品を調べる", "x) eXamine an item in the shop"), 23 + xtra_stock,30);
		}

		/* 基本的なコマンドの追加表示 */
		prt(_("i/e) 持ち物/装備の一覧", "i/e) Inventry/Equipment list"), 21 + xtra_stock, 56);

		if (rogue_like_commands)
		{
			prt(_("w/T) 装備する/はずす", "w/T) Wear/Take off equipment"), 22 + xtra_stock, 56);
		}
		else
		{
			prt(_("w/t) 装備する/はずす", "w/t) Wear/Take off equipment"), 22 + xtra_stock, 56);
		}

		/* Prompt */
		prt(_("コマンド:", "You may: "), 20 + xtra_stock, 0);

		/* Get a command */
		request_command(TRUE);

		/* Process the command */
		store_process_command();

		/* Hack -- Character is still in "icky" mode */
		character_icky = TRUE;

		/* Notice stuff */
		notice_stuff();

		/* Handle stuff */
		handle_stuff();

		/* XXX XXX XXX Pack Overflow */
		if (inventory[INVEN_PACK].k_idx)
		{
			int item = INVEN_PACK;

			object_type *o_ptr = &inventory[item];

			/* Hack -- Flee from the store */
			if (cur_store_num != STORE_HOME)
			{
				/* Message */
				msg_print(_("ザックからアイテムがあふれそうなので、あわてて店から出た...",
					"Your pack is so full that you flee the store..."));

				/* Leave */
				leave_store = TRUE;
			}

			/* Hack -- Flee from the home */
			else if (!store_check_num(o_ptr))
			{
				/* Message */
				msg_print(_("ザックからアイテムがあふれそうなので、あわてて家から出た...",
					"Your pack is so full that you flee your home..."));

				/* Leave */
				leave_store = TRUE;
			}

			/* Hack -- Drop items into the home */
			else
			{
				int item_pos;

				object_type forge;
				object_type *q_ptr;

				char o_name[MAX_NLEN];


				/* Give a message */
				msg_print(_("ザックからアイテムがあふれてしまった！", "Your pack overflows!"));

				/* Get local object */
				q_ptr = &forge;

				/* Grab a copy of the item */
				object_copy(q_ptr, o_ptr);

				/* Describe it */
				object_desc(o_name, q_ptr, 0);

				/* Message */
				msg_format(_("%sが落ちた。(%c)", "You drop %s (%c)."), o_name, index_to_label(item));

				/* Remove it from the players inventory */
				inven_item_increase(item, -255);
				inven_item_describe(item);
				inven_item_optimize(item);

				/* Handle stuff */
				handle_stuff();

				/* Let the home carry it */
				item_pos = home_carry(q_ptr);

				/* Redraw the home */
				if (item_pos >= 0)
				{
					store_top = (item_pos / store_bottom) * store_bottom;
					display_inventory();
				}
			}
		}

		/* Hack -- Redisplay store prices if charisma changes */
		if (tmp_chr != p_ptr->stat_use[A_CHR]) display_inventory();

		/* Hack -- get kicked out of the store */
		if (st_ptr->store_open >= turn) leave_store = TRUE;
	}


	/* See avobe */
	p_ptr->town_num = old_town_num;

	/* Free turn XXX XXX XXX */
	energy_use = 0;


	/* Hack -- Character is no longer in "icky" mode */
	character_icky = FALSE;


	/* Hack -- Cancel automatic command */
	command_new = 0;

	/* Hack -- Cancel "see" mode */
	command_see = FALSE;


	/* Flush messages XXX XXX XXX */
	msg_print(NULL);


	/* Clear the screen */
	Term_clear();


	/* Update everything */
	p_ptr->update |= (PU_VIEW | PU_LITE | PU_MON_LITE);
	p_ptr->update |= (PU_MONSTERS);

	/* Redraw entire screen */
	p_ptr->redraw |= (PR_BASIC | PR_EXTRA | PR_EQUIPPY);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

	/* BGM */
	bgm_dungeon();
}


/*
 * Shuffle one of the stores.
 */
void store_shuffle(int which)
{
	int i, j;


	/* Ignore home */
	if (which == STORE_HOME) return;


	/* Save the store index */
	cur_store_num = which;

	/* Activate that store */
	st_ptr = &town[p_ptr->town_num].store[cur_store_num];

	/* Pick a new owner */
	for (j = st_ptr->owner; j == st_ptr->owner; )
	{
		st_ptr->owner = (byte)randint0(MAX_OWNERS);
	}

	/* Activate the new owner */
	ot_ptr = &owners[cur_store_num][st_ptr->owner];


	/* Reset the owner data */
	st_ptr->store_open = 0;

	/* Hack -- discount all the items */
	for (i = 0; i < st_ptr->stock_num; i++)
	{
		object_type *o_ptr;

		/* Get the item */
		o_ptr = &st_ptr->stock[i];

		/* Hack -- Sell all old items for "half price" */
		if (!(o_ptr->art_name))
			o_ptr->discount = 50;

		/* Hack -- Items are no longer "fixed price" */
		o_ptr->ident &= ~(IDENT_FIXED);

		/* Mega-Hack -- Note that the item is "on sale" */
#ifdef JP
		o_ptr->inscription = quark_add("売出中");
#else
		o_ptr->inscription = quark_add("on sale");
#endif
	}
}


/*
 * Maintain the inventory at the stores.
 */
void store_maint(int town_num, int store_num)
{
	int 		j;

	cur_store_num = store_num;

	/* Ignore home */
	if (store_num == STORE_HOME) return;

	/* Activate that store */
	st_ptr = &town[town_num].store[store_num];

	/* Activate the owner */
	ot_ptr = &owners[store_num][st_ptr->owner];

	/* Mega-Hack -- prune the black market */
	if (store_num == STORE_BLACK)
	{
		/* Destroy crappy black market items */
		for (j = st_ptr->stock_num - 1; j >= 0; j--)
		{
			object_type *o_ptr = &st_ptr->stock[j];

			/* Destroy crappy items */
			if (black_market_crap(o_ptr))
			{
				/* Destroy the item */
				store_item_increase(j, 0 - o_ptr->number);
				store_item_optimize(j);
			}
		}
	}


	/* Choose the number of slots to keep */
	j = st_ptr->stock_num;

	/* Sell a few items */
	j = j - randint1(STORE_TURNOVER);

	/* Never keep more than "STORE_MAX_KEEP" slots */
	if (j > STORE_MAX_KEEP) j = STORE_MAX_KEEP;

	/* Always "keep" at least "STORE_MIN_KEEP" items */
	if (j < STORE_MIN_KEEP) j = STORE_MIN_KEEP;

	/* Hack -- prevent "underflow" */
	if (j < 0) j = 0;

	/* Destroy objects until only "j" slots are left */
	while (st_ptr->stock_num > j) store_delete();


	/* Choose the number of slots to fill */
	j = st_ptr->stock_num;

	/* Buy some more items */
	j = j + randint1(STORE_TURNOVER);

	/* Never keep more than "STORE_MAX_KEEP" slots */
	if (j > STORE_MAX_KEEP) j = STORE_MAX_KEEP;

	/* Always "keep" at least "STORE_MIN_KEEP" items */
	if (j < STORE_MIN_KEEP) j = STORE_MIN_KEEP;

	/* Hack -- prevent "overflow" */
	if (j >= st_ptr->stock_size) j = st_ptr->stock_size - 1;

	/* Acquire some new items */
	while (st_ptr->stock_num < j) store_create();
}


/*
 * Initialize the stores
 */
void store_init(int town_num, int store_num)
{
	int 		k;

	cur_store_num = store_num;

	/* Activate that store */
	st_ptr = &town[town_num].store[store_num];


	/* Pick an owner */
	st_ptr->owner = (byte)randint0(MAX_OWNERS);

	/* Activate the new owner */
	ot_ptr = &owners[store_num][st_ptr->owner];


	/* Initialize the store */
	st_ptr->store_open = 0;

	/* Nothing in stock */
	st_ptr->stock_num = 0;

	/*
	 * MEGA-HACK - Last visit to store is
	 * BEFORE player birth to enable store restocking
	 */
	st_ptr->last_visit = -100L * STORE_TURNS;

	/* Clear any old items */
	for (k = 0; k < st_ptr->stock_size; k++)
	{
		object_wipe(&st_ptr->stock[k]);
	}
}


void move_to_black_market(object_type *o_ptr)
{
	/* Not in town */
	if (!p_ptr->town_num) return;

	st_ptr = &town[p_ptr->town_num].store[STORE_BLACK];

	o_ptr->ident |= IDENT_STORE;

	(void)store_carry(o_ptr);

	object_wipe(o_ptr); /* Don't leave a bogus object behind... */
}
