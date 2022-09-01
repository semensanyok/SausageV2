import sys
sys.path.append("..")

from _light import light
from _texture import texture

parameters = light.copy()
parameters.update(texture)