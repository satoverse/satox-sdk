{
  "targets": [
    {
      "target_name": "satox_sdk_native",
      "sources": [
        "src/satox_sdk_native.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "../../include",
        "../../src",
        "../../src/core/include",
        "../../src/database/include",
        "../../src/security/include",
        "../../src/wallet/include",
        "../../src/asset/include",
        "../../src/nft/include",
        "../../src/blockchain/include",
        "../../src/ipfs/include",
        "../../src/network/include"
      ],
      "libraries": [
        "-L../../build_debug/src/core",
        "-L../../build_debug/src/database",
        "-L../../build_debug/src/security",
        "-L../../build_debug/src/wallet",
        "-L../../build_debug/src/asset",
        "-L../../build_debug/src/nft",
        "-L../../build_debug/src/blockchain",
        "-L../../build_debug/src/ipfs",
        "-L../../build_debug/src/network"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ],
      "xcode_settings": {
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
        "CLANG_CXX_LIBRARY": "libc++",
        "MACOSX_DEPLOYMENT_TARGET": "10.15"
      },
      "msvs_settings": {
        "VCCLCompilerTool": {
          "ExceptionHandling": 1
        }
      },
      "conditions": [
        ["OS=='win'", {
          "libraries": [
            "-lws2_32.lib",
            "-liphlpapi.lib"
          ]
        }],
        ["OS=='linux'", {
          "libraries": [
            "-lsatox-core",
            "-lsatox-database",
            "-lsatox-security",
            "-lsatox-wallet",
            "-lsatox-asset",
            "-lsatox-nft",
            "-lsatox-blockchain",
            "-lsatox-ipfs",
            "-lsatox-network",
            "-lssl",
            "-lcrypto",
            "-lpthread"
          ]
        }],
        ["OS=='mac'", {
          "libraries": [
            "-lsatox-core",
            "-lsatox-database",
            "-lsatox-security",
            "-lsatox-wallet",
            "-lsatox-asset",
            "-lsatox-nft",
            "-lsatox-blockchain",
            "-lsatox-ipfs",
            "-lsatox-network",
            "-framework Security",
            "-framework CoreFoundation"
          ]
        }]
      ]
    }
  ]
} 