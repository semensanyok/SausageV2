import sys
sys.path.append("..")

from _ui import ui
from _controller import controller_uniforms

parameters = ui.copy()
parameters.update(controller_uniforms)