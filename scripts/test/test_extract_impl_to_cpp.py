from scripts.extract_impl_to_cpp import Parser

def test_parser():
    Parser(file_name="FileWatcherIn.h",in_dir="data",out_dir="out").write_header_and_impl()
