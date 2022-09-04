import sys
sys.path.append("..")

from _ui import ui
from _controller import controller_uniforms
from _constants import constants
from _texture import texture

parameters = ui.copy()
parameters.update(controller_uniforms)
parameters.update(constants)
parameters.update(texture)