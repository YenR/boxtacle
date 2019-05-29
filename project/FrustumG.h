#pragma once

#include "Plane.h"
#include <glm/glm.hpp>
#include <iostream>


class FrustumG {

private:

	enum {
		TOP = 0, BOTTOM, LEFT,
		RIGHT, NEARP, FARP
	};

public:

	static enum { OUTSIDE, INTERSECT, INSIDE };
	Plane pl[6];
	glm::vec3 ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr;
	float nearD, farD, ratio, angle, tang;
	float nw, nh, fw, fh;
	bool enabled;

	FrustumG::FrustumG();
	FrustumG::~FrustumG();

	void setCamInternals(float angle, float ratio, float nearD, float farD);
	void setCamDef(glm::vec3 &p, glm::vec3 &l, glm::vec3 &u);
	bool boxInFrustum(glm::mat4 &translation, glm::mat4 &scale);
	bool boxInFrustum(glm::vec3 &scene_min, glm::vec3 &scene_max);
};