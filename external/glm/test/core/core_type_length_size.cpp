///////////////////////////////////////////////////////////////////////////////////
/// OpenGL Mathematics (glm.g-truc.net)
///
/// Copyright (c) 2005 - 2015 G-Truc Creation (www.g-truc.net)
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// 
/// Restrictions:
///		By making use of the Software for military purposes, you choose to make
///		a Bunny unhappy.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///
/// @file test/core/core_type_length_size.cpp
/// @date 2011-05-25 / 2014-11-25
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#define GLM_FORCE_SIZE_FUNC
#include <glm/glm.hpp>
#include <glm/gtc/vec1.hpp>

int test_length_mat_non_squared()
{
	int Error = 0;

	Error += glm::mat2x3().size() == 2 ? 0 : 1;
	Error += glm::mat2x4().size() == 2 ? 0 : 1;
	Error += glm::mat3x2().size() == 3 ? 0 : 1;
	Error += glm::mat3x4().size() == 3 ? 0 : 1;
	Error += glm::mat4x2().size() == 4 ? 0 : 1;
	Error += glm::mat4x3().size() == 4 ? 0 : 1;
	
	Error += glm::dmat2x3().size() == 2 ? 0 : 1;
	Error += glm::dmat2x4().size() == 2 ? 0 : 1;
	Error += glm::dmat3x2().size() == 3 ? 0 : 1;
	Error += glm::dmat3x4().size() == 3 ? 0 : 1;
	Error += glm::dmat4x2().size() == 4 ? 0 : 1;
	Error += glm::dmat4x3().size() == 4 ? 0 : 1;
	
	return Error;
}

int test_length_mat()
{
	int Error = 0;
	
	Error += glm::mat2().size() == 2 ? 0 : 1;
	Error += glm::mat3().size() == 3 ? 0 : 1;
	Error += glm::mat4().size() == 4 ? 0 : 1;
	Error += glm::mat2x2().size() == 2 ? 0 : 1;
	Error += glm::mat3x3().size() == 3 ? 0 : 1;
	Error += glm::mat4x4().size() == 4 ? 0 : 1;

	Error += glm::dmat2().size() == 2 ? 0 : 1;
	Error += glm::dmat3().size() == 3 ? 0 : 1;
	Error += glm::dmat4().size() == 4 ? 0 : 1;
	Error += glm::dmat2x2().size() == 2 ? 0 : 1;
	Error += glm::dmat3x3().size() == 3 ? 0 : 1;
	Error += glm::dmat4x4().size() == 4 ? 0 : 1;
	
	return Error;
}

int test_length_vec()
{

	int Error = 0;

	Error += glm::vec1().size() == 1 ? 0 : 1;
	Error += glm::vec2().size() == 2 ? 0 : 1;
	Error += glm::vec3().size() == 3 ? 0 : 1;
	Error += glm::vec4().size() == 4 ? 0 : 1;

	Error += glm::ivec1().size() == 1 ? 0 : 1;
	Error += glm::ivec2().size() == 2 ? 0 : 1;
	Error += glm::ivec3().size() == 3 ? 0 : 1;
	Error += glm::ivec4().size() == 4 ? 0 : 1;

	Error += glm::uvec1().size() == 1 ? 0 : 1;
	Error += glm::uvec2().size() == 2 ? 0 : 1;
	Error += glm::uvec3().size() == 3 ? 0 : 1;
	Error += glm::uvec4().size() == 4 ? 0 : 1;

	Error += glm::dvec1().size() == 1 ? 0 : 1;
	Error += glm::dvec2().size() == 2 ? 0 : 1;
	Error += glm::dvec3().size() == 3 ? 0 : 1;
	Error += glm::dvec4().size() == 4 ? 0 : 1;
	
	return Error;
}

int main()
{
	int Error = 0;
	
	Error += test_length_vec();
	Error += test_length_mat();
	Error += test_length_mat_non_squared();
	
	return Error;
}

