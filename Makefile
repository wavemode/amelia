build:
	NINJA_STATUS="[%f/%t | %r processes | %e s] " CMAKE_BUILD_PARALLEL_LEVEL=$(shell nproc) \
	cmake --build build/debug --target amelia

configure:
	cmake -DCMAKE_C_COMPILER=clang \
      -DCMAKE_CXX_COMPILER=clang++ \
      -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld" \
      -DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=lld" \
      -DCMAKE_MODULE_LINKER_FLAGS="-fuse-ld=lld" \
			-B build/debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug -G "Ninja"

release:
	NINJA_STATUS="[%f/%t | %r processes | %e s] " \
	cmake --build build/release --target amelia

configure-release:
	cmake -DCMAKE_C_COMPILER=clang \
      -DCMAKE_CXX_COMPILER=clang++ \
      -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld" \
      -DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=lld" \
      -DCMAKE_MODULE_LINKER_FLAGS="-fuse-ld=lld" \
			-B build/release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release -G "Ninja"

build-test:
	NINJA_STATUS="[%f/%t | %r processes | %e s] " CMAKE_BUILD_PARALLEL_LEVEL=$(shell nproc) \
	cmake --build build/debug --target amelia_test

test: build-test
	UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 ./build/debug/amelia_test $$AMELIA_TEST_ARGS

format:
	find src -iname '*.h' -print0 -o -iname '*.hpp' -print0 -o -iname '*.cpp' -print0 | xargs -0 clang-format -i
	find test -iname '*.h' -print0 -o -iname '*.hpp' -print0 -o -iname '*.cpp' -print0 | xargs -0 clang-format -i

clean:
	rm -rf build

.PHONY: build configure release configure-release build-test test format clean
