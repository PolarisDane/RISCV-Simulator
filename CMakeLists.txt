CMAKE_MINIMUM_REQUIRED(VERSION 3.11)
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_COMPILER /usr/bin/g++)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
PROJECT(RISCV_SIMULATOR)
INCLUDE_DIRECTORIES(./src)
INCLUDE_DIRECTORIES(./lib)
ADD_EXECUTABLE(code ./src/Utils.h ./src/Predictor.cpp ./src/Register.cpp ./src/InstructionUnit.cpp ./src/ALU.cpp ./src/Memory.cpp ./src/LoadStoreBuffer.cpp ./src/ReservationStation.cpp ./src/ReorderBuffer.cpp ./src/RISCV_Simulator.cpp ./src/main.cpp)
