
# Expense 8 Puzzle Solver

## Description

The **Expense 8 Puzzle Solver** is a program designed to solve a modified version of the classic 8 puzzle problem, called the **Expense 8 Puzzle**. In this puzzle, the goal is to arrange a 3x3 grid of 8 numbered tiles in a specific target configuration. The twist is that each tile has an associated movement cost, and the agent needs to minimize the total cost of moving the tiles to the goal state.

You can use various search algorithms to solve the puzzle, including **Breadth-First Search (BFS)**, **Uniform Cost Search (UCS)**, **Greedy Search**, and **A* Search**, each providing different trade-offs between search efficiency and the cost of reaching the goal.

## Features

- Solve the **Expense 8 Puzzle** using multiple search algorithms.
- Track search process, including fringe, closed set, and the nodes expanded at each step.
- Dump the search trace to a text file for analysis.
- Provide custom heuristics for **Greedy** and **A\*** search algorithms.
  
### Parameters:
- `<start-file>`: The file containing the start state of the puzzle (3x3 grid).
- `<goal-file>`: The file containing the goal configuration of the puzzle (3x3 grid).
- `<method>`: The search algorithm to use. Choose one of the following:
  - `bfs` - **Breadth First Search**
  - `ucs` - **Uniform Cost Search**
  - `greedy` - **Greedy Search**
  - `a*` - **A* Search** (default method if no method is provided)
- `<dump-flag>` (optional): If set to `true`, a detailed search trace will be written to a file named `trace-<date>-<time>.txt`. If not specified, it defaults to `false`.

## Search Trace Format

If the `<dump-flag>` is set to `true`, the following data will be included in the search trace file:

- **Fringe**: Contents of the fringe (queue/priority queue) during each iteration.
- **Closed Set**: The set of visited nodes (states) during the search.
- **Nodes Expanded**: The number of nodes expanded during the search.
- **Nodes Generated**: The number of nodes generated in the search.
- **Search Step-by-Step**: Information on which state is being processed, the current cost, and the moves made.

## Heuristics

For **Greedy Search** and **A\*** Search, the program uses the **Manhattan Distance** as the heuristic function. This is a common heuristic for the 8-puzzle, where the cost is calculated as the sum of the absolute differences in row and column positions between each tile's current position and its target position in the goal state.

### Example Input Files:
- `start.txt`: Contains the starting configuration of the puzzle.
- `goal.txt`: Contains the goal configuration of the puzzle.

Both files must contain a 3x3 grid of integers, with `0` representing the blank space.

## License

This project is licensed under the **Personal License**. Unauthorized copying, distribution, or commercial use of this code, in whole or in part, without explicit permission from the author, is strictly prohibited. For permissions or inquiries, please contact [sujalm7002@gmail.com](mailto:sujalm7002@gmail.com).

## Acknowledgements

- The **8 Puzzle Problem** is a classic problem in artificial intelligence and is widely used to teach search algorithms.
- This project was developed as part of coursework for CSE 4308 [Artifical Intelligence]
