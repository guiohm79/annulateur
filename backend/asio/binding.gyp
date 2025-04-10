{
  "targets": [
    {
      "target_name": "asio_addon",
      "sources": [
        "<(module_root_dir)/asio_processor.cpp",
        "<(module_root_dir)/asiodrivers.cpp",
        "<(module_root_dir)/asiolist.cpp",
        "<(module_root_dir)/iasiodrv.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "<(module_root_dir)"   
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "defines": [
        "NAPI_DISABLE_CPP_EXCEPTIONS",
        "ASIO_INCLUDED" 
      ],
      "conditions": [
        ["OS=='win'", {
          "libraries": [
            "-lwinmm.lib",
            "-lole32.lib"
          ]
        }]
      ]
    }
  ]
}
