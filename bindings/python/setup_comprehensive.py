from setuptools import setup, find_packages
from pybind11.setup_helpers import Pybind11Extension, build_ext
import os
import sys

# Get the absolute path to the build directory
build_dir = os.path.abspath("../../build/lib")
include_dir = os.path.abspath("../../include")
src_dir = os.path.abspath("../../src")

# Define the comprehensive extension module
ext_modules = [
    Pybind11Extension(
        "satox_bindings._core",
        ["src/satox_python_bindings.cpp"],
        include_dirs=[
            include_dir,
            src_dir,
            src_dir + "/core/include",
            src_dir + "/database/include", 
            src_dir + "/blockchain/include",
            src_dir + "/asset/include",
            src_dir + "/nft/include",
            src_dir + "/ipfs/include",
            src_dir + "/network/include",
            src_dir + "/security/include",
            src_dir + "/wallet/include",
            "/usr/local/include",  # For spdlog, fmt, etc.
        ],
        library_dirs=[build_dir, "/usr/local/lib"],
        libraries=[
            "satox-core",
            "satox-database",
            "satox-blockchain",
            "satox-asset",
            "satox-nft",
            "satox-ipfs",
            "satox-network",
            "satox-security",
            "satox-wallet",
            "spdlog",
            "fmt",
            "nlohmann_json",
            "ssl",
            "crypto",
            "curl",
            "pthread",
            "dl",
        ],
        extra_link_args=[
            f"-L{build_dir}",
            "-L/usr/local/lib",
            "-Wl,-rpath,/usr/local/lib",
        ],
        extra_compile_args=[
            "-std=c++17",
            "-O3",
            "-Wall",
            "-Wextra",
            "-fPIC",
        ],
        define_macros=[
            ("PYBIND11_DETAILED_ERROR_MESSAGES", None),
        ],
    ),
]

setup(
    name="satox_bindings",
    version="1.0.0",
    packages=find_packages(),
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    install_requires=[
        "pybind11>=2.10.0",
        "numpy>=1.20.0",
        "asyncio",
        "typing-extensions>=4.0.0",
    ],
    python_requires=">=3.8",
    author="Satoxcoin Core Developer",
    author_email="team@satox.com",
    description="Comprehensive Python bindings for Satox SDK",
    long_description=open("README.md").read() if os.path.exists("README.md") else "",
    long_description_content_type="text/markdown",
    url="https://github.com/satoverse/satox-sdk",
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
        "Topic :: Software Development :: Libraries :: Python Modules",
        "Topic :: System :: Distributed Computing",
        "Topic :: Database",
        "Topic :: Security :: Cryptography",
    ],
    keywords="blockchain, cryptocurrency, satox, sdk, python, bindings",
    project_urls={
        "Bug Reports": "https://github.com/satoverse/satox-sdk/issues",
        "Source": "https://github.com/satoverse/satox-sdk",
        "Documentation": "https://satoverse.io/docs",
    },
) 