.PHONY: build
.PHONY: test
.PHONY: coverage
.PHONY: clean
.PHONY: bench

build:
	mkdir -p build && cd build && cmake -G Ninja .. && ninja

test:
	build/unit_tests

coverage:
	lcov --capture --directory . --output-file coverage.info && genhtml coverage.info --output-directory out && google-chrome out/index.html

clean:
	git clean -Xf && rm -rf out build

bench:
	build/benchmark
