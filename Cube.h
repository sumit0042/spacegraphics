#pragma once
#include<GL/glew.h>     // use this before GLFW
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Camera.h"
#include "Shader.h"

#include <vector>


const int no_rects = 6;


GLuint create_program()
{
	GLuint program = glCreateProgram();
	shaderAttach(program, GL_VERTEX_SHADER, vertexShaderSrc);
	shaderAttach(program, GL_FRAGMENT_SHADER, fragmentShaderSrc);
	glLinkProgram(program);
	glValidateProgram(program);
	return program;
}


void bind_vert_n_texel(GLuint program)
{
	GLuint posAttrib = glGetAttribLocation(program, "pos");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);

	posAttrib = glGetAttribLocation(program, "normal");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

	posAttrib = glGetAttribLocation(program, "texco");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
}

void bind_texture(GLuint program, int texId)
{
	GLint locScale2 = glGetUniformLocation(program, "sampler1");
	glUniform1i(locScale2, texId);
}


struct Vertex
{
	glm::vec3 xyz;
	glm::vec3 nor;
	float tx = 0.0; float ty = 0.0;
};


class Cube
{
public:
	GLuint m_g;
	GLuint program[no_rects];
	std::vector<glm::mat4x3> m_faces;
	Vertex m_c[no_rects][4];
	GLuint m_texId;
	glm::mat4 m_ModelMatrix;
	std::vector<float> m_obj_color;
	std::vector<float> m_lightSrc;


	Cube(float x, float y, float z, int texId)
	{
		m_ModelMatrix = glm::mat4(1.0f);
		m_faces = cube_faces(x, y, z);
		this->create_faces_data();
		m_bind_buffer();
		m_texId = texId;

		for (size_t i = 0; i < no_rects; i++)
		{
			program[i] = create_program();
		}
	}

	~Cube()
	{
		for (size_t i = 0; i < no_rects; i++)
		{
			glDeleteProgram(program[i]);
		}
		glDeleteVertexArrays(1, &m_g);
	}

	void bind_obj_color(std::vector<float> objCol)
	{
		m_obj_color = objCol;
	}

	void bind_static_attribs()
	{
		for (size_t i = 0; i < no_rects; i++)
		{
			glUseProgram(program[i]);
			bind_vert_n_texel(program[i]);
		}
	}

	void m_bind_buffer()
	{
		glGenBuffers(1, &m_g);
		glBindBuffer(GL_ARRAY_BUFFER, this->m_g);
		glBufferData(GL_ARRAY_BUFFER, sizeof(this->m_c), this->m_c, GL_STATIC_DRAW);
	}

	void translate(float x, float y, float z)
	{
		m_ModelMatrix = glm::translate(m_ModelMatrix, glm::vec3(x, y, z));
	}

	void rotate(float angle, float x, float y, float z)
	{
		m_ModelMatrix = glm::rotate(m_ModelMatrix, angle, glm::vec3(x, y, z));
	}

	void draw(Camera camera)
	{
		glBindBuffer(GL_ARRAY_BUFFER, this->m_g);
		bind_static_attribs();

		for (size_t i = 0; i < no_rects; i++)
		{
			glUseProgram(program[i]);

			bind_texture(this->program[i], m_texId);

			auto locL = glGetUniformLocation(program[i], "light");
			std::vector<float> lightC(3);
			lightC = { 1.0f, 1.0f, 1.0f };
			glUniform3fv(locL, 1, &lightC[0]);

			auto locLP = glGetUniformLocation(program[i], "lightPos");
			glUniform3fv(locLP, 1, &m_lightSrc[0]);

			auto locVP = glGetUniformLocation(program[i], "viewPos");
			glUniform3fv(locVP, 1, &camera.Position[0]);

			auto loc = glGetUniformLocation(program[i], "coral");
			glUniform3fv(loc, 1, &m_obj_color[0]);

			auto locM = glGetUniformLocation(program[i], "model");
			glUniformMatrix4fv(locM, 1, GL_FALSE, &m_ModelMatrix[0][0]);

			glm::mat4 view = camera.GetViewMatrix();
			GLint viewLoc = glGetUniformLocation(program[i], "view");
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

			glm::mat4 pro = glm::mat4(1.0f);
			pro = glm::perspective(glm::radians(camera.Zoom), 1280.0f / 720.0f, 0.01f, 100.0f);
			GLint projectionLoc = glGetUniformLocation(program[i], "projection");
			glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(pro));

			glDrawArrays(GL_TRIANGLE_FAN, i * 4, 4);
			glUseProgram(0);
		}
	}

	std::vector<glm::mat4x3> cube_faces(float x, float y, float z)
	{
		std::vector<glm::mat4x3> faces(6);
		glm::mat4x3 frontFace = {
			{-x,-y, z},
			{-x, y, z},
			{ x, y, z},
			{ x,-y, z}
		};

		glm::mat4x3 backFace = {
			{-x,-y,-z},
			{-x, y,-z},
			{ x, y,-z},
			{ x,-y,-z},
		};

		glm::mat4x3 topFace = {
			{ x, y,-z},
			{-x, y,-z},
			{-x, y, z},
			{ x, y, z}
		};

		glm::mat4x3 bottomFace = {
			{-x,-y, z},
			{-x,-y,-z},
			{ x,-y,-z},
			{ x,-y, z},
		};

		glm::mat4x3 rightFace = {
			{ x, y, z},
			{ x, y,-z},
			{ x,-y,-z},
			{ x,-y, z},
		};

		glm::mat4x3 leftFace = {
			{-x, y, z},
			{-x,-y, z},
			{-x,-y,-z},
			{-x, y,-z},
		};

		faces[0] = frontFace;
		faces[1] = backFace;
		faces[2] = leftFace;
		faces[3] = rightFace;
		faces[4] = topFace;
		faces[5] = bottomFace;

		return faces;
	}

	void assign_normals()
	{
		for (size_t i = 0; i < no_rects; i++)
		{
			glm::vec3 normal_xyz;
			switch (i)
			{
			case 0:
				normal_xyz = { 0.0f, 0.0f, 1.0f };
				break;
			case 1:
				normal_xyz = { 0.0f, 0.0f,-1.0f };
				break;
			case 2:
				normal_xyz = { -1.0f, 0.0f, 0.0f };
				break;
			case 3:
				normal_xyz = { 1.0f, 0.0f, 0.0f };
				break;
			case 4:
				normal_xyz = { 0.0f, 1.0f, 0.0f };
				break;
			case 5:
				normal_xyz = { 0.0f,-1.0f, 0.0f };
				break;
			default:
				break;
			}
			for (size_t j = 0; j < 4; j++)
			{
				m_c[i][j].nor = normal_xyz;
			}
		}
	}

	void create_faces_data()
	{
		assign_normals();
		for (size_t i = 0; i < no_rects; i++)
		{
			for (size_t j = 0; j < 4; j++)
			{
				m_c[i][j].xyz[0] = m_faces[i][j].x;
				m_c[i][j].xyz[1] = m_faces[i][j].y;
				m_c[i][j].xyz[2] = m_faces[i][j].z;

				switch (j)
				{
				case 1:
				{
					m_c[i][j].tx = 0.0f;
					m_c[i][j].ty = 0.0f;
					break;
				}
				case 2:
				{
					m_c[i][j].tx = 0.0f;
					m_c[i][j].ty = 1.0f;
					break;
				}
				case 3:
				{
					m_c[i][j].tx = 1.0f;
					m_c[i][j].ty = 1.0f;
					break;
				}
				case 0:
				{
					m_c[i][j].tx = 1.0f;
					m_c[i][j].ty = 0.0f;
					break;
				}
				default:
					break;
				}
			}

		}
	}
};