.DEFAULT_GOAL := all
SHELL         := bash

ifeq ($(shell uname -s), Darwin)
    ASTYLE        := astyle
    BOOST         := /usr/local/include/boost
    CHECKTESTDATA := checktestdata
    CPPCHECK      := cppcheck
    CXX           := clang++
    CXXFLAGS      := --coverage -g -std=c++20 -Wall -Wextra -Wpedantic
    DOXYGEN       := doxygen
    GCOV          := llvm-cov gcov
    GTEST         := /usr/local/include/gtest
    LDFLAGS       := -lgtest -lgtest_main
    LIB           := $(LIBRARY_PATH)
    VALGRIND      :=
else ifeq ($(shell uname -p), x86_64)
    ASTYLE        := astyle
    BOOST         := /lusr/opt/boost-1.82/include/boost
    CHECKTESTDATA := checktestdata
    CPPCHECK      := cppcheck
    CXX           := g++-11
    CXXFLAGS      := --coverage -g -std=c++20 -Wall -Wextra -Wpedantic
    DOXYGEN       := doxygen
    GCOV          := gcov-11
    GTEST         := /usr/include/gtest
    LDFLAGS       := -L/usr/local/opt/boost-1.77/lib/ -lgtest -lgtest_main -pthread
    LIB           := /usr/lib/x86_64-linux-gnu
    VALGRIND      := valgrind-3.17
else
    ASTYLE        := astyle
    BOOST         := /usr/include/boost
    CHECKTESTDATA := checktestdata
    CPPCHECK      := cppcheck
    CXX           := g++
    CXXFLAGS      := --coverage -g -std=c++20 -Wall -Wextra -Wpedantic
    DOXYGEN       := doxygen
    GCOV          := gcov
    GTEST         := /usr/include/gtest
    LDFLAGS       := -lgtest -lgtest_main -pthread
    LIB           := /usr/lib
    VALGRIND      := valgrind
endif

# run/test files, compile with make all
FILES :=               \
    run_Allocator  \
    test_Allocator

# run docker
docker:
	docker run --rm -it -v $(PWD):/usr/gcc -w /usr/gcc gpdowning/gcc

# get git config
config:
	git config -l

# get git log
Allocator.log.txt:
	git log > Allocator.log.txt

# get git status
status:
	make --no-print-directory clean
	@echo
	git branch
	git remote -v
	git status

# download files from the Allocator code repo
pull:
	make --no-print-directory clean
	@echo
	git pull
	git status

# upload files to the Allocator code repo
push:
	make --no-print-directory clean
	@echo
	git add .gitignore
	git add .gitlab-ci.yml
	-git add Allocator.csv
	-git add Allocator.ctd.txt
	git add Allocator.hpp
	-git add Allocator.log.txt
	-git add html
	git add Makefile
	git add README.md
	git add run_Allocator.cpp
	git add test_Allocator.cpp
	git commit -m "another commit"
	git push
	git status

# compile run harness
run_Allocator: Allocator.hpp run_Allocator.cpp
	-$(CPPCHECK) run_Allocator.cpp
	$(CXX) $(CXXFLAGS) run_Allocator.cpp -o run_Allocator

# compile test harness
test_Allocator: Allocator.hpp test_Allocator.cpp
	-$(CPPCHECK) test_Allocator.cpp
	$(CXX) $(CXXFLAGS) test_Allocator.cpp -o test_Allocator $(LDFLAGS)

# compile all
all: $(FILES)

# execute test harness with coverage
test: test_Allocator
	$(VALGRIND) ./test_Allocator
ifeq ($(shell uname -s), Darwin)
	$(GCOV) test_Allocator-test_Allocator.cpp | grep -B 2 "hpp.gcov"
else
	$(GCOV) test_Allocator.cpp | grep -B 2 "hpp.gcov"
endif

# clone the Allocator test repo
../cs371p-allocator-tests:
	git clone https://gitlab.com/gpdowning/cs371p-allocator-tests.git ../cs371p-allocator-tests

# generate a random input file
ctd-generate:
	$(CHECKTESTDATA) -g Allocator.ctd.txt >> Allocator.gen.txt

# execute the run harness against your test files in the Allocator test repo and diff with the expected output
# change gpdowning to your GitLab-ID
run: run_Allocator ../cs371p-allocator-tests
	-$(CHECKTESTDATA) Allocator.ctd.txt test.in.txt
	./run_Allocator < test.in.txt > Allocator.tmp.txt
	diff Allocator.tmp.txt test.out.txt

# execute the run harness against all of the test files in the Allocator test repo and diff with the expected output
run-all: run_Allocator ../cs371p-allocator-tests
	-@for v in `ls ../cs371p-allocator-tests/*.in.txt`;         \
    do                                                              \
        echo $(CHECKTESTDATA) Allocator.ctd.txt $${v};          \
             $(CHECKTESTDATA) Allocator.ctd.txt $${v};          \
        echo ./run_Allocator \< $${v} \> Allocator.tmp.txt; \
             ./run_Allocator  < $${v}  > Allocator.tmp.txt; \
        echo diff Allocator.tmp.txt $${v/.in/.out};             \
             diff Allocator.tmp.txt $${v/.in/.out};             \
    done

# auto format the code
format:
	$(ASTYLE) Allocator.hpp
	$(ASTYLE) run_Allocator.cpp
	$(ASTYLE) test_Allocator.cpp

# you must edit Doxyfile and
# set EXTRACT_ALL     to YES
# set EXTRACT_PRIVATE to YES
# set EXTRACT_STATIC  to YES
# create Doxfile
Doxyfile:
	$(DOXYGEN) -g

# create html directory
html: Doxyfile
	$(DOXYGEN) Doxyfile

# check the existence of check files
check: .gitignore .gitlab-ci.yml Allocator.log.txt html

# remove executables and temporary files
clean:
	rm -f  *.gcda
	rm -f  *.gcno
	rm -f  *.gcov
	rm -f  *.gen.txt
	rm -f  *.tmp.txt
	rm -f  $(FILES)
	rm -rf *.dSYM

# remove executables, temporary files, and generated files
scrub:
	make --no-print-directory clean
	rm -f  Allocator.log.txt
	rm -f  Doxyfile
	rm -rf html
	rm -rf latex

# output versions of all tools
versions:
	uname -p

	@echo
	uname -s

	@echo
	which $(ASTYLE)
	@echo
	$(ASTYLE) --version

	@echo
	which $(CHECKTESTDATA)
	@echo
	$(CHECKTESTDATA) --version | head -n 1

	@echo
	which cmake
	@echo
	cmake --version | head -n 1

	@echo
	which $(CPPCHECK)
	@echo
	$(CPPCHECK) --version

	@echo
	which $(DOXYGEN)
	@echo
	$(DOXYGEN) --version

	@echo
	which $(CXX)
	@echo
	$(CXX) --version | head -n 1

	@echo
	which $(GCOV)
	@echo
	$(GCOV) --version | head -n 1

	@echo
	which git
	@echo
	git --version

	@echo
	which make
	@echo
	make --version | head -n 1

ifneq ($(VALGRIND),)
	@echo
	which $(VALGRIND)
	@echo
	$(VALGRIND) --version
endif

	@echo
	which vim
	@echo
	vim --version | head -n 1

	@echo
	grep "#define BOOST_LIB_VERSION " $(BOOST)/version.hpp

	@echo
	ls -al $(GTEST)/gtest.h
	@echo
	pkg-config --modversion gtest
	@echo
	ls -al $(LIB)/libgtest*.a
