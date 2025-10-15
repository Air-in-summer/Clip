all:
	g++ -I include -L lib src/glad.c main.cpp  -lglfw3 -lopengl32 -lgdi32 -o main.exe -mwindows
	.\main.exe
	

