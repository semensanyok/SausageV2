import os
from typing import Tuple, List, IO
import regex as re

IN_DIR = "../engine/src"
OUT_DIR = "../engine/src/testparser"

ignore_files = [
    "sausage.h"
]

CLASS_NAME_PATTERN = "class ([a-zA-Z]*)[ \r\n]"
FUNC_IMPL_START_PATTERN = "([a-zA-Z*&_0-9<>]*) ([a-zA-Z_0-9<>:]*)(\(.*\))([ \r\n]{?)"

class Parser:
    def __init__(self, file_name: str, in_dir: str, out_dir: str):
        self.file_name = file_name
        self.in_file_path = os.path.join(in_dir, self.file_name)
        self.impl_file_name = file_name.replace(".h", ".cpp")
        self.out_impl_file_path = os.path.join(out_dir, self.impl_file_name)
        self.out_header_file_path = os.path.join(out_dir, self.file_name)
        if not os.path.exists(out_dir):
            os.mkdir(out_dir)
        self.header_content = ""
        self.impl_content = f"#include {self.file_name} {os.linesep}"
        self.cur_class_name = ""
    def write_header_and_impl(self):
        with open(self.in_file_path, 'r') as f:

            for line in f:
                if match_class:=re.findall(CLASS_NAME_PATTERN, line):
                    self.cur_class_name = match_class[0]
                func_start = re.findall(FUNC_IMPL_START_PATTERN, line)
                if func_start:
                    # keep constructor destructor in header
                    if self.cur_class_name in func_start[0][1]:
                        self.header_content += line
                    else:
                        self.header_content += f"{func_start[0][0]};{os.linesep}"
                        self.impl_content += f"{''.join(func_start[0])}{os.linesep}"
                        self.__write_impl(f)
                else:
                    self.header_content += line
        with open(self.out_header_file_path, 'w') as f:
            f.write(self.header_content)
        with open(self.out_impl_file_path, 'w') as f:
            f.write(self.impl_content)

    def __write_impl(self, f: IO):
        brackets_open = 1
        for line in f:
            if brackets_open <= 0:
                break
            brackets_open = brackets_open + line.count("{") - line.count("}")
            self.impl_content += line

class FuncDesc:
    def __init__(self, decl: str, impl: str):
        self.decl = decl
        self.impl = impl

class ClassDesc:
    def __init__(self, name: str):
        self.name = name
        self.functions: List[FuncDesc] = []


if __name__ == '__main__':
    for filename in os.listdir(IN_DIR):
        if filename.endswith(".h") \
                and filename not in ignore_files \
                and not os.path.exists(os.path.join(OUT_DIR, filename.replace(".h", ".cpp"))):
            Parser(file_name=filename, in_dir=IN_DIR, out_dir=OUT_DIR).write_header_and_impl()
