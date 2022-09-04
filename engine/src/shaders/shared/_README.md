engine/scripts/generate_shaders.py

each subdirectory under `shaders/shared` contains `parameters` variable, used by ninja to expand templates.

this script takes subdirectory name and use it as prefix to find corresponding shaders templates in `shaders` directory.

keep only `__init__.py` file under each `shared` subdirectory. import_module in `generate_shaders.py` picks it up and looks for `parameters` variable.

place other dictionaries in root directory, and prefix with single underscore `_`