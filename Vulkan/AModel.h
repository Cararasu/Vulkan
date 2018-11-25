#ifndef MODEL_H
#define MODEL_H

#include <render/Model.h>
#include <render/Header.h>

struct SimpleObjectData {
	glm::mat4 m2vMatrix;
	glm::mat4 normalMatrix;
	
	void set_matrix(glm::mat4 m2vMatrix) {
		this->m2vMatrix = m2vMatrix;
		this->normalMatrix = glm::transpose ( glm::inverse ( m2vMatrix ) );
	}
};

struct XWingModel {
	Array<glm::vec3> models;
	Context context;
	
};

struct AModel {
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;
	
	float velocity;
	
	glm::vec3 original_direction = glm::vec3(0.0f, 0.0f, 1.0f);
	
	AModel() {
	}
	
	void init(glm::vec3 position, glm::quat rotation, float scale, float velocity) {
		this->position = position;
		this->rotation = rotation;
		this->scale = glm::vec3(scale, scale, scale);
		this->velocity = velocity;
	}
	void init(glm::vec3 position, glm::quat rotation, glm::vec3 scale, float velocity) {
		this->position = position;
		this->rotation = rotation;
		this->scale = scale;
		this->velocity = velocity;
	}
	
	void move(float delta) {
		glm::vec3 direction = glm::normalize(glm::rotate(rotation, original_direction));
		position += direction * (delta * velocity);
	}
	glm::mat4 m2w_mat() {
		//T * R * S
		return glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), scale);
	}
	
};

#endif // MODEL_H
