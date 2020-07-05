import os
import pathlib

from distutils.file_util import copy_file
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext as build_ext_orig

class CMakeExtension(Extension):
    def __init__(self, name):
        super().__init__(name, sources=[])

class build_ext(build_ext_orig):
    def run(self):
        for ext in self.extensions:
            self.build_cmake(ext)
        super().run()

    def build_cmake(self, ext):
        print('debug')
        print(print(', '.join("%s: %s" % item for item in vars(self).items())))
        cwd = pathlib.Path().absolute()

        build_temp = pathlib.Path(self.build_temp)
        build_lib = pathlib.Path(self.build_lib)
        build_temp.mkdir(parents=True, exist_ok=True)
        extdir = pathlib.Path(self.get_ext_fullpath(ext.name))
        extdir.mkdir(parents=True, exist_ok=True)

        config = 'Debug' if self.debug else 'Release'

        cmake_args = [
            '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + str(extdir.parent.absolute()),
            '-DCMAKE_BUILD_TYPE=' + config
        ]

        build_args = [
            '--target', 'az_connect_four'
        ]

        os.chdir(str(build_temp))
        self.spawn(['cmake', str(cwd)])
        if not self.dry_run:
            self.spawn(['cmake', '--build', '.'] + build_args)
            copy_file(
                src='az_connect_four.so', 
                dst=cwd / build_lib / 'az_connect_four' / 'az_connect_four.so'
            )
        os.chdir(str(cwd))

setup(
    name='az_connect_four',
    version='0.1',
    packages=['az_connect_four'],
    ext_modules=[CMakeExtension('az_connect_four')],
    cmdclass={
        'build_ext': build_ext
    }
)
