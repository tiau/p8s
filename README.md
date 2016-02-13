# Pokereights: A silly card game

## It's like Crazy Eights and Poker got squished together for no reason whatsoever.

Pokereights (or p8s) is a card game and C implementation of the game. See how to build, run and play below.

#### Building
Try:
> make

Want more info about what the AI is up to? Try:
> make loud

Want it to be faster? Try:
> make profile

#### Running
See:
> ./p8

Try:
> ./p8 -m60

Want to watch the AI play against itself? Try:
> make loud
>
> ./p8 -m66 -vv


## The Game
#### Setup
- Find 2 to 4 players
- Deal 11 cards to each player
- Flip 1 card off the top of the deck to start the pile
- A player adjacent to the dealer goes first, play proceeds in this direction for the duration of the game

#### Playing
Normally the first thing a player does at the beginning of their turn is check what card is on top of the pile. If it's a two, they start their turn by drawing two cards. If it's a three, they get no turn at all and play moves on to whoever goes next. If it's an eight, they must think back a few seconds about what suit their friend told them is legal to play on top of this eight. The first turn is a bit different. If a two, three or eight shows up as the first card flipped off the deck the player treats it as though it were a normal card. They do not draw two cards, have their turn skipped or have to play a different suit than what is shown.

Once the player has noted any twos, threes or eights and taken the appropriate actions, they get to make their move. Any poker hand (high card -- any single card, two of a kind, three of a kind, two pair, full house, flush or straight,) may be played. This hand can be ordered however the player wishes: if they have a straight from A to 5 they may start at the 2 and end on the 3, should they so choose. However, the first card must match either the suit or the value of the top card of the pile on which it is played. Furthermore, only the top-most effect-card (2, 3 or 8) matters. For example, if someone plays a four of a kind of all twos, the person after them needn't despair, only the last card of the move (the card that will become the top of the pile) carries over its special effect to the next turn.

It's possible, and likely later in the game, that a player won't actually be able to make any moves! For example, the pile shows a five of spades yet the player has only a ten of hearts and a nine of diamonds in their hand. This brings us to drawing. If a player can make no moves they must draw a card. After drawing they may make any move that is then legal. Drawing need not only happen when one cannot make a move, one may choose to draw a single card each turn, regardless of any cards drawn due to twos. After drawing, either because they must or they choose, the player cannot draw again until their next turn. If they still have no legal moves they must pass. Even if they do have legal moves, they may always choose to pass for strategic reasons. They may not choose to pass before drawing a card: every player either draws a card or makes a move each turn.<sup>[1](#note1)</sup>

If upon wishing to draw the player finds the deck empty they must take all but the top card of the pile and shuffle them to create a new deck from which to draw. They then may draw as usual.

#### Conclusion
The game ends when one player has no more cards. This player has won.

## An example
```
isaac@galapagos ~/p8s $ ./p8 -m66 -vv
In progress, Turn 0 (player 0)
Deck (29 cards) JS, 3S, 8S, AH, 6H, KH, 5H, 2S, KC, QC, 9C, AS, 10D, 10H, 3C, QS, AC, 9D, 7C, QD, KS, 2D, KD, 6C, 9H, 7D, QH, 6S, 2H
Pile (1 cards)  JH*
(11 cards)      2C, 8C, 10C, AD, 4D, 6D, 8D, JD, 4H, 4S, 7S
(11 cards)      4C, 5C, JC, 3D, 5D, 3H, 7H, 8H, 5S, 9S, 10S
Playing JD, AD, 4D, 6D, 8D
Suit is now C

In progress, Turn 1 (player 1)
Deck (29 cards) JS, 3S, 8S, AH, 6H, KH, 5H, 2S, KC, QC, 9C, AS, 10D, 10H, 3C, QS, AC, 9D, 7C, QD, KS, 2D, KD, 6C, 9H, 7D, QH, 6S, 2H
Pile (6 cards)  JH, JD, AD, 4D, 6D, 8D*
(6 cards)       2C, 8C, 10C, 4H, 4S, 7S
(11 cards)      4C, 5C, JC, 3D, 5D, 3H, 7H, 8H, 5S, 9S, 10S
Playing 4C

In progress, Turn 2 (player 0)
Deck (29 cards) JS, 3S, 8S, AH, 6H, KH, 5H, 2S, KC, QC, 9C, AS, 10D, 10H, 3C, QS, AC, 9D, 7C, QD, KS, 2D, KD, 6C, 9H, 7D, QH, 6S, 2H
Pile (7 cards)  JH, JD, AD, 4D, 6D, 8D, 4C*
(6 cards)       2C, 8C, 10C, 4H, 4S, 7S
(10 cards)      5C, JC, 3D, 5D, 3H, 7H, 8H, 5S, 9S, 10S
Playing 10C

In progress, Turn 3 (player 1)
Deck (29 cards) JS, 3S, 8S, AH, 6H, KH, 5H, 2S, KC, QC, 9C, AS, 10D, 10H, 3C, QS, AC, 9D, 7C, QD, KS, 2D, KD, 6C, 9H, 7D, QH, 6S, 2H
Pile (8 cards)  JH, JD, AD, 4D, 6D, 8D, 4C, 10C*
(5 cards)       2C, 8C, 4H, 4S, 7S
(10 cards)      5C, JC, 3D, 5D, 3H, 7H, 8H, 5S, 9S, 10S
Playing 5C, 3D, 5D, 5S, 3H

In progress, Turn 4 (player 0)
Deck (29 cards) JS, 3S, 8S, AH, 6H, KH, 5H, 2S, KC, QC, 9C, AS, 10D, 10H, 3C, QS, AC, 9D, 7C, QD, KS, 2D, KD, 6C, 9H, 7D, QH, 6S, 2H
Pile (13 cards) JH, JD, AD, 4D, 6D, 8D, 4C, 10C, 5C, 3D, 5D, 5S, 3H*
(5 cards)       2C, 8C, 4H, 4S, 7S
(5 cards)       JC, 7H, 8H, 9S, 10S
3 played, skipping turn

In progress, Turn 5 (player 1)
Deck (29 cards) JS, 3S, 8S, AH, 6H, KH, 5H, 2S, KC, QC, 9C, AS, 10D, 10H, 3C, QS, AC, 9D, 7C, QD, KS, 2D, KD, 6C, 9H, 7D, QH, 6S, 2H
Pile (13 cards) JH, JD, AD, 4D, 6D, 8D, 4C, 10C, 5C, 3D, 5D, 5S, 3H*
(5 cards)       2C, 8C, 4H, 4S, 7S
(5 cards)       JC, 7H, 8H, 9S, 10S 
Playing 7H, JC, 8H, 9S, 10S

Concluded, Turn 6 (player 1)
```

Turn 0 starts with each player having 11 cards and the pile showing a **JH**. The first player chooses to play a flush of diamonds. Since the pile isn't showing diamonds they have to match their jack of diamonds to the pile's jack of hearts. They could've instead chosen to order their move, for example, **8D, JD, AD, 4D, 6D** since the eight is crazy and may be played on top of any card; however, had they done so they would not have gotten to choose which suit would follow next.

Turn 1 is relatively simple, the previous turn it was declared that clubs must follow so player 1 has to make a move that starts with either a club or an eight. They choose to play a four of clubs.

Turn 2 has more single clubs being played.

Turn 3 has player 1 playing a full house ending on a three. Since threes skip the next player, player 0's next turn is skipped. Note that the three in the middle of the move has no effect since it doesn't end up being the top card of the pile.

Turn 5 (given that turn 4 was someone being skipped) has player 1 playing a straight thus dropping all their cards. Player 1 is victorious.


## Another example
```
isaac@galapagos ~/p8s $ ./p8 -m06 -vv -h "AH 5H 0H JH 8C JC 2S QS KS AD" -p "8D 2C"
In progress, Turn 0 (player 0)
Deck (29 cards)	5D, 6D, 9D, 3S, QH, KD, 6H, 5S, 2H, KH, 6S, QC, 3C, AS, 10S, 4S, 5C, 7H, AC, KC, 7S, 3D, JD, 10D, 8S, 4D, 4H, QD, 9S
Pile (2 cards)	8D, 2C*
(10 cards)	8C, JC, AD, AH, 5H, 10H, JH, 2S, QS, KS
(11 cards)	4C, 6C, 7C, 9C, 10C, 2D, 7D, 3H, 8H, 9H, JS
HAND (10 cards)	8C, JC, AD, AH, 5H, 10H, JH, 2S, QS, KS
#CARDS		11
PILE (2 cards)	8D, 2C*
DECK (29 cards)
MOVES
	0: 8C |
	1: JC |
	2: 2S |
	3: JC, JH |
	4: JC, AD, AH, JH | 5: JC, AD, JH, AH | 6: JC, AH, JH, AD |
	7: JC, AD, 10H, QS, KS | 8: JC, AD, 10H, KS, QS | 9: JC, AD, QS, KS, 10H | 10: JC, 10H, QS, KS, AD |
	11: JC, AH, 10H, QS, KS | 12: JC, AH, 10H, KS, QS | 13: JC, AH, QS, KS, 10H | 14: JC, 10H, QS, KS, AH |
move -> ?
aiMonte: 0	(0.0%	0.2%)	25.0%	8C
aiMonte: 1	(0.0%	0.2%)	20.0%	8C
aiMonte: 4	(0.5%	0.2%)	40.0%	JC
aiMonte: 6	(0.5%	0.2%)	40.0%	JC, JH
aiMonte: 2	(0.8%	0.4%)	47.6%	8C
aiMonte: 8	(0.8%	0.4%)	47.6%	JC, AD, JH, AH
aiMonte: 9	(0.8%	0.4%)	47.6%	JC, AH, JH, AD
aiMonte: 15	(0.5%	0.2%)	40.0%	JC, AH, 10H, KS, QS
aiMonte: 18	(0.1%	0.2%)	30.0%	(draw)
aiMonte: 3	(1.0%	1.2%)	46.3%	8C
aiMonte: 7	(0.3%	1.6%)	46.8%	JC, AD, AH, JH
aiMonte: 13	(0.7%	1.2%)	48.0%	JC, 10H, QS, KS, AD
aiMonte: 5	(0.8%	15.3%)	49.6%	2S
aiMonte: 14	(0.6%	17.3%)	49.5%	JC, AH, 10H, QS, KS
aiMonte: 11	(1.0%	73.8%)	51.4%	JC, AD, 10H, KS, QS
aiMonte: 10	(0.1%	93.6%)	51.5%	JC, AD, 10H, QS, KS
aiMonte: 12	(3.7%	105.3%)	52.8%	JC, AD, QS, KS, 10H
aiMonte: 17	(50.0%	105.3%)	54.0%	JC, 10H, QS, KS, AH
aiMonte: 16	(50.0%	93.6%)	52.8%	JC, AH, QS, KS, 10H
AI recommends	14 :: JC, 10H, QS, KS, AH
move -> ?
aiMonte: 0	(0.5%	0.4%)	26.2%	8C
aiMonte: 1	(0.1%	0.9%)	36.6%	8C
aiMonte: 2	(0.9%	0.9%)	43.0%	8C
aiMonte: 7	(0.3%	0.9%)	39.8%	JC, AD, AH, JH
aiMonte: 8	(0.9%	0.9%)	43.0%	JC, AD, JH, AH
aiMonte: 3	(0.4%	1.6%)	43.6%	8C
aiMonte: 6	(0.3%	1.6%)	42.9%	JC, JH
aiMonte: 18	(0.4%	1.9%)	40.9%	(draw)
aiMonte: 15	(1.0%	3.3%)	43.8%	JC, AH, 10H, KS, QS
aiMonte: 5	(0.7%	11.8%)	48.1%	2S
aiMonte: 9	(0.8%	13.5%)	48.5%	JC, AH, JH, AD
aiMonte: 14	(1.0%	25.1%)	49.9%	JC, AH, 10H, QS, KS
aiMonte: 11	(0.9%	32.0%)	51.2%	JC, AD, 10H, KS, QS
aiMonte: 16	(0.8%	32.0%)	51.1%	JC, AH, QS, KS, 10H
aiMonte: 10	(0.7%	83.1%)	51.7%	JC, AD, 10H, QS, KS
aiMonte: 4	(50.0%	105.3%)	53.7%	JC
aiMonte: 12	(19.2%	105.3%)	53.1%	JC, AD, QS, KS, 10H
aiMonte: 17	(6.4%	105.3%)	52.6%	JC, 10H, QS, KS, AH
aiMonte: 13	(50.0%	93.6%)	52.9%	JC, 10H, QS, KS, AD
AI recommends	1 :: JC
```

Here we see what the program output looks like if **make loud** is used to compile it.<sup>[2](#note2)</sup> We use the human 'AI' to give us control of the first player. We also specify what our hand will be and what cards should be in the pile.<sup>[3](#note3)</sup> The non-specified cards (our opponent's hand and the deck) are randomly assigned from the remaining cards. Instead of choosing our own move, we ask the AI to analyze our hand and suggest a move by passing '?' rather than a move number.<sup>[4](#note4)</sup>

The information that the Monte Carlo AI is reporting is as follows:
- Name of the AI: aiMonte
- Number of move: e.g. 12
- Chance that move is better than current best move: e.g. 19.2%
- Percent search window exhausted: e.g. 105.3%
- Percent of games won given move: e.g. 53.1%

So what can we tell from all this? Tons! For example, we quickly ruled out throwing both Jacks. Doing so would leave us unable to play our straight. Throwing our 8 this early doesn't seem like a good idea either.<sup>[5](#note5)</sup>

On the other hand, we can't really decide between certain straight variants and throwing one jack (preserving our straight). This is especially stark as running the AI multiple times confirms that it will fluctuate wildly between recommended moves. However all of the moves that make it to the end of the search window are likely very good and it's entirely possible there is not enough signal to distinguish them even with further Monte Carlo probing.




### Notes
- <a name="note1">1</a>: Except under the extremely unlikely circumstance where the deck is out of cards and there's only one card in the pile. Then they must pass if they do not wish to make a move.

- <a name="note2">2</a>: AIs that will report move info given **make loud**: 6 (monte carlo), 7 (stacked monte carlo), 8 (cheats), 9 (minimax cheats). **make louder** can be used to get move info from AIs 4 (judges) and 5 (judges/draws). This will cause the higher AIs to output **a lot** of (mixed up, due to threading) info.

- <a name="note3">3</a>: **0** or **X** represents a 10 for the -h and -p command-line options.

- <a name="note4">4</a>: Human AI commands: **d** draw/pass, **#** make play number. **?** to invoke monte carlo AI, **h** to invoke stacked monte carlo AI, **H** to invoke cheat monte carlo AI. **Q** to quit.

- <a name="note5">5</a>: Note that given how far into the search window the numbers are reported at their accuracy will vary wildly. Some moves we ruled out very quickly, but we didn't play enough hypothetical games with them to get an accurate picture of how bad they were. We know they were almost certainly worse than the best move at the time, but that is all.
