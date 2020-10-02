cc_binary(
    name = 'httpserver',
    srcs = [
        'testhttpserver.cc'
    ],
    deps = [
        '//chive/net/:chive_net',
        '//chive/base/:chive_base',
        '//chive/net/http/:chive_http',
        '#pthread'
    ],
    extra_cppflags = [
        '-std=c++17',
#        '-Wall',
        '-w',
         '-g',
    ]
)
