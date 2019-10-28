#include <fstream>
#include <iostream>
#include <IgorStalevskiyPathFinder.hpp>
#include "IgorStalevskiyPathFinder.hpp"

uint32_t Step::_stepId = 0;
IgorStalevskiyPathFinder::IgorStalevskiyPathFinder() = default;
IgorStalevskiyPathFinder::~IgorStalevskiyPathFinder() = default;
void IgorStalevskiyPathFinder::init(const char* inputJasonFile) {
	std::ifstream	input(inputJasonFile);
	Json::Value		root;

	if (!input.is_open())
		throw std::logic_error("file not found");

	input >> root;

	initShip(root);
	initTargetBox(root);
	initBox(root);
}
void IgorStalevskiyPathFinder::checkingBox(Json::Value const &box)
{
	Box boxForChecking = box;
	if (_targetPoints.count(boxForChecking._targetPointId) > 0 &&
		Vector3i::isSmallThenRef(boxForChecking._boxSize, _myship._maxHalfCarryingCapacity) &&
		boxForChecking._boxSize != 0 &&
		Box::isHeavy(boxForChecking, _myship._maxCarryingWeight))
		_boxesWork.push_back(boxForChecking);
	else
		_boxesFalse.push_back(boxForChecking);
}
void IgorStalevskiyPathFinder::initBox(Json::Value const &root)
{
	int size = root["boxes"].size();
	for (auto i = 0u ; i < size; ++i) {
		checkingBox(root["boxes"][i]);
	}
	_boxesWork.sort([](Box const &one, Box const &two) {
		return one._targetPointId < two._targetPointId;
	});
}
void IgorStalevskiyPathFinder::initBase(std::list<TargetPoint> &parsingPoint)
{
	parsingPoint.sort([](TargetPoint const &one, TargetPoint const &two) -> bool { return one._pointId < two._pointId; });
	if (parsingPoint.front()._pointId != 0)
		throw std::logic_error("Haven't base TargetPoint with pointId 0!");
	_myship._base = parsingPoint.front()._posFloat;
	parsingPoint.pop_front();
}
void IgorStalevskiyPathFinder::initDistanceToBase(std::list<TargetPoint> &parsingPoint)
{
	Vector3f base = _myship._base;
	std::for_each(parsingPoint.begin(),parsingPoint.end(), [&base](TargetPoint &point){
		point._distToBase = V3f::dist(point._posFloat, base);
	});

	double maxDistance = _myship._maxResourcesWeight / 2 / _myship._resourcesConsumption;
	for (auto it = parsingPoint.cbegin(), stop = parsingPoint.cend(); it != stop; ++it)
	{
		if (it->_distToBase > maxDistance)
			parsingPoint.erase(it);
	}
}
void IgorStalevskiyPathFinder::initTargetBox(Json::Value const &root)
{
	std::list<TargetPoint> parsing;
	for (auto i = 0u, size = root["targetPoints"].size(); i < size; ++i) {
		parsing.push_front(root["targetPoints"][i]);
	}

	initBase(parsing);
	initDistanceToBase(parsing);

	auto spaceMap = &_targetPoints;
	std::for_each(parsing.begin(), parsing.end(), [spaceMap](TargetPoint const &point){
		spaceMap->insert(std::make_pair(point._pointId, point));
	});
}
void IgorStalevskiyPathFinder::initShip(Json::Value const &root)
{
	_myship = root["ship"];
	if (_myship._maxHalfCarryingCapacity <= 0 ||
		_myship._maxCarryingWeight <= 0 ||
		_myship._resourcesConsumption <= 0 ||
		_myship._maxResourcesWeight <= 0)
		throw std::logic_error("Space ship is broken!");
}
bool IgorStalevskiyPathFinder::simpleBoxChecker(Step const &step, std::list<Box>::iterator const &itBoxes) const
{
	if (itBoxes->_w + step._shippedRes + step._shippedBoxW > _myship._maxCarryingWeight)
		return false;
	return step._destinationPointId == itBoxes->_targetPointId;
}
void IgorStalevskiyPathFinder::stamp(Step &step)
{
	for (auto it = step._shippedBox.begin(), end = step._shippedBox.end(); it != end; ++it)
		step.addBox(*it);
	step.addInfo();
}
bool IgorStalevskiyPathFinder::simpleLoader(Step const &step, std::list<Box>::iterator &itBoxes) {
	if (!step._shippedBox.size())
		return true;

	auto itStepBox = step._shippedBox.begin(), end = step._shippedBox.end();
	do {
		if (Box::isIntersect(*itBoxes, *itStepBox)) {
			if (!itBoxes->moveBox(*itStepBox, _myship._maxCarryingCapacity))
				return false;
		} else {
			return true;
		}
	} while (++itStepBox != end || !Box::isIntersect(*itBoxes, *itStepBox));
	return false;
}
void IgorStalevskiyPathFinder::primitiveDelivery(const char* outputFileName)
{
	std::ofstream output(outputFileName);

	Json::Value root;
	Json::Value& friend_node = root["steps"];

	Step step;
	auto stepID = 0u;
	std::list<Box> stepBox;
	std::list<Box>::iterator itBoxes;
	for (;;)
	{
		// задать точку полета
		itBoxes = _boxesWork.begin();
		step._destinationPointId = itBoxes->_targetPointId;

		// загрузить топливо - получаем доступ к точке, берем дистанцию, делем на расход ресурсов, добовляем дорогу домой
		_myship._resToDelivery = _targetPoints.at(step._destinationPointId)._distToBase * _myship._resourcesConsumption;
		_myship._resToBackHome = _targetPoints.at(step._destinationPointId)._distToBase * _myship._resourcesConsumption;
		step._shippedRes = _myship._resToDelivery + _myship._resToBackHome;

		// добавить все контейнеры из данной точки направления, КОЛИЗИИ с другими контейнерами (по трем координатам)
		for (;; ++itBoxes)
		{
			// проверка контейнера по TargetId
			// проверка контейнера по Wight
			if (simpleBoxChecker(step, itBoxes) && simpleLoader(step, itBoxes))
			{
				// обновляем центральные координаты
				itBoxes->_boxSize = (itBoxes->_max / 2) - _myship._maxHalfCarryingCapacity;
				// плюсуем вес коробки к общему весу;
				step._shippedBoxW += itBoxes->_w;
				step._shippedBox.splice(step._shippedBox.end(), _boxesWork, _boxesWork.begin());
				// после перемещения контейнера нужно обязательно обновить итератор!!!
				itBoxes = _boxesWork.begin();

			}
			else
				break;
		}

		// сделать снимок
		stamp(step);
		friend_node.insert(stepID++, step._jStep);

		// отчистить степ и сделать снимок полета домой

		step.clearStep();
		stamp(step);
		friend_node.insert(stepID++, step._jStep);
		// уловие выхода из бесконечного цикла
		if (!_boxesWork.size())
			break;
	}
	// загрузка всех невалидных контиейнеров
	step.endStep(_boxesFalse);
	friend_node.insert(stepID, step._jStep);

	output << root;
}
void IgorStalevskiyPathFinder::core(const char *outputFileName)
{
	primitiveDelivery(outputFileName);
}

void IgorStalevskiyPathFinder::FindSolution(const char* inputJasonFile, const char* outputFileName)
{
	auto begin = std::chrono::steady_clock::now();
	try {
		if (!inputJasonFile || !outputFileName)
			throw std::invalid_argument("inputJasonFile or outputFileName == nullptr");
		init(inputJasonFile);
		core(outputFileName);
	} catch (std::exception const &ex) {
		std::cerr << ex.what() << std::endl;
	}
	auto end = std::chrono::steady_clock::now();
	auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
	std::cout << "runtime = " << elapsed_ms.count() << " ms" << std::endl;
	std::cout << "all steps = " << Step::_stepId << std::endl;
}
