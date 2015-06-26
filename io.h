#include <ctype.h>
#include "defines.h"
#include "engine.h"

#ifndef IO_H
#define IO_H

/* Colours */
#define ANSI_DEFAULT	"\033[0m"
#define ANSI_GRAY		"\033[30m\033[1m"
#define ANSI_RED		"\033[31m\033[1m"
#define ANSI_WHITE		"\033[37m\033[1m"
#define ANSI_DRED		"\033[31m\033[22m" /* Dark red */
#define ANSI_GREEN		"\033[32m\033[22m"
#define ANSI_YELLOW		"\033[33m\033[1m"
#define ANSI_BLUE		"\033[34m\033[1m"
#define ANSI_CYAN		"\033[36m\033[22m"

/* Go back two spaces */
#define ANSI_BACK		"\033[1D\033[1D"

void showSuit(const suit_t c)
	__attribute__((cold));

void showCard(const card_t c)
	__attribute__((cold));

void showPlay(const struct play* const restrict play)
	__attribute__((nonnull,cold));

void showHand(const struct player* const restrict player)
	__attribute__((nonnull,cold));

void showDeck(const struct deck* const deck)
	__attribute__((nonnull,cold));

void showPile(const struct deck* const pile)
	__attribute__((nonnull,cold));

void showGameState(const struct gamestate* const restrict gs)
	__attribute__((nonnull,cold));

card_t readCard(const char* const restrict str)
	__attribute__((nonnull,pure,cold));

#endif /* IO_H */
