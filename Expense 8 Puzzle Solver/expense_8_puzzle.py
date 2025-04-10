"""
 * Personal License
 *
 * Author: Sujal More
 * Project: Expense 8 Puzzle Solver
 *
 * This code is provided solely for educational and
 * personal use. Unauthorized copying, distribution,
 * or commercial use of this code, in whole or in
 * part, without the explicit permission of the author
 * is strictly prohibited.
 *
 * For permissions or inquiries, please contact:
 * sujalm7200@gmail.com
 *
 * © 2025 Sujal More. All rights reserved.
"""

import sys
from collections import deque
import pdb
import heapq
from datetime import datetime

#Function for opening and reading files
def open_read_file(fileName):
    with open(fileName, "r") as file:
        lines = file.readlines()

    if lines[-1].strip() == "END OF FILE":

        lines.pop()
        #Convert to a 2D matrix for easier calculations
        matrix = [list(map(int, line.split())) for line in lines]
        return matrix
    else:
        print("No END OF FILE found, please check format of the input file.\n")
        exit(-1)

#Getting data into tuples for easier comparison
def state_to_tuple(state):
    return tuple(tuple(row) for row in state)

#Find the zero
def find_blank_space(matrix):
    for i in range(3):
        for j in range(3):
            if matrix[i][j] == 0:
                return i, j

    return None

#Get the possible reachable states
def possible_moves(state):
    moves = []

    state_matrix = [list(row) for row in state]
    result = find_blank_space(state_matrix)

    if result is None:
        print("Error: Blank space (0) not found in state!")
        exit(0)

    blank_r, blank_c = result

    #Get the possible moves,
    #First two pairs move one position above and below. (Row)
    #Second two pairs move one place before and after the current position. (Cols)
    move_directions = [(-1, 0, "Up"), (1, 0, "Down"), (0, -1, "Left"), (0, 1, "Right")]

    for r, c, direction in move_directions:
        new_r, new_c = blank_r + r, blank_c + c
        if 0 <= new_r < 3 and 0 <= new_c < 3:
            new_state = [list(row) for row in state]
            new_state[blank_r][blank_c], new_state[new_r][new_c] = new_state[new_r][new_c], new_state[blank_r][blank_c]
            moves.append((state_to_tuple(new_state), state[new_r][new_c], direction, state[new_r][new_c]))

    return moves

#Find the Manhattan Distance between the current state and the goal state
def find_manH_Dist(state, goal):
    h_N = {}
    g_Posi = {}
    state_matrix = [list(row) for row in state]

    for i in range(3):
        for j in range(3):
            #Get the position of each number and put it at that idx of the map/ key
            g_Posi[goal[i][j]] = (i, j)

    for i in range(3):
        for j in range(3):
            num = state[i][j]
            if num != 0:
                goal_r, goal_c = g_Posi[num]
                h_N[num] = abs(goal_r - i) + abs(goal_c - j)

    h_N_cost = sum(h_N.values())

    return h_N_cost

#Function that implements the search algorithms
def find_path(initial, goal, method, dump):

    global nodes_Popped, nodes_Expanded, nodes_Generated, max_Fringe, cost, dFile
    # Globals
    nodes_Popped = 0
    nodes_Expanded = 0
    nodes_Generated = 0
    max_Fringe = 0
    cost = 0

    # pdb.set_trace()
    if dump:
        curr_Time = datetime.now()
        fileName = curr_Time.strftime("%Y-%m-%d-%H;%M;%S")
        fileName += ".txt"
        dFile = open(fileName, "w")
        dFile.write(f"Method: {method}\nRunning {method}\n\n")


    #Using a set to keep track of unique elements
    visited = set()
    #Solution path map
    parent_map = {}

    #BFS
    if method == "bfs":
        print("Running BFS...")
        fringe = deque()
        fringe.append((state_to_tuple(initial), 0))

        if dump:
            dFile.write(f"{"Initial FRINGE\n"}{list(fringe)}\n")

        # pdb.set_trace()

        while fringe:
            if len(fringe) > max_Fringe:
                max_Fringe = len(fringe)

            state, steps = fringe.popleft()
            nodes_Popped += 1
            if dump:
                dFile.write(f"{"Generating Successor to:"}{list(state)}\n")

            if state == state_to_tuple(goal):
                path = []
                while state in parent_map:
                    prev_state, moved_tile, direction = parent_map[state]
                    path.append(f"Move {moved_tile} {direction}")
                    cost += moved_tile
                    state = prev_state

                return steps, path

            if state in visited:
                continue

            visited.add(state)
            nodes_Expanded += 1
            if dump:
                dFile.write(f"{" ":<15}{"Closed: \n"}{" ":<15}{list(visited)}\n")

            for next_state, moved_tile, direction, _ in possible_moves(state):
                if next_state not in visited:
                    fringe.append((next_state, steps + 1))
                    parent_map[next_state] = (state, moved_tile, direction)
                    nodes_Generated += 1

            if dump:
                dFile.write(f"{" ":<15}Nodes Generated = {nodes_Generated}\n"
                            f"{" ":<15}{"FRINGE\n"}{" ":<15}{list(fringe)}\n")


    #UCS
    elif method == 'ucs':
        print("Running UCS... ")

        fringe = []
        heapq.heappush(fringe, (0, state_to_tuple(initial)))
        #Cost map
        cost_map = {state_to_tuple(initial): 0}

        if dump:
            dFile.write(f"{"Initial FRINGE\n"}{list(fringe)}\n")

        while fringe:
            if len(fringe) > max_Fringe:
                max_Fringe = len(fringe)

            current_cost, state = heapq.heappop(fringe)
            nodes_Popped += 1
            if dump:
                dFile.write(f"{"Generating Successor to:"}{list(state)}"
                            f" Cost: {current_cost}\n")

            if state == state_to_tuple(goal):
                path = []
                depth = 0
                while state in parent_map:
                    prev_state, moved_tile, direction = parent_map[state]
                    path.append(f"Move {moved_tile} {direction}")
                    state = prev_state
                    depth += 1
                cost = current_cost

                return depth, path

            if state in visited:
                continue

            visited.add(state)
            nodes_Expanded += 1
            if dump:
                dFile.write(f"{" ":<15}{"Closed: \n"}{" ":<15}{list(visited)}\n")

            for next_state, moved_tile, direction, move_cost in possible_moves(state):
                new_cost = current_cost + move_cost
                if next_state not in cost_map or new_cost < cost_map[next_state]:
                    cost_map[next_state] = new_cost
                    parent_map[next_state] = (state, moved_tile, direction)
                    heapq.heappush(fringe, (new_cost, next_state))
                    nodes_Generated += 1

            if dump:
                dFile.write(f"{" ":<15}Nodes Generated = {nodes_Generated}\n"
                            f"{" ":<15}{"FRINGE\n"}{" ":<15}{list(fringe)}\n")

    #GREEDY
    elif method == "greedy":
        print('Running Greedy...')

        fringe = []
        start_HValue = find_manH_Dist(initial, goal)
        heapq.heappush(fringe, (start_HValue, state_to_tuple(initial)))
        if dump:
            dFile.write(f"{"Initial FRINGE(With h(n)\n"}{list(fringe)}\n")

        while fringe:
            if len(fringe) > max_Fringe:
                max_Fringe = len(fringe)

            curr_Heu, state = heapq.heappop(fringe)
            nodes_Popped += 1
            if dump:
                dFile.write(f"{"Generating Successor to:"}{list(state)}"
                            f"  h(n) = {curr_Heu}\n")

            if state == state_to_tuple(goal):
                path = []
                depth = 0
                while state in parent_map:
                    prev_state, moved_tile, direction = parent_map[state]
                    path.append(f"Move {moved_tile} {direction}")
                    state = prev_state
                    cost += moved_tile
                    depth += 1

                return depth, path

            if state in visited:
                continue

            visited.add(state)
            nodes_Expanded += 1
            if dump:
                dFile.write(f"{" ":<15}{"Closed: \n"}{" ":<15}{list(visited)}\n")

            for next_state, moved_tile, direction, move_cost in possible_moves(state):
                h_Value = find_manH_Dist(next_state, goal)
                if next_state not in visited:
                    parent_map[next_state] = (state, moved_tile, direction)
                    heapq.heappush(fringe, (h_Value, next_state))
                    nodes_Generated += 1

            if dump:
                dFile.write(f"{" ":<15}Nodes Generated = {nodes_Generated}\n"
                            f"{" ":<15}{"FRINGE with h(n)\n"}{" ":<15}{list(fringe)}\n")

    elif method == "a*":
        print("Running A*...")

        fringe = []
        start_HValue = find_manH_Dist(initial, goal)
        heapq.heappush(fringe, (start_HValue, 0, state_to_tuple(initial)))
        if dump:
            dFile.write(f"{"Initial FRINGE\n"}{list(fringe)}\n")

        #g(n) Map
        g_map = {state_to_tuple(initial): 0}

        while fringe:
            if len(fringe) > max_Fringe:
                max_Fringe = len(fringe)

            f_value, g_value, state = heapq.heappop(fringe)
            nodes_Popped += 1
            if dump:
                dFile.write(f"{"Generating Successor to:"}{list(state)}"
                            f"  f(n) = {f_value}\n")

            if state == state_to_tuple(goal):
                path = []
                depth = 0
                while state in parent_map:
                    prev_state, moved_tile, direction = parent_map[state]
                    path.append(f"Move {moved_tile} {direction}")
                    state = prev_state
                    cost += moved_tile
                    depth += 1

                return depth, path

            if state in visited:
                continue

            visited.add(state)
            nodes_Expanded += 1
            if dump:
                dFile.write(f"{" ":<15}{"Closed: \n"}{" ":<15}{list(visited)}\n")

            for next_state, moved_tile, direction, move_cost in possible_moves(state):
                new_g = g_value + move_cost
                if next_state not in g_map or new_g < g_map[next_state]:
                    g_map[next_state] = new_g
                    f_value = new_g + find_manH_Dist(next_state, goal)
                    heapq.heappush(fringe, (f_value, new_g, next_state))
                    parent_map[next_state] = (state, moved_tile, direction)
                    nodes_Generated += 1

            if dump:
                dFile.write(f"{" ":<15}Nodes Generated = {nodes_Generated}\n"
                            f"{" ":<15}{"FRINGE h(n), g(n)\n"}{" ":<15}{list(fringe)}\n")

    #No solution found
    return -1, -1

def main():
    if len(sys.argv) < 4:
        print("Command Line Parameters are incorrect. \nCorrect format: "
              "expense_8_puzzle.py <start-file> <goal-file> <method> <dump-flag>")
    else:
        if len(sys.argv) == 5 and sys.argv[4].lower() == "true":
            dump_flag = sys.argv[4]
        else:
            dump_flag = False

        start_state = open_read_file(sys.argv[1])
        goal_state = open_read_file(sys.argv[2])

        if (sys.argv[3].lower() == "bfs" or  sys.argv[3].lower() == "ucs" or sys.argv[3].lower() == "greedy"
                or sys.argv[3] == "a*"):

            moves, path = find_path(start_state, goal_state, sys.argv[3].lower(), dump_flag)
            if moves == -1:
                print(f'Nodes Popped: {nodes_Popped}\n'
                      f'Nodes Expanded: {nodes_Expanded}\n'
                      f'Nodes Generated: {nodes_Generated}\n'
                      f'Max Fringe Size: {max_Fringe}\n')
                print("No Solution Found!!")
                exit(0)
            else:
                print(f'Nodes Popped: {nodes_Popped}\n'
                      f'Nodes Expanded: {nodes_Expanded}\n'
                      f'Nodes Generated: {nodes_Generated}\n'
                      f'Max Fringe Size: {max_Fringe}\n'
                      f'Solution Found at depth {moves} with a cost of {cost}\n'
                      f'Steps: ')
                for move in reversed(path):
                    print(" ", move)
        else:
            print("Please Provide a valid Search Algorithm. bfs, ucs, greedy, a*")
            exit(-1)

main()
