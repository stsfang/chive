cc_library(
    name = 'chive_base',
    srcs = [
        'CurrentThread.cc',
        'CountDownLatch.cc',
        'Thread.cc',
        'FileUtil.cc',
    ],
    deps = [
        '#pthread',
        '//chive/base/clog/:chive_log'
    ],
    extra_cppflags = [
        '-std=c++17',
        # '-Wall',  # show warning as error
        '-w',       # no warning
         '-g',
    ]
)

cc_library(
    name = 'chive_fileutil',
    srcs = [
        'FileUtil.cc'
    ],
    extra_cppflags = [
        '-std=c++17',
        # '-Wall',  # show warning as error
        '-w',       # no warning
         '-g',
    ]
)
