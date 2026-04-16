build:
	cmake --build build --target amelia --parallel

configure:
	cmake -B build

build-test:
	cmake --build build --target amelia_test --parallel

test: build-test
	./build/amelia_test $(ARGS)

format:
	find src -path src/vendor -prune -o -iname '*.h' -print0 -o -iname '*.cpp' -print0 | xargs -0 clang-format -i

clean:
	rm -rf build

.PHONY: format configure build test build-test clean
