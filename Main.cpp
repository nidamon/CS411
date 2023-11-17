// Main.cpp
// Nathan Damon
// 2023-11-15
// A program to run a QuickHull algorithm and display its work
// Uses One Lone Coder's olcPixelGameEngine
// 
// This program was running on Visual Studio 2022, but should work for 
// some of the previous ones. If using outside of Visual Studio and 
// having issues, see: https://www.github.com/onelonecoder for possible 
// fixes.

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <random>
#include <chrono>

class QuickHullSim : public olc::PixelGameEngine
{
public:
	class Point
	{
	public:
		Point(olc::vf2d position = olc::vf2d(), olc::Pixel color = olc::CYAN) : _position(position), _color(color) {}
		~Point() {}

		void drawSelf(olc::PixelGameEngine* gfx, float scaleFactor = 1.0f, olc::vf2d offset = { 0.0f, 0.0f })
		{
			gfx->Draw(_position * scaleFactor + offset, _color);
		}

		void setAsOnHull()
		{
			_color = olc::YELLOW;
		}
		bool isOnHull()
		{
			return _color == olc::YELLOW;
		}
		void resetColor(olc::Pixel color = olc::CYAN)
		{
			_color = color;
		}

	public:
		olc::vf2d _position;
		olc::Pixel _color = olc::CYAN;
	};
	class Line
	{
	public:
		Line(Point& p1, Point& p2, olc::Pixel color = olc::YELLOW) : _p1(p1), _p2(p2), _color(color) {}
		~Line() {}

		void drawSelf(olc::PixelGameEngine* gfx, float scaleFactor = 1.0f, olc::vf2d offset = { 0.0f, 0.0f })
		{
			gfx->DrawLine(_p1._position * scaleFactor + offset, _p2._position * scaleFactor + offset, _color);
		}

	public:
		Point& _p1;
		Point& _p2;
		olc::Pixel _color = olc::CYAN;
	};

public:
	QuickHullSim()
	{
		sAppName = "QuickHullSim";
	}

public:
	bool OnUserCreate() override
	{
		_uniDistX = std::uniform_real_distribution<float>(0.0f, 1.0f);
		_uniDistY = std::uniform_real_distribution<float>(0.0f, 1.0f);

		_points = std::vector<Point>(_pointCount);
		placePointsUniformly();

		_scaleFactor = std::max(0.01f, std::min(_scaleFactor, float(GetScreenSize().x)));
		_pointOffset = (olc::vf2d(GetScreenSize()) - olc::vf2d(1.0f, 1.0f) * _scaleFactor) * 0.5f;

		instructions();
		return true;
	}
	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(olc::BLACK);

		handleInput();

		// Draw points
		for (auto& point : _points)
			point.drawSelf(this, _scaleFactor, _pointOffset);

		// Draw lines
		for (auto& line : _lines)
			line.drawSelf(this, _scaleFactor, _pointOffset);


		// Computation execution here
		if (_simulationComplete == false)
		{
			auto timeStart = std::chrono::steady_clock::now();
			_simulationComplete = QuickHull(_points, _lines);
			std::chrono::duration<float> timeTaken = std::chrono::steady_clock::now() - timeStart;

			_quickHullRunTimes.push_back(timeTaken.count() * 1000.0f);

			// Get the average
			float averageTime = _quickHullRunTimes[0];
			for (size_t i = 1; i < _quickHullRunTimes.size(); i++)
			{
				averageTime += _quickHullRunTimes[i];
			}
			averageTime /= float(_quickHullRunTimes.size());

			// Display
			std::cout << "Time to QuickHull " << _pointCount << " points (milliseconds): " << _quickHullRunTimes.back() << " avg: " << averageTime << std::endl;
		}

		return true;
	}

	void placePointsUniformly()
	{
		for (auto& point : _points)
		{
			point._position = olc::vf2d(_uniDistX(_r), _uniDistY(_r));
		}
	}
	void placePointsUniformlyOnCircle()
	{
		float radiansOfChange = 360.0f / float(_points.size()) * 0.01745329f;

		for (size_t i = 0; i < _points.size(); i++)
		{
			_points[i]._position.x = cosf(radiansOfChange * float(i)) * 0.5f + 0.5f;
			_points[i]._position.y = sinf(radiansOfChange * float(i)) * 0.5f + 0.5f;
		}
	}
	void resetPointColors()
	{
		for (auto& point : _points)
		{
			point.resetColor();
		}
	}

	void instructions()
	{
		std::cout << 
			"Point count: " << _points.size() << "\n"
			"Press R key to restart simulation\n"
			"Press D key to toggle debug lines\n"
			"Press S key to toggle showing the final hull\n"
			"Press W key to toggle worst case performance for QuickHull\n"
			"Press 1 for 10 points\n"
			"Press 2 for 100 points\n"
			"Press 3 for 1,000 points\n"
			"Press 4 for 10,000 points\n"
			"Press 5 for 100,000 points\n"
			"Press 6 for 1,000,000 points\n"
			"\n";
	}
	void handleInput()
	{
		// Resetting
		bool resetSimulation = false;
		if (GetKey(olc::R).bReleased)
			resetSimulation = true;

		if (GetKey(olc::D).bPressed)
		{
			_debugDisplay = !_debugDisplay;
			if (_debugDisplay)
				std::cout << "Debug lines turned on" << std::endl;
			else
				std::cout << "Debug lines turned off" << std::endl;
		}
		if (GetKey(olc::S).bPressed)
		{
			_showFinalHull = !_showFinalHull;
			if (_showFinalHull)
				std::cout << "Show final hull turned on" << std::endl;
			else
				std::cout << "Show final hull turned off" << std::endl;
		}
		if (GetKey(olc::W).bPressed)
		{
			_worstCaseEnabled = !_worstCaseEnabled;
			if (_worstCaseEnabled)
				std::cout << "Worst case enabled" << std::endl;
			else
				std::cout << "Worst case diabled" << std::endl;
		}

		// Point count changing
		bool resetPointVector = false;
		if (GetKey(olc::NP1).bReleased || GetKey(olc::K1).bReleased)
		{
			_pointCount = 10;
			resetPointVector = true;
		}
		if (GetKey(olc::NP2).bReleased || GetKey(olc::K2).bReleased)
		{
			_pointCount = 100;
			resetPointVector = true;
		}
		if (GetKey(olc::NP3).bReleased || GetKey(olc::K3).bReleased)
		{
			_pointCount = 1000;
			resetPointVector = true;
		}
		if (GetKey(olc::NP4).bReleased || GetKey(olc::K4).bReleased)
		{
			_pointCount = 10000;
			resetPointVector = true;
		}
		if (GetKey(olc::NP5).bReleased || GetKey(olc::K5).bReleased)
		{
			_pointCount = 100000;
			resetPointVector = true;
		}
		if (GetKey(olc::NP6).bReleased || GetKey(olc::K6).bReleased)
		{
			_pointCount = 1000000;
			resetPointVector = true;
		}

		if (resetPointVector)
		{
			_quickHullRunTimes.clear();
			_lines.clear();
			_points.clear();
			_points = std::vector<Point>(_pointCount);

			std::cout << "Point count = " << _points.size() << std::endl;
		}
		if (resetSimulation || resetPointVector)
		{
			_lines.clear();
			if (_worstCaseEnabled)
				placePointsUniformlyOnCircle();
			else
				placePointsUniformly();
			resetPointColors();
			_simulationComplete = false;
		}
	}


	// #######################################################################################################
	// QuickHull algorithm - see Point and Line at the top of QuickHullSim class definition
	// #######################################################################################################
	
	// Returns true on complete
	bool QuickHull(std::vector<Point>& points, std::vector<Line>& lines)
	{
		// Greatest x value
		Point* B = &points.front();
		// Least x Value
		Point* A = &points.front();
		for (size_t i = 1; i < points.size(); i++)
		{
			if (points[i]._position.x < A->_position.x)
				A = &points[i]; // Set min point
			if (points[i]._position.x > B->_position.x)
				B = &points[i]; // Set max point
		}

		A->setAsOnHull();
		B->setAsOnHull();

		if(_debugDisplay)
			lines.push_back(Line(*A, *B, olc::GREEN));

		// 2n additional pointer memory
		std::vector<Point*> pointsSource(points.size());
		std::vector<Point*> pointsDestination(points.size());

		// Split
		int lastAboveIndex = 0;
		int firstBelowIndex = 0;
		splitAllPoints(A, B, points, pointsSource, lastAboveIndex, firstBelowIndex);

		// Begin recurse
		QuickHullSub(A, B, pointsSource, 0, lastAboveIndex, pointsDestination); // Top
		QuickHullSub(B, A, pointsSource, firstBelowIndex, points.size() - 1, pointsDestination); // Bottom

		return true;
	}
	void QuickHullSub(Point* A, Point* B, std::vector<Point*>& pointsSource, const int rangeStart, const int rangeEnd, std::vector<Point*>& pointsDestination, int depthCount = 0)
	{
		depthCount++;
		// These pointers are set in the splitPoints function when no points are copied over
		if (pointsSource[rangeStart] == nullptr || pointsSource[rangeEnd] == nullptr)
		{
			if(_showFinalHull)
				_lines.push_back(Line(*A, *B));
			return;
		}

		Point* C = getFarthestPointFromAB(A, B, pointsSource, rangeStart, rangeEnd);
		C->setAsOnHull();

		if (_debugDisplay)
		{
			_lines.push_back(Line(*A, *C, olc::RED));
			_lines.push_back(Line(*B, *C, olc::RED));
		}
		
		// Divide
		int lastACIndex = 0;
		int firstCBIndex = 0;
		splitPoints(A, B, C, pointsSource, rangeStart, rangeEnd, pointsDestination, lastACIndex, firstCBIndex);

		// Recurse
		QuickHullSub(A, C, pointsDestination, rangeStart, lastACIndex, pointsSource, depthCount); // Left
		QuickHullSub(C, B, pointsDestination, firstCBIndex, rangeEnd, pointsSource, depthCount); // Right
	}

	bool isAboveline(const Point* A, const Point* B, const Point* point)
	{
		olc::vf2d lineVector = B->_position - A->_position;
		olc::vf2d lineStartToPointVector = point->_position - A->_position;

		if (lineVector.perp().dot(lineStartToPointVector) < 0.0f)
			return true;
		return false;
	}
	float distFromLine(float a, float b, float c, const Point* point)
	{
		return abs(a * point->_position.x + b * point->_position.y + c) /
			sqrtf(a * a + b * b);
	}
	/*void splitPoints(const Point* A, const Point* B, std::vector<Point>& points, std::vector<Point*>& pointsAbove, std::vector<Point*>& pointsBelow)
	{
		for (auto& point : points)
		{
			if (point.isOnHull())
				continue;
			if (isAboveline(*A, *B, point))
				pointsAbove.push_back(&point);
			else
				pointsBelow.push_back(&point);
		}
	}*/
	// Returns the split point (index of the last element of the )
	void splitPoints(Point* A, Point* B, Point* C, const std::vector<Point*>& pointsSource, const int rangeStart, const int rangeEnd, std::vector<Point*>& pointsDestination, int& lastACIndex, int& firstCBIndex)
	{
		lastACIndex = rangeStart;
		firstCBIndex = rangeEnd;

		bool correctLastACIndex = false;
		bool correctFirstCBIndex = false;

		for (size_t i = rangeStart; i < rangeEnd + 1; i++)
		{
			if (pointsSource[i]->isOnHull())
				continue;
			if (isAboveline(A, C, pointsSource[i]))
			{
				pointsDestination[lastACIndex] = pointsSource[i];
				lastACIndex++;
				correctLastACIndex = true;
			}
			if (isAboveline(C, B, pointsSource[i]))
			{
				pointsDestination[firstCBIndex] = pointsSource[i];
				firstCBIndex--;
				correctFirstCBIndex = true;
			}
		}

		// Remove 1 to prevent off by 1 error
		if(correctLastACIndex)
			lastACIndex--;
		// Add 1 to prevent off by 1 error
		if(correctFirstCBIndex)
			firstCBIndex++;

		// When the range doesn't have any points for use
		if (correctLastACIndex == false)
			pointsDestination[lastACIndex] = nullptr;
		if (correctFirstCBIndex == false)
			pointsDestination[firstCBIndex] = nullptr;
	}
	// Splits the specified range of points on the pointsSource vector to the same range on the pointsDestination vector
	void splitAllPoints(Point* A, Point* B, std::vector<Point>& pointsSource, std::vector<Point*>& pointsDestination, int& lastAboveIndex, int& firstBelowIndex)
	{
		lastAboveIndex = 0;
		firstBelowIndex = pointsSource.size() - 1;

		bool correctLastAboveIndex = false;
		bool correctfirstBelowIndex = false;

		for (size_t i = 0; i < pointsSource.size(); i++)
		{
			if (pointsSource[i].isOnHull())
				continue;
			if (isAboveline(A, B, &(pointsSource[i])))
			{
				pointsDestination[lastAboveIndex] = &(pointsSource[i]);
				lastAboveIndex++;
				correctLastAboveIndex = true;
			}
			else
			{
				pointsDestination[firstBelowIndex] = &(pointsSource[i]);
				firstBelowIndex--;
				correctfirstBelowIndex = true;
			}
		}

		// Remove 1 to prevent off by 1 error
		if(correctLastAboveIndex)
			lastAboveIndex--;
		// Add 1 to prevent off by 1 error
		if(correctfirstBelowIndex)
			firstBelowIndex++;
	}
	Point* getFarthestPointFromAB(Point* A, Point* B, std::vector<Point*>& points, const int rangeStart, const int rangeEnd)
	{
		// Used for line distance checking
		float a = A->_position.y - B->_position.y;
		float b = B->_position.x - A->_position.x;
		float c = A->_position.x * B->_position.y - B->_position.x * A->_position.y;

		float farthestDist = distFromLine(a, b, c, points[rangeStart]);
		Point* farthestPoint = points[rangeStart];

		for (size_t i = rangeStart + 1; i < rangeEnd + 1; i++)
		{
			auto tempDist = distFromLine(a, b, c, points[i]);
			if (tempDist > farthestDist)
			{
				farthestDist = tempDist;
				farthestPoint = points[i];
			}
		}
		return farthestPoint;
	}

	// #######################################################################################################
	// END QuickHull algorithm and helpers
	// #######################################################################################################

public:
	// The number of points to use
	int _pointCount = 10;
	std::vector<float> _quickHullRunTimes;
	std::vector<Point> _points;
	std::vector<Line> _lines;

	// Display scaling
	float _scaleFactor = 250.0f;
	olc::vf2d _pointOffset = { 0.0f, 0.0f };

	bool _simulationComplete = false;
	bool _debugDisplay = false;
	bool _showFinalHull = true;

	bool _worstCaseEnabled = false;

	// Position resetting
	std::random_device _r;
	std::uniform_real_distribution<float> _uniDistX;
	std::uniform_real_distribution<float> _uniDistY;
};

int main()
{
	QuickHullSim quickHullSim;
	if (quickHullSim.Construct(300, 300, 3, 3))
		quickHullSim.Start();
	return 0;
}