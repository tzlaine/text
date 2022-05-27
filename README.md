# text

### Motivation
C and C++ are the only major production languages that do not have robust Unicode support. This library attempts to provide that support for C++.

This library provides a Unicode implementation suitable for use by anyone — including those who know next to nothing about Unicode.

### Features
- Iterators, views, and algorithms that convert among UTF-8, UTF-16, and UTF-32.

- An implementation of the Unicode algorithms for dealing with encodings, normalization, text segmentation, etc.

- A family of types for use in Unicode-aware text processing. This includes text, text_view, rope, and rope_view.

- Code familiarity -- all of the above types and algorithms work like the STL containers and algorithms.

This library targets submission to Boost and eventual standardization.

It's more interesting than it sounds.

Online docs: https://tzlaine.github.io/text

[![Ubuntu-20.04 - GCC 9,10](https://github.com/tzlaine/text/actions/workflows/ubuntu-20.04.yml/badge.svg)](https://github.com/tzlaine/text/actions/workflows/ubuntu-20.04.yml) [![Ubuntu-22.04 - GCC 10,11](https://github.com/tzlaine/text/actions/workflows/ubuntu-22.04.yml/badge.svg)](https://github.com/tzlaine/text/actions/workflows/ubuntu-22.04.yml)

[![Windows-2019 - Visual Studio 2019](https://github.com/tzlaine/text/actions/workflows/windows-2019.yml/badge.svg)](https://github.com/tzlaine/text/actions/workflows/windows-2019.yml) [![Windows-2022 - Visual Studio 2022](https://github.com/tzlaine/text/actions/workflows/windows-2022.yml/badge.svg)](https://github.com/tzlaine/text/actions/workflows/windows-2022.yml)

[![License](https://img.shields.io/badge/license-boost-brightgreen.svg)](LICENSE_1_0.txt)
