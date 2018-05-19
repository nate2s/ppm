##
## This file is part of ppm, a pretty printer for math
## Copyright (C) 2018 Nate Smith (nat2e.smith@gmail.com)
##
## ppm is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.
##

ACLOCAL_AMFLAGS = -I m4

TAFFY_LIBRARY=library/Taffy-2.71/project/src

AM_CPPFLAGS = -Wall \
              -Wshadow \
              -Wsign-compare \
              -D_GNU_SOURCE \
              --std=c++11 \
              -DINSTALL_DIR='$(prefix)' \
              -I$(TAFFY_LIBRARY) \
              -I$(TAFFY_LIBRARY)/class \
              -I$(TAFFY_LIBRARY)/class/special \
              -I$(TAFFY_LIBRARY)/class/org/taffy/core/exception \
              -I$(TAFFY_LIBRARY)/container \
              -I$(TAFFY_LIBRARY)/evaluator \
              -I$(TAFFY_LIBRARY)/global \
              -I$(TAFFY_LIBRARY)/global/text \
              -I$(TAFFY_LIBRARY)/graph \
              -I$(TAFFY_LIBRARY)/io \
              -I$(TAFFY_LIBRARY)/maths \
              -I$(TAFFY_LIBRARY)/method \
              -I$(TAFFY_LIBRARY)/scope \
              -I$(TAFFY_LIBRARY)/taffy \
              -I$(TAFFY_LIBRARY)/pie \
              -I$(TAFFY_LIBRARY)/pie/text \
              -I$(TAFFY_LIBRARY)/taffy/text \
              -I$(TAFFY_LIBRARY)/third_party/decNumber \
              -I$(TAFFY_LIBRARY)/org/taffy/help \
              -I$(TAFFY_LIBRARY)/org/taffy/help/warranty

VERSION=1

MY_FLAGS=-version-info ${VERSION}

THE_FILES = Font.cpp \
	        Lattice.cpp \
	        Renderer.cpp \
	        TaffyBridge.cpp \
            PPMApp.cpp \
            FontFactory.cpp \
            CommandLineArguments.cpp \
            Color.cpp

libppm_la_SOURCES = ${THE_FILES}
libppm_la_LDFLAGS = ${MY_FLAGS} -ltaffy

# main executable
ppm_SOURCES = main.cpp
ppm_LDADD = libppm.la libtaffy.la

liblatticeTest_la_SOURCES = Lattice.cpp Color.cpp
liblatticeTest_la_LDFLAGS = ${MY_FLAGS}

latticeTest_SOURCES = latticeTest.cpp
latticeTest_LDADD = liblatticeTest.la

data_doc_dir = $(pkgdatadir)/ppm

data_doc__DATA = fonts/banner.flf \
                 fonts/big.flf \
                 fonts/small.flf

lib_LTLIBRARIES = libppm.la liblatticeTest.la
bin_PROGRAMS = ppm
check_PROGRAMS = latticeTest

everything: ppm
