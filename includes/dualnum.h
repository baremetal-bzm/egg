#pragma once
//#include <iostream>
#include <glad/glad.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include <vector>
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
			return dual2(v1.real + v2.real, glm::vec2(v1.D.x + v2.D.x, v1.D.y + v2.D.y ));
		}
		friend dual2 operator+(const dual2 v1, const float f1) {
			return dual2(v1.real + f1, glm::vec2(v1.D.x, v1.D.y));
		}
		friend dual2 operator+(const float f1, const dual2 v1) {
			return dual2(v1.real + f1, glm::vec2(v1.D.x, v1.D.y));
		}
		friend dual2 operator-(const dual2 v1, const dual2 v2) {
			return dual2(v1.real - v2.real, glm::vec2(v1.D.x - v2.D.x, v1.D.y - v2.D.y ));
		}
		friend dual2 operator*(const dual2 v1, const dual2 v2) {
			return dual2(v1.real * v2.real, glm::vec2(v1.real * v2.D.x + v1.D.x * v2.real, v1.real * v2.D.y + v1.D.y * v2.real ));
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
			return dual2(-v1.real, glm::vec2(-v1.D.x , -v1.D.y));
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
		std::vector<unsigned int> Indices; //remember to add to constructor if you decide to use this

		//constructor
		DualSurface(int divisionCount = 10) : DivisionCount(divisionCount), VAO(0), VBO(0), EBO(0) {}

		void Draw() const {
			if (EBO_in_use) {
				glBindVertexArray(VAO);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
				glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(Indices.size()), GL_UNSIGNED_INT, (void*)0);
				glBindVertexArray(0);
			}
			else
			{
				glBindVertexArray(VAO);
				glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(VertexData.size())); //swap to glDrawElements if using ebo
				glBindVertexArray(0);
			}
		}
		
	protected:
		int DivisionCount;
		int IC = DivisionCount + 2; //iteration count
		// render data 
		unsigned int VBO;
		unsigned int EBO;
		bool EBO_in_use = false;

		virtual void ParamSurface(glm::vec2 UV, glm::vec3& position, glm::vec3& normal, glm::vec3& tangent, glm::vec3& bitangent) { return; }
		virtual void setupIndices() { return; }
		std::vector<SVertex> GenerateSurface()
		{
			int IterationCount = DivisionCount + 2;
			int VertexCount = IterationCount * IterationCount;

			VertexData.resize(VertexCount);

			for (int y = 0; y < IterationCount; y++)
			{
				for (int x = 0; x < IterationCount; x++)
				{
					glm::vec2 UV = glm::vec2(static_cast<float>(x / (DivisionCount + 1.0f)), static_cast<float>(y / (DivisionCount + 1.0f)));
					glm::vec3 P, N, T, B;
					ParamSurface(UV, P, N, T, B);
					int Index = x + y * IterationCount;
					VertexData[Index].Position = P;
					VertexData[Index].Normal = N;
					VertexData[Index].Tangent = T;
					VertexData[Index].Bitangent = B;
					VertexData[Index].TexCoord = UV;
					VertexData[Index].index = Index; //for debugging delete later
					VertexData[Index].deformedFloat = 0.0f;
				}
			}
			return VertexData;
		}

		// initializes all the buffer objects/arrays
		void setupVAO()
		{
			// create buffers/arrays
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);

			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, VertexData.size() * sizeof(SVertex), &VertexData[0], GL_STATIC_DRAW);

			//vertex attribute pointers
			glEnableVertexAttribArray(0); //pos
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SVertex), (void*)0);
			glEnableVertexAttribArray(1); //norm
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SVertex), (void*)offsetof(SVertex, Normal));
			glEnableVertexAttribArray(2); //tan
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(SVertex), (void*)offsetof(SVertex, Tangent));
			glEnableVertexAttribArray(3); //bitan
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(SVertex), (void*)offsetof(SVertex, Bitangent));
			glEnableVertexAttribArray(4); //texcoord
			glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(SVertex), (void*)offsetof(SVertex, TexCoord));
			glEnableVertexAttribArray(5); //debug index
			glVertexAttribIPointer(5, 1, GL_INT, sizeof(SVertex), (void*)offsetof(SVertex, index)); //index for troubleshooting, note diff function for integer index
			glEnableVertexAttribArray(6); //splithalf
			glVertexAttribIPointer(6, 1, GL_INT, sizeof(SVertex), (void*)offsetof(SVertex, SplitHalf)); //splithalf check
			glEnableVertexAttribArray(7); //deofrmed 
			glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(SVertex), (void*)offsetof(SVertex, deformedFloat));
			glBindVertexArray(0);
		}
		void setupEBO()
		{
			glGenBuffers(1, &EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.size() * sizeof(unsigned int), &Indices[0], GL_STATIC_DRAW);
			EBO_in_use = true;
		}
		void updateVBO() {
			//glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, VertexData.size() * sizeof(SVertex), &VertexData[0]);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
	};

	class Sphere : public DualSurface {
	public: 
		std::vector<std::vector<int>> HitArray; //can rewrite as a single std::vector<int> and manually do row*IC + col indexing if performance is ever an issue
		std::vector<glm::ivec3> cpath;

		Sphere(int divisionCount = 10, GLFWwindow* window = nullptr, float r1 = 1.0f) : DualSurface(divisionCount), radius(r1) {
			GenerateSurface();
			setupVAO();
			setupIndices();
			setupEBO();
			if(window != nullptr)
				glfwSetWindowUserPointer(window, reinterpret_cast<void*>(this));
			HitArray.resize(IC, std::vector<int>(IC, 0));
		}
		
		int RaycastNearestVertex(glm::vec3 ray_origin_modelspace, glm::vec3 ray_direction_modelspace) {
			/*first check whether ray hits sphere
			for ||ray_origin + direction*t - sphere_center|| - sphere_radius = 0, sphere_center = 0, radius=dSphere.radius
			let b= dot(direction, (ray_origin - sphere_center)), c = dot((ray_origin - sphere_center), (ray_origin - sphere_center)) - sphere_radius*sphere_radius
			b^2-c<0 = miss, b^2-c>0 intersects and hits two points, b^2-c = 0 hits edge */
			float b = glm::dot(ray_direction_modelspace, ray_origin_modelspace);
			float c = glm::dot(ray_origin_modelspace, ray_origin_modelspace) - radius * radius;
			
			int nearestVertex{}; //just return index
			float minDistanceSq = FLT_MAX;
			float distanceSq;

			if (b * b - c < 0.01f) { return -1; } //arbitary tolerance
						
			for (int i = 0; i < VertexData.size(); i++)
			{
				//make sure only evaluatinv half of sphere facing viewer
				if (glm::dot(ray_direction_modelspace, VertexData[i].Normal) > 0.01) { continue; } //ray dir dot vector normal should be -
				// Calculate vector from ray origin to vertex
				glm::vec3 diff = VertexData[i].Position - ray_origin_modelspace;
				//calculate dot product with ray direction vector
				float d_proj = glm::dot(ray_direction_modelspace, diff);
				if (d_proj <= 0) { distanceSq = glm::dot(diff, diff); } //closest pt is ray origin
				else
				{
					glm::vec3 cross = glm::cross(ray_direction_modelspace, diff);
					distanceSq = glm::dot(cross, cross);
				}
				if (distanceSq < minDistanceSq) { minDistanceSq = distanceSq; nearestVertex = i; }
			}
			return nearestVertex;
		}
		
		void DeformFromVertex(int hit_vertex)
		{
			int hit_vertex_strip = hit_vertex / IC; 
			int hit_vertex_offset = hit_vertex - hit_vertex_strip * IC;
			float crack_scale_factor = 0.0005f;
			int crack_length = IC / 2 + rand() % (IC / 2);

			switch (rand() % 5)
			{
			case 0: // across a single strip
				for (int i = -crack_length/2; i < crack_length/2; i++)
				{
					int strip = hit_vertex_strip;
					int offset = (hit_vertex_offset + i) % IC; //handle wrapping
					offset = (hit_vertex_offset + i < 0 ? offset + IC : offset); //handle negative number modulo since C/C++ do truncated mod not floor mod
					int index = strip * IC + offset;
					////int index = (hit_vertex_strip * IC) + (hit_vertex_offset + i) % IC; // strip(y) + offset(x)
					//VertexData[index].Position -= crack_scale_factor * VertexData[index].Normal;
					////HitArray[floor(index / IC)][index - IC*(index / IC)]++; //need to copy this change to other values
					//HitArray[strip][offset]++;

					if (index < IC) //handle bottom pole
						for (int k = 0;k < IC;k++) {
							VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * (VertexData[k].deformedFloat < 0.01f ? 1.0f : 0.0f); //deformed = 1.0f, not deformed = 0.0f| if not deformed, mult by 1 to subtract inward, if deformed mult by 0 to do nthing
							//VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * static_cast<float>(!VertexData[k].deformed);
							//VertexData[k].deformed = true;
							VertexData[k].deformedFloat = 1.0f;
							HitArray[0][k]++;
						}
					else if (index > IC * IC - IC - 1) { //handle top pole
						for (int k = IC * IC - IC; k < IC * IC; k++) { 
							VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * (VertexData[k].deformedFloat < 0.01f ? 1.0f : 0.0f);
							//VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * static_cast<float>(!VertexData[k].deformed); 
							//VertexData[k].deformed = true;
							VertexData[k].deformedFloat = 1.0f;
						}
						for (int k = 0; k < IC; k++) { HitArray[IC - 1][k]++; }
					}
					else {
						VertexData[index].Position -= crack_scale_factor * VertexData[index].Normal * (VertexData[index].deformedFloat < 0.01f ? 1.0f : 0.0f);
						//VertexData[index].Position -= crack_scale_factor * VertexData[index].Normal * static_cast<float>(!VertexData[index].deformed);
						//VertexData[index].deformed = true;
						VertexData[index].deformedFloat = 1.0f;
						HitArray[strip][offset]++;
					}
				}
				break;

			case 1: // every vertex different strip
				for (int i = -crack_length / 2; i < crack_length / 2; i++)
				{
					int strip = hit_vertex_strip + i;
					int offset = hit_vertex_offset;
					if (hit_vertex_strip + i > IC - 1){
						strip = 2 * IC - 2 - strip; //handle wrapping over top, come back down
						offset = (offset + (IC / 2)) % IC; //horizontal completely wraps, vertical doesnt, so need to come back down shifted by half sphere
					}
					else if (hit_vertex_strip + i < 0){
						strip = abs(hit_vertex_strip + i); //handle wrapping below, come back up
						offset = (offset + (IC / 2)) % IC;
					}
					//int strip = (hit_vertex_strip + i > IC ? (IC - i) : (hit_vertex_strip + i));
					//int offset = (hit_vertex_offset + (hit_vertex_strip + i > IC ? IC / 2 : 0));
					int index = strip * IC + offset;	
					// int index = (hit_vertex_strip + i > IC ? (IC - i)*IC : (hit_vertex_strip + i)*IC) + (hit_vertex_offset + (hit_vertex_strip + i > IC ? IC/2 : 0)); //if it crosses top pole, come back down other side
					//int index = (hit_vertex_strip + i > IC ? (IC - i) * IC + IC / 2 : (hit_vertex_strip + i) * IC) + hit_vertex_offset; // same as above just one line
/*					VertexData[index].Position -= crack_scale_factor * VertexData[index].Normal;
					HitArray[strip][offset]++;	*/			

					if (index < IC) //handle bottom pole
						for (int k = 0;k < IC;k++) {
							VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * (VertexData[k].deformedFloat < 0.01f ? 1.0f : 0.0f);
							//VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * static_cast<float>(!VertexData[k].deformed);
							//VertexData[k].deformed = true;
							VertexData[k].deformedFloat = 1.0f;
							HitArray[0][k]++;
						}
					else if (index > IC * IC - IC - 1) { //handle top pole
						for (int k = IC * IC - IC; k < IC * IC; k++) { 
							VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * (VertexData[k].deformedFloat < 0.01f ? 1.0f : 0.0f);
							//VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * static_cast<float>(!VertexData[k].deformed);
							//VertexData[k].deformed = true;
							VertexData[k].deformedFloat = 1.0f;
						}
						for (int k = 0; k < IC; k++) { HitArray[IC - 1][k]++; }
					}
					else {
						VertexData[index].Position -= crack_scale_factor * VertexData[index].Normal * (VertexData[index].deformedFloat < 0.01f ? 1.0f : 0.0f);
						//VertexData[index].Position -= crack_scale_factor * VertexData[index].Normal * static_cast<float>(!VertexData[index].deformed);
						//VertexData[index].deformed = true;
						VertexData[index].deformedFloat = 1.0f;
						HitArray[strip][offset]++;
					}
				}
				break;

			case 2: // y=x pattern along vertex grid
				for (int i = -crack_length / 2; i < crack_length / 2; i++)
				{
					int offset = (hit_vertex_offset + i) % IC; 
					offset = (hit_vertex_offset + i < 0 ? offset + IC : offset); 
					int strip = hit_vertex_strip + i;
					if (strip > IC - 1) {
						strip = 2 * IC - 2 - strip; //handle wrapping over top, come back down
						offset = (offset + (IC / 2)) % IC; //horizontal completely wraps, vertical doesnt, so need to come back down shifted by half sphere
					}
					else if (strip < 0) {
						strip = abs(strip); //handle wrapping below, come back up
						offset = (offset + (IC / 2)) % IC;
					}
					int index = strip * IC + offset;
					//int index = (hit_vertex_strip + i > IC ? (IC - i) * IC : (hit_vertex_strip + i) * IC) + (hit_vertex_offset + i) % IC;
					//VertexData[index].Position -= crack_scale_factor * VertexData[index].Normal;
					//HitArray[strip][offset]++;

					if (index < IC) //handle bottom pole
						for (int k = 0;k < IC;k++) {
							VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * (VertexData[k].deformedFloat < 0.01f ? 1.0f : 0.0f);
							//VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * static_cast<float>(!VertexData[k].deformed);
							//VertexData[k].deformed = true;
							VertexData[k].deformedFloat = 1.0f;
							HitArray[0][k]++;
						}
					else if (index > IC * IC - IC - 1) { //handle top pole
						for (int k = IC * IC - IC; k < IC * IC; k++) { 
							VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * (VertexData[k].deformedFloat < 0.01f ? 1.0f : 0.0f);
							//VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * static_cast<float>(!VertexData[k].deformed);
							//VertexData[k].deformed = true; 
							VertexData[k].deformedFloat = 1.0f;
						}
						for (int k = 0; k < IC; k++) { HitArray[IC - 1][k]++; }
					}
					else {
						VertexData[index].Position -= crack_scale_factor * VertexData[index].Normal * (VertexData[index].deformedFloat < 0.01f ? 1.0f : 0.0f);
						//VertexData[index].Position -= crack_scale_factor * VertexData[index].Normal * static_cast<float>(!VertexData[index].deformed);
						//VertexData[index].deformed = true;
						VertexData[index].deformedFloat = 1.0f;
						HitArray[strip][offset]++;
					}
				}
				break;

			case 3:										   // two x one y pattern: ..--**
				for (int i = -crack_length / 4; i < crack_length / 4; i++) // half loop range
				{
					int offset1 = (hit_vertex_offset + 2 * i) % IC;
					offset1 = (hit_vertex_offset + 2 * i < 0 ? offset1 + IC : offset1);
					int offset2 = (hit_vertex_offset + (2 * i + 1)) % IC;
					offset2 = (hit_vertex_offset + (2 * i + 1) < 0 ? offset2 + IC : offset2);
					int strip = hit_vertex_strip + i;
					if (strip > IC - 1) {
						strip = 2 * IC - 2 - strip; 
						offset1 = (offset1 + (IC / 2)) % IC; 
						offset2 = (offset2 + (IC / 2)) % IC; 
					}
					else if (strip < 0) {
						strip = abs(strip); 
						offset1 = (offset1 + (IC / 2)) % IC;
						offset2 = (offset2 + (IC / 2)) % IC;
					}
					//int index = strip * IC + offset1;
					//VertexData[index].Position -= crack_scale_factor * VertexData[index].Normal;
					//index = strip * IC + offset2;
					//VertexData[index].Position -= crack_scale_factor * VertexData[index].Normal;
					//HitArray[strip][offset1]++;
					//HitArray[strip][offset2]++;

					int index = strip * IC + offset1;
					if (index < IC) //handle bottom pole
						for (int k = 0;k < IC;k++) {
							VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * (VertexData[k].deformedFloat < 0.01f ? 1.0f : 0.0f);
							//VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * static_cast<float>(!VertexData[k].deformed);
							//VertexData[k].deformed = true;
							VertexData[k].deformedFloat = 1.0f;
							HitArray[0][k]++;
						}
					else if (index > IC * IC - IC - 1) { //handle top pole
						for (int k = IC * IC - IC; k < IC * IC; k++) { 
							VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * (VertexData[k].deformedFloat < 0.01f ? 1.0f : 0.0f);
							//VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * static_cast<float>(!VertexData[k].deformed);
							//VertexData[k].deformed = true;
							VertexData[k].deformedFloat = 1.0f;
						}
						for (int k = 0; k < IC; k++) { HitArray[IC - 1][k]++; }
					}
					else {
						VertexData[index].Position -= crack_scale_factor * VertexData[index].Normal * (VertexData[index].deformedFloat < 0.01f ? 1.0f : 0.0f);
						//VertexData[index].Position -= crack_scale_factor * VertexData[index].Normal * static_cast<float>(!VertexData[index].deformed);
						//VertexData[index].deformed = true;
						VertexData[index].deformedFloat = 1.0f;
						HitArray[strip][offset1]++;
					}
					
					index = strip * IC + offset2;
					if (index < IC) //handle bottom pole
						for (int k = 0;k < IC;k++) {
							VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * (VertexData[k].deformedFloat < 0.01f ? 1.0f : 0.0f);
							//VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * static_cast<float>(!VertexData[k].deformed);
							//VertexData[k].deformed = true;
							VertexData[k].deformedFloat = 1.0f;
							HitArray[0][k]++;
						}
					else if (index > IC * IC - IC - 1) { //handle top pole
						for (int k = IC * IC - IC; k < IC * IC; k++) { 
							VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * (VertexData[k].deformedFloat < 0.01f ? 1.0f : 0.0f);
							//VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * static_cast<float>(!VertexData[k].deformed);
							//VertexData[k].deformed = true;
							VertexData[k].deformedFloat = 1.0f;
						}
						for (int k = 0; k < IC; k++) { HitArray[IC - 1][k]++; }
					}
					else {
						VertexData[index].Position -= crack_scale_factor * VertexData[index].Normal * (VertexData[index].deformedFloat < 0.01f ? 1.0f : 0.0f);
						//VertexData[index].Position -= crack_scale_factor * VertexData[index].Normal * static_cast<float>(!VertexData[index].deformed);
						//VertexData[index].deformed = true;
						VertexData[index].deformedFloat = 1.0f;
						HitArray[strip][offset2]++;
					}
					////int strip = (hit_vertex_strip + i > IC ? (IC - i) * IC : (hit_vertex_strip + i) * IC);
					////int index = strip + (hit_vertex_offset + 2 * i) % IC; // first x
					////VertexData[index].Position -= crack_scale_factor * VertexData[index].Normal;
					//////HitIndices.push_back(index);
					////index = strip + (hit_vertex_offset + (2 * i + 1)) % IC; // second x
					////VertexData[index].Position -= crack_scale_factor * VertexData[index].Normal;
					//////HitIndices.push_back(index);
					////HitArray[index / IC][index - index / IC]++;
				}
				break;

			case 4:										   // one x two y
				for (int i = -crack_length / 4; i < crack_length / 4; i++) // half loop range
				{
					int offset1 = (hit_vertex_offset + i) % IC;
					offset1 = (hit_vertex_offset + i < 0 ? offset1 + IC : offset1);
					int offset2 = offset1;
					int strip1 = hit_vertex_strip + 2 * i;
					int strip2 = hit_vertex_strip + 2 * i + 1;
					if (strip1 > IC - 1) {
						strip1 = 2 * IC - 2 - strip1; 
						offset1 = (offset1 + (IC / 2)) % IC; 
					}
					else if (strip1 < 0) {
						strip1 = abs(strip1); 
						offset1 = (offset1 + (IC / 2)) % IC;
					}					
					if (strip2 > IC - 1) {
						strip2 = 2 * IC - 2 - strip2; 
						offset2 = (offset2 + (IC / 2)) % IC; 
					}
					else if (strip2 < 0) {
						strip2 = abs(strip2); 
						offset2 = (offset2 + (IC / 2)) % IC;
					}

					//int index = strip1 * IC + offset1;
					//VertexData[index].Position -= crack_scale_factor * VertexData[index].Normal;
					//index = strip2 * IC + offset2;
					//VertexData[index].Position -= crack_scale_factor * VertexData[index].Normal;
					//HitArray[strip1][offset1]++;
					//HitArray[strip2][offset2]++;

					////int index = (hit_vertex_strip + 2 * i > IC ? (IC - 2 * i) * IC : (hit_vertex_strip + 2 * i) * IC) + offset; // first y
					////VertexData[index].Position -= crack_scale_factor * VertexData[index].Normal;
					//////HitIndices.push_back(index);
					////index = (hit_vertex_strip + (2 * i + 1) > IC ? (IC - (2 * i + 1)) * IC : (hit_vertex_strip + (2 * i + 1)) * IC) + offset; // second y
					////VertexData[index].Position -= crack_scale_factor * VertexData[index].Normal;
					//////HitIndices.push_back(index);
					////HitArray[index / IC][index - index / IC]++;
					int index = strip1 * IC + offset1;
					if (index < IC) //handle bottom pole
						for (int k = 0;k < IC;k++) {
							VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * (VertexData[k].deformedFloat < 0.01f ? 1.0f : 0.0f);
							//VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * static_cast<float>(!VertexData[k].deformed);
							//VertexData[k].deformed = true;
							VertexData[k].deformedFloat = 1.0f;
							HitArray[0][k]++;
						}
					else if (index > IC * IC - IC - 1) { //handle top pole
						for (int k = IC * IC - IC; k < IC * IC; k++) {
							VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * (VertexData[k].deformedFloat < 0.01f ? 1.0f : 0.0f);
							//VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * static_cast<float>(!VertexData[k].deformed);
							//VertexData[k].deformed = true;
							VertexData[k].deformedFloat = 1.0f;
						}
						for (int k = 0; k < IC; k++) { HitArray[IC - 1][k]++; }
					}
					else {
						VertexData[index].Position -= crack_scale_factor * VertexData[index].Normal * (VertexData[index].deformedFloat < 0.01f ? 1.0f : 0.0f);
						//VertexData[index].Position -= crack_scale_factor * VertexData[index].Normal * static_cast<float>(!VertexData[index].deformed);
						//VertexData[index].deformed = true;
						VertexData[index].deformedFloat = 1.0f;
						HitArray[strip1][offset1]++;
					}
					
					index = strip2 * IC + offset2;
					if (index < IC) //handle bottom pole
						for (int k = 0;k < IC;k++) {
							VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * (VertexData[k].deformedFloat < 0.01f ? 1.0f : 0.0f);
							//VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * static_cast<float>(!VertexData[k].deformed);
							//VertexData[k].deformed = true;
							VertexData[k].deformedFloat = 1.0f;
							HitArray[0][k]++;
						}
					else if (index > IC * IC - IC - 1) { //handle top pole
						for (int k = IC * IC - IC; k < IC * IC; k++) { 
							VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * (VertexData[k].deformedFloat < 0.01f ? 1.0f : 0.0f);
							//VertexData[k].Position -= crack_scale_factor * VertexData[k].Normal * static_cast<float>(!VertexData[k].deformed);
							//VertexData[k].deformed = true;
							VertexData[k].deformedFloat = 1.0f;
						}
						for (int k = 0; k < IC; k++) { HitArray[IC - 1][k]++; }
					}
					else {
						VertexData[index].Position -= crack_scale_factor * VertexData[index].Normal * (VertexData[index].deformedFloat < 0.01f ? 1.0f : 0.0f);
						//VertexData[index].Position -= crack_scale_factor * VertexData[index].Normal * static_cast<float>(!VertexData[index].deformed);
						//VertexData[index].deformed = true;
						VertexData[index].deformedFloat = 1.0f;
						HitArray[strip2][offset2]++;
					}
				}
				break;
			}
			updateVBO();
			return;
		}

		void DFS(int i, int j, std::vector<std::vector<int>>& visited, const std::vector<std::vector<int>>& array,
			int start_row, int start_col, int prev_dir_i, int prev_dir_j, bool& loop, std::vector<glm::ivec3>& path)
		{
			const int min_path_size = IC;

			struct Frame { //store state of visited cell
				int i, j;
				int prev_dir_i, prev_dir_j;
				bool isPole;
				int nextN, nextM;      // resume point for the 3x3 neighbor scan (non-pole)
				int nextM_pole;        // resume point for the all-columns scan (pole)
			};

			std::vector<Frame> stack;
			stack.reserve(IC * IC); // worst case: every cell visited once, should guarantee push_back never reallocates

			auto pushFrame = [&](int fi, int fj, int pdi, int pdj) //frame i, frame j, previous direction i, previous direction j
				{
					bool isPole = (fi == 0 || fi == IC - 1); //handle poles separately
					if (isPole) { for (int k = 0; k < IC; ++k) visited[fi][k] = 1; }//treat pole pts as all visited
					else { visited[fi][fj] = 1; }
					path.push_back(glm::ivec3(fi, fj, 0)); //only one pole point in path
					stack.push_back(Frame{ fi, fj, pdi, pdj, isPole, -1, -1, 0 }); //start indices for next search are n,m = -1, ..., 1; pole goes 0 to ic-1
				};

			pushFrame(i, j, prev_dir_i, prev_dir_j);

			while (!stack.empty())
			{
				Frame& f = stack.back();
				bool advanced = false;

				if (f.isPole)
				{
					int n = (f.i == 0 ? 1 : -1);//n=1 when i=0, n=-1 when i = ic-1
					int adjusted_i = f.i + n;
					while (f.nextM_pole < IC)//check ALL j values when at pole
					{
						int m = f.nextM_pole++;
						if (n == f.prev_dir_i && m == f.prev_dir_j) continue;//skip calling in direciton just came from
						if (adjusted_i == start_row && m == start_col && path.size() > min_path_size) { loop = true; break; }
						if ((adjusted_i == start_row && (start_row == 0 || start_row == IC - 1)) && path.size() > min_path_size) { loop = true; break; } //if at either pole, dont care j value
						if (array[adjusted_i][m] > 0 && visited[adjusted_i][m] == 0) //recurse 
						{
							pushFrame(adjusted_i, m, -n, -m);
							advanced = true;
							break;
						}
					}
				}
				else
				{
					while (!advanced && !loop && f.nextN < 2) //check all adjacent vertices. for n = -1, .. 1
					{
						while (f.nextM < 2) //for m = -1, .. 1
						{
							int n = f.nextN, m = f.nextM;
							f.nextM++;
							int adjusted_i = f.i + n;
							int adjusted_j = (f.j + m) % IC;
							adjusted_j = (adjusted_j < 0 ? adjusted_j + IC : adjusted_j);

							if ((n == 0 && m == 0) || (n == f.prev_dir_i && m == f.prev_dir_j)) continue;//skip calling on self, skip calling in direciton just came from , perhaps unnecessary due to visited array check below
							if (adjusted_i == start_row && adjusted_j == start_col && path.size() > min_path_size) { loop = true; break; }
							if ((adjusted_i == start_row && (start_row == 0 || start_row == IC - 1)) && path.size() > min_path_size) { loop = true; break; }//if at either pole, dont care j value
							if (array[adjusted_i][adjusted_j] > 0 && visited[adjusted_i][adjusted_j] == 0) //recurse
							{
								pushFrame(adjusted_i, adjusted_j, -n, -m);
								advanced = true;
								break;
							}
						}
						if (!advanced && !loop) { f.nextM = -1; f.nextN++; } //reset m loop, increment n loop
					}
				}

				if (loop) break; 

				if (!advanced) //if no cells to visit, pop back
				{
					path.pop_back();
					stack.pop_back();
				}
			}
		}
		bool DFSDriver(){
			std::vector<std::vector<int>> VisitedVertices(IC, std::vector<int>(IC, 0));//can rewrite as a single std::vector<int> and manually do row*IC + col indexing if performance is ever an issue
			bool loopcheck = false;
			for (int i = 0; i < IC; i++) {
				if (loopcheck == true) { return loopcheck; };
				for (int j = 0; j < IC; j++) {
					if (loopcheck == true) { return loopcheck; };
					if (HitArray[i][j] > 1) { 
						DFS(i, j, VisitedVertices, HitArray, i, j, 0, 0, loopcheck, cpath);
						if (loopcheck == false) { 
							cpath.clear(); 
							for (auto& inner_vec : VisitedVertices) {
								std::fill(inner_vec.begin(), inner_vec.end(), 0);
							}
						}
					}
				}
			}
			return loopcheck; 
		}
		
		void EggSeparator(std::vector<glm::ivec3>& path) { //absolutely cursed point in polygon find for wrapped space
			path[0].z = 1; //assume start dir is up
			int bb_max_row = 0; //maximum row/y val for bounding box. path[0].x is the min by construction
			for (size_t i = 1; i < path.size(); ++i) { //set direction info in path vector 
				//xyz holds i,j,direction info. note that i is row and j is column, so its essentially backwards from cartesian coords, meaning i=y j=x
				if (path[i].x - path[i - 1].x > 0)
					path[i].z = 1;
				else if (path[i].x - path[i - 1].x < 0)
					path[i].z = -1;
				if (path[i].z != path[i - 1].z) //set dir to 0 for pivot points
					path[i - 1].z = 0;
				if (path[i].x > bb_max_row)
					bb_max_row = path[i].x;
			}

			for (const auto& point : path) {
				for (int j = 0; j < IC; ++j) {
					VertexData[point.x * IC + j].SplitHalf += abs(point.z) * ((j - point.y) * point.z > -1 ? 1 : -1); //add 1 if to right of point, -1 if to left, do nothing if dir=0
				}
			}
			//check for case when path terminates on both horizontal ends and need to fill in top portion of grid
			if (abs(path.front().y - path.back().y) > 1) {
				int maxval = std::max(path.front().y, path.back().y);
				for (int i = bb_max_row * IC; i < VertexData.size();++i) {
					VertexData[i].SplitHalf = 10; //arbitrary positive value
				}
			}
			//move the halves apart for debugging
			for (auto& vertex : VertexData) {
				vertex.SplitHalf = vertex.SplitHalf > -1 ? 1 : -1; //temp for now
				vertex.Position += static_cast<float>((vertex.SplitHalf > -1 ? 1 : -1)) * glm::vec3(0.5f, 0.5f, 0.5f);
			}
			updateVBO();
			cpath.clear();
		}

	protected:
		float radius;
		
		void ParamSurface(glm::vec2 UV, glm::vec3& position, glm::vec3& normal, glm::vec3& tangent, glm::vec3& bitangent) override
		{
			float theta = 2.0f * PI * UV.x; //azimuthal
			float phi = PI * UV.y; //polar
			dual2 U = dual2(theta, glm::vec2(1.0f, 0.0f));
			dual2 V = dual2(phi, glm::vec2(0.0f, 1.0f));

			dual2 X = radius * sin(V) * cos(U);
			dual2 Y = radius * sin(V) * sin(U);
			dual2 Z = radius * cos(V);

			position = glm::vec3(X.real, Y.real, Z.real);
			tangent = glm::vec3(X.D.x, Y.D.x, Z.D.x);
			if(tangent == glm::vec3(0.0f, 0.0f, 0.0f)) //hack to workaround overloaded dual2 sine not ever spitting out proper tangent for 0,0 angle pair
			{
				tangent = glm::vec3(0.0f, 1.0f, 0.0f);
			}
			else {
				tangent = glm::normalize(tangent);
			}
			bitangent = glm::vec3(X.D.y, Y.D.y, Z.D.y);
			bitangent = glm::normalize(bitangent);
			//explicit normal calculation not needed for sphere centered at origin
			normal = glm::normalize(position);
		}
		void setupIndices() override
		{
			unsigned int IterationCount = static_cast<unsigned int>(DivisionCount) + 2;
			long long size = 6LL * IterationCount * IterationCount - 6LL * IterationCount;
			if (DivisionCount < 0 || size > INT_MAX) return;
			else
			{
			Indices.resize(static_cast<unsigned int>(size));

			for (unsigned int i = 0; i < IterationCount - 1; i++)  //bottom; could also do this as a fan/set of strips/fan but would need 3 draw calls
			{
				Indices[3 * i] = i;
				Indices[3 * i + 1] = i + IterationCount;
				Indices[3 * i + 2] = i + IterationCount + 1;
			}
			Indices[3 * IterationCount - 3] = IterationCount - 1; //need to handle last term separately 
			Indices[3 * IterationCount - 2] = 2 * IterationCount - 1;
			Indices[3 * IterationCount - 1] = IterationCount;	
			//last number wraps back down, (((1+j)*IterationCount) + i+1 > (2+j)*IterationCount - 1 ? j*IterationCount + IterationCount : ((1+j)*IterationCount) + i+1)
			//										  realistically this ^ is an =, hence the - 1
			unsigned int offset = 3 * IterationCount;

			for (unsigned int j = 1; j < IterationCount - 1; j++) //string together everything but top and bottom, j=0 and j=ic-1 are bottom and top done separately
			{
				for (unsigned int i = 0; i < IterationCount - 1; i++) //two sets of tris
				{
					Indices[offset + ((j - 1) * 6 * IterationCount) + (6 * i) + 0] = j * IterationCount + i; //[offset from bottom, offset per strip, adjustment within strip]
					Indices[offset + ((j - 1) * 6 * IterationCount) + (6 * i) + 1] = ((1 + j) * IterationCount) + i;
					Indices[offset + ((j - 1) * 6 * IterationCount) + (6 * i) + 2] = ((1 + j) * IterationCount) + i + 1;
					Indices[offset + ((j - 1) * 6 * IterationCount) + (6 * i) + 3] = j * IterationCount + i;
					Indices[offset + ((j - 1) * 6 * IterationCount) + (6 * i) + 4] = ((j + 1) * IterationCount) + i + 1;
					Indices[offset + ((j - 1) * 6 * IterationCount) + (6 * i) + 5] = j * IterationCount + i + 1;
				}
				//last terms done separately
				Indices[offset + ((j-1)*6*IterationCount) + (6*IterationCount - 6)] = j * IterationCount + (IterationCount - 1);
				Indices[offset + ((j-1)*6*IterationCount) + (6*IterationCount - 5)] = ((1 + j) * IterationCount) + (IterationCount - 1);
				Indices[offset + ((j-1)*6*IterationCount) + (6*IterationCount - 4)] = j * IterationCount + IterationCount; //this term wraps
				Indices[offset + ((j-1)*6*IterationCount) + (6*IterationCount - 3)] = j * IterationCount + (IterationCount - 1);
				Indices[offset + ((j-1)*6*IterationCount) + (6*IterationCount - 2)] = j * IterationCount + IterationCount; //wraps, same as up two lines
				Indices[offset + ((j-1)*6*IterationCount) + (6*IterationCount - 1)] = j * IterationCount; //wraps
			}
			offset = 6 * IterationCount * IterationCount - 9 * IterationCount;

			for (unsigned int i = 0; i < IterationCount - 1; i++) // do top
			{
				Indices[offset + 3 * i] = (IterationCount * IterationCount - 2 * IterationCount) + i; //2nd to top row, first col+i
				Indices[offset + 3 * i + 1] = (IterationCount * IterationCount - 2 * IterationCount + 1) + i; //2nt to top row, 2nd col+i
				Indices[offset + 3 * i + 2] = (IterationCount * IterationCount - IterationCount) + i; //top row, first col+i 
			}
			Indices[6 * IterationCount * IterationCount - 6 * IterationCount - 3] = IterationCount * IterationCount - IterationCount - 1;//last terms
			Indices[6 * IterationCount * IterationCount - 6 * IterationCount - 2] = IterationCount * IterationCount - 2 * IterationCount; //wraps
			Indices[6 * IterationCount * IterationCount - 6 * IterationCount - 1] = IterationCount * IterationCount - 1;
			}
		}
	};
	
	class Torus : public DualSurface {
	public:
		//constructor
		Torus(int divisionCount = 10, float r1 = 2.0f, float r2 = 0.5f) : DualSurface(divisionCount), torusRadius(r1), tubeRadius(r2) {
			GenerateSurface();
			setupVAO();
		}
	protected:
		float torusRadius;
		float tubeRadius;

		void ParamSurface(glm::vec2 UV, glm::vec3& position, glm::vec3& normal, glm::vec3& tangent, glm::vec3& bitangent) override
			{
				float phi = 2.0f * PI * UV.x; //about axis of revolution
				float theta = 2.0f * PI * UV.y; //about the tube/circle being revovled
				dual2 U = dual2(phi, glm::vec2(1.0f, 0.0f));
				dual2 V = dual2(theta, glm::vec2(0.0f, 1.0f));
				//note: theres some freedom in which axes you define your angles off of, different formula will produce same results just with the points shuffled
				//--------about X
				//dual2 X = tubeRadius * sin(V);		//this draws in big loops around the torus
				//dual2 Y = (torusRadius + (cos(V) * tubeRadius)) * sin(U);
				//dual2 Z = (torusRadius + (cos(V) * tubeRadius)) * cos(U);
				//dual2 X = tubeRadius * cos(U);		//this draws each circular cross section sequentially, see also Z below
				//dual2 Y = (torusRadius + (sin(U) * tubeRadius)) * sin(V);
				//dual2 Z = (torusRadius + (sin(U) * tubeRadius)) * cos(V);
				//--------about Y
				//dual2 X = (torusRadius + tubeRadius * cos(V)) * cos(U);
				//dual2 Y = tubeRadius * sin(V);
				//dual2 Z = (torusRadius + tubeRadius * cos(V)) * sin(U);
				//--------about Z
				dual2 X = (torusRadius + (sin(U) * tubeRadius)) * cos(V);
				dual2 Y = (torusRadius + (sin(U) * tubeRadius)) * sin(V);
				dual2 Z = tubeRadius * cos(U);

				position = glm::vec3(X.real, Y.real, Z.real);
				tangent = glm::vec3(X.D.x, Y.D.x, Z.D.x);
				tangent = glm::normalize(tangent);
				bitangent = glm::vec3(X.D.y, Y.D.y, Z.D.y);
				bitangent = glm::normalize(bitangent);
				normal = glm::normalize(glm::cross(tangent, bitangent));
			}
	};
	
	class Cylinder : public DualSurface {
	public:
		//constructor
		Cylinder(int divisionCount = 10, float r1 = 0.5f, float h1 = 3.0f) : DualSurface(divisionCount), radius(r1), length(h1) {
			GenerateSurface();
			setupVAO();
		}
	protected:
		float radius;
		float length;

		void ParamSurface(glm::vec2 UV, glm::vec3& position, glm::vec3& normal, glm::vec3& tangent, glm::vec3& bitangent) override
		{
			float parametricLength =  length * UV.x;
			float theta = 2.0f * PI * UV.y;
			dual2 U = dual2(parametricLength, glm::vec2(1.0f, 0.0f));
			dual2 V = dual2(theta, glm::vec2(0.0f, 1.0f));

			dual2 X = U;
			dual2 Y = radius * cos(V);
			dual2 Z = radius * sin(V);

			position = glm::vec3(X.real, Y.real, Z.real);
			tangent = glm::vec3(X.D.x, Y.D.x, Z.D.x);
			tangent = glm::normalize(tangent);
			bitangent = glm::vec3(X.D.y, Y.D.y, Z.D.y);
			bitangent = glm::normalize(bitangent);
			normal = glm::normalize(glm::cross(tangent, bitangent));
		}
	};

}

