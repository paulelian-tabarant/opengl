main: main.cpp Shader.h Mesh.h Model.h
	g++ -o main main.cpp glad.c -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lassimp
