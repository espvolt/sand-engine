default:
	g++ -c ./src/main.cpp -Isrc/include -o build/main.o
	g++ -c ./src/sand.cpp -Isrc/include -o build/sand.o
	g++ -c ./src/entity.cpp -Isrc/include -o build/entity.o

	g++ ./build/main.o ./build/sand.o ./build/entity.o -o build/build-app -lsfml-graphics -lsfml-window -lsfml-system
	./build/build-app

debug:
	g++ -c ./src/main.cpp -Isrc/include -o build/main.o
	g++ -c ./src/sand.cpp -Isrc/include -o build/sand.o
	g++ -c ./src/entity.cpp -Isrc/include -o build/entity.o

	g++ -fsanitize=address ./build/main.o ./build/sand.o ./build/entity.o -o build/build-app -lsfml-graphics -lsfml-window -lsfml-system
	./build/build-app












