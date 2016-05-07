all: rotk2

rotk2: rotk2.o BRANCH.o OPCODE_STEP.o ROM_DATA.o
	g++ -shared -o rotk2.dll ./obj/rotk2.o ./obj/BRANCH.o ./obj/OPCODE_STEP.o ./obj/ROM_DATA.o

rotk2.o: rotk2.cpp
	g++ -c -BUILD_DLL -Wall -o ./obj/rotk2.o rotk2.cpp

BRANCH.o: ../bsnes/snes/cpu/BRANCH.cpp
	g++ -c -Wall -o ./obj/BRANCH.o ../bsnes/snes/cpu/BRANCH.cpp

OPCODE_STEP.o: ../bsnes/snes/cpu/OPCODE_STEP.cpp
	g++ -c -Wall -o ./obj/OPCODE_STEP.o ../bsnes/snes/cpu/OPCODE_STEP.cpp

ROM_DATA.o: ../bsnes/snes/cpu/ROM_DATA.cpp
	g++ -c -Wall -o ./obj/ROM_DATA.o ../bsnes/snes/cpu/ROM_DATA.cpp

clean:
	del .\\obj\\*.o
