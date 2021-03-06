#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef DEFINES_H
#define DEFINES_H

/* These can safely be changed */
#define CPP 11		// Cards dealt per player at the beginning of the game
#define MINPLRS 2	// Minimum number of players
#define MAXPLRS 4	// Maximum number of players

/* Changing these will require /significant/ reworks (poker rules + more...) */
#define DECKLEN 52	// Deck size
#define MAXCIP 5	// Maximum number of cards in a play

/* Macros to deal with AIs returning their result as a packed 32-bit int.  We
 * pack the values like this so our AI functions can be pure */
#define ESPACK(A,B)	(A) = ((A) & 0xFFFF) | ((B) << 16)
#define ESUPACK(A)	((A) >> 16)
#define MPACK(A,B)	(A) = ((A) & 0xFFFF0000) | (B)
#define MUPACK(A)	((A) & 0xFFFF)

typedef uint_fast8_t  suit_t, card_t;
typedef uint_fast16_t draw_t;

enum suit {
	Clubs,
	Diamonds,
	Hearts,
	Spades,
	Unknown
};

enum gamestatus {
	Concluded,
	InProgress,
	NotStarted
};

/* Players can only hold DECKLEN-2 cards since there must be at least one card
 * in the pile and at least one card in >=1 opponent's hand */
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

struct aistate {
	const struct gamestate* const restrict gs;
	struct plist* restrict pl;
};

struct gamestate {
	size_t turn;
	struct deck deck;
	struct deck pile;
	size_t nplayers;
	struct player players[MAXPLRS];
	bool drew;			// If the current player has drawn a card
	bool magic;			// If the current card should have a special effect
	suit_t eightSuit;	// Suit to go on top of 8s, if magic
	/* AI to invoke for each player slot */
	uint_fast32_t (*ai[MAXPLRS])(const struct aistate* const restrict);
	/* A bitmap of recent drawing events, 2 bits (<=3 draws) per turn */
	draw_t draws[MAXPLRS];
};


__attribute__((hot,const,always_inline))
inline static suit_t getSuit(const card_t c)
{
	assert(c > 0 && c <= DECKLEN);
	/* Lookup tables are much faster than computation for this and getVal */
	static const suit_t vals[] = {Unknown,
								  0,0,0,0,0,0,0,0,0,0,0,0,0,
								  1,1,1,1,1,1,1,1,1,1,1,1,1,
								  2,2,2,2,2,2,2,2,2,2,2,2,2,
								  3,3,3,3,3,3,3,3,3,3,3,3,3};
	return vals[c];
}

__attribute__((hot,const,always_inline))
inline static card_t getVal(const card_t c)
{
	assert(c > 0 && c <= DECKLEN);
	static const card_t vals[] = {0,
								  1,2,3,4,5,6,7,8,9,10,11,12,13,
								  1,2,3,4,5,6,7,8,9,10,11,12,13,
								  1,2,3,4,5,6,7,8,9,10,11,12,13,
								  1,2,3,4,5,6,7,8,9,10,11,12,13};
	return vals[c];
}

__attribute__((hot,const,always_inline))
inline static size_t min(const size_t x, const size_t y)
{
	return (x > y ? y : x);
}

__attribute__((hot,const,always_inline))
inline static size_t max(const size_t x, const size_t y)
{
	return (x > y ? x : y);
}

#define likely(x)	__builtin_expect((x),1)
#define unlikely(x)	__builtin_expect((x),0)

#endif /* DEFINES_H */
