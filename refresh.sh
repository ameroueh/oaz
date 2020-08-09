#! /usr/bin/bash
rm -rf build
python setup.py bdist_wheel 
pushd .
cd dist
pip install pyoaz-0.1-cp36-cp36m-linux_x86_64.whl --force-reinstall
popd
