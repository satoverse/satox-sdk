#!/usr/bin/env python
from setuptools import setup

setup(
    name="satox_sdk",
    version="0.1.0",
    description="Satox SDK Python Bindings",
    author="Satox Team",
    packages=["satox_sdk"],
    package_data={"satox_sdk": ["*.so"]},
    include_package_data=True,
    python_requires=">=3.8",
    install_requires=[
        'pybind11>=2.6.0',
    ],
) 