.POSIX:
.SUFFIXES:

NAME = ivy
TYPE = Debug
CMAKEOPTS = -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

CMAKE = cmake
NINJA = ninja
MKDIR = mkdir

BUILDDIR = Build

all: $(NAME)

.PHONY: $(NAME)
$(NAME): $(BUILDDIR)/build.ninja
	$(NINJA) -C$(BUILDDIR)

$(BUILDDIR)/build.ninja: $(BUILDDIR) Makefile
	$(CMAKE) -B$(BUILDDIR) -DCMAKE_BUILD_TYPE=$(TYPE) $(CMAKEOPTS) -GNinja .

$(BUILDDIR):
	$(MKDIR) $(BUILDDIR)

.PHONY: modules
modules:
	git submodule update --init --recursive

.PHONY: format
format:
	clang-format -i Source/*.h
	clang-format -i Source/*.c

.PHONY: test
test:
	ctest --verbose --test-dir Build/Tests


.PHONY: clean
clean:
	$(NINJA) -C$(BUILDDIR) clean
  
