import setuptools

setuptools.setup(
    name='addie',
    version='0.0.0',
    packages=setuptools.find_packages(),
    install_requires=[
        'spidev',
        'Flask',
    ],
    scripts=['read_monitors.py']
)
