#pragma once

#include "Std.hpp"

class Loader {

private:
	
public:
	Loader() {}
	~Loader() {}
	
	bool loadAssImp(
		const char * path,
		std::vector<unsigned short> & indices,
		std::vector<glm::vec3> & vertices,
		std::vector<glm::vec2> & uvs,
		std::vector<glm::vec3> & normals
	);
};