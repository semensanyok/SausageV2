import sys
sys.path.append("..")

from _light import light
from _texture import texture
from _constants import constants

parameters = light.copy()
parameters.update(texture)
parameters.update(constants)