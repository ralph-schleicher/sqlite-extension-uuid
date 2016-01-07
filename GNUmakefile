## GNUmakefile --- make file for GNU Make and GCC.

# Copyright (C) 2015 Ralph Schleicher

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#    * Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#
#    * Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in
#      the documentation and/or other materials provided with the
#      distribution.
#
#    * Neither the name of the copyright holder nor the names of its
#      contributors may be used to endorse or promote products derived
#      from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

## Code:

GCC = gcc -std=c99 -Wall -W -Wno-unused-parameter -O2 -s

ifeq ($(OS),Windows_NT)
EXEEXT = .exe
SHREXT = .dll
LIBS = -lole32
else # not Windows_NT
ifeq ($(shell uname),Linux)
GCC += -fPIC
EXEEXT =
SHREXT = .so
LIBS = -luuid
else # not Linux
endif # not Linux
endif # not Windows_NT

.PHONY: all
all: sqlite3ext_uuid$(SHREXT)

sqlite3ext_uuid$(SHREXT): sqlite3ext_uuid.c sqlite3ext.h
	$(GCC) -shared -o $@ sqlite3ext_uuid.c $(LIBS)

.PHONY: clean
clean:
	rm -f sqlite3ext_uuid$(SHREXT)

.PHONY: check
check:
	sqlite3 -batch < uuid.sql

## GNUmakefile ends here
