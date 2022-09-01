import sys
sys.path.append("..")

from _light import light
from _texture import texture
from _mesh import mesh

parameters = light.copy()
parameters.update(texture)
parameters.update(mesh)