#include "first.h"

// TODO: tune tunables all throughout this function and evalPlay
int_fast16_t evalPlayerf(const struct player* const restrict player, const size_t nplayers, const size_t cn)
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
		ret -= SHM1 / 5.0 * mr;
		ret -= SHM2 / 5.0 * mr / player->n;
		switch(mr) {
			case 1:
			case 2:
				ret -= abs(zSmallS); break;
			case 3:
				ret -= abs(z3S); break;
			case 4:
				ret -= abs(z4S); break;
			case 5:
				ret -= abs(z5S); break;
			case 6:
				ret -= abs(z6S); break;
			case 7:
				ret -= abs(z7S); break;
			case 8:
				ret -= abs(z8S); break;
			case 9:
				ret -= abs(z9S); break;
			case 10:
				ret -= abs(z10S); break;
			default:
				ret -= abs(LargeS); break;
		}
	}

	/* Flush check and 0 suit scoring */
	for(i = Clubs; i <= Spades; i++) {
		switch(nsuits[i]) {
			ret -= SuM1 / 5.0 * nsuits[i];
			ret -= SuM2 / 5.0 * nsuits[i] / player->n;
			case 0:
				if(player->n > 2)
					ret += player->n;
				else
					ret -= abs(z0Su);
				break;
			case 1:
				ret -= abs(z1Su);
				break;
			case 2:
				ret -= abs(z2Su); break;
			case 3:
				ret -= abs(z3Su); break;
			case 4:
				ret -= abs(z4Su); break;
			case 5:
				ret -= abs(z5Su); break;
			case 6:
				ret -= abs(z6Su); break;
			case 7:
				ret -= abs(z7Su); break;
			case 8:
				ret -= abs(z8Su); break;
			case 9:
				ret -= abs(z9Su); break;
			case 10:
				ret -= abs(z10Su); break;
			default:
				ret -= abs(LargeSu); break;
		}
	}

	/* Full house / n of a kind check and overall scoring */
	for(i = 0; i < player->n; i++) {
		switch(vals[i]) {
			case 2:
				ret -= Draw2 - Draw22 / 5.0 * cn + nplayers * 2;
				break;
			case 3:
				switch(nplayers) {
					case 2:
						ret -= Skip1 + Skip2/5.0*nsuits[suits[i]] + Skip3/5.0*nvals[3] + Skip4/5.0*nvals[8];
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
				ret -= Eight + Eight2 / 5.0 / player->n - nplayers * 2;
				break;
			default:
				ret += ACard + ACard2 / 5.0 / player->n + ACard3 / 5.0 / cn;
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
	ret -= DMod * dubs;
	ret -= TMod * trips;
	ret -= QMod * quads;
	ret -= abs(DTMod) * (dubs && trips);
	ret -= abs(DQMod) * (dubs && quads);
	ret -= abs(TQMod) * (trips && quads);
	return ret;
} __attribute__((hot,pure,nonnull))

static int_fast16_t evalPlay(const struct play* const restrict play, const size_t nplayers, const size_t* const restrict cih, const suit_t bestsuit)
{
	size_t i;
	int_fast16_t ret = 0;
	card_t cv;

	{	assert(play);
		assert(cih);}

	const float cn = cih[0],
		  		cnn = (nplayers > 2) ? cih[1] : cih[0];

	/* For the card at the end of the play */
	cv = getVal(play->c[play->n-1]);
	if(cv == 2)
		ret += EP2 + EP22/5.0/cn - nplayers*2;
	else if(cv == 3)
		ret += EP3 + EP32/5.0/cn - nplayers*2;
	if(cv == 8)
		ret += EP8 - nplayers*2;
	else if(bestsuit == getSuit(play->c[play->n-1]))
		ret += (cv == 3 ? EPBS3 * (EPBS32/5.0 - nplayers) : EPBS);

	/* Not at the end of the play */
	for(i = 0; i < play->n - 1; i++) {
		cv = getVal(play->c[i]);
		if(cv == 2)
			ret -= W2/5.0/cn;
		else if(cv == 3)
			ret -= (nplayers > 2) ? cnn <= 4.0 : W3/5.0/cn;
		else if(cv == 8 && i != 0)
			ret -= W8 + W8M/5.0/cn;
	}

	return ret;
} __attribute__((hot,pure,nonnull))

uint_fast32_t aiFirst(const struct aistate* const restrict as)
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
		playereval = evalPlayerf(&tplayer, as->gs->nplayers, cih[0]);
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
