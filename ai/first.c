#include "first.h"

// TODO: tune tunables all throughout this function and evalPlay
static int_fast16_t evalPlayer(const struct player* const restrict player, const size_t nplayers, const size_t cn)
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
		switch(mr) {
			case 3:
				ret -= (int)((float)abs(z3S) - (float)abs(SHM3)/5.0f * (float)player->n); break;
			case 4:
				ret -= (int)((float)abs(z4S) - (float)abs(SHM4)/5.0f * (float)player->n); break;
			case 5:
				ret -= (int)((float)abs(z5S) - (float)abs(SHM5)/5.0f * (float)player->n); break;
			case 6:
				ret -= (int)((float)abs(z6S) - (float)abs(SHM6)/5.0f * (float)player->n); break;
			case 7:
				ret -= (int)((float)abs(z7S) - (float)abs(SHM7)/5.0f * (float)player->n); break;
			case 8:
				ret -= (int)((float)abs(z8S) - (float)abs(SHM8)/5.0f * (float)player->n); break;
			case 9:
				ret -= (int)((float)abs(z9S) - (float)abs(SHM9)/5.0f * (float)player->n); break;
			case 10:
				ret -= (int)((float)abs(z10S) - (float)abs(SHM10)/5.0f * (float)player->n); break;
			default:
				ret -= (int)((float)abs(LargeS) - (float)abs(SHMLarge)/5.0f * (float)player->n); break;
		}
	}

	/* Flush check and 0 suit scoring */
	for(i = Clubs; i <= Spades; i++) {
		switch(nsuits[i]) {
			case 0:
				if(player->n > 2)
					ret += player->n;
				else
					ret -= (int)((float)abs(z0Su) - (float)abs(z0SuM)/5.0f * (float)player->n); break;
				break;
			case 1:
				ret -= (int)((float)abs(z1Su) - (float)abs(z1SuM)/5.0f * (float)player->n); break;
			case 2:
				ret -= (int)((float)abs(z2Su) - (float)abs(z2SuM)/5.0f * (float)player->n); break;
			case 3:
				ret -= (int)((float)abs(z3Su) - (float)abs(z3SuM)/5.0f * (float)player->n); break;
			case 4:
				ret -= (int)((float)abs(z4Su) - (float)abs(z4SuM)/5.0f * (float)player->n); break;
			case 5:
				ret -= (int)((float)abs(z5Su) - (float)abs(z5SuM)/5.0f * (float)player->n); break;
			case 6:
				ret -= (int)((float)abs(z6Su) - (float)abs(z6SuM)/5.0f * (float)player->n); break;
			case 7:
				ret -= (int)((float)abs(z7Su) - (float)abs(z7SuM)/5.0f * (float)player->n); break;
			case 8:
				ret -= (int)((float)abs(z8Su) - (float)abs(z8SuM)/5.0f * (float)player->n); break;
			case 9:
				ret -= (int)((float)abs(z9Su) - (float)abs(z9SuM)/5.0f * (float)player->n); break;
			case 10:
				ret -= (int)((float)abs(z10Su) - (float)abs(z10SuM)/5.0f * (float)player->n); break;
			default:
				ret -= (int)((float)abs(LargeSu) - (float)abs(LargeSuM)/5.0f * (float)player->n); break;
		}
	}

	/* Full house / n of a kind check and overall scoring */
	size_t dubs = 0;
	size_t trips = 0;
	size_t quads = 0;
	for(i = 0; i < player->n; i++) {
		switch(vals[i]) {
			case 2:
				ret -= Draw2 + (int)((float)Draw22 / (float)cn) - nplayers;
				break;
			case 3:
				switch(nplayers) {
					case 2:
						ret -= Skip0 + (int)((float)Skip1*(float)nsuits[suits[i]]/5.0f) + (int)((float)Skip2 * (float)nvals[3]/5.0f) + (int)((float)Skip3 * (float)nvals[8] / 5.0f);
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
				ret -= Eight + Eight2 / player->n - nplayers;
				break;
			default:
				ret += ACard + ACard2 / player->n + ACard3 / cn;
		}
		switch(nvals[vals[i]]) {
			case 4:
				quads++;
			case 3:
				trips++;
			case 2:
				dubs++;
			default:
				break;
		}
	}
	ret -= DMod * dubs;
	ret -= TMod * trips;
	ret -= QMod * quads;
	ret -= DTMod * (dubs && trips);
	ret -= DQMod * (dubs && quads);
	ret -= TQMod * (trips && quads);
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
		ret += EP2 + EP22 / cn - nplayers;
	else if(cv == 3)
		ret += EP3 + EP32 / cn - nplayers;
	if(cv == 8)
		ret += EP8 - nplayers;
	else if(bestsuit == getSuit(play->c[play->n-1]))
		ret += (cv == 3 ? EPBS3 * (EPBS32 - nplayers) : EPBS);

	/* Not at the end of the play */
	for(i = 0; i < play->n - 1; i++) {
		cv = getVal(play->c[i]);
		if(cv == 2)
			ret -= W2 / cn;
		else if(cv == 3)
			ret -= (nplayers > 2) ? cnn > 4.0 : W3;
		else if(cv == 8 && i != 0)
			ret -= W8 + W8M * (cn > 4.0);
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
