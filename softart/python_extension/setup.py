#!/usr/bin/env python

from distutils.core import setup

setup(name='Signature',
      version='0.1',
      description='Get the md5 of files and compare with .md5 file',
      author='Ye Wu',
      author_email='wuye9036@gmail.com',
      url='http://softart.sourceforge.net/',
      packages=['softart'],
      py_modules=['softart.signature']
     )
