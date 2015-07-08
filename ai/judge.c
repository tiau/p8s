#include "judge.h"

// TODO: tune tunables all throughout this function and evalPlay
int_fast16_t evalPlayer(const struct player* const restrict player, const size_t nplayers)
{
	size_t i, run = 0, mr = 0;
	int_fast16_t ret = 0;
	size_t nsuits[4] = { 0 },  nvals[(DECKLEN/4)+1] = { 0 };
	size_t suits[player->n], vals[player->n];

	assert(player);
	assert(nplayers >= MINPLRS && nplayers <= MAXPLRS);

	if(!player->n)
		return -32768;

	for(i = 0; i < player->n; i++) {
		suits[i] = getSuit(player->c[i]);
		vals[i] = getVal(player->c[i]);
		nsuits[suits[i]]++;
		nvals[vals[i]]++;
	}

	if(player->n >= 4) {
		for(i = 1; i <= 13; i++) {
			if(nvals[i])
				run++;
			else
				run = 0;
			if(run > mr)
				mr = run;
		}

		/* Straight check */
		if(mr == 4)
			ret -= 2;
		else if(mr > 4)
			ret -= max(20 - player->n, 10*(10-mr) - 4*(player->n - 5));
	}

	/* Flush check and 0 suit scoring */
	for(i = Clubs; i <= Spades; i++) {
		switch(nsuits[i]) {
			case 0:
				if(player->n > 2)
					ret += player->n;
				break;
			case 1:
			case 2:
			case 3:
				break;
			case 4:
				ret -= 5;
				break;
			case 6:
			case 7:
			case 8:
			case 9:
				ret -= 25;
				break;
			case 5:
			case 10:
				ret -= 30;
				break;
			default:
				ret -= 20;
				break;
		}
	}

	/* Full house / n of a kind check and overall scoring */
	for(i = 0; i < player->n; i++) {
		switch(vals[i]) {
			case 2:
				ret += 6 + nplayers;
				ret -= nvals[2]-1;
				break;
			case 3:
				switch(nplayers) {
					case 2:
						if(nsuits[suits[i]] > 1)
							ret -= 2;
						else
							ret += ((nvals[3] == 1) ? 15 : 3*(4 - nvals[3]) - (2*nvals[8]));
						break;
					case 4:
						ret += 1;
						/* fallthrough */
					case 3:
						ret += 8;
						ret -= nvals[3]-1;
						break;
					default:
						assert(false);
				}
				break;
			case 8:
				ret -= 10 - nplayers;
				ret -= nvals[8];
				break;
			default:
				ret += 3;
				ret -= nvals[vals[i]]-1;
		}
	}

	return ret;
} __attribute__((hot,pure,nonnull))

int_fast16_t evalPlay(const struct play* const restrict play, const size_t nplayers, const size_t* const restrict cih, const suit_t bestsuit)
{
	size_t i;
	int_fast16_t ret = 0;
	card_t cv;

	assert(play);
	assert(cih);

	const float cn = cih[0],
		  		cnn = (nplayers > 2) ? cih[1] : cih[0],
		  		f = 40.0 / (8.0 + cn * cn - cn);

	/* For the card at the end of the play */
	cv = getVal(play->c[play->n-1]);
	if(cv == 2)
		ret += f;
	else if(cv == 3)
		ret += (nplayers == 2) ? f : (4.0 + 2.57 * cnn) / ((32.3 + cn) / (cnn + -1.26 * cn / cnn) + cnn - 3.47) + 22.3 / (4.89 + cn) - 3.57;
	if(cv == 8)
		ret += 8 - nplayers;
	else if(bestsuit == getSuit(play->c[play->n-1]))
		ret += (cv == 3 ? 5 * (4 - nplayers) : 1);

	/* Not at the end of the play */
	for(i = 0; i < play->n - 1; i++) {
		cv = getVal(play->c[i]);
		if(cv == 2)
			ret -= 2.0 / (((cn - 1.0) / 3.0) + .5);
		else if(cv == 3)
			ret -= (nplayers > 2) ? cnn > 4.0 : 1;
		else if(cv == 8 && i != 0)
			ret -= 4 + (cn > 4.0);
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
		playereval = evalPlayer(&tplayer, as->gs->nplayers);
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
