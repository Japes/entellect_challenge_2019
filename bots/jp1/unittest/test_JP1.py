import unittest
import os
import sys
dir_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.abspath(dir_path + "/.."))
from JP1 import StarterBot

def test_powerup_positions():
    bot = StarterBot()
    state_location = os.path.join(dir_path, 'state.json')
    bot.game_state = bot.load_state_json(state_location)
    bot.full_map = bot.game_state['map']
    bot.get_map()
    assert (bot.health_pack_positions == [[15,14],[17,18]])