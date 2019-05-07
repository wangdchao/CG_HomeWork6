#ifndef SHADER_H
#define SHADER_H

#include "allheader.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

class shader
{
private:

	unsigned int ID;

	// utility function for checking shader compilation/linking errors.
	// ------------------------------------------------------------------------
	bool checkCompileErrors(GLuint shader, std::string type) const
	{
		GLint success;
		GLchar infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
				return false;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
				return false;
			}
		}
		return true;
	}

public:
	
	// constructor generates the shade
	// ------------------------------------------------------------------------
	shader()=default;

	bool createShaderWithFile(const char* vertexPath, const char* fragmentPath) {
		std::string vertexCode;
		std::string fragmentCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		// ensure ifstream objects can throw exceptions:
		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			// open files
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			std::stringstream vShaderStream, fShaderStream;
			// read file's buffer contents into streams
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			// close file handlers
			vShaderFile.close();
			fShaderFile.close();
			// convert stream into string
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
			return false;
		}
		const char* vShaderCode = vertexCode.c_str();
		const char * fShaderCode = fragmentCode.c_str();
		// 2. compile shaders
		unsigned int vertex, fragment;
		// vertex shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		if (!checkCompileErrors(vertex, "VERTEX")) return false;
		// fragment Shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		if (!checkCompileErrors(fragment, "FRAGMENT")) return false;
		// shader Program
		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		glLinkProgram(ID);
		if (!checkCompileErrors(ID, "PROGRAM")) return false;
		// delete the shaders as they're linked into our program now and no longer necessery
		glDeleteShader(vertex);
		glDeleteShader(fragment);
		printf("create with file success\n");
		return true;
	}

	bool createShaderWithChar(const char* vShaderCode, const char* fShaderCode)
	{
		
		unsigned int vertex, fragment;
		// vertex shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		if (!checkCompileErrors(vertex, "VERTEX")) return false;
		// fragment Shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		if (!checkCompileErrors(fragment, "FRAGMENT")) return false;

		// shader progream
		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);

		glLinkProgram(ID);
		if (!checkCompileErrors(ID, "PROGRAM")) return false;
		// delete the shaders as they're linked into our program now and no longer necessery
		glDeleteShader(vertex);
		glDeleteShader(fragment);
		printf("create with char success\n");
		return true;
	}
 
	// ------------------------------------------------------------------------
	void use()
	{
		glUseProgram(ID);
	}
	//向片段着色器传入颜色
	void setFragmentUniformColor(const ImVec4 &color,const std::string name) {
		int vertexColorLocation = glGetUniformLocation(ID, name.c_str());
		glUniform4f(vertexColorLocation, color.x, color.y, color.z, color.w);
	}
	void setVec3(const std::string &name, const glm::vec3 &value) const
	{
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	//向着色器传入模型矩阵
	void setVertexUniformModel(const glm::mat4 &model, const std::string name) {
		int vertexColorLocation = glGetUniformLocation(ID, name.c_str());
		glUniformMatrix4fv(vertexColorLocation, 1, GL_FALSE, &model[0][0]);
	}
	//向着色器传入View矩阵
	void setVertexUniformView(const glm::mat4 &view, const std::string name) {
		int vertexColorLocation = glGetUniformLocation(ID, name.c_str());
		glUniformMatrix4fv(vertexColorLocation, 1, GL_FALSE, &view[0][0]);
	}
	//向着色器传入透视矩阵
	void setVertexUniformPers(const glm::mat4 &pers,const std::string name) {
		int vertexColorLocation = glGetUniformLocation(ID, name.c_str());
		glUniformMatrix4fv(vertexColorLocation, 1, GL_FALSE, &pers[0][0]);
		
	}
	void setInt(const std::string &name, int value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}
	void setFloat(const std::string &name, float value) const
	{
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}
};
#endif