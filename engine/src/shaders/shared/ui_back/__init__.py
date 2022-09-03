import sys
sys.path.append("..")

from _ui import ui
from _controller import controller_uniforms
from _constants import constants

parameters = ui.copy()
parameters.update(controller_uniforms)
parameters.update(constants)