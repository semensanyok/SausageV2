import os
from scripts.extract_impl_to_cpp import Parser

IN_PATH = "data"
OUT_PATH = "out"

def test_parser():
    for filename in os.listdir(OUT_PATH):
        os.remove(os.path.join(OUT_PATH, filename))
    Parser(file_name="FileWatcherIn.h",in_dir=IN_PATH,out_dir=OUT_PATH).write_header_and_impl()
