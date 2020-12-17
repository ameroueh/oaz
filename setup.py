import multiprocessing
import os
import pathlib

from distutils.file_util import copy_file
from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext as build_ext_orig

# REQUIREMENTS = [
#     line
#     for line in pathlib.Path("requirements.txt")
#     .read_text()
#     .strip()
#     .split("\n")
# ]

CPU_COUNT = multiprocessing.cpu_count()

TARGETS = [
    {
        "name": "thread_pool",
        "target": "thread_pool",
        "extension_file_name": "thread_pool.so",
        "module_directory": "thread_pool",
    },
    {
        "name": "search",
        "target": "search",
        "extension_file_name": "search.so",
        "module_directory": "search",
    },
    {
        "name": "selection",
        "target": "selection",
        "extension_file_name": "selection.so",
        "module_directory": "selection",
    },
    {
        "name": "nn_evaluator",
        "target": "nn_evaluator",
        "extension_file_name": "nn_evaluator.so",
        "module_directory": "evaluator/nn_evaluator",
    },
    {
        "name": "evaluator",
        "target": "evaluator",
        "extension_file_name": "evaluator.so",
        "module_directory": "evaluator",
    },
    {
        "name": "simulation_evaluator",
        "target": "simulation_evaluator",
        "extension_file_name": "simulation_evaluator.so",
        "module_directory": "evaluator/simulation_evaluator",
    },
    {
        "name": "cache",
        "target": "cache",
        "extension_file_name": "cache.so",
        "module_directory": "cache",
    },
    {
        "name": "simple_cache",
        "target": "simple_cache",
        "extension_file_name": "simple_cache.so",
        "module_directory": "cache/simple_cache",
    },
    {
        "name": "game",
        "target": "game",
        "extension_file_name": "game.so",
        "module_directory": "games",
    },
    {
        "name": "connect_four",
        "target": "connect_four",
        "extension_file_name": "connect_four.so",
        "module_directory": "games/connect_four",
    },
    {
        "name": "tic_tac_toe",
        "target": "tic_tac_toe",
        "extension_file_name": "tic_tac_toe.so",
        "module_directory": "games/tic_tac_toe",
    },
    {
        "name": "bandits",
        "target": "bandits",
        "extension_file_name": "bandits.so",
        "module_directory": "games/bandits",
    },
]


class CMakeExtension(Extension):
    def __init__(self, name):
        super().__init__(name, sources=[])


class build_ext(build_ext_orig):
    def run(self):
        for ext in self.extensions:
            self.build_cmake(ext)
        super().run()

    def build_cmake(self, ext):
        cwd = pathlib.Path(__file__).parent.absolute()

        build_temp = cwd / self.build_temp
        build_lib = cwd / self.build_lib
        build_temp.mkdir(parents=True, exist_ok=True)
        extdir = cwd / self.get_ext_fullpath(ext.name)
        extdir.mkdir(parents=True, exist_ok=True)

        config = "Debug" if self.debug else "Release"

        os.chdir(str(build_temp))
        self.spawn(["cmake", str(cwd)])
        if not self.dry_run:
            self.spawn(
                ["cmake"]
                + ["--build", "."]
                + ["--config", config]
                + ["--parallel", str(CPU_COUNT)]
                + ["--target", "all_python"]
            )
            for target in TARGETS:
                copy_file(
                    src=target["extension_file_name"],
                    dst=cwd
                    / build_lib
                    / "pyoaz"
                    / target["module_directory"]
                    / target["extension_file_name"],
                )

        os.chdir(str(cwd))


setup(
    name="pyoaz",
    version="0.1",
    packages=find_packages(),
    ext_modules=[CMakeExtension("pyoaz")],
    # install_requires=REQUIREMENTS,
    dependency_links=[
        "git+https://www.github.com/keras-team/keras-contrib.git"
    ],
    cmdclass={"build_ext": build_ext},
)
