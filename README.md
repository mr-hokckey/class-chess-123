Leo Assimes

I did this project on Windows. But I didn't fully finish the first part of the chess movement assignment before, so I had to get all of that working along with the new stuff. I'll explain my approach to everything here.

## Part 1

Most of this program was writing canBitMoveFrom and canBitMoveFromTo. The former works by checking which piece is moving, and matching movement patterns to whether or not they're currently being blocked by friendly pieces. The latter works in many of the same ways that generating move lists worked in the Coding Adventure video.

Capturing pieces sort of worked automatically, since if you move a bit to a bitholder, the old bit gets deleted and replaced. But I end up adding a check for that in Part 2 using pieceTaken(). It was kind of weird to write because in Game.cpp, pieceTaken() is called even on incomplete illegal moves. So I ended up making a separate variable to use in comparisons to keep this in check.

## Part 2

FEN strings were interesting to set up. First thing I did was piece placement, which involved some loops with complex branching and variables to keep track of. I also had to iterate through the rows backwards, since FEN strings go from top to bottom.

Then I had to add variables for active color, castling rights, en passant, and move timers. Some of these variables and functions kind of already existed in other parts of the program (i.e. Game.h and Game.cpp). But I also made some new variables to keep track of things, as well as new functions. You can see these in Chess.h.

A lot of things were handled in bitMovedFromTo(). checkCastlingRights() was made to check when castling would become permanently unavailable, but the actual motions of moving the King was checked for in canBitMoveFromTo(). En Passant could be programmed using existing code in the Pawn movement check, and pawn promotion was as simple as checking if a piece is at the end of the board and creating a new Queen bit.

Sorry I'm running out of time to write things in more detail. But this is how things went in a nutshell. I had help in office hours from Reza to help understand how this code is similar and different to Coding Adventure.

## Writing Negamax

Going into the negamax AI, I have a problem: I am very short on time because I've been bogged down with other assignments and haven't had any time to start this project until Friday. And I haven't had time to implement the concept of check. It's a pretty complicated concept to implement and I'm not sure I'll have time. So instead, I wrote my negamax AI to play with the end goal of capturing the King. It's a scuffed solution, but at the very least the AI will be functional.