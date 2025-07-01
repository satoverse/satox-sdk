{
  "targets": [
    {
      "target_name": "satox-sdk",
      "sources": [
        "src/binding.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('nan').include\")",
        "../../include",
        "../../src"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "xcode_settings": {
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
        "CLANG_CXX_LIBRARY": "libc++",
        "MACOSX_DEPLOYMENT_TARGET": "10.7"
      },
      "msvs_settings": {
        "VCCLCompilerTool": { "ExceptionHandling": 1 }
      }
    }
  ]
} 