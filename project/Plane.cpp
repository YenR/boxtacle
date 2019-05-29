#include "Plane.h"
#include <stdio.h>

Plane::Plane(glm::vec3 &v1, glm::vec3 &v2, glm::vec3 &v3) {

	set3Points(v1, v2, v3);
}


Plane::Plane() {}

Plane::~Plane() {}


void Plane::set3Points(glm::vec3 &v1, glm::vec3 &v2, glm::vec3 &v3) {


	glm::vec3 aux1, aux2;

	aux1 = v1 - v2;
	aux2 = v3 - v2;

	normal = glm::cross(aux2, aux1);
	normal = glm::normalize(normal);
	point.x = v2.x;
	point.y = v2.y;
	point.z = v2.z;
	
	d = -(glm::dot(normal, point));
}

void Plane::setNormalAndPoint(glm::vec3 &normal, glm::vec3 &point) {
	this->normal.x = normal.x;
	this->normal.y = normal.y;
	this->normal.z = normal.z;

	this->normal = glm::normalize(this->normal);

	d = -(glm::dot(normal, point));
}




float Plane::distance(glm::vec3 &p) {
	return (d + glm::dot(normal, p));
}
