import os, sys
from jinja2 import Environment, PackageLoader, select_autoescape
from importlib import import_module

SHADERS_RELATIVE_PATH = "shaders"

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: generate_shaders SHADERS_MODULE_PATH BUILD_PATH\n"
                         f" Notes:\n"
                         f"     SHADERS_PATH must contain {SHADERS_RELATIVE_PATH=} folder\n"
                         )
        exit(1)
    SHADERS_MODULE_PATH = sys.argv[1]
    BUILD_PATH = sys.argv[2]
    sys.path.append(SHADERS_MODULE_PATH)
    SHADERS_PATH = os.path.join(SHADERS_MODULE_PATH, SHADERS_RELATIVE_PATH)
    TEMPLATE_VARS_PATH = os.path.join(SHADERS_PATH, "shared")
    sys.path.append(TEMPLATE_VARS_PATH)
    env = Environment(
        loader=PackageLoader("shaders", "."),
        autoescape=select_autoescape()
    )
    OUT_SHADERS_PATH = os.path.join(BUILD_PATH, SHADERS_RELATIVE_PATH)
    if not os.path.exists(OUT_SHADERS_PATH):
        os.mkdir(OUT_SHADERS_PATH)

    templated_shaders = set()
    # substitute templated shaders
    for template_params in os.listdir(TEMPLATE_VARS_PATH):
        if template_params.startswith("_"):
            continue
        module = import_module(template_params)
        for filename in os.listdir(SHADERS_PATH):
            if filename.startswith(template_params):
                templated_shaders.add(filename)
                template = env.get_template(filename)
                # OVERWRITE TEMPLATE FILE
                with open(os.path.join(OUT_SHADERS_PATH, filename), "w") as out:
                    out.write(template.render(module.parameters))
    # just copy non templated shaders
    for filename in os.listdir(SHADERS_PATH):
        file_abs = os.path.join(SHADERS_PATH, filename)
        if os.path.isdir(file_abs) or filename.startswith("_") or filename in templated_shaders:
            continue
        with open(os.path.join(OUT_SHADERS_PATH, filename), "w") as out:
            with open(file_abs, "r") as in_file:
                out.write(in_file.read())
