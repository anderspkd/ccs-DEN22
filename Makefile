BUILD_DIR=build

default: test

test: _cmake
	cd ${BUILD_DIR} && make && ./tests.x --use-colour no

# TODO: Add proper main file for this target once we create one
run:
	@echo "no main file"

_cmake:
	cmake -B ${BUILD_DIR} -DCMAKE_EXPORT_COMPILE_COMMANDS=1 .

.PHONY: _cmake
	test
	default
	run
