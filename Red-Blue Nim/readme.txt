# Expense 8 Puzzle Solver

## Overview
The **Expense 8 Puzzle Solver** is a Python-based game implementation using the **Minimax Algorithm with Alpha-Beta Pruning**. It involves two players taking turns to pick red and blue marbles according to predefined rules. The objective depends on the selected game mode: `standard` or `misere`.

## Features
- **Minimax Algorithm**: Implements an optimal decision-making process for the computer player.
- **Alpha-Beta Pruning**: Reduces the number of nodes evaluated in the minimax algorithm.
- **Two Gameplay Modes**:
  - **Standard Mode**: The player who picks the last marble loses.
  - **Misere Mode**: The player who picks the last marble wins.
- **Human vs. Computer Gameplay**
- **Command Line Interface**

## How to Play
### Game Rules
1. Players take turns picking marbles from a set of red and blue marbles.
2. Possible moves:
   - Pick 2 red marbles
   - Pick 2 blue marbles
   - Pick 1 red marble
   - Pick 1 blue marble
3. The game continues until one color is entirely removed.
4. The winner is determined based on the selected mode (`standard` or `misere`).

### Command Line Usage
```sh
python ed_blue_nim.py <num-red> <num-blue> <version> <first-player>
```
- `<num-red>`: Initial number of red marbles.
- `<num-blue>`: Initial number of blue marbles.
- `<version>`: `standard` or `misere` (default: `standard`).
- `<first-player>`: `human` or `computer` (default: `computer`).

#### Example
```sh
python ed_blue_nim.py 5 6 standard human
```
This starts the game with 5 red and 6 blue marbles in **standard mode**, with the human playing first.

## Algorithm Explanation
### Minimax Algorithm with Alpha-Beta Pruning
1. **Find All Possible Moves**: Generate valid moves for the current state.
2. **Evaluate Terminal States**:
   - If a player has no valid moves, the game ends.
   - The utility function assigns scores based on the game mode.
3. **Recursive Decision Making**:
   - The computer chooses the move that minimizes the opponent’s best possible outcome.
   - Alpha-beta pruning eliminates unnecessary computations.
4. **Game Execution**:
   - The game alternates turns between the human and computer until a terminal state is reached.

### Complexity Analysis
- **Minimax Algorithm Complexity**: `O(b^d)`, where `b` is the branching factor and `d` is the depth of the game tree.
- **Alpha-Beta Pruning Improvement**: Reduces the effective branching factor to `O(b^(d/2))` in the best case.

## Author
- **Sujal More**
- Contact: [sujalm7002@gmail.com](mailto:sujalm7002@gmail.com)
- License: Personal License © 2025 Sujal More

## License
This code is for **educational and personal use only**. Unauthorized copying, distribution, or commercial use is prohibited without explicit permission from the author.

