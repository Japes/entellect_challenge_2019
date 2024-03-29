# -*- coding: utf-8 -*-
"""
Entelect StarterBot for Python3
"""
import time
import json
import os
import logging
import numpy as np
from scipy.spatial import distance

from cell import Cell, AugmentedCell
from direction_helper import get_cardinal_direction, get_straight_line
import astar

logging.basicConfig(filename='sample_python_bot.log', filemode='w', level=logging.DEBUG)
logger = logging.getLogger(__name__)


class StarterBot:

    def __init__(self):
        """
        Initialize Bot .
        """

        self.current_round = None
        self.command = ''

        self.game_state = None
        self.full_map = None
        self.flattened_map = None
        self.player_info = None
        self.enemy_info = None
        self.columns = None
        self.rows = None
        self.max_rounds = None
        self.current_worm_id = None
        self.consecutive_no_commands = None
        self.current_worm_info = None
        self.worms = None
        self.health_pack_positions = []

        self.augmented_map = None

    def get_current_round_details(self):
        """
        Reads in all relevant information required for the round.
        """

        state_location = os.path.join('rounds', str(self.current_round), 'state.json')
        self.game_state = self.load_state_json(state_location)

        self.command = ''

        self.rows = self.game_state['mapSize']
        self.columns = self.game_state['mapSize']

        self.full_map = self.game_state['map']
        self.flattened_map = self.get_map()

        self.max_rounds = self.game_state['maxRounds']
        self.current_worm_id = self.game_state['currentWormId']
        self.consecutive_no_commands = self.game_state

        self.player_info = self.game_state['myPlayer']
        self.enemy_info = self.game_state['opponents']

        self.worms = self.player_info['worms']
        self.current_worm_info = self.get_worm_player_info(self.current_worm_id)

        self.augmented_map = self.get_augmented_map()
        logger.info('Done loading new round info.  health packs are at ' + str(self.health_pack_positions))

        return None

    def get_map(self):
        map = []
        self.health_pack_positions = []
        for row in self.full_map:
            for cell in row:
                if 'occupied' in cell.keys():
                    map.append(Cell(cell['x'], cell['y'], cell['type'], cell['occupied']))
                else:
                    map.append(Cell(cell['x'], cell['y'], cell['type'], None))
                if 'powerup' in cell.keys():
                    for x in cell:
                        logger.info (str(x) + ':' + str(cell[x]))
                    self.health_pack_positions.append([cell['x'], cell['y']])
        return map

    def get_worm_player_info(self, worm_id):
        """
        Retrieve info for player's requested worm id
        """
        for worm in self.worms:
            if worm['id'] == worm_id:
                break
        return worm

    def get_worms_in_range(self):
        """
        Returns a list of coordinates with worms in shooting range
        Takes into account if there are any obstacles in the way.
        If there is an obstacle, the worm is seen as not in range.
        """
        max_range = self.current_worm_info['weapon']['range']
        current_x = self.current_worm_info['position']['x']
        current_y = self.current_worm_info['position']['y']

        cells_in_range = []
        for opponent in self.enemy_info:
            for w in opponent['worms']:
                worm = w['position']
                dist = np.floor(distance.euclidean([worm['x'], worm['y']], [current_x, current_y]))
                if dist <= max_range:
                    direction = get_cardinal_direction([current_x, current_y], [worm['x'], worm['y']])
                    obstacles = self.check_for_obstacles_in_path([current_x, current_y], [worm['x'], worm['y']],
                                                                 direction)
                    if (direction is None) or (obstacles is True):
                        continue
                    else:
                        cells_in_range.append([worm['x'], worm['y'], direction])

        return cells_in_range

    def check_for_obstacles_in_path(self, reference, target, direction):
        """
        Takes in coordinates for a reference cell [x,y] ,target cell [x,y], and the direction.
        Checks if there are any DIRT cells within the path from reference and target in the specified direction.
        """
        if direction is not None:
            x_diff = target[0] - reference[0]
            y_diff = target[1] - reference[1]

            line_points = get_straight_line(reference, target, direction)

            obstacle_in_path = False

            for cell in line_points:
                logger.info(cell)
                cell_type = self.get_cell_type(cell[0], cell[1])
                if cell_type is None:
                    continue
                elif (cell_type == 'DIRT') or (cell_type == 'DEEP_SPACE'):
                    obstacle_in_path = True
                    break
        else:
            obstacle_in_path = True

        return obstacle_in_path

    def get_cell_type(self, x, y):
        """
        return the type of a cell at a specified set of coordinates
        """
        cell_type = None
        for cell in self.flattened_map:
            if (cell.x == x) and (cell.y == y):
                cell_type = cell.type
        return cell_type

    def get_augmented_map(self):
        """
        1. Calculates the distance to every cell from current cell.
        2. Checks the cardinal direction for cell with reference to current cell.
        3. Removes all cells that are 'DEEP_SPACE', since these cannot be interacted with.
        Returns a dataframe of self.flattened_map with additional columns ['x','y','type',''Distance', 'Direction']
        """
        augmented_map = []

        current_x = self.current_worm_info['position']['x']
        current_y = self.current_worm_info['position']['y']

        for cell in self.flattened_map:
            cell_distance = int(np.floor(distance.euclidean([cell.x, cell.y], [current_x, current_y])))
            direction = get_cardinal_direction([current_x, current_y], [cell.x, cell.y])
            augmented_map.append(AugmentedCell(cell.x, cell.y, cell.type, cell_distance, direction))
        return augmented_map

    def get_available_cells(self, objective):
        """
        Gets a list of valid cells for a specific objective.
        """
        available_cells = []
        if objective == 'dig':
            digging_range = self.current_worm_info['diggingRange']
            for cell in self.augmented_map:
                if (cell.distance <= digging_range) and (cell.type == 'DIRT'):
                    available_cells.append(cell)
        elif objective == 'move':
            movement_range = self.current_worm_info['movementRange']
            for cell in self.augmented_map:
                if (cell.distance <= movement_range) and (cell.type == 'AIR'):
                    available_cells.append(cell)
        return available_cells

    def write_action(self):
        """
        command in form : C;<round number>;<command>
        """

        print(f'C;{self.current_round};{self.command}')
        logger.info(f'Writing command : C;{self.current_round};{self.command};')

        return None

    def load_state_json(self, state_location):
        """
        Gets the current Game State json file.
        """
        json_map = ''
        try:
            json_map = json.load(open(state_location, 'r'))
        except IOError:
            logger.error("Cannot load Game State")
        return json_map

    def wait_for_round_start(self):
        next_round = int(input())
        return next_round

    def should_i_fight(self, worms_in_range):
        return None
        if len(worms_in_range) == 0:
            return None
        #can I win a fight here?
        number_worms = len(worms_in_range)
        #for i, worm in enumerate(worms_in_range):
            
        #choice = np.random.randint(number_worms)
        #attack_x = worms_in_range[choice][0]
        #attack_y = worms_in_range[choice][1]
        #direction = worms_in_range[choice][2]
        #return f'shoot {direction}'

    def astar_cost(self, current, neighbor ):
        if self.get_cell_type(neighbor[0], neighbor[1]) == "AIR":
            return 1
        elif self.get_cell_type(neighbor[0], neighbor[1]) == "DIRT":
            return 2
        elif self.get_cell_type(neighbor[0], neighbor[1]) == "DEEP_SPACE":
            return 999999
        else:
            logger.error("ERROR IN astar_cost, what is this cell? " + str(neighbor))

    def run_bot(self):
        logger.info("Bot has started Running")
        while True:
            logger.info('Waiting for next round.')
            next_round_number = self.wait_for_round_start()
            logger.info('Starting Round : ' + str(next_round_number))
            self.current_round = next_round_number
            self.get_current_round_details()
            logger.info('Beginning StarterBot Logic Sequence')
            self.bot_logic()

            self.write_action()

    def bot_logic(self):
        '''
        am I in a fight I can win?  
            Keep fighting.
        else:
            are the healthpacks still there? 
                race for them
            else,
                is there a target I can beat in a fight?
                    hunt them
                else 
                    am I far from an ally?
                        move towards nearest
                    else
                        mine for points
        '''
        worms_in_range = self.get_worms_in_range()
        fight_command = self.should_i_fight(worms_in_range)
        if(fight_command != None):
            self.command = fight_command
            return None

#NOTE THIS BOT IS TOTALLY UNFINISHED AND BROKEN-------------------------

        #make a beeline for the healthpacks
        #finder = astar.pathfinder( distance=distance.euclidean, cost=self.astar_cost)#, neighbors=grid_neighbors(10,10) )
        finder = astar.pathfinder()

        current_x = self.current_worm_info['position']['x']
        current_y = self.current_worm_info['position']['y']
        path = finder( (current_x,current_y), (self.health_pack_positions[0][0],self.health_pack_positions[0][1]) )
        logger.info("GOT A PATH FROM : " + str([current_x, current_y]) + " to " + str(self.health_pack_positions[0]) + " : " + str(path))


        return None
'''
        else:
            move_options = ['move', 'dig', 'nothing']
            choice = np.random.randint(len(move_options))
            selected_move = move_options[choice]

            if selected_move == 'nothing':
                self.command = selected_move

            elif selected_move == 'dig':
                available_cells = self.get_available_cells('dig')
                number_avail_cells = len(available_cells)
                if number_avail_cells == 0:
                    self.command = f'nothing'
                else:
                    choice = np.random.randint(len(available_cells))
                    selected_cell = available_cells[choice]
                    self.command = f"dig {selected_cell.x} {selected_cell.y}"

            elif selected_move == 'move':
                available_cells = self.get_available_cells('move')
                number_avail_cells = len(available_cells)
                if number_avail_cells == 0:
                    self.command = f'nothing'
                else:
                    choice = np.random.randint(len(available_cells))
                    selected_cell = available_cells[choice]
                    self.command = f"move {selected_cell.x} {selected_cell.y}"
            else:
                self.command = f'nothing'
                '''

if __name__ == '__main__':
    bot = StarterBot()
    bot.run_bot()
