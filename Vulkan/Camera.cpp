
#include "Camera.h"


//  http://www.convertalot.com/sphere_solver.html
//  Recursive definition of determinate using expansion by minors.
float determinant( glm::vec4 input[4], int n ) {
    float d = 0.0f;
	glm::vec4 mat[4];

    if (n == 2) { // terminate recursion
        d = input[0][0]*input[1][1] - input[1][0]*input[0][1];
    }
    else {
        d = 0;
        for (int l = 0; l < n; l++ ) { // do each column
            for (int i = 1; i < n; i++) { // create minor
                for (int j = 0, k = 0; j < n; j++) {
                    if (j == l) continue;
                    mat[i-1][k] = input[i][j];
                    k++;
                }
            }
            // sum (+/-)cofactor * minor  
            d = d + glm::pow(-1.0, l)*input[0][l]*determinant( mat, n-1 );
        }
    }
    return d;
}

Sphere find_center_of_sphere(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4) {
	
	glm::vec3 input[4] = {p1, p2, p3, p4};
	glm::vec4 mat[4];
	
    for (int i = 0; i < 4; i++) { // find minor 11
        mat[i][0] = input[i][0];
        mat[i][1] = input[i][1];
        mat[i][2] = input[i][2];
        mat[i][3] = 1.0f;
    }
	float m11 = determinant(mat, 4);
    for (int i = 0; i < 4; i++) { // find minor 12 
        mat[i][0] = input[i][0]*input[i][0] + input[i][1]*input[i][1] + input[i][2]*input[i][2];
        mat[i][1] = input[i][1];
        mat[i][2] = input[i][2];
        mat[i][3] = 1;
    }
    float m12 = determinant( mat, 4 );
    for (int i = 0; i < 4; i++) { // find minor 13
        mat[i][0] = input[i][0]*input[i][0] + input[i][1]*input[i][1] + input[i][2]*input[i][2];
        mat[i][1] = input[i][0];
        mat[i][2] = input[i][2];
        mat[i][3] = 1;
    }
    float m13 = determinant( mat, 4 );
    for (int i = 0; i < 4; i++) { // find minor 14
        mat[i][0] = input[i][0]*input[i][0] + input[i][1]*input[i][1] + input[i][2]*input[i][2];
        mat[i][1] = input[i][0];
        mat[i][2] = input[i][1];
        mat[i][3] = 1;
    }
    float m14 = determinant( mat, 4 );
    for (int i = 0; i < 4; i++) { // find minor 15
        mat[i][0] = input[i][0]*input[i][0] + input[i][1]*input[i][1] + input[i][2]*input[i][2];
        mat[i][1] = input[i][0];
        mat[i][2] = input[i][1];
        mat[i][3] = input[i][2];
    }
    float m15 = determinant( mat, 4 );

	float r = 0.0f;
	float Xo = 0.0f, Yo = 0.0f, Zo = 0.0f;
    if (m11) {
        Xo =  0.5f*m12/m11;                     // center of sphere
        Yo = -0.5f*m13/m11;
        Zo =  0.5f*m14/m11;
        r  = glm::sqrt( Xo*Xo + Yo*Yo + Zo*Zo - m15/m11 );
    }
	return {glm::vec3(Xo, Yo, Zo), r};
}

Camera generate_shadowmap_camera(Camera* cam, glm::vec3 light_dir, float near, float far){
	
	Camera shadowmap_camera = *cam;
	shadowmap_camera.near = near;
	shadowmap_camera.far = far;
	
	//maybe we want to transform view space directly into light view space
	glm::mat4 s2w_matrix = glm::inverse(shadowmap_camera.v2s_mat() * shadowmap_camera.orientation.w2v_mat());
	
	glm::vec3 forward = glm::normalize(light_dir);
	//glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	//glm::vec3 right = glm::normalize(glm::cross(forward, up));
	//up = glm::normalize(glm::cross(right, forward));
	
	glm::vec4 near1 = s2w_matrix * glm::vec4(-1.0f, -1.0f, 0.0f, 1.0);//top left
	glm::vec4 near2 = s2w_matrix * glm::vec4(1.0f, 1.0f, 0.0f, 1.0);//bottom right
	
	glm::vec4 far1 = s2w_matrix * glm::vec4(-1.0f, 1.0f, 1.0f, 1.0);//bottom left
	glm::vec4 far2 = s2w_matrix * glm::vec4(1.0f, -1.0f, 1.0f, 1.0);//top right
	
	near1 /= near1.w;
	near2 /= near2.w;
	far1 /= far1.w;
	far2 /= far2.w;
	
	//can probably be calculated more easily with aspect and fov only but I have no idea how to do that
	Sphere sphere = find_center_of_sphere(near1, near2, far1, far2);
	return Camera( 
		sphere.center,
		forward * (sphere.range + 1000.0f), 
		glm::vec3(0.0f, 1.0f, 0.0f), 
		-1.0f * sphere.range, sphere.range,
		sphere.range, -1.0f * sphere.range, 
		0.0f, 
		1000.0f + (2.0f * sphere.range));
}