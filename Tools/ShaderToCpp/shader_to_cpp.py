#!/usr/bin/env python3

import sys
import os
from enum import Enum


def underscore_to_camel(text, is_first_lower = False):
    ret = "".join(map(lambda s: s[0].upper() + s[1:], text.lower().split('_')))
    if is_first_lower:
        ret = ret[0].lower() + ret[1:]
    return ret

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
        self._definitions = set()

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
                self._definitions.add(stripped_line.split()[1])

    def generate(self):
        self._generate_cpp_file()
        self._generate_h_file()

    def _generate_h_file(self):
        output_file = self._h_file
        output_file.write("#include <string>\n")
        output_file.write("namespace Loong::Resource {\n")
        output_file.write("\n")
        output_file.write("class LoongRuntimeShaderCode {\n")
        output_file.write("public:\n")
        output_file.write("\tstd::string vertexShader;\n")
        output_file.write("\tstd::string fragmentShader;\n")
        output_file.write("};\n")
        output_file.write("\n")
        output_file.write("class LoongRuntimeShader {\n")
        output_file.write("public:\n")
        for definition in self._definitions:
            camel_def = underscore_to_camel(definition, False)
            low_camel_def = underscore_to_camel(definition, True)
            output_file.write("\tvoid Set{}(bool b) {{ {}_ = b; }}\n".format(camel_def, low_camel_def))
            output_file.write("\tbool Is{}() {{ return {}_; }}\n".format(camel_def, low_camel_def))
            output_file.write("\n")

        output_file.write("\tLoongRuntimeShaderCode GenerateShaderSources() const;\n")

        output_file.write("private:\n")
        for definition in self._definitions:
            low_camel_def = underscore_to_camel(definition, True)
            output_file.write("\tbool {}_ {{false}};\n".format(low_camel_def))

        output_file.write("};\n")
        output_file.write("\n")
        output_file.write("}\n")

        pass

    def _generate_cpp_file(self):
        output_file = self._cpp_file
        output_file.write('#include "LoongResource/LoongRuntimeShader.h"\n')
        output_file.write("\n")
        output_file.write("namespace Loong::Resource {\n")
        output_file.write("\n")
        output_file.write("LoongRuntimeShaderCode LoongRuntimeShader::GenerateShaderSources() const\n")
        output_file.write("{\n")
        output_file.write("\tLoongRuntimeShaderCode code {};\n")
        output_file.write('\tcode.vertexShader = R"({})";\n'.format("".join(self._vertex_shader_sources)))
        output_file.write("\treturn code;\n")
        output_file.write("}\n")
        output_file.write("\n")
        output_file.write("}\n")

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
    parse.generate()
    parse.print()
    parse.close()


if __name__ == "__main__":
    main()
    exit(0)
