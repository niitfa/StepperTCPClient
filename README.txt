Прием данных с сервера осуществляется через объект в файлах
stepper_controller_reciever.h/cpp. Их нужно копировать в свой проект.
Собирать в общем случае ничего не нужно, разве что для запуска примера.

Для сборки требуется: 
Windows: CMake, Visual Studio
Linux: CMake

Сборка и запуск на Windows:
1) Из папки проекта выполнить
mkdir build
cmake -B build
2) Открыть сгенерированный проект VS /build/run.sln
3) В VS собрать проект emulator (выбираем решение emulator - ПКМ - Build/Собрать)
4) Запустить файл build/Debug/run.exe

Сборка и запуск на Linux:
1) Из папки проекта выполнить
cmake .
make
2) Запустить проект
./run

