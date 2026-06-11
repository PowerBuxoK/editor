all:
	cmake -S . -B ./build/
	cmake --build ./build/ --parallel

clean:
	rm -rf ./build/
