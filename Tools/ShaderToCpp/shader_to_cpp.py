#!/usr/bin/env python3

import sys
import os
from enum import Enum


class ParsingStage(Enum):
    NONE = 0
    VERTEX_SHADER = 1
    FRAGMENT_SHADER = 2


class ShaderParser:

    def __init__(self, input_file_path, cpp_file_path, h_file_path):
        self._parsing_stage = ParsingStage.NONE
        self._input_file_path = input_file_path
        self._cpp_file_path = cpp_file_path
        self._h_file_path = h_file_path
        self._shader_file = open(input_file_path, 'r')
        self._cpp_file = open(cpp_file_path, 'w')
        self._h_file = open(h_file_path, 'w')

        self._vertex_shader_sources = []
        self._fragment_shader_sources = []
        self._definitions = []

    def close(self):
        if self._shader_file is not None:
            self._shader_file.close()
        if self._cpp_file is not None:
            self._cpp_file.close()
        if self._h_file is not None:
            self._h_file.close()

    def parse_shader_file_to_cpp(self):
        while True:
            line = self._shader_file.readline()
            if not line:
                break

            if line.startswith("#shader "):
                if line.startswith("#shader vertex"):
                    self._parsing_stage = ParsingStage.VERTEX_SHADER
                elif line.startswith("#shader fragment"):
                    self._parsing_stage = ParsingStage.FRAGMENT_SHADER
                else:
                    print("Unsupported shader type '%s'" % line)
                    exit(2)
                continue

            if self._parsing_stage == ParsingStage.VERTEX_SHADER:
                self._vertex_shader_sources.append(line)
            elif self._parsing_stage == ParsingStage.FRAGMENT_SHADER:
                self._fragment_shader_sources.append(line)

            stripped_line = line.strip()
            if stripped_line.startswith("#ifdef "):
                self._definitions.append(stripped_line.split()[1])

    def print(self):
        print(self._vertex_shader_sources)
        print(self._fragment_shader_sources)
        print(self._definitions)


def main():
    if len(sys.argv) < 3:
        print("Usage: %s input_file(shader) output_dir" % sys.argv[0])
        exit(1)

    print(sys.argv)
    input_file_path = sys.argv[1]
    output_dir_path = sys.argv[2]
    shader_file_name = os.path.split(input_file_path)[1]
    cpp_file_path = output_dir_path + '/' + shader_file_name + ".cpp"
    h_file_path = output_dir_path + '/' + shader_file_name + ".h"

    parse = ShaderParser(input_file_path, cpp_file_path, h_file_path)
    parse.parse_shader_file_to_cpp()
    parse.print()
    parse.close()


if __name__ == "__main__":
    main()
    exit(0)
