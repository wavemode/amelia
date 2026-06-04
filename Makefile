build:
	NINJA_STATUS="[%f/%t | %r processes | %e s] " cmake --build build/debug --target amelia --parallel

configure:
	cmake -B build/debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug -G "Ninja"

release:
	NINJA_STATUS="[%f/%t | %r processes | %e s] " cmake --build build/release --target amelia --parallel

configure-release:
	cmake -B build/release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release -G "Ninja"

build-test:
	NINJA_STATUS="[%f/%t | %r processes | %e s] " cmake --build build/debug --target amelia_test --parallel

test: build-test
	UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 ./build/debug/amelia_test $$AMELIA_TEST_ARGS

format:
	find src -path src/vendor -prune -o -iname '*.h' -print0 -o -iname '*.hpp' -print0 -o -iname '*.cpp' -print0 | xargs -0 clang-format -i

clean:
	rm -rf build

.PHONY: build configure release configure-release build-test test format clean
