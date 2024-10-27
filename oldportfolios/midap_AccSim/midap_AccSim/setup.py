from setuptools import setup, find_packages

setup(name='MIDAPSim',

      version='1.4.0',

      url='https://caplab.snu.ac.kr/MIDAP/AccSim',

      license='MIT',

      author='Donghyun Kang, Duseok Kang',

      author_email='kangdongh@snu.ac.kr, kangds0829@snu.ac.kr',

      description='MIDAP Virtual Prototyping Environment for End-to-End network acceleration',

      packages=find_packages(),

      long_description=open('README.md').read(),

      zip_safe=False,

      setup_requires=['future >= 0.16.0', 'Pillow >= 5.2.0',
                      'numpy >= 1.15.1', 'torch >= 1.0.0', 'scikit-image >= 0.14.3'],

      test_suite=None)
