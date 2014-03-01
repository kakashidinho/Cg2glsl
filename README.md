Nvidia Cg to GLSL shader language translator
========

DX9 style Cg in, GLSL / GLSL ES out.

A modification of development from [aras-p's HLSL2GLSL](https://github.com/aras-p/hlsl2glslfork). I'm changing it to make it support Cg additional syntax, such as profile specific functions, and non-square matrices, etc.

Changes from original aras-p's HLSL2GLSL (October 2012 version)
--------

* Translate non-square matrices to array of vectors, so that it can be used in GLSL 1.10 and GLSL ES 1.00
* Support Cg profile versions of functions.


Notes from aras-p
--------

* Only Direct3D 9 style HLSL is supported. No Direct3D 10/11 "template like" syntax, no geometry/tesselation/compute shaders, no abstract interfaces.
* I bumped into some issues of HLSL2GLSL's preprocessor that I am not fixing. Most issues were with token pasting operator. So I preprocess source using [mojoshader's](http://icculus.org/mojoshader/) preprocessor. Grab latest from [mojoshader hg](http://hg.icculus.org/icculus/mojoshader/), it's awesome!
* On Windows, the library is built with `_HAS_ITERATOR_DEBUGGING=0,_SECURE_SCL=0` defines, which affect MSVC's STL behavior. If this does not match defines in your application, _totally strange_ things can start to happen!
* The library is not currently thread-safe.

* No optimizations are performed on the generated GLSL, so it is expected that your platform will have a decent GLSL compiler. Or, use [GLSL Optimizer](http://github.com/aras-p/glsl-optimizer), at Unity we use it to optimize shaders produced by HLSL2GLSL; gives a substantial performance boost on mobile platforms.
