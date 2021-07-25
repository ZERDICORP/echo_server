@echo off

g++ ./src/*.cpp -o ./build/main -lws2_32

echo Press any button..
pause > nul