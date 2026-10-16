#pragma once
//#include <iostream>
#include <glad/glad.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include <vector>
#include <utility>
#include <GLFW/glfw3.h> //do this because we pass a GLFWwindow ptr to the sphere constructor
#include <cmath>
#include <cstddef>

#ifndef PI
#define PI 3.14159265358979323846
#endif

namespace dualnum
{
	template<typename T>
	struct dual
	{
		T real = {}; //real component
		T derivative = {}; //dual/derivative component

		dual() {}
		dual(T _real) : real(std::move(_real)) {}
		dual(T _real, T _derivative) : real(std::move(_real)), derivative(std::move(_derivative)) {}

		friend dual<T> operator+(const dual<T>& v1, const dual<T>& v2) {
			return dual(v1.real + v2.real, v1.derivative + v2.derivative);
		}

		friend dual<T> operator-(const dual<T>& v1, const dual<T>& v2) {
			return dual(v1.real - v2.real, v1.derivative - v2.derivative);
		}

		friend dual<T> operator*(const dual<T>& v1, const dual<T>& v2) {
			return dual(v1.real * v2.real, v1.real * v2.derivative + v2.real * v1.derivative);
		}

		friend dual<T> operator/(const dual<T>& v1, const dual<T>& v2) {
			return dual(v1.real / v2.real, ((v1.derivative * v2.real - v1.real * v2.derivative) / (v2.real * v2.real)));
		}

		friend dual<T> operator-(const dual<T>& v1) {
			return dual(-v1.real, -v1.derivative);
		}

		friend bool operator<(const dual<T>& v1, const dual<T>& v2) {
			return v1.real < v2.real;
		}

		friend bool operator>(const dual<T>& v1, const dual<T>& v2) {
			return v1.real > v2.real;
		}
	};
	template<typename T>
	inline
		dual<T> sin(const dual<T>& in)
	{
		return dual<T>(std::sin(in.real), in.derivative * std::cos(in.real));
	}

	template<typename T>
	inline
		dual<T> cos(const dual<T>& in)
	{
		return dual<T>(std::cos(in.real), -in.derivative * std::sin(in.real));
	}

	template<typename T>
	inline
		dual<T> atan2(const dual<T>& v1, const dual<T>& v2) {
		return dual(std::atan2(v1.real, v2.real), (v1.derivative * v2.real - v1.real * v2.derivative) / (v2.real * v2.real + v1.real * v1.real));
	}

	//template<typename T>
	//inline
	//	dual<T> lambert_w0(const dual<T>& v1)
	//{
	//	return dual<T>(lambert_w0(v1.real), v1.derivative * lambert_w0(v1.real) / (v1.real * lambert_w0(v1.real) + v1.real));
	//}

	struct dual2
	{
		float real;
		glm::vec2 D; // u, v derivative components

		dual2(float s, glm::vec2 Duv) : real(s), D(Duv) {}

		friend dual2 operator+(const dual2 v1, const dual2 v2) {
			return dual2(v1.real + v2.real, glm::vec2(v1.D.x + v2.D.x, v1.D.y + v2.D.y));
		}
		friend dual2 operator+(const dual2 v1, const float f1) {
			return dual2(v1.real + f1, glm::vec2(v1.D.x, v1.D.y));
		}
		friend dual2 operator+(const float f1, const dual2 v1) {
			return dual2(v1.real + f1, glm::vec2(v1.D.x, v1.D.y));
		}
		friend dual2 operator-(const dual2 v1, const dual2 v2) {
			return dual2(v1.real - v2.real, glm::vec2(v1.D.x - v2.D.x, v1.D.y - v2.D.y));
		}
		friend dual2 operator*(const dual2 v1, const dual2 v2) {
			return dual2(v1.real * v2.real, glm::vec2(v1.real * v2.D.x + v1.D.x * v2.real, v1.real * v2.D.y + v1.D.y * v2.real));
		}
		friend dual2 operator*(const dual2 v1, const float s1) {
			return dual2(v1.real * s1, glm::vec2(v1.D.x * s1, v1.D.y * s1));
		}
		friend dual2 operator*(const float s1, const dual2 v1) {
			return dual2(v1.real * s1, glm::vec2(v1.D.x * s1, v1.D.y * s1));
		}
		friend dual2 operator/(const dual2 v1, const dual2 v2) {
			return dual2(v1.real / v2.real, glm::vec2((v1.D.x * v2.real - v1.real * v2.D.x) / (v2.real * v2.real), (v1.D.y * v2.real - v1.real * v2.D.y) / (v2.real * v2.real)));
		}
		friend dual2 operator-(const dual2 v1) {
			return dual2(-v1.real, glm::vec2(-v1.D.x, -v1.D.y));
		}
	};
	inline
		dual2 sin(const dual2 in)
	{
		return dual2(std::sin(in.real), glm::vec2(in.D.x * std::cos(in.real), in.D.y * std::cos(in.real)));
	}
	inline
		dual2 cos(const dual2 in)
	{
		return dual2(std::cos(in.real), glm::vec2(-in.D.x * std::sin(in.real), -in.D.y * std::sin(in.real)));
	}

	struct SVertex
	{
		glm::vec3 Position{ glm::vec3(0.0f) };
		glm::vec3 Normal{ glm::vec3(0.0f) };
		glm::vec3 Tangent{ glm::vec3(0.0f) };
		glm::vec3 Bitangent{ glm::vec3(0.0f) };
		glm::vec2 TexCoord{ glm::vec2(0.0f) };
		int index{ 0 }; //for debugging
		int SplitHalf{ -1 }; //not currently used, might pick it up later
		float deformedFloat{ 0.0f }; //not deformed = 0, deformed =1, needs to be float for interpolating between vertices in some of the frag shaders
	};
	class DualSurface {
	public:
		std::vector<SVertex> VertexData;
		unsigned int VAO;
		std::vector<unsigned int> Indices;
		//constructor
		DualSurface(int divisionCount = 10) : DivisionCount(divisionCount), VAO(0), VBO(0), EBO(0) {}
		virtual ~DualSurface() = default;

		void Draw() const;

	protected:
		int DivisionCount;
		int IC = DivisionCount + 2; //iteration count
		// render data 
		unsigned int VBO;
		unsigned int EBO;
		bool EBO_in_use = false;

		virtual void ParamSurface(glm::vec2 UV, glm::vec3& position, glm::vec3& normal, glm::vec3& tangent, glm::vec3& bitangent) { return; }
		virtual void setupIndices() { return; }
		std::vector<SVertex> GenerateSurface();

		// initializes all the buffer objects/arrays
		void setupVAO();
		void setupEBO();
		void updateVBO();
	};

	class Sphere : public DualSurface {
	public:
		std::vector<std::vector<int>> HitArray; //can rewrite as a single std::vector<int> and manually do row*IC + col indexing if performance is ever an issue
		std::vector<glm::ivec3> cpath;

		Sphere(int divisionCount = 10, GLFWwindow* window = nullptr, float r1 = 1.0f);
		int RaycastNearestVertex(glm::vec3 ray_origin_modelspace, glm::vec3 ray_direction_modelspace);
		void DeformFromVertex(int hit_vertex);
		void DFS(int i, int j, std::vector<std::vector<int>>& visited, const std::vector<std::vector<int>>& array,
			int start_row, int start_col, int prev_dir_i, int prev_dir_j, bool& loop, std::vector<glm::ivec3>& path);
		bool DFSDriver();
		void EggSeparator(std::vector<glm::ivec3>& path);
	protected:
		float radius;

		void ParamSurface(glm::vec2 UV, glm::vec3& position, glm::vec3& normal, glm::vec3& tangent, glm::vec3& bitangent) override;
		void setupIndices() override;
	};
	class Torus : public DualSurface {
	public:
		Torus(int divisionCount = 10, float r1 = 2.0f, float r2 = 0.5f);
	protected:
		float torusRadius;
		float tubeRadius;

		void ParamSurface(glm::vec2 UV, glm::vec3& position, glm::vec3& normal, glm::vec3& tangent, glm::vec3& bitangent) override;
	};
	class Cylinder : public DualSurface {
	public:
		Cylinder(int divisionCount = 10, float r1 = 0.5f, float h1 = 3.0f);

	protected:
		float radius;
		float length;

		void ParamSurface(glm::vec2 UV, glm::vec3& position, glm::vec3& normal, glm::vec3& tangent, glm::vec3& bitangent) override;
	};
};