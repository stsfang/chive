cc_binary(
    name = 'test01',
    srcs = [
        'test01.cc',
    ],
    deps = [
        '//chive/net/:chive_net',
        '//chive/base/:chive_base',
        '#pthread',
    ],
    extra_cppflags = [
        '-std=c++14',
        #'-Wall',  # show warning as error
        '-w',       # no warning
        '-g',
    ]   
)