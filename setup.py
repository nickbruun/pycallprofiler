from distutils.core import setup, Extension


setup(
    name='pycallprofiler',
    version='1.0.0',
    description='Python 3.6+ deterministic profiler, that maintains full call stacks for detailed analysis.',
    author='Nick Bruun',
    author_email='nick@bruun.co',
    license=open('LICENSE').read(),
    url='https://github.com/nickbruun/pycallprofiler',
    packages=['callprofiler'],
    package_data={'': ['LICENSE']},
    package_dir={'callprofiler': 'callprofiler'},
    ext_modules=[
        Extension(
            'callprofilerext',
            [
                'callprofilerext/module.c',
                'callprofilerext/profiler.c',
                'callprofilerext/description.c',
                'callprofilerext/profiler_frame.c',
            ],
            extra_compile_args=['-std=c99', '-O3'],
            depends=[
                'callprofilerext/profiler.h',
                'callprofilerext/description.h',
                'callprofilerext/profiler_frame.h',
            ]
        ),
    ],
    classifiers=(
        'Intended Audience :: Developers',
        'Natural Language :: English',
        'License :: OSI Approved :: Apache Software License',
        'Programming Language :: Python',
        'Programming Language :: Python :: 3.6',
    )
)
