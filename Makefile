build:
	cmake --build build --target amelia

configure:
	cmake -B build

build-test:
	cmake --build build --target amelia_test

test: build-test
	./build/amelia_test $(ARGS)

format:
	find src -path src/vendor -prune -o -iname '*.h' -o -iname '*.cpp' -print | xargs clang-format -i

.PHONY: format configure build test build-test
