#pragma once

#include "clay.h"
#include "clayex.h"
#include <raylib.h>
#include <raymath.h>
#include <functional>
#include <memory>

struct CursorPosition
{
	uint16_t line;
	uint16_t column;

	bool operator==(const CursorPosition& other) const
	{
		return line == other.line && column == other.column;
	}
};

struct LayoutDimensions
{
	float width;
	float height;

	LayoutDimensions()
	{
		width = 0;
		height = 0;
	}

	LayoutDimensions(float w, float h)
	{
		width = w;
		height = h;
	}
	LayoutDimensions& operator= (const LayoutDimensions& other)
	{
		if (this == &other)
			return *this;

		width = other.width;
		height = other.height;
		return *this;
	}

	LayoutDimensions(const Clay_BoundingBox& bb)
	{
		width = bb.width;
		height = bb.height;
	}

	bool operator==(const LayoutDimensions& other) const
	{
		return width == other.width && height == other.height;
	}
};