"""
 * Personal License
 *
 * Author: Sujal More
 * Project: Red Blue Nim
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
import math
# import pdb

#Find the possible moves and states
def find_moves(num_red, num_blue):
    moves = {}
    if num_red >= 2:
        moves["Pick 2 red"] = (num_red - 2,  num_blue)
    if num_blue >= 2:
        moves["Pick 2 blue"] = (num_red, num_blue - 2)
    if num_red >= 1:
        moves["Pick 1 red"] = (num_red - 1, num_blue)
    if num_blue >= 1:
        moves["Pick 1 blue"] = (num_red, num_blue - 1)

    return  moves

#Do the min-max alpha beta search
def utility_value(num_red, num_blue, g_version):
   if g_version == "standard":
        return 2 * num_red + 3 * num_blue
   else:
       return -(2 * num_red + 3 * num_blue)

def ab_decision(num_red, num_blue, g_version):
    alpha = -math.inf
    beta = math.inf
    best_move = None
    best_value = -math.inf

    #Get the possible moves from the current state
    moves = find_moves(num_red, num_blue)

    for move in moves:
        new_red, new_blue = moves[move]
        v = min_value(new_red, new_blue, alpha, beta, g_version)

        if v > best_value:
            best_value = v
            best_move = move

        alpha = max(alpha, best_value)

    return best_move

def max_value(num_red, num_blue, alpha, beta, g_version):
    if num_red == 0 or num_blue == 0:
        return utility_value(num_red, num_blue, g_version)

    v = -math.inf
    #Get the possible moves from the current state
    moves = find_moves(num_red, num_blue)

    for move in moves:
        new_red, new_blue = moves[move]
        v = max(v, min_value(new_red, new_blue, alpha, beta, g_version))

        if v >= beta:
            return v
        alpha = max(alpha, v)

    return v

def min_value(num_red, num_blue, alpha, beta, g_version):
    if num_red == 0 or num_blue == 0:
        return utility_value(num_red, num_blue, g_version)

    v = math.inf
    moves = find_moves(num_red, num_blue)

    for move in moves:
        new_red, new_blue = moves[move]
        v = min(v, max_value(new_red, new_blue, alpha, beta, g_version))

        if v <= alpha:
            return v

        beta = min(beta, v)

    return v

def main():
    #Make sure we have a valid command line argument
    if len(sys.argv) < 3 or len(sys.argv) > 5:
        print("Incorrect command line parameters. "
              "\nCorrect Format: ed_blue_nim.py <num-red> <num-blue> <version> <first-player>"
              "\n<version> = standard / misere"
              "\n<first-player> = human / computer")
        exit(0)
    else:
        #Initialize the parameters
        arg_len = len(sys.argv)
        num_red = int(sys.argv[1])
        num_blue = int(sys.argv[2])

        #Assign default or user requested values for <version> <first-player>
        if arg_len == 3:
            g_version = "standard"
            f_player = "computer"

        if arg_len == 4:
            #Check g_version is valid
            if sys.argv[3].lower() == "standard" or sys.argv[3].lower() == "misere":
                g_version  = sys.argv[3].lower()
                f_player = "computer"
            else:
                #Assign default values for invalid g_version input. Also assign the f_player as
                #not given in this case
                print("Wrong input for <version>. Setting to default to Standard version")
                g_version = "standard"
                f_player = "computer"

        if arg_len == 5:
            #Check f_player is valid
            if sys.argv[4] == "computer" or sys.argv[4] == "human":
                f_player = sys.argv[4].lower()

                #Check g_version is valid as it will be present for arg_len == 5
                if sys.argv[3].lower() == "standard" or sys.argv[3].lower() == "misere":
                    g_version = sys.argv[3].lower()
                else:
                    print("Wrong input for <version>. Setting to default to Standard version")
                    g_version = "standard"

            else:
                #Assign default to f_player for invalid input and check g_version
                print("Wrong input for <first-player>. Setting to default to Computer as first player")
                f_player = "computer"

                #Check g_version is valid
                if sys.argv[3].lower() == "standard" or sys.argv[3].lower() == "misere":
                    g_version = sys.argv[3].lower()
                else:
                    print("Wrong input for <version>. Setting to default to Standard version")
                    g_version = "standard"


        while(True):
            #Human Player's Turn
            if f_player == "human":

                print("\nYour Turn -> ")
                #Game ended
                if num_red == 0 or num_blue == 0:
                    score = utility_value(num_red, num_blue, g_version)
                    print(f"\nRed = {num_red} Blue = {num_blue}")
                    if g_version == "standard":
                        print(f"Computer Won!!\nScore = {score}")
                    else:
                        print(f"Human Wins!!\nScore = {-1 * score}")
                    break

                #Find the moves possible from the current pool of marbles
                moves = find_moves(num_red, num_blue)

                #Convert move dictionary keys into a list
                move_list = list(moves.keys())
                print(f"Red = {num_red} Blue = {num_blue}")
                print("Choose one of the moves below: ")

                #Print according to the version
                if g_version == "standard":
                    for i, move in enumerate(move_list, start=1):
                        print(f"{i}. {move}")
                else:
                    reversed_move_list = list(reversed(move_list))
                    for i, move in enumerate(reversed(move_list), start=1):
                        print(f"{i}. {move}")


                #Get input and validate
                while True:
                    try:
                        choice = int(input("Move No.: "))
                        if 1 <= choice <= len(move_list):
                            if g_version == "standard":
                                chosen_move = move_list[choice - 1]
                            else:
                                chosen_move = reversed_move_list[choice - 1]
                            #Update game state
                            num_red, num_blue = moves[chosen_move]
                            break
                        else:
                            print("Invalid choice. Pick a number from the list.")
                    except ValueError:
                        print("Invalid input. Please enter a number.")

            #Computer Player's Turn
            else:
                print("\nComputer's Turn -> ")
                #Game ended
                if num_red == 0 or num_blue == 0:
                    score = utility_value(num_red, num_blue, g_version)
                    print(f"\nRed = {num_red} Blue = {num_blue}")
                    if g_version == "standard":
                        print(f"Human Won!!\nScore = {score}")
                    else:
                        print(f"Computer Wins!!\nScore = {-1 * score}")
                    break

                #Find the moves possible from the current pool of marbles
                moves = find_moves(num_red, num_blue)

                print(f"\nRed = {num_red} Blue = {num_blue}")
                print("Choosing one of the available moves: ")
                if g_version == "standard":
                    i = 1
                    for move in moves:
                        print(f"{i}. {move}")
                        i += 1
                else:
                    i = 1
                    for move in reversed(moves):
                        print(f"{i}. {move}")
                        i += 1

                #Find the best move using alpha beta search
                best_move = ab_decision(num_red, num_blue, g_version)
                print(f"Move Chosen:  {best_move}")

                #Update game state
                num_red, num_blue = moves[best_move]

            #Swap player turns
            f_player = "computer" if f_player == "human" else "human"

main()
