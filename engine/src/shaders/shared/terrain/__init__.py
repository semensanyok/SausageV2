import sys
sys.path.append("..")

from _terrain import terrain
from _constants import constants
from _mesh import mesh
from _light import light
from _texture import texture

parameters = terrain.copy()

# vs
parameters["uniforms_locations"] = constants["uniforms_locations"]
parameters["mesh_set_transform"] = mesh["mesh_set_transform"];
parameters["mesh_set_res_position"] = mesh["mesh_set_res_position"];
parameters["mesh_set_res_normal"] = mesh["mesh_set_res_normal"];
parameters["mesh_set_gl_Position"] = mesh["mesh_set_gl_Position"];
parameters["mesh_fs_functions"] = mesh["mesh_fs_functions"];
# parameters["mesh_vs_out"] = mesh["mesh_vs_out"];
# parameters["mesh_set_vs_out"] = mesh["mesh_set_vs_out"];

# fs
# parameters["mesh_fs_in"] = mesh["mesh_fs_in"];

parameters.update(light)
parameters.update(texture)
parameters.update(constants)