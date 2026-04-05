{
  "targets": [
    {
      "target_name": "obs_addon",
      "cflags!": ["-fno-exceptions"],
      "cflags_cc!": ["-fno-exceptions"],
      "cflags": ["-std=c++17", "-fPIC"],
      "cflags_cc": ["-std=c++17", "-fPIC"],
      "sources": [
        "src/obs_addon.cc"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "../core/include"
      ],
      "libraries": [
        "-L../core/build",
        "-lobs_core",
        "-lavcodec",
        "-lavformat",
        "-lavutil",
        "-lswscale",
        "-lswresample"
      ],
      "conditions": [
        ["OS=='win'", {
          "libraries": [
            "-l../core/build/Release/obs_core.lib",
            "-lC:/vcpkg/installed/x64-windows-rel/lib/avcodec.lib",
            "-lC:/vcpkg/installed/x64-windows-rel/lib/avformat.lib",
            "-lC:/vcpkg/installed/x64-windows-rel/lib/avutil.lib",
            "-lC:/vcpkg/installed/x64-windows-rel/lib/swscale.lib",
            "-lC:/vcpkg/installed/x64-windows-rel/lib/swresample.lib",
            "-lole32.lib",
            "-loleaut32.lib",
            "-lstrmiids.lib",
            "-lmfplat.lib",
            "-lmfuuid.lib"
          ],
          "msvs_settings": {
            "VCCLCompilerTool": {
              "ExceptionHandling": 1,
              "AdditionalOptions": ["/std:c++17", "/GR"]
            }
          },
          "defines": ["WIN32_LEAN_AND_MEAN", "NOMINMAX"]
        }],
        ["OS=='mac'", {
          "libraries": [
            "-lbsm",
            "-framework", "CoreFoundation",
            "-framework", "CoreVideo",
            "-framework", "AVFoundation"
          ],
          "xcode_settings": {
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
            "CLANG_CXX_LANGUAGE_STANDARD": "libc++",
            "MACOSX_DEPLOYMENT_TARGET": "10.15",
            "CLANG_CXX_LANGUAGE_STANDARD": "c++17"
          }
        }],
        ["OS=='linux'", {
          "libraries": [
            "-lpthread",
            "-ldl",
            "-lX11"
          ]
        }]
      ]
    }
  ]
}