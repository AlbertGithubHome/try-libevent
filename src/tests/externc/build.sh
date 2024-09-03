gcc -c foo.c -o foo.o
g++ -c main.cpp -o main.o
g++ foo.o main.o -o main

#g++ main.cpp foo.c -o main