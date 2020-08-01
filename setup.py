import os
import pathlib

from distutils.file_util import copy_file
from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext as build_ext_orig


GAMES = [
    {
        "name": "connect_four",
        "target": "pyoaz_connect_four_core",
        "extension_file_name": "pyoaz_connect_four_core.so",
    },
    {
        "name": "tic_tac_toe",
        "target": "pyoaz_tic_tac_toe_core",
        "extension_file_name": "pyoaz_tic_tac_toe_core.so",
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
        # print("sfsadfd", pathlib.Path(".").resolve())
        # print(
        #     "sfsadfd",
        #     pathlib.Path(".")
        #     .resolve()
        #     .relative_to("/home/simon/code/notebooks"),
        # )
        build_temp = cwd / self.build_temp
        build_lib = cwd / self.build_lib
        build_temp.mkdir(parents=True, exist_ok=True)
        extdir = cwd / self.get_ext_fullpath(ext.name)
        extdir.mkdir(parents=True, exist_ok=True)

        config = "Debug" if self.debug else "Release"

        os.chdir(str(build_temp))
        for game in GAMES:

            self.spawn(["cmake", str(cwd)])
            if not self.dry_run:
                self.spawn(
                    ["cmake"] + ["--build", ".", "--target", game["target"]]
                )
                copy_file(
                    src=game["extension_file_name"],
                    dst=cwd
                    / build_lib
                    / "pyoaz"
                    / "games"
                    / game["name"]
                    / game["extension_file_name"],
                )

        os.chdir(str(cwd))


setup(
    name="pyoaz",
    version="0.1",
    packages=find_packages(),
    ext_modules=[CMakeExtension("pyoaz")],
    cmdclass={"build_ext": build_ext},
)
