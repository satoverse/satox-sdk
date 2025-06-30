#!/usr/bin/env python3
"""
Satox SDK Python Bindings Setup
Copyright (c) 2025 Satoxcoin Core Developers
MIT License
"""

from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext
import sys
import os
import subprocess
import platform
import pybind11

# Get the current date for dynamic versioning
import datetime
current_date = datetime.datetime.now().strftime("%Y.%m.%d")

# Get the directory containing this setup.py
here = os.path.abspath(os.path.dirname(__file__))

# Read the README file
with open(os.path.join(here, 'README.md'), encoding='utf-8') as f:
    long_description = f.read()

class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)

class CMakeBuild(build_ext):
    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
        
        # Required for auto-detection of auxiliary "native" libs
        if not extdir.endswith(os.path.sep):
            extdir += os.path.sep

        cmake_args = [
            '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + extdir,
            '-DPYTHON_EXECUTABLE=' + sys.executable,
            '-DCMAKE_BUILD_TYPE=Release',
            '-DSATOX_SDK_ROOT=' + os.path.join(here, '..', '..'),
            '-DPYBIND11_PYTHON_VERSION=' + str(sys.version_info[0]) + '.' + str(sys.version_info[1])
        ]

        # Add platform-specific arguments
        if sys.platform == "win32":
            cmake_args += ['-G', 'Visual Studio 16 2019']
        else:
            cmake_args += ['-G', 'Unix Makefiles']

        # Build the extension
        build_temp = os.path.join(self.build_temp, ext.name)
        if not os.path.exists(build_temp):
            os.makedirs(build_temp)

        subprocess.check_call(['cmake', ext.sourcedir] + cmake_args, cwd=build_temp)
        subprocess.check_call(['cmake', '--build', '.', '--config', 'Release'], cwd=build_temp)

setup(
    name='satox-sdk',
    version=current_date,
    author='Satoxcoin Core Developers',
    author_email='dev@satoverse.io',
    description='Python bindings for the Satox SDK',
    long_description=long_description,
    long_description_content_type='text/markdown',
    url='https://github.com/satoverse/satox-sdk',
    project_urls={
        'Documentation': 'https://docs.satoverse.io',
        'Source': 'https://github.com/satoverse/satox-sdk',
        'Tracker': 'https://github.com/satoverse/satox-sdk/issues',
    },
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: MIT License',
        'Operating System :: OS Independent',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Programming Language :: Python :: 3.10',
        'Programming Language :: Python :: 3.11',
        'Programming Language :: Python :: 3.12',
        'Topic :: Software Development :: Libraries :: Python Modules',
        'Topic :: System :: Distributed Computing',
        'Topic :: Security :: Cryptography',
    ],
    keywords='blockchain, cryptocurrency, wallet, nft, ipfs, quantum-resistant',
    packages=find_packages(),
    python_requires='>=3.8',
    install_requires=[
        'pybind11>=2.10.0',
        'numpy>=1.21.0',
        'requests>=2.25.0',
        'cryptography>=3.4.0',
    ],
    extras_require={
        'dev': [
            'pytest>=6.0.0',
            'pytest-cov>=2.10.0',
            'black>=21.0.0',
            'flake8>=3.8.0',
            'mypy>=0.800',
            'sphinx>=4.0.0',
            'sphinx-rtd-theme>=0.5.0',
        ],
        'examples': [
            'jupyter>=1.0.0',
            'matplotlib>=3.3.0',
            'pandas>=1.3.0',
        ],
    },
    ext_modules=[CMakeExtension('satox_sdk._satox_sdk', sourcedir='src')],
    cmdclass={'build_ext': CMakeBuild},
    zip_safe=False,
    include_package_data=True,
    package_data={
        'satox_sdk': ['*.pyi', 'py.typed'],
    },
    entry_points={
        'console_scripts': [
            'satox-sdk-cli=satox_sdk.cli:main',
        ],
    },
)
