
import subprocess
for path, url in [
    ("engine/dependencies/assimp", "https://github.com/assimp/assimp.git"),
    ("engine/dependencies/bullet3", "https://github.com/bulletphysics/bullet3.git"),
    ("engine/dependencies/glad", "https://github.com/Dav1dde/glad.git"),
    ("engine/dependencies/assimp", "https://github.com/assimp/assimp.git"),
    ("engine/dependencies/glm", "https://github.com/g-truc/glm.git"),
    ("engine/dependencies/stb", "https://github.com/nothings/stb.git"),
    ("engine/dependencies/zlib", "https://github.com/madler/zlib.git"),
    ("engine/dependencies/freetype", "https://github.com/freetype/freetype.git"),
    # ("engine/dependencies/libRocket", "https://github.com/libRocket/libRocket.git"),
    ("engine/dependencies/imgui", "https://github.com/ocornut/imgui.git"),
    # ("engine/dependencies/gtk", "https://gitlab.gnome.org/GNOME/gtk.git")
    ]:
    subprocess.run(args=["git", "submodule", "add", url, path])
