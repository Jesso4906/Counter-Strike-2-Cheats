#pragma once
#include <math.h>

const float pi = 3.14159;
const float rToD = (180 / pi); // radians to degrees
const float dToR = (pi / 180); // degrees to radians

struct Vector3
{
	float x, y, z;

	Vector3 operator + (Vector3 v)
	{
		return { x + v.x, y + v.y, z + v.z };
	}
	Vector3 operator - (Vector3 v)
	{
		return { x - v.x, y - v.y, z - v.z };
	}
	Vector3 operator * (float v)
	{
		return { x * v, y * v, z * v };
	}
	Vector3 operator / (float v)
	{
		return { x / v, y / v, z / v };
	}
};

struct Vector2
{
	float x, y;

	Vector2 operator + (Vector2 v)
	{
		return { x + v.x, y + v.y };
	}
	Vector2 operator - (Vector2 v)
	{
		return { x - v.x, y - v.y };
	}
	Vector2 operator * (float v)
	{
		return { x * v, y * v };
	}
	Vector2 operator / (float v)
	{
		return { x / v, y / v };
	}
};