project ("sausage")
# generated 4.6 compability profile with ARB and CGI extentions https://glad.dav1d.de/

project(Glad)

add_library(glad "include/glad/glad.h" "include/KHR/khrplatform.h" "src/glad.c")
target_include_directories(glad PUBLIC include/)
