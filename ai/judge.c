#include "judge.h"

int_fast16_t evalPlayer(const struct player* const restrict player, const size_t nplayers, const size_t cn)
{
	size_t i, run = 0, mr = 0;
	int_fast16_t ret = 0;
	size_t nsuits[4] = { 0 },  nvals[(DECKLEN/4)+1] = { 0 };
	size_t suits[player->n], vals[player->n];

	{	assert(player);
		assert(nplayers >= MINPLRS && nplayers <= MAXPLRS);}

	if(!player->n)
		return -16384;

	for(i = 0; i < player->n; i++) {
		suits[i] = getSuit(player->c[i]);
		vals[i] = getVal(player->c[i]);
		nsuits[suits[i]]++;
		nvals[vals[i]]++;
	}

	if(player->n >= 3) {
		for(i = 0; i <= 13; i++) {
			if(nvals[!i ? 13 : i])
				run++;
			else
				run = 0;
			if(run > mr)
				mr = run;
		}

		/* Straight check */
		ret -= mr + mr / player->n;
		switch(mr) {
			case 1:
			case 2:
				break;
			case 3:
				ret -= 4; break;
			case 4:
				ret -= 5; break;
			case 5:
				ret -= 20; break;
			case 6:
				ret -= 20; break;
			case 7:
				ret -= 16; break;
			case 8:
				ret -= 12; break;
			case 9:
				ret -= 18; break;
			case 10:
				ret -= 24; break;
			default:
				ret -= 14; break;
		}
	}

	/* Flush check and 0 suit scoring */
	for(i = Clubs; i <= Spades; i++) {
		switch(nsuits[i]) {
			ret -= 2.5 * nsuits[i] - 1.5 * nsuits[i] / player->n;
			case 0:
				if(player->n > 2)
					ret += player->n;
				else
					ret -= 1;
				break;
			case 1:
				break;
			case 2:
				ret -= 4; break;
			case 3:
				ret -= 8; break;
			case 4:
				ret -= 10; break;
			case 5:
				ret -= 20; break;
			case 6:
				ret -= 16; break;
			case 7:
				ret -= 13; break;
			case 8:
				ret -= 6; break;
			case 9:
				ret -= 26; break;
			case 10:
				ret -= 30; break;
			default:
				ret -= 16; break;
		}
	}

	/* Full house / n of a kind check and overall scoring */
	for(i = 0; i < player->n; i++) {
		switch(vals[i]) {
			case 2:
				ret += 40 - 2*cn - 2*nplayers;
				break;
			case 3:
				switch(nplayers) {
					case 2:
						ret += 31 - 8*nsuits[suits[i]] + nvals[3] + 3.5*nvals[8];
						break;
					case 4:
						ret += 1;
						/* fallthrough */
					case 3:
						ret += 8;
						ret -= nvals[3]-1;
						break;
					default:
						ret -= 2;
				}
				break;
			case 8:
				ret -= 46 + 4/player->n - 2*nplayers;
				break;
			default:
				ret += 26 + 3.5/cn;
		}
	}

	size_t dubs = 0, trips = 0, quads = 0;
	for(i = 0; i < (DECKLEN/4)+1; i++) {
		switch(nvals[i]) {
			case 4:
				quads++;
				break;
			case 3:
				trips++;
				break;
			case 2:
				dubs++;
				break;
			default:
				break;
		}
	}
	ret -= 42 * dubs;
	ret -= 58 * trips;
	ret -= 54 * quads;
	ret -= 28 * (dubs && trips);
	ret -= 8 * (dubs && quads);
	ret -= 10 * (trips && quads);
	return ret;
}

int_fast16_t evalPlay(const struct play* const restrict play, const size_t nplayers, const size_t* const restrict cih, const suit_t bestsuit)
{
	size_t i;
	int_fast16_t ret = 0;
	card_t cv;

	{	assert(play);
		assert(cih);}

	const float cn = cih[0], cnn = (nplayers > 2) ? cih[1] : cih[0];

	/* For the card at the end of the play */
	cv = getVal(play->c[play->n-1]);
	if(cv == 2)
		ret += 11 + 2.5/cn - 2*nplayers;
	else if(cv == 3)
		ret += 23 + 1.5/cn - 2*nplayers;
	if(cv == 8)
		ret += 34 - 2*nplayers;
	else if(bestsuit == getSuit(play->c[play->n-1]))
		ret += (cv == 3 ? 13 - nplayers : 4);

	/* Not at the end of the play */
	for(i = 0; i < play->n - 1; i++) {
		cv = getVal(play->c[i]);
		if(cv == 2)
			ret += 2.5/cn;
		else if(cv == 3)
			ret += (nplayers > 2) ? cnn > 4.0 : 0.5/cn;
		else if(cv == 8 && i != 0)
			ret += 2 - 0.5/cn;
	}

	return ret;
} __attribute__((hot,pure,nonnull))

uint_fast32_t aiJudge(const struct aistate* const restrict as)
{
	size_t i;
	struct player* player;
	struct player tplayer;
	const struct play* play;
	ssize_t ep, best = 32767;
	ssize_t playereval, moveeval;
	suit_t bestsuit;
	uint_fast32_t ret = 0;

	{	assert(as);
		assert(as->gs);
		assert(as->pl);
		assert(as->pl->n);
		assert(as->gs->nplayers >= MINPLRS && as->gs->nplayers <= MAXPLRS);}

	size_t cih[as->gs->nplayers];
	populateCIH(as->gs, cih);
	player = stateToPlayer(as->gs);
	MPACK(ret, as->pl->n);
	for(i = 0; i < as->pl->n; i++) {
		tplayer = *player;

		play = plistGet(as->pl, i);
		removeCards(&tplayer, play);
		bestsuit = freqSuit(&tplayer);
		playereval = evalPlayer(&tplayer, as->gs->nplayers, cih[0]);
		moveeval = evalPlay(play, as->gs->nplayers, cih, bestsuit);
		ep = playereval - moveeval;

		if(ep < best) {
			MPACK(ret, i);
			ESPACK(ret, bestsuit);
			best = ep;
		}

#ifdef JUDGE_VERBOSE
		printf("%sjudge:%s\t%zu\t%zi\t%zi\t%zi\t", ANSI_CYAN, ANSI_DEFAULT, i, playereval, moveeval, ep);
		showPlay(play);
		printf("%s %s\n", ANSI_BACK, ANSI_DEFAULT);
#endif
	}
	return ret;
}
