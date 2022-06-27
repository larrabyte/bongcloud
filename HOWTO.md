# Chess

Chess is a two-player board game played on a chessboard (a square-checkered board, typically with sixty four squares laid out in an eight-by-eight grid).

In a standard chess game, each player begins with sixteen pieces: one king, one queen, two rooks, two knights, two bishops and eight pawns. The objective of the game is to checkmate the opponent's king, whereby the king is under immediate attack (in check) and there is no possible way to remove or defend it from attack.

## Terminology

The board is split up into rows and columns, which go by the names of rank and file.

A trivial move refers to a move that does not involve a pawn or a capture of a piece.

## Ending Conditions

A game of chess can end in one of three ways:
- Checkmate, where a player's king is under immediate attack and there is no possible way of escaping it. In such a case, the player who was checkmated loses.
- Stalemate, where a player is NOT under immediate attack but has no legal moves available to them. In such a case, the game is a draw.
- Resignation, where a player willingly forfeits the game. In such a case, the resigner loses.

## The Pieces

There are six types of pieces in standard chess:
- The king, which is the most important piece in the game. Protect it at all costs.
    - The king can move one square in any direction at all times.
    - It can also "castle", a move involving the king and one rook on the starting file.
        - This is only permitted if neither the king and rook have moved.
        - The king slides over two squares towards the rook, while the rook itself moves to the square opposite the king.
        - This is only permitted if the squares which the king moves through are not attacked by the opponent.

- The queen is the most agile piece in the game, being able to move to the most number of squares.
    - The queen can move in a horizontal or vertical line as well as diagionally.

- The rook, which is a more limited version of the queen.
    - The rook can move in horizontal or vertical lines only.

- The bishop, which is another, more limited version of the queen.
    - The bishop can move in diagonal lines only.

- The knight, which is a special piece that can jump across squares.
    - The knight moves in an L-shape, moving two squares in one direction and moving one in another direction perpendicular to the first.

- The pawn, which is simultaneously the most expendable and valuable piece in the game.
    - The pawn can move one square forward (eg. up for white and down for black).
    - Pawns may capture diagionally one square ahead of them if there is a piece present.
    - When a pawn reaches the last rank, it must promote to another piece.
        - Possible pieces include the knight, bishop, rook and queen. In `bongcloud`, the pawn automatically promotes to a queen.
    - On its first move, a pawn may also choose to move two squares instead of one.
    - Due to the rule above, a special provision known as en passant also exists, which allows the opponent to capture a pawn that has moved two squares if capture would have been possible if the pawn had only moved one square.

## Special Rules

Chess is subject to a hard limit of 50 trivial moves in a row. This means that if more than 50 trivial moves are played in a row, the game automatically concludes in a forced draw.

Any moves that are played must not leave the moving player in check.
