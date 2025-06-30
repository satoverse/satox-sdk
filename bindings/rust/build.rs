use std::env;
use std::path::PathBuf;

fn main() {
    // Tell cargo to look for shared libraries in the specified search path.
    println!("cargo:rustc-link-search=native={}", env::var("OUT_DIR").unwrap());
    
    // Tell cargo to tell rustc to link our C++ library.
    println!("cargo:rustc-link-lib=static=satox-core");
    println!("cargo:rustc-link-lib=static=satox-blockchain");
    println!("cargo:rustc-link-lib=static=satox-network");
    println!("cargo:rustc-link-lib=static=satox-security");
    println!("cargo:rustc-link-lib=static=satox-asset");
    println!("cargo:rustc-link-lib=static=satox-nft");
    println!("cargo:rustc-link-lib=static=satox-ipfs");
    println!("cargo:rustc-link-lib=static=keccak");
    
    // Link system libraries
    println!("cargo:rustc-link-lib=ssl");
    println!("cargo:rustc-link-lib=crypto");
    println!("cargo:rustc-link-lib=curl");
    println!("cargo:rustc-link-lib=boost_system");
    println!("cargo:rustc-link-lib=boost_filesystem");
    println!("cargo:rustc-link-lib=stdc++");
    
    // Only rerun this script if the C++ source changes
    println!("cargo:rerun-if-changed=../satox-core/src/");
    println!("cargo:rerun-if-changed=../satox-blockchain/src/");
    println!("cargo:rerun-if-changed=../satox-network/src/");
    println!("cargo:rerun-if-changed=../satox-security/src/");
    println!("cargo:rerun-if-changed=../satox-asset/src/");
    println!("cargo:rerun-if-changed=../satox-nft/src/");
    println!("cargo:rerun-if-changed=../satox-ipfs/src/");
    
    // Build the C++ SDK
    let mut config = cmake::Config::new("../..");
    config.define("CMAKE_BUILD_TYPE", "Release");
    config.define("BUILD_TESTS", "OFF");
    config.define("BUILD_PYTHON_BINDINGS", "OFF");
    
    let dst = config.build();
    
    // Add the build directory to the library search path
    println!("cargo:rustc-link-search=native={}/lib", dst.display());
} 