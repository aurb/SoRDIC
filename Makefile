# Software Rendering Demo Engine In C
# Copyright (C) 2024 Andrzej Urbaniak
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

INC := inc
SRC := src
OBJ := obj
CLANG := clang
ENGINE := engine
EXAMPLES := examples
ANALYSES := analyses
ENGINE_SRC := $(wildcard $(ENGINE)/$(SRC)/*.c)
ENGINE_OBJ := $(ENGINE_SRC:$(ENGINE)/$(SRC)/%.c=$(ENGINE)/$(OBJ)/%.o)
ENGINE_BC := $(ENGINE_SRC:$(ENGINE)/$(SRC)/%.c=$(ENGINE)/$(CLANG)/%.bc)
ENGINE_AST := $(ENGINE_SRC:$(ENGINE)/$(SRC)/%.c=$(ENGINE)/$(CLANG)/%.ast)
EXAMPLES_SRC := $(wildcard $(EXAMPLES)/$(SRC)/*.c)
EXAMPLES_BIN := $(EXAMPLES_SRC:$(EXAMPLES)/$(SRC)/%.c=$(EXAMPLES)/%)
EXAMPLES_BC := $(EXAMPLES_SRC:$(EXAMPLES)/$(SRC)/%.c=$(EXAMPLES)/$(CLANG)/%.bc)
EXAMPLES_AST := $(EXAMPLES_SRC:$(EXAMPLES)/$(SRC)/%.c=$(EXAMPLES)/$(CLANG)/%.ast)

CC = clang
# Build examples to render on window with dimensions (DISPLAY_W, DISPLAY_H)
CUSTOM_FLAGS := -DDISPLAY_W=1200 -DDISPLAY_H=900
# Build examples to render on full screen with current(desktop) resolution
#CUSTOM_FLAGS := -DFULL_DESKTOP=1
# Build engine to log music annotations
#CUSTOM_FLAGS += -DLOG_ANNOTATIONS
# Build examples for dynamic analysis (each example exits after rendering single frame)
#CUSTOM_FLAGS += -DRUN_ONE_FRAME

CFLAGS := -std=c99 -I$(ENGINE)/$(INC) $(CUSTOM_FLAGS) -Wall -Wformat -Werror=format-security #Universal compilation flags
DEBUG_FLAGS := -O0 -g
RELEASE_FLAGS := -O3 -D_FORTIFY_SOURCE=2 -fstack-protector-strong -DNDEBUG -march=haswell
LDFLAGS := 
LDLIBS := -lm -lSDL2 -lSDL2main -lSDL2_image -lSDL2_mixer
.PHONY: clean dirs release debug run scan-build llvm-build ast-build database ctu-index all
all: dirs clean release
release:: CFLAGS += $(RELEASE_FLAGS)
release::
	@echo "Building RELEASE configuration"
release:: $(EXAMPLES_BIN)
	strip --strip-all $^
debug:: CFLAGS += $(DEBUG_FLAGS) #-fsanitize=address -static-libasan ???
debug:: LDFLAGS += #-fsanitize=address
debug::
	@echo "Building DEBUG configuration"
debug:: $(EXAMPLES_BIN)
dirs:
	mkdir -p $(ENGINE)/$(OBJ)
	mkdir -p $(ENGINE)/$(CLANG)
	mkdir -p $(EXAMPLES)/$(CLANG)
	mkdir -p $(ANALYSES)
clean:
	rm -f $(ENGINE_OBJ) $(EXAMPLES_BIN) $(EXAMPLES_BC) $(ENGINE_BC) $(EXAMPLES_AST) $(ENGINE_AST) compile_commands.json externalDefMap.txt *.plist strace.* ltrace.* sltrace.*
# Run single target defined in variable bin.
# Usage example:
#     make run BIN=cube
run: $(EXAMPLES_BIN)
	$(EXAMPLES)/$(BIN)
# Run all targets
run-all: $(EXAMPLES_BIN)
	@for example in $(EXAMPLES_BIN); do ./$$example; done
# Generate strace and filter out hexadecinal numbers for given example.
# Usage example:
#     make strace BIN=cube
strace:: CFLAGS += $(DEBUG_FLAGS) -DRUN_ONE_FRAME=1
strace:: $(EXAMPLES_BIN)
	strace $(EXAMPLES)/$(BIN) 2>&1 1>/dev/null | grep -v -E 'getpid|Timeout|sched_yield' | sed 's/0x[0-9a-fA-F]\{8,\}/X/g' > strace.$(BIN).out
# Generate ltrace and filter out hexadecinal numbers for given example.
# Usage example:
#     make ltrace BIN=cube
ltrace:: CFLAGS += $(DEBUG_FLAGS) -DRUN_ONE_FRAME=1
ltrace:: $(EXAMPLES_BIN)
	ltrace $(EXAMPLES)/$(BIN) 2>&1 1>/dev/null | grep -v -E 'sin|cos|fmod|sqrt|pow' | sed 's/0x[0-9a-fA-F]\{8,\}/X/g' > ltrace.$(BIN).out
# Generate combined system and library traces and filter out hexadecinal numbers for given example.
# Usage example:
#     make sltrace BIN=cube
sltrace:: CFLAGS += $(DEBUG_FLAGS) -DRUN_ONE_FRAME=1
sltrace:: $(EXAMPLES_BIN)
	ltrace -S $(EXAMPLES)/$(BIN) 2>&1 1>/dev/null | grep -v -E 'SYS_getpid|SYS_poll|sin|cos|fmod|sqrt|pow' | sed 's/0x[0-9a-fA-F]\{8,\}/X/g' > sltrace.$(BIN).out

#llvm-build - Intermediate Representation target. Used for static analysis only
llvm-build:: CFLAGS += $(DEBUG_FLAGS) -emit-llvm 
llvm-build:: $(EXAMPLES_BC) $(ENGINE_BC)
#ast-build - Abstract Syntax Tree target. Hierarchical structure of the program for the index generation.
ast-build:: CFLAGS += $(DEBUG_FLAGS) -emit-ast
ast-build:: $(EXAMPLES_AST) $(ENGINE_AST)
database:
	bear -- make llvm-build
ctu-index: ast-build
	clang-extdef-mapping-14 -p . $(ENGINE_SRC) >> externalDefMap.txt
	sed -i -e "s|\.c|.ast|g" externalDefMap.txt
	sed -i -e "s|$$(pwd)/||g" externalDefMap.txt
	sed -i -e "s|$(SRC)|$(CLANG)|g" externalDefMap.txt
#ctu - Cross Translation Unit static analysis
ctu:: database ctu-index
ctu:: CFLAGS += -O0 -g3 -ggdb
ctu:: $(ENGINE_SRC) $(EXAMPLES_SRC)
	clang --analyze $(CFLAGS) \
	-Xclang -analyzer-config -Xclang experimental-enable-naive-ctu-analysis=true \
	-Xclang -analyzer-config -Xclang ctu-dir=. \
	-Xclang -analyzer-output=plist-multi-file $^
#stu - Single Translation Unit static analysis
stu:
	scan-build -o $(ANALYSES) --status-bugs -v --view make llvm_build

$(EXAMPLES_BIN): $(EXAMPLES)/%: $(EXAMPLES)/$(SRC)/%.c $(ENGINE_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LDLIBS)
$(ENGINE)/$(OBJ)/%.o: $(ENGINE)/$(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(EXAMPLES_BC): $(EXAMPLES)/$(CLANG)/%.bc: $(EXAMPLES)/$(SRC)/%.c
	clang $(CFLAGS) -c $< -o $@
$(ENGINE_BC): $(ENGINE)/$(CLANG)/%.bc: $(ENGINE)/$(SRC)/%.c
	clang $(CFLAGS) -c $< -o $@

$(EXAMPLES_AST): $(EXAMPLES)/$(CLANG)/%.ast: $(EXAMPLES)/$(SRC)/%.c
	clang $(CFLAGS) -c $< -o $@
$(ENGINE_AST): $(ENGINE)/$(CLANG)/%.ast: $(ENGINE)/$(SRC)/%.c
	clang $(CFLAGS) -c $< -o $@

