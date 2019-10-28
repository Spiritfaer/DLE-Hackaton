#pragma once

#include <cstdint>
#include <list>
#include <forward_list>
#include <map>
#include <json.h>
#include <cmath>

#include "IGalaxyPathFinder.h"

enum { X, Y, Z, W };

typedef struct V3f
{
	V3f() : x(0.0f), y(0.0f), z(0.0f) {}
	V3f &operator=(Json::Value const &ref)
	{
		x = ref["x"].asFloat();
		y = ref["y"].asFloat();
		z = ref["z"].asFloat();
		return *this;
	}

	static float dist(struct V3f const &one, struct V3f const &two)
	{
		return std::sqrt(pow(two.x - one.x, 2) + pow(two.y - one.y, 2) + pow(two.z - two.z, 2));
	}

	float	x;
	float	y;
	float	z;
}	Vector3f;

typedef struct V3i
{
//	V3i() : x(0), y(0), z(0) {}
//	V3i(int32_t x, int32_t y, int32_t z) : x(z), y(y), z(z) {}
	V3i &operator=(Json::Value const &ref)
	{
		x = ref["half_x"].asInt();
		y = ref["half_y"].asInt();
		z = ref["half_z"].asInt();
		return *this;
	}
	V3i &operator=(int32_t n)
	{
		x = y = z = n;
		return *this;
	}
	V3i operator*(int32_t n)
	{
		V3i temp = {x * n, y * n, z * n};
		return temp;
	}
	V3i operator/(int32_t n)
	{
		V3i temp = {x / n, y / n, z / n};
		return temp;
	}
	V3i operator-(V3i const &ref)
	{
		V3i temp = {x - ref.x, y - ref.y, z - ref.z};
		return temp;
	}
	static bool isSmallThenRef(struct V3i const &small, struct V3i const &ref)
	{
		return small.x <= ref.x && small.y <= ref.y && small.z <= ref.z;
	}

	bool operator==(int32_t n) const
	{
		return x == n && y == n && z == n;
	}

	bool operator!=(int32_t n) const
	{
		return !(*this == n);
	}

	bool operator<(int32_t n) const
	{
		return x < n || y < n || z < n;
	}
	bool operator<(V3i const &ref) const
	{
		return x < ref.x && y < ref.y && z < ref.z;
	}
	bool operator<=(int32_t n) const
	{
		return *this < n || *this == n;
	}

	int32_t	x;
	int32_t	y;
	int32_t	z;
}	Vector3i;

struct TargetPoint
{
	TargetPoint(Json::Value const &ref)	{ *this = ref; }
	TargetPoint &operator=(Json::Value const &ref)
	{
		_pointId		= ref["pointId"].asInt();
		_posFloat		= ref;
		return *this;
	}
	int32_t		_pointId;
	Vector3f	_posFloat;
	double 		_distToBase;
};

struct Box
{
	Box(Json::Value const &ref)	{ *this = ref; }
	Box(Box const &ref)	{ *this = ref; }
	Box &operator=(Json::Value const &ref)
	{
		_targetPointId	= ref["targetPointId"].asInt();
		_boxId			= ref["boxId"].asInt();
		_boxSize		= ref;
		_w				= ref["weight"].asFloat();
		_min			= 0;
		_max			= _boxSize * 2;
		return *this;
	}
	static bool isHeavy(Box const &box, float weight)
	{
		return box._w < weight;
	}
	static bool isIntersect(Box const &a, Box const &b)
	{
		if (a._max.x <= b._min.x || a._min.x >= b._max.x)
			return false;
		if (a._max.y <= b._min.y || a._min.y >= b._max.y)
			return false;
		if (a._max.z <= b._min.z || a._min.z >= b._max.z)
			return false;
		return true;
	}
	bool moveBox(Box const &ref, Vector3i const &maxShipStor)
	{
		Box testBox(*this);
		switch (0)
		{
			case X:
				(testBox._min.x) += ref._max.x;
				(testBox._max.x) += ref._max.x;
				if (testBox._max < maxShipStor)
				{
					if (!Box::isIntersect(testBox, ref))
					{
						*this = testBox;
						return true;
					}
					else
						testBox = *this;
				} else
					break;
			case Y:
				(testBox._min.y) += ref._max.y;
				(testBox._max.y) += ref._max.y;
				if (testBox._max < maxShipStor)
				{
					if (!Box::isIntersect(testBox, ref))
					{
						*this = testBox;
						return true;
					}
					else
						testBox = *this;
				} else
					break;
			case Z:
				(testBox._min.z) += ref._max.z;
				(testBox._max.z) += ref._max.z;
				if (testBox._max < maxShipStor)
				{
					if (!Box::isIntersect(testBox, ref))
					{
						*this = testBox;
						return true;
					}
					else
						testBox = *this;
				} else
					break;
			default:
				std::logic_error("It can't be true! AXIS ERROR!");
		}
		return false;
	}
	int32_t		_targetPointId;
	int32_t		_boxId;
	Vector3i	_boxSize;
	Vector3i	_min;
	Vector3i	_max;
	float		_w;
};

struct Ship
{
	Ship()
	{
		_resToBackHome = _resToDelivery = 0;
		_maxResourcesWeight = _maxCarryingWeight = 0;
	}
	Ship &operator=(Json::Value const &ref)
	{
		_maxResourcesWeight = ref["maxResourcesWeight"].asFloat();
		_maxCarryingWeight = ref["maxCarryingWeight"].asFloat();
		_resourcesConsumption = ref["resourcesConsumption"].asFloat();
		_maxHalfCarryingCapacity = ref["maxCarryingCapacity"];
		_minCarryingCapacity = { .x = 0, .y = 0, .z = 0 };
		_maxCarryingCapacity = { .x = _maxHalfCarryingCapacity.x * 2,
						   		 .y = _maxHalfCarryingCapacity.y * 2,
						   		 .z = _maxHalfCarryingCapacity.z * 2 };
		return *this;
	}

//	std::list<Box>	_shipHold;
	float_t			_resToDelivery;
	float_t			_resToBackHome;

	float_t			_maxResourcesWeight;
	float_t			_resourcesConsumption;

	float_t			_maxCarryingWeight;
	Vector3i		_maxHalfCarryingCapacity;

	Vector3i		_minCarryingCapacity;
	Vector3i		_maxCarryingCapacity;
	Vector3f		_base;
};

struct Step
{
	void addBox(Box const &refBox)
	{
		_jBox["boxId"] = refBox._boxId;
		_jBox["x"] = refBox._boxSize.x;
		_jBox["y"] = refBox._boxSize.y;
		_jBox["z"] = refBox._boxSize.z;
		_jStep["shippedBoxes"].insert(0, _jBox);
	}
	void addInfo(void)
	{
		_jStep["destinationPointId"] = _destinationPointId;
		_jStep["shippedResources"] = _shippedRes;
		++_stepId;
	}
	void addAllBoxes(std::list<Box>	&box)
	{
		for (auto itBoxes = box.begin(), stop = box.end(); itBoxes != stop; ++itBoxes)
			addBox(*itBoxes);
	}
	void clearStep(void)
	{
		_jStep["shippedBoxes"].clear();
		_shippedBox.clear();
		_destinationPointId = 0;
		_shippedRes = 0;
		_shippedBoxW = 0;
	}
	void endStep(std::list<Box>	&boxesFalse)
	{
		_shippedBox = boxesFalse;
		addAllBoxes(_shippedBox);
		for_each(_shippedBox.begin(), _shippedBox.end(), [&](auto const &itBox){
			_shippedBoxW += itBox._w;
		});
	}
	Json::Value				_jStep;
	Json::Value				_jBox;

	std::list<Box>			_shippedBox;
	float					_shippedBoxW;
	float					_shippedRes;
	int32_t					_destinationPointId;
	static uint32_t			_stepId;
};

class IgorStalevskiyPathFinder : public IGalaxyPathFinder {
private:
	std::list<Box>					_boxesWork;
	std::list<Box>					_boxesFalse;
	std::map<int32_t, TargetPoint>	_targetPoints;
	Ship							_myship;

	IgorStalevskiyPathFinder(IgorStalevskiyPathFinder const &);
	IgorStalevskiyPathFinder &operator=(IgorStalevskiyPathFinder const &);
	void init(const char* inputJasonFile);
	void core(const char *outputFileName);
	void initTargetBox(Json::Value const &root);
	void initBase(std::list<TargetPoint> &parsingPoint);
	void initDistanceToBase(std::list<TargetPoint> &parsingPoint);
	void initBox(Json::Value const &root);
	void initShip(Json::Value const &root);
	void stamp(Step &step);
	void checkingBox(Json::Value const &box);
	void primitiveDelivery(const char* outputFileName);
	bool simpleBoxChecker(Step const &step, std::list<Box>::iterator const &itBoxes) const;
	bool simpleLoader(Step const &step, std::list<Box>::iterator &itBoxes);
public:
	IgorStalevskiyPathFinder();
	~IgorStalevskiyPathFinder();

	void FindSolution(const char* inputJasonFile, const char* outputFileName) override;
	const char* ShowCaptainName()  override { return "Igor Stalevskiy"; }
};
