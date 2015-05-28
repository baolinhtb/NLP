#!/bin/bash
# To use “caffe”, you need to load this file using “source”.
PREFIX=/share/tools/miyazawa
export CAFFE_ROOT=$PREFIX/caffe
export PKG_CONFIG_PATH=$PREFIX/local/lib/pkgconfig:$PKG_CONFIG_PATH
export PYTHONPATH=$PREFIX/opencv/lib/python2.7/site-packages:$PYTHONPATH:$PREFIX/swig-srilm:$PREFIX/caffe/python:$PREFIX/selsearch:$PREFIX/coco-caption
export LD_LIBRARY_PATH=$PREFIX/opencv/lib:$PREFIX/srilm/lib/i686-m64:$PREFIX/local/OpenBLAS:$PREFIX/local/lib:$LD_LIBRARY_PATH
export JAVA_HOME=$PREFIX/jvm/default
export PATH=$PREFIX/local/bin:$PREFIX/anaconda/bin:$PREFIX/node.js/bin:$PREFIX/cmake/bin:$PREFIX/SWIG/bin:$PREFIX/srilm/bin:$PREFIX/swig:$JAVA_HOME/bin:$PREFIX/matlab/bin:$PATH
export PS1="${debian_chroot:+($debian_chroot)}(anaconda-caffe)\u@\h:\w\$ "
echo "The libraries for Caffe have been loaded."
