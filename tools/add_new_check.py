#!/usr/bin/python3
import logging
import sys
import os
import argparse

BASE_DIR = os.path.dirname(os.path.abspath(__file__)) + '/'
logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    datefmt='%m-%d %H:%M')
logger = logging.getLogger(__name__)

def get_verbosity_level(level: int):
    level_map = \
        {
            0: logging.CRITICAL,
            1: logging.ERROR,
            2: logging.WARNING,
            3: logging.INFO,
            4: logging.DEBUG
        }
    if level not in level_map:
        level = 4
    return level_map[level]

def checker_name_to_pascal(s: str) -> str:
    return ''.join(word.capitalize() for word in s.split('-'))

def checker_name_to_snake(s: str) -> str:
    return s.replace('-', '_')

pch_template = '''#pragma once
#include "clang-tidy/ClangTidy.h"
#include "clang-tidy/ClangTidyCheck.h"
#include "clang-tidy/ClangTidyModule.h"
#include "clang-tidy/ClangTidyModuleRegistry.h"
#include "clang-tidy/utils/TransformerClangTidyCheck.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Lex/Lexer.h"
#include "clang/Tooling/Transformer/RewriteRule.h"
#include "clang/Tooling/Transformer/Stencil.h"
#include "clang/Tooling/Transformer/Transformer.h"
'''

stub_template = ''

def get_cmake_template(checker_name: str) -> str:
    return f'''include(${{CMAKE_CURRENT_LIST_DIR}}/../../cmake/clang_tidy.cmake)

add_library({checker_name_to_pascal(checker_name)} MODULE "")
target_compile_options({checker_name_to_pascal(checker_name)} PRIVATE -fno-rtti)
target_include_directories({checker_name_to_pascal(checker_name)}
    PRIVATE
        ${{CLANG_INCLUDE_DIRS}}
        ${{LLVM_INCLUDE_DIRS}}
)
target_precompile_headers({checker_name_to_pascal(checker_name)} PRIVATE pch.hpp)
target_sources({checker_name_to_pascal(checker_name)}
    PRIVATE
        ${{CMAKE_CURRENT_LIST_DIR}}/{checker_name_to_snake(checker_name)}.cpp
)

create_clang_tidy_test({checker_name_to_pascal(checker_name)}
    {checker_name}
    ${{CMAKE_CURRENT_LIST_DIR}}/stub_{checker_name_to_snake(checker_name)}.cpp
)
'''

def get_checker_transformer_template(checker_name: str) -> str:
    class_name = checker_name_to_pascal(checker_name)
    return f'''#include "pch.hpp"

using namespace clang;
using namespace clang::tidy;
using namespace clang::ast_matchers;
using namespace clang::transformer;
using namespace clang::tidy::utils;

namespace {{

class {class_name} : public TransformerClangTidyCheck {{
    static auto rule() {{
        // clang-format off
        auto matcher = traverse(TraversalKind::TK_IgnoreUnlessSpelledInSource,
            cxxMemberCallExpr()
        ).bind("bind_whole_expr");
        // clang-format on

        return makeRule(matcher,
            {{change(node("bind_whole_expr"), cat("SampleReplacement"))}},
            cat("Describe what has changed")
        );
    }}

public:
  {class_name}(StringRef Name, ClangTidyContext *Context)
      : TransformerClangTidyCheck(rule(), Name, Context) {{}}
}};

class {class_name}Module : public ClangTidyModule {{
public:
  void addCheckFactories(ClangTidyCheckFactories &Factories) override {{
    Factories.registerCheck<{class_name}>("{checker_name}");
  }}
}};

}} // end anonymous namespace

// Register the module using this statically initialized variable
static ClangTidyModuleRegistry::Add<{class_name}Module>
    X("{checker_name}", "Adds the new replacer check.");
'''

def create_file(file: str, content: str):
    with open(file, 'w') as f:
        f.write(content)

def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description="Sample script")
    parser.add_argument('--verbose', type=int, default = 3, help="Verbosity level (0=CRITICAL .. 4=DEBUG)")
    parser.add_argument('--name', help='the name of the new check')
    parser.add_argument('--simple', help='whether to use clang transformer or more complex template', action='store_true')
    args = parser.parse_args(argv)
    logger.setLevel(get_verbosity_level(args.verbose))
    
    checker_dir = f"{BASE_DIR}/../src/{checker_name_to_snake(args.name)}"
    main_cpp_file = f"{checker_dir}/{checker_name_to_snake(args.name)}.cpp"
    stub_cpp_file = f"{checker_dir}/stub_{checker_name_to_snake(args.name)}.cpp"
    pch_file = f"{checker_dir}/pch.hpp"
    cmake_file = f"{checker_dir}/CMakeLists.txt"
    os.makedirs(checker_dir)
    create_file(main_cpp_file, get_checker_transformer_template(args.name))
    create_file(stub_cpp_file, "// Test content file")
    create_file(pch_file, pch_template)
    create_file(cmake_file, get_cmake_template(args.name))
    with open(f"{BASE_DIR}/../src/CMakeLists.txt", 'a') as f:
        f.write(f"add_subdirectory({checker_name_to_snake(args.name)})\n")
    return 0

if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))