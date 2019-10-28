#include <iostream>
#include "IgorStalevskiyPathFinder.hpp"

int main() {
	IgorStalevskiyPathFinder findYourWay;
	findYourWay.FindSolution("../test/inputData1.json", "outDataTest.json");

	system("leaks DragonsLakeEntertainmentHackaton");
	return 0;
}
