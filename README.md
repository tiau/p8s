# Pokereights: A silly card game

## It's like Crazy Eights and Poker got squished together for no reason whatsoever.

Pokereights (or p8s) is a card game and C implementation of the game.  See how to build, run and play below.

#### Building
Try:
> make

Want more info about what the AI is up to?  Try:
> make loud

Want it to be faster?  Try:
> make profile

#### Running
See:
> ./p8

Try:
> ./p8 -m60

Want to watch the AI play against itself?  Try:
> make loud
>
> ./p8 -m66 -vv


## The Game
#### Setup
- Find 2 to 4 players
- Deal 11 cards to each player
- Flip 1 more card off the top of the deck to start the pile
- A player adjacent to the dealer goes first, play proceeds in this direction for the duration of the game

#### Playing
Normally the first thing a player does at the beginning of their turn is check what card is on top of the pile.  If it's a two, they start their turn by drawing two cards.  If it's a three, they get no turn at all and play moves on to whoever goes next.  If it's an eight then they have to think back to a few seconds ago about what suit their friend told them is legal to play on top of this eight and follow suit.  The first turn is a bit different.  If a two, three or eight shows up as the first card flipped off the deck the player treats it as though it were a normal card.  They do not draw two cards, have their turn skipped or have to play a different suit than what is shown.

Once the player has noted any twos, threes or eights and taken the appropriate actions, they get to make their move.  Any poker hand (high card -- any single card, two of a kind, three of a kind, two pair, full house, flush or straight,) may be played.  This hand can be ordered however the player wishes, if they have a straight from A to 5 they may start at the 2 and end on the 3, should they so choose.  However, the first card must match either the suit or the value of the top card of the pile on which it is played.  Furthermore, only the top-most effect card matters.  For example, if someone plays a four of a kind of all twos the person after them needn't despair, only the last card of the move (the card that will become the top of the pile) carries over its special effect to the next turn.

It's possible, and likely later in the game, that the player won't actually be able to make any moves!  For example, the pile shows a five of spades yet the player has only a ten of hearts and a nine of diamonds in their hand.  This brings us to drawing.  If a player can make no moves they must draw a card.  After drawing they may make any move that is then legal.  Drawing need not only happen when one cannot make a move, one may choose to draw a single card each turn, regardless of any cards drawn due to twos.  After drawing, either because they must or they choose, the player cannot draw again until their next turn.  If they still have no legal moves they must pass and, even if they do have legal moves, they may always choose to pass.  They may not choose to pass before drawing a card: every player either draws a card or makes a move each turn (except under the extremely unlikely circumstance where the deck is out of cards and there's only one card in the pile.  Then they must pass if they do not wish to make a move.)

If upon wishing to draw the player finds the deck empty they must take all but the top card of the pile and shuffle them to create a new deck from which to draw.  They then may draw as usual.

#### Conclusion
The game ends when one player has no more cards.  This player has won.

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

Turn 0 starts with each player having 11 cards and the pile showing a **JH** the first player chooses to play a flush of diamonds.  Since the pile isn't showing diamonds they have to match their jack of diamonds to the pile's jack of hearts.  They could have also chosen to order their move, for example,  **8D, JD, AD, 4D, 6D** since the eight is crazy and may be played on top of any card; however, had they done so they would not have gotten to choose which suit would follow next.

Turn 1 is relatively simple, the previous turn it was declared that clubs must follow so player 1 has to make a move that starts with either a club or an eight.  They choose to play a four of clubs.

Turn 2 has more single clubs being played.

Turn 3 has player 1 playing a full house ending on a three.  Since threes skip the next player, player 0's next turn is skipped.  Note that the three in the middle of the move has no effect since it doesn't end up being the top card of the pile.

Turn 5 (given that turn 4 was someone being skipped) has player 1 playing a straight thus dropping all their cards.  Player 1 is victorious.


## Another example
```
isaac@galapagos ~/p8s $ ./p8 -m66 -vv
In progress, Turn 0 (player 0)
Deck (29 cards) AS, 6D, 10H, 8D, 7H, 9C, 7D, 5C, 4H, 6H, JD, QD, 8S, KD, 6C, 3C, KC, 9D, 3S, KS, 5D, QS, 10S, 2S, 8H, 4S, QC, 2D, AC
Pile (1 cards)  KH*
(11 cards)      2C, 7C, AD, AH, 2H, 3H, JH, QH, 6S, 7S, 9S
(11 cards)      4C, 8C, 10C, JC, 3D, 4D, 10D, 5H, 9H, 5S, JS
monte:  0      (0.7% :: 25.8 / 25.5)   28.8%   AH
monte:  10     (1.9% :: 12.6 / 11.9)   34.2%   AH, 2C, AD, 2H
monte:  3      (2.3% :: 11.7 / 9.8)    35.5%   JH
monte:  15     (1.9% :: 9.8 / 9.8)     37.3%   AH, 7C, 7S, AD
monte:  11     (2.8% :: 9.4 / 8.2)     36.8%   AH, 2C, 2H, AD
monte:  4      (3.3% :: 7.7 / 7.0)     38.4%   QH
monte:  13     (3.3% :: 7.4 / 7.0)     38.7%   2H, 2C, AD, AH
monte:  6      (3.9% :: 7.8 / 6.0)     38.1%   AH, AD
monte:  7      (3.9% :: 7.5 / 6.0)     38.4%   2H, 2C, 7C, 7S
monte:  12     (3.9% :: 6.7 / 6.0)     39.2%   AH, AD, 2H, 2C
monte:  22     (3.3% :: 6.0 / 6.0)     39.9%   (draw)
monte:  1      (4.5% :: 6.2 / 5.2)     40.5%   2H
monte:  5      (4.5% :: 5.7 / 5.2)     40.9%   2H, 2C
monte:  9      (3.9% :: 5.9 / 5.2)     40.7%   2H, 7C, 7S, 2C
monte:  19     (3.9% :: 6.2 / 5.2)     40.5%   AH, 2H, JH, QH, 3H
monte:  2      (4.5% :: 4.9 / 4.5)     40.7%   3H
monte:  14     (4.5% :: 4.6 / 4.5)     40.9%   AH, 7C, AD, 7S
monte:  8      (5.2% :: 6.1 / 4.5)     39.5%   2H, 2C, 7S, 7C
monte:  16     (7.0% :: 4.0 / 3.0)     41.2%   AH, AD, 7S, 7C
monte:  17     (9.1% :: 2.4 / 2.3)     42.9%   AH, 2H, 3H, JH, QH
monte:  18     (14.6% :: 2.8 / 1.8)    43.3%   AH, 2H, 3H, QH, JH
monte:  21     (11.7% :: 2.1 / 1.8)    44.0%   2H, 3H, JH, QH, AH
monte:  20     (11.7% :: 0.0 / 1.8)    46.1%   AH, 3H, JH, QH, 2H
Playing AH, 3H, JH, QH, 2H
```

Here we see what the program output looks like if **make loud** was used to compile it.  In this case the Monte Carlo AI will report some information about each possible move.

First comes the move number: move 0 is **AH**.  Next is how close to the maximum number of games had to be played to determine that this move is worse or better than the remaining moves.  The **AH** was very quickly ruled out as a bad move, while the four possible configurations of flushes of hearts took much longer to determine which move was best.

Next comes a number showing how far (in win percentage) the move was from the best move at the time.  So the **AH** move was tending to win ~25.8% less than the best alternative move when it was dropped from consideration.  The flush of hearts ending on a two was 0% less likely to win than the best move at the time since it was the best move.

The next number shows (in win percentage) what the cutoff was to stop considering a move at the time.  Since so few attempts had been made with the **AH** move the cutoff was huge, 25.5%.  Later on, when trying to decide between the various flushes of hearts the cutoff was much lower as each move had been considered for many more hypothetical games and the AI was much more sure of their relative goodness.

The final number is the win percentage for the given move.  The **AH** move, if made, would tend to lead to a much lower chance of winning, 28.8%, than the **AH, 3H, JH, QH, 2H** move, 46.1%; this is clearly because playing just the **AH** would result in the inability to play either the flush of hearts or even the pair of aces.
