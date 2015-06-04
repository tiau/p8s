#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef DEFINES_H
#define DEFINES_H

#define likely(x)	__builtin_expect((x),1)
#define unlikely(x)	__builtin_expect((x),0)

typedef uint_fast8_t  suit_t, card_t;

#define CPP 11		// Cards dealt per player at the beginning of the game
#define MINPLRS 2	// Minimum number of players
#define MAXPLRS 4	// Maximum number of players

/* N.B. Changing this will require some small reworks (e.g. initDeck, prbably others) */
#define DECKLEN 52	// Deck size

/* N.B. Changing this will require significant rework (poker rules...) */
#define MAXCIP 5	// Maximum number of cards in a play

/* Macros to deal with AIs returning their result as a packed 32-bit int.  We
 * pack the values like this as this makes our AI functions pure. */
#define ESPACK(A,B)	(A) = ((A) & 65535) | ((B) << 16)
#define ESUPACK(A)	((A) >> 16)
#define MPACK(A,B)	(A) = ((A) & 4294901760) | (B)
#define MUPACK(A)	((A) & 65535)

enum suit {
	CLUBS,
	DIAMONDS,
	HEARTS,
	SPADES,
	UNKNOWN
};

enum gamestatus {
	CONCLUDED,
	INPROGRESS,
	NOTSTARTED
};

/* Players can only hold DECKLEN-2 cards since there must be at least one card
 * in the pile and at least one card in, at least, one opponent's hand. */
struct player {
	card_t c[DECKLEN-2];
	size_t n;
};

struct play {
	card_t c[MAXCIP];
	size_t n;
};

struct deck {
	card_t c[DECKLEN];
	card_t* top;
	size_t n;
};

struct gamestate {
	size_t turn;
	struct deck deck;
	struct deck pile;
	size_t nplayers;
	struct player* players;
	bool drew;			// If the current player has drawn a card
	bool magic;			// If the current card should be have a special effect
	suit_t eightSuit;	// Suit to go on top of 8s, if magic
	const uint8_t* ai;	// Array of AIs to invoke for each player slot
};

struct aistate {
	const struct gamestate* const restrict gs;
	struct plist* restrict pl;
};

#endif /* DEFINES_H */
