#include "FrustumG.h"
#include <math.h>

#define ANG2RAD 3.14159265358979323846/180.0

FrustumG::FrustumG() {}

FrustumG::~FrustumG() {}

void FrustumG::setCamInternals(float angle, float ratio, float nearD, float farD) {

	// store the information
	this->ratio = ratio;
	this->angle = angle;
	this->nearD = nearD;
	this->farD = farD;

	// compute width and height of the near and far plane sections
	tang = (float)tan(ANG2RAD * angle * 0.5f);
	
	nh = nearD * tang;
	nw = nh * ratio;
	fh = farD  * tang;
	fw = fh * ratio;
}


void FrustumG::setCamDef(glm::vec3 &p, glm::vec3 &l, glm::vec3 &u) {

	glm::vec3 dir, nc, fc, X, Y, Z;

	// compute the Z axis of camera
	// this axis points in the opposite direction from
	// the looking direction
	Z = p - l;
	Z = glm::normalize(Z);

	// X axis of camera with given "up" vector and Z axis
	//X = u * Z;
	X = glm::cross(u, Z);
	X = glm::normalize(X);


	// the real "up" vector is the cross product of Z and X
	Y = glm::cross(Z, X);

	// compute the centers of the near and far planes
	nc = p - (Z * nearD);
	fc = p - (Z * farD);

	// compute the 4 corners of the frustum on the near plane
	ntl = nc + (Y * nh) - (X * nw);
	ntr = nc + (Y * nh) + (X * nw);
	nbl = nc - (Y * nh) - (X * nw);
	nbr = nc - (Y * nh) + (X * nw);

	// compute the 4 corners of the frustum on the far plane
	ftl = fc + (Y * fh) - (X * fw);
	ftr = fc + (Y * fh) + (X * fw);
	fbl = fc - (Y * fh) - (X * fw);
	fbr = fc - (Y * fh) + (X * fw);

	// compute the six planes
	// the function set3Points assumes that the points
	// are given in counter clockwise order
	pl[TOP].set3Points(ntr, ntl, ftl);
	pl[BOTTOM].set3Points(nbl, nbr, fbr);
	pl[LEFT].set3Points(ntl, nbl, fbl);
	pl[RIGHT].set3Points(nbr, ntr, fbr);
	pl[NEARP].set3Points(ntl, ntr, nbr);
	pl[FARP].set3Points(ftr, ftl, fbl);
}

// Test for boxes
bool FrustumG::boxInFrustum(glm::mat4 &translation, glm::mat4 &scale) {
	bool result = true;
	if (enabled) {
		
		int in, out;
		float size = glm::max(glm::max(scale[0].x, scale[1].y), scale[2].z) * 0.5f;

		glm::vec3 points[8];
		points[0] = glm::vec3(translation[3].x + size, translation[3].y + size, translation[3].z + size);
		points[1] = glm::vec3(translation[3].x + size, translation[3].y + size, translation[3].z - size);
		points[2] = glm::vec3(translation[3].x + size, translation[3].y - size, translation[3].z + size);
		points[3] = glm::vec3(translation[3].x + size, translation[3].y - size, translation[3].z - size);
		points[4] = glm::vec3(translation[3].x - size, translation[3].y + size, translation[3].z + size);
		points[5] = glm::vec3(translation[3].x - size, translation[3].y + size, translation[3].z - size);
		points[6] = glm::vec3(translation[3].x - size, translation[3].y - size, translation[3].z + size);
		points[7] = glm::vec3(translation[3].x - size, translation[3].y - size, translation[3].z - size);

		for (int i = 0; i < 6; i++) {
			in = 0;
			out = 0;
			for (int j = 0; j < 8; j++) {
				if (pl[i].distance(points[j]) < 0) {
					out++;
				}
				else {
					in++;
				}
			}

			/*std::cout << "in: " << in << "\n";
			std::cout << "out: " << out << "\n";*/
			if (in == 0) {
				return false;
			}
		}
	}
	return result;
}

// Test for Models
bool FrustumG::boxInFrustum(glm::vec3 &scene_min, glm::vec3 &scene_max) {
	bool result = true;
	if (enabled) {

		int in, out;

		for (int i = 0; i < 6; i++) {
			in = 0;
			out = 0;
			if (pl[i].distance(scene_min) < 0) {
				out++;
			}
			else {
				in++;
			}

			if (pl[i].distance(scene_max) < 0) {
				out++;
			}
			else {
				in++;
			}

			/*std::cout << "in: " << in << "\n";
			std::cout << "out: " << out << "\n";*/
			if (in == 0) {
				return false;
			}
		}
	}
	return result;
}
