#pragma once


struct QuadVertex {
	glm::vec2 pos;
};
struct QuadInstance {
	glm::vec4 dim;
	glm::vec4 uvdim;
	glm::vec4 data;
	glm::vec4 color;
};