// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#include "SymbolTable.h"
#include "ParseHelper.h"

#include "InitializeDll.h"

#include "../../include/hlsl2glsl.h"
#include "Initialize.h"
#include "../GLSLCodeGen/hlslSupportLib.h"

#include "../GLSLCodeGen/hlslCrossCompiler.h"
#include "../GLSLCodeGen/hlslLinker.h"


// A symbol table for each language.  Each has a different set of built-ins, and we want to preserve that 
// from compile to compile.
TSymbolTable SymbolTables[EShLangCount];


// Global pool allocator (per process)
TPoolAllocator* PerProcessGPA = 0;


// add support for non square matrix
static void initFullSource(std::string &fullSource)
{
	{
	fullSource = 
	"//-----------------float--------------------\n\
	struct float2x3\n\
	{\n\
		float3 col0;\n\
		float3 col1;\n\
	};\n\
	\n\
	struct float3x2\n\
	{\n\
		float2 col0;\n\
		float2 col1;\n\
		float2 col2;\n\
	};\n\
	\n\
	struct float3x4\n\
	{\n\
		float4 col0;\n\
		float4 col1;\n\
		float4 col2;\n\
	};\n\
	\n\
	struct float4x3\n\
	{\n\
		float3 col0;\n\
		float3 col1;\n\
		float3 col2;\n\
		float3 col3;\n\
	};\n\
	\n\
	struct float2x4\n\
	{\n\
		float4 col0;\n\
		float4 col1;\n\
	};\n\
	\n\
	struct float4x2\n\
	{\n\
		float2 col0;\n\
		float2 col1;\n\
		float2 col2;\n\
		float2 col3;\n\
	};\n\
	//-----------conversion--------------------\n\
	float2x2 __constructfloat2x2(float2x3 m)\n\
	{\n\
		return float2x2( float2( m.col0), float2( m.col1));\n\
	}\n\
	float2x2 __constructfloat2x2(float3x2 m)\n\
	{\n\
		return float2x2( float2( m.col0), float2( m.col1));\n\
	}\n\
	float2x2 __constructfloat2x2(float3x4 m)\n\
	{\n\
		return float2x2( float2( m.col0), float2( m.col1));\n\
	}\n\
	float2x2 __constructfloat2x2(float4x3 m)\n\
	{\n\
		return float2x2( float2( m.col0), float2( m.col1));\n\
	}\n\
	float2x2 __constructfloat2x2(float2x4 m)\n\
	{\n\
		return float2x2( float2( m.col0), float2( m.col1));\n\
	}\n\
	float2x2 __constructfloat2x2(float4x2 m)\n\
	{\n\
		return float2x2( float2( m.col0), float2( m.col1));\n\
	}\n\
	\n\
	float3x3 __constructfloat3x3(float3x4 m)\n\
	{\n\
		return float3x3( float3( m.col0), float3( m.col1), float3( m.col2));\n\
	}\n\
	float3x3 __constructfloat3x3(float4x3 m)\n\
	{\n\
		return float3x3( float3( m.col0), float3( m.col1), float3( m.col2));\n\
	}\n\
	float2x3 __constructfloat2x3(float s)\n\
	{\n\
		return float2x3( float3( s), float3( s));\n\
	}\n\
	float2x3 __constructfloat2x3(float s00, float s01, float s02, \n\
								float s10, float s11, float s12 )\n\
	{\n\
		return float2x3( float3( s00, s01, s02),\n\
						 float3(s10, s11, s12));\n\
	}\n\
	\n\
	float2x3 __constructfloat2x3(float3x3 m )\n\
	{\n\
		return float2x3( float3( m[0]),\n\
						 float3( m[1]));\n\
	}\n\
	float2x3 __constructfloat2x3(float2x3 m )\n\
	{\n\
		return float2x3( float3( m[0]),\n\
						 float3( m[1]));\n\
	}\n\
	\n\
	\n\
	float2x3 __constructfloat2x3(float3x4 m )\n\
	{\n\
		return float2x3( float3( m.col0),\n\
						 float3( m.col1));\n\
	}\n\
	\n\
	\n\
	float2x3 __constructfloat2x3(float4x3 m )\n\
	{\n\
		return float2x3( float3( m.col0),\n\
						 float3( m.col1));\n\
	}\n\
	float2x3 __constructfloat2x3(float4x4 m )\n\
	{\n\
		return float2x3( float3( m[0]),\n\
						 float3( m[1]));\n\
	}\n\
	\n\
	float3x2 __constructfloat3x2(float s)\n\
	{\n\
		return float3x2( float2( s), float2( s), float2(s));\n\
	}\n\
	float3x2 __constructfloat3x2(float s00, float s01, \n\
								float s10, float s11, \n\
								float s20, float s21 )\n\
	{\n\
		return float3x2( float2( s00, s01),\n\
						 float2(s10, s11), \n\
						 float2(s20, s21));\n\
	}\n\
	\n\
	float3x2 __constructfloat3x2(float3x3 m )\n\
	{\n\
		return float3x2( float2( m[0]),\n\
						 float2( m[1]),\n\
						 float2( m[2]));\n\
	}\n\
	float3x2 __constructfloat3x2(float3x2 m )\n\
	{\n\
		return float3x2( float2( m[0]),\n\
						 float2( m[1]),\n\
						 float2( m[2]));\n\
	}\n\
	\n\
	\n\
	float3x2 __constructfloat3x2(float3x4 m )\n\
	{\n\
		return float3x2( float2( m.col0),\n\
						 float2( m.col1),\n\
						 float2( m.col2));\n\
	}\n\
	\n\
	\n\
	float3x2 __constructfloat3x2(float4x3 m )\n\
	{\n\
		return float3x2( float2( m.col0),\n\
						 float2( m.col1),\n\
						 float2( m.col2));\n\
	}\n\
	float3x2 __constructfloat3x2(float4x4 m )\n\
	{\n\
		return float3x2( float2( m[0]),\n\
						 float2( m[1]),\n\
						 float2( m[2]));\n\
	}\n\
	\n\
	float3x4 __constructfloat3x4(float s)\n\
	{\n\
		return float3x4( float4( s), float4( s), float4(s));\n\
	}\n\
	float3x4 __constructfloat3x4(float s00, float s01, float s02, float s03,\n\
								float s10, float s11, float s12, float s13,\n\
								float s20, float s21, float s22, float s23)\n\
	{\n\
		return float3x4( float4( s00, s01, s02, s03),\n\
						 float4(s10, s11, s12, s13),\n\
						 float4(s20, s21, s22, s23));\n\
	}\n\
	\n\
	float3x4 __constructfloat3x4(float3x4 m )\n\
	{\n\
		return float3x4( float4( m[0]),\n\
						 float4( m[1]),\n\
						 float4( m[2]));\n\
	}\n\
	float3x4 __constructfloat3x4(float4x4 m )\n\
	{\n\
		return float3x4( float4( m[0]),\n\
						 float4( m[1]),\n\
						 float4( m[2]));\n\
	}\n\
	\n\
	float4x3 __constructfloat4x3(float s)\n\
	{\n\
		return float4x3( float3( s), float3( s), float3(s), float3(s));\n\
	}\n\
	\n\
	float4x3 __constructfloat4x3(float s00, float s01, float s02, \n\
								float s10, float s11, float s12, \n\
								float s20, float s21, float s22, \n\
								float s30, float s31, float s32 )\n\
	{\n\
		return float4x3( float3( s00, s01, s02),\n\
						 float3(s10, s11, s12),\n\
						 float3(s20, s21, s22), \n\
						 float3(s30, s31, s32));\n\
	}\n\
	\n\
	float4x3 __constructfloat4x3(float4x3 m )\n\
	{\n\
		return float4x3( float3( m[0]),\n\
						 float3( m[1]),\n\
						 float3( m[2]),\n\
						 float3( m[3]));\n\
	}\n\
	float4x3 __constructfloat4x3(float4x4 m )\n\
	{\n\
		return float4x3( float3( m[0]),\n\
						 float3( m[1]),\n\
						 float3( m[2]),\n\
						 float3( m[3]));\n\
	}\n\
	\n\
	float2x4 __constructfloat2x4(float s)\n\
	{\n\
		return float2x4( float4( s), float4( s));\n\
	}\n\
	float2x4 __constructfloat2x4(float s00, float s01, float s02, float s03,\n\
								float s10, float s11, float s12, float s13)\n\
	{\n\
		return float2x4( float4( s00, s01, s02, s03),\n\
						 float4(s10, s11, s12, s13));\n\
	}\n\
	\n\
	float2x4 __constructfloat2x4(float2x4 m )\n\
	{\n\
		return float2x4( float4( m[0]),\n\
						 float4( m[1]));\n\
	}\n\
	float2x4 __constructfloat2x4(float3x4 m )\n\
	{\n\
		return float2x4( float4( m[0]),\n\
						 float4( m[1]));\n\
	}\n\
	\n\
	float2x4 __constructfloat2x4(float4x4 m )\n\
	{\n\
		return float2x4( float4( m[0]),\n\
						 float4( m[1]));\n\
	}\n\
	\n\
	float4x2 __constructfloat4x2(float s)\n\
	{\n\
		return float4x2( float2( s), float2( s), float2(s), float2(s));\n\
	}\n\
	\n\
	float4x2 __constructfloat4x2(float s00, float s01, \n\
								float s10, float s11, \n\
								float s20, float s21, \n\
								float s30, float s31 )\n\
	{\n\
		return float4x2( float2( s00, s01),\n\
						 float2(s10, s11),\n\
						 float2(s20, s21), \n\
						 float2(s30, s31));\n\
	}\n\
	\n\
	float4x2 __constructfloat4x2(float4x2 m )\n\
	{\n\
		return float4x2( float2( m[0]),\n\
						 float2( m[1]),\n\
						 float2( m[2]),\n\
						 float2( m[3]));\n\
	}\n\
	float4x2 __constructfloat4x2(float4x3 m )\n\
	{\n\
		return float4x2( float2( m[0]),\n\
						 float2( m[1]),\n\
						 float2( m[2]),\n\
						 float2( m[3]));\n\
	}\n\
	\n\
	float4x2 __constructfloat4x2(float4x4 m )\n\
	{\n\
		return float4x2( float2( m[0]),\n\
						 float2( m[1]),\n\
						 float2( m[2]),\n\
						 float2( m[3]));\n\
	}\n\
	\n";
	}

	{
	fullSource += 
	"//---------get matrix rows-----------------\n\
	void getRows(float3x4 m, out float3 row[4])\n\
	{\n\
		row[0] = float3(m.col0.x, m.col1.x, m.col2.x );\n\
		row[1] = float3(m.col0.y, m.col1.y, m.col2.y );\n\
		row[2] = float3(m.col0.z, m.col1.z, m.col2.y );\n\
		row[3] = float3(m.col0.w, m.col1.w, m.col2.w );\n\
	}\n\
	void getRows(float4x4 m, out float4 row[4])\n\
	{\n\
		row[0] = float4(m[0].x, m[1].x, m[2].x, m[3].x );\n\
		row[1] = float4(m[0].y, m[1].y, m[2].y, m[3].y );\n\
		row[2] = float4(m[0].z, m[1].z, m[2].y, m[3].z );\n\
		row[3] = float4(m[0].w, m[1].w, m[2].w, m[3].w );\n\
	}\n\
	void getRows(float2x4 m, out float2 row[4])\n\
	{\n\
		row[0] = float2(m.col0.x, m.col1.x );\n\
		row[1] = float2(m.col0.y, m.col1.y );\n\
		row[2] = float2(m.col0.z, m.col1.z );\n\
		row[3] = float2(m.col0.w, m.col1.w );\n\
	}\n\
	void getRows(float3x3 m, out float3 row[3])\n\
	{\n\
		row[0] = float3(m[0].x, m[1].x, m[2].x );\n\
		row[1] = float3(m[0].y, m[1].y, m[2].y );\n\
		row[2] = float3(m[0].z, m[1].z, m[2].y );\n\
	}\n\
	void getRows(float4x3 m, out float4 row[3])\n\
	{\n\
		row[0] = float4(m.col0.x, m.col1.x, m.col2.x, m.col3.x );\n\
		row[1] = float4(m.col0.y, m.col1.y, m.col2.y, m.col3.y );\n\
		row[2] = float4(m.col0.z, m.col1.z, m.col2.z, m.col3.z );\n\
	}\n\
	void getRows(float2x3 m, out float2 row[3])\n\
	{\n\
		row[0] = float2(m.col0.x, m.col1.x );\n\
		row[1] = float2(m.col0.y, m.col1.y );\n\
		row[2] = float2(m.col0.z, m.col1.z );\n\
	}\n\
	void getRows(float4x2 m, out float4 row[2])\n\
	{\n\
		row[0] = float4(m.col0.x, m.col1.x, m.col2.x, m.col3.x );\n\
		row[1] = float4(m.col0.y, m.col1.y, m.col2.y, m.col3.y );\n\
	}\n\
	void getRows(float2x2 m, out float2 row[2])\n\
	{\n\
		row[0] = float2(m[0].x, m[1].x );\n\
		row[1] = float2(m[0].y, m[1].y );\n\
	}\n\
	void getRows(float3x2 m, out float3 row[2])\n\
	{\n\
		row[0] = float3(m.col0.x, m.col1.x, m.col2.x );\n\
		row[1] = float3(m.col0.y, m.col1.y, m.col2.y );\n\
	}\n\
	//---------scalar mul matrix---------------\n\
	float2x3 __mulComp(float f, float2x3 m)\n\
	{\n\
		return float2x3(m.col0 * f, m.col1 * f);\n\
	}\n\
	float3x2 __mulComp(float f, float3x2 m)\n\
	{\n\
		return float3x2(m.col0 * f, m.col1 * f, m.col2 * f);\n\
	}\n\
	float3x4 __mulComp(float f, float3x4 m)\n\
	{\n\
		return float3x4(m.col0 * f, m.col1 * f, m.col2 * f);\n\
	}\n\
	float4x3 __mulComp(float f, float4x3 m)\n\
	{\n\
		return float4x3(m.col0 * f, m.col1 * f, m.col2 * f, m.col3 * f);\n\
	}\n\
	float2x4 __mulComp(float f, float2x4 m)\n\
	{\n\
		return float2x4(m.col0 * f, m.col1 * f);\n\
	}\n\
	float4x2 __mulComp(float f, float4x2 m)\n\
	{\n\
		return float4x2(m.col0 * f, m.col1 * f, m.col2 * f, m.col3 * f);\n\
	}\n\
	//---------matrix mul scalar---------------\n\
	float2x3 __mulComp(float2x3 m, float f)\n\
	{\n\
		return float2x3(m.col0 * f, m.col1 * f);\n\
	}\n\
	float3x2 __mulComp(float3x2 m, float f)\n\
	{\n\
		return float3x2(m.col0 * f, m.col1 * f, m.col2 * f);\n\
	}\n\
	float3x4 __mulComp(float3x4 m, float f)\n\
	{\n\
		return float3x4(m.col0 * f, m.col1 * f, m.col2 * f);\n\
	}\n\
	float4x3 __mulComp(float4x3 m, float f)\n\
	{\n\
		return float4x3(m.col0 * f, m.col1 * f, m.col2 * f, m.col3 * f);\n\
	}\n\
	float2x4 __mulComp(float2x4 m, float f)\n\
	{\n\
		return float2x4(m.col0 * f, m.col1 * f);\n\
	}\n\
	float4x2 __mulComp(float4x2 m, float f)\n\
	{\n\
		return float4x2(m.col0 * f, m.col1 * f, m.col2 * f, m.col3 * f);\n\
	}\n\
	//---------matrix add scalar---------------\n\
	float2x3 __addComp(float2x3 m, float f)\n\
	{\n\
		return float2x3(m.col0 + f, m.col1 + f);\n\
	}\n\
	float3x2 __addComp(float3x2 m, float f)\n\
	{\n\
		return float3x2(m.col0 + f, m.col1 + f, m.col2 + f);\n\
	}\n\
	float3x4 __addComp(float3x4 m, float f)\n\
	{\n\
		return float3x4(m.col0 + f, m.col1 + f, m.col2 + f);\n\
	}\n\
	float4x3 __addComp(float4x3 m, float f)\n\
	{\n\
		return float4x3(m.col0 + f, m.col1 + f, m.col2 + f, m.col3 + f);\n\
	}\n\
	float2x4 __addComp(float2x4 m, float f)\n\
	{\n\
		return float2x4(m.col0 + f, m.col1 + f);\n\
	}\n\
	float4x2 __addComp(float4x2 m, float f)\n\
	{\n\
		return float4x2(m.col0 + f, m.col1 + f, m.col2 + f, m.col3 + f);\n\
	}\n\
	//---------matrix add matrix component wise---------------\n\
	float2x3 __addComp(float2x3 m, float2x3 m2)\n\
	{\n\
		return float2x3(m.col0 + m2.col0, m.col1 + m2.col1);\n\
	}\n\
	float3x2 __addComp(float3x2 m, float3x2 m2)\n\
	{\n\
		return float3x2(m.col0 + m2.col0, m.col1 + m2.col1, m.col2 + m2.col2);\n\
	}\n\
	float3x4 __addComp(float3x4 m, float3x4 m2)\n\
	{\n\
		return float3x4(m.col0 + m2.col0, m.col1 + m2.col1, m.col2 + m2.col2);\n\
	}\n\
	float4x3 __addComp(float4x3 m, float4x3 m2)\n\
	{\n\
		return float4x3(m.col0 + m2.col0, m.col1 + m2.col1, m.col2 + m2.col2, m.col3 + m2.col3);\n\
	}\n\
	float2x4 __addComp(float2x4 m, float2x4 m2)\n\
	{\n\
		return float2x4(m.col0 + m2.col0, m.col1 + m2.col1);\n\
	}\n\
	float4x2 __addComp(float4x2 m, float4x2 m2)\n\
	{\n\
		return float4x2(m.col0 + m2.col0, m.col1 + m2.col1, m.col2 + m2.col2, m.col3 + m2.col3);\n\
	}\n\
	\n\
	//---------matrix sub matrix component wise---------------\n\
	float2x3 __subComp(float2x3 m, float2x3 m2)\n\
	{\n\
		return float2x3(m.col0 - m2.col0, m.col1 - m2.col1);\n\
	}\n\
	float3x2 __subComp(float3x2 m, float3x2 m2)\n\
	{\n\
		return float3x2(m.col0 - m2.col0, m.col1 - m2.col1, m.col2 - m2.col2);\n\
	}\n\
	float3x4 __subComp(float3x4 m, float3x4 m2)\n\
	{\n\
		return float3x4(m.col0 - m2.col0, m.col1 - m2.col1, m.col2 - m2.col2);\n\
	}\n\
	float4x3 __subComp(float4x3 m, float4x3 m2)\n\
	{\n\
		return float4x3(m.col0 - m2.col0, m.col1 - m2.col1, m.col2 - m2.col2, m.col3 - m2.col3);\n\
	}\n\
	float2x4 __subComp(float2x4 m, float2x4 m2)\n\
	{\n\
		return float2x4(m.col0 - m2.col0, m.col1 - m2.col1);\n\
	}\n\
	float4x2 __subComp(float4x2 m, float4x2 m2)\n\
	{\n\
		return float4x2(m.col0 - m2.col0, m.col1 - m2.col1, m.col2 - m2.col2, m.col3 - m2.col3);\n\
	}\n\
	\n\
	//-----------matrix mul matrix component wise--------------\n\
	float2x3 __mulComp(float2x3 m, float2x3 m2)\n\
	{\n\
		return float2x3(m.col0 * m2.col0, m.col1 * m2.col1);\n\
	}\n\
	float3x2 __mulComp(float3x2 m, float3x2 m2)\n\
	{\n\
		return float3x2(m.col0 * m2.col0, m.col1 * m2.col1, m.col2 * m2.col2);\n\
	}\n\
	float3x4 __mulComp(float3x4 m, float3x4 m2)\n\
	{\n\
		return float3x4(m.col0 * m2.col0, m.col1 * m2.col1, m.col2 * m2.col2);\n\
	}\n\
	float4x3 __mulComp(float4x3 m, float4x3 m2)\n\
	{\n\
		return float4x3(m.col0 * m2.col0, m.col1 * m2.col1, m.col2 * m2.col2, m.col3 * m2.col3);\n\
	}\n\
	float2x4 __mulComp(float2x4 m, float2x4 m2)\n\
	{\n\
		return float2x4(m.col0 * m2.col0, m.col1 * m2.col1);\n\
	}\n\
	float4x2 __mulComp(float4x2 m, float4x2 m2)\n\
	{\n\
		return float4x2(m.col0 * m2.col0, m.col1 * m2.col1, m.col2 * m2.col2, m.col3 * m2.col3);\n\
	}\n\
	\n\
	//---------matrix div matrix component wise---------------\n\
	float2x3 __divComp(float2x3 m, float2x3 m2)\n\
	{\n\
		return float2x3(m.col0 / m2.col0, m.col1 / m2.col1);\n\
	}\n\
	float3x2 __divComp(float3x2 m, float3x2 m2)\n\
	{\n\
		return float3x2(m.col0 / m2.col0, m.col1 / m2.col1, m.col2 / m2.col2);\n\
	}\n\
	float3x4 __divComp(float3x4 m, float3x4 m2)\n\
	{\n\
		return float3x4(m.col0 / m2.col0, m.col1 / m2.col1, m.col2 / m2.col2);\n\
	}\n\
	float4x3 __divComp(float4x3 m, float4x3 m2)\n\
	{\n\
		return float4x3(m.col0 / m2.col0, m.col1 / m2.col1, m.col2 / m2.col2, m.col3 / m2.col3);\n\
	}\n\
	float2x4 __divComp(float2x4 m, float2x4 m2)\n\
	{\n\
		return float2x4(m.col0 / m2.col0, m.col1 / m2.col1);\n\
	}\n\
	float4x2 __divComp(float4x2 m, float4x2 m2)\n\
	{\n\
		return float4x2(m.col0 / m2.col0, m.col1 / m2.col1, m.col2 / m2.col2, m.col3 / m2.col3);\n\
	}\n\
	\n\
	//---------matrix mul vector---------------\n\
	float2 mul(float2x3 m, float3 v)\n\
	{\n\
		float2 vec;\n\
		vec.x = dot(m.col0, v);\n\
		vec.y = dot(m.col1, v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	float3 mul(float3x2 m, float2 v)\n\
	{\n\
		float3 vec;\n\
		vec.x = dot(m.col0, v);\n\
		vec.y = dot(m.col1, v);\n\
		vec.z = dot(m.col2, v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	float3 mul(float3x4 m, float4 v)\n\
	{\n\
		float3 vec;\n\
		vec.x = dot(m.col0, v);\n\
		vec.y = dot(m.col1, v);\n\
		vec.z = dot(m.col2, v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	float4 mul(float4x3 m, float3 v)\n\
	{\n\
		float4 vec;\n\
		vec.x = dot(m.col0, v);\n\
		vec.y = dot(m.col1, v);\n\
		vec.z = dot(m.col2, v);\n\
		vec.w = dot(m.col3, v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	float2 mul(float2x4 m, float4 v)\n\
	{\n\
		float2 vec;\n\
		vec.x = dot(m.col0, v);\n\
		vec.y = dot(m.col1, v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	float4 mul(float4x2 m, float2 v)\n\
	{\n\
		float4 vec;\n\
		vec.x = dot(m.col0, v);\n\
		vec.y = dot(m.col1, v);\n\
		vec.z = dot(m.col2, v);\n\
		vec.w = dot(m.col3, v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	";
	}
	{
	fullSource +=
	"//---------vector mul matrix---------------\n\
	float2 mul(float3 v, float3x2 m)\n\
	{\n\
		float2 vec;\n\
		\n\
		float3 row[2];\n\
		getRows(m, row);\n\
		\n\
		vec.x = dot(v, row[0]);\n\
		vec.y = dot(v, row[1]);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	float3 mul(float2 v, float2x3 m)\n\
	{\n\
		float3 vec;\n\
		\n\
		float2 row[3];\n\
		getRows(m, row);\n\
		\n\
		vec.x = dot(row[0], v);\n\
		vec.y = dot(row[1], v);\n\
		vec.z = dot(row[2], v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	float3 mul(float4 v, float4x3 m)\n\
	{\n\
		float3 vec;\n\
		\n\
		float4 row[3];\n\
		getRows(m, row);\n\
		\n\
		vec.x = dot(row[0], v);\n\
		vec.y = dot(row[1], v);\n\
		vec.z = dot(row[2], v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	float4 mul(float3 v, float3x4 m)\n\
	{\n\
		float4 vec;\n\
		\n\
		float3 row[4];\n\
		getRows(m, row);\n\
		\n\
		vec.x = dot(row[0], v);\n\
		vec.y = dot(row[1], v);\n\
		vec.z = dot(row[2], v);\n\
		vec.w = dot(row[3], v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	float2 mul(float4 v, float4x2 m)\n\
	{\n\
		float2 vec;\n\
		\n\
		float4 row[2];\n\
		getRows(m, row);\n\
		\n\
		vec.x = dot(row[0], v);\n\
		vec.y = dot(row[1], v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	float4 mul(float2 v, float2x4 m)\n\
	{\n\
		float4 vec;\n\
		\n\
		float2 row[4];\n\
		getRows(m, row);\n\
		\n\
		vec.x = dot(row[0], v);\n\
		vec.y = dot(row[1], v);\n\
		vec.z = dot(row[2], v);\n\
		vec.w = dot(row[3], v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	//-----------transpose----------------\n\
	\n\
	float2x3 transpose (float3x2 m)\n\
	{\n\
		float2x3 result ;\n\
		\n\
		result.col0 = float3(m.col0.x, m.col1.x, m.col2.x );\n\
		result.col1 = float3(m.col0.y, m.col1.y, m.col2.y );\n\
		\n\
		return result;\n\
	}\n\
	\n\
	float3x2 transpose (float2x3 m)\n\
	{\n\
		float3x2 result ;\n\
		\n\
		result.col0 = float2(m.col0.x, m.col1.x );\n\
		result.col1 = float2(m.col0.y, m.col1.y);\n\
		result.col2 = float2(m.col0.z, m.col1.z);\n\
		\n\
		return result;\n\
	}\n\
	\n\
	float3x4 transpose (float4x3 m)\n\
	{\n\
		float3x4 result ;\n\
		\n\
		result.col0 = float4(m.col0.x, m.col1.x, m.col2.x, m.col3.x );\n\
		result.col1 = float4(m.col0.y, m.col1.y, m.col2.y, m.col3.y );\n\
		result.col2 = float4(m.col0.z, m.col1.z, m.col2.z, m.col3.z );\n\
		\n\
		return result;\n\
	}\n\
	\n\
	float4x3 transpose (float3x4 m)\n\
	{\n\
		float4x3 result ;\n\
		\n\
		result.col0 = float3(m.col0.x, m.col1.x, m.col2.x );\n\
		result.col1 = float3(m.col0.y, m.col1.y, m.col2.y );\n\
		result.col2 = float3(m.col0.z, m.col1.z, m.col2.z );\n\
		result.col3 = float3(m.col0.w, m.col1.w, m.col2.w );\n\
		\n\
		return result;\n\
	}\n\
	\n\
	float2x4 transpose (float4x2 m)\n\
	{\n\
		float2x4 result ;\n\
		\n\
		result.col0 = float4(m.col0.x, m.col1.x, m.col2.x, m.col3.x );\n\
		result.col1 = float4(m.col0.y, m.col1.y, m.col2.y, m.col3.y );\n\
		\n\
		return result;\n\
	}\n\
	\n\
	float4x2 transpose (float2x4 m)\n\
	{\n\
		float4x2 result ;\n\
		\n\
		result.col0 = float2(m.col0.x, m.col1.x );\n\
		result.col1 = float2(m.col0.y, m.col1.y );\n\
		result.col2 = float2(m.col0.z, m.col1.z );\n\
		result.col3 = float2(m.col0.w, m.col1.w );\n\
		\n\
		return result;\n\
	}\n\
	\n\
	//------------matrix multiplication---------\n\
	\n\
	float4x4 mul (float4x3 m1, float3x4 m2)\n\
	{\n\
		float4x4 result ;\n\
		\n\
		float3 row[4]; \n\
		getRows(m2, row);\n\
		\n\
		result[0] = float4( \n\
						dot(m1.col0, row[0] ), \n\
						dot(m1.col0, row[1] ), \n\
						dot(m1.col0, row[2] ),\n\
						dot(m1.col0, row[3] ));\n\
		result[1] = float4( \n\
						dot(m1.col1, row[0] ), \n\
						dot(m1.col1, row[1] ), \n\
						dot(m1.col1, row[2] ),\n\
						dot(m1.col1, row[3] ));\n\
		result[2] = float4( \n\
						dot(m1.col2, row[0] ), \n\
						dot(m1.col2, row[1] ), \n\
						dot(m1.col2, row[2] ),\n\
						dot(m1.col2, row[3] ));\n\
		result[3] = float4( \n\
						dot(m1.col3, row[0] ), \n\
						dot(m1.col3, row[1] ), \n\
						dot(m1.col3, row[2] ),\n\
						dot(m1.col3, row[3] ));\n\
		\n\
		return result;\n\
	}\n\
	\n\
	float3x3 mul (float3x4 m1, float4x3 m2)\n\
	{\n\
		float3x3 result ;\n\
		\n\
		float4 row[3]; \n\
		getRows(m2, row);\n\
		\n\
		result[0] = float3( \n\
						dot(m1.col0, row[0] ), \n\
						dot(m1.col0, row[1] ), \n\
						dot(m1.col0, row[2] ));\n\
		result[1] = float3( \n\
						dot(m1.col1, row[0] ), \n\
						dot(m1.col1, row[1] ), \n\
						dot(m1.col1, row[2] ));\n\
		result[2] = float3( \n\
						dot(m1.col2, row[0] ), \n\
						dot(m1.col2, row[1] ), \n\
						dot(m1.col2, row[2] ));\n\
						\n\
		return result;\n\
	}\n\
	\n\
	float4x4 mul (float4x2 m1, float2x4 m2)\n\
	{\n\
		float4x4 result ;\n\
		\n\
		float2 row[4]; \n\
		getRows(m2, row);\n\
		\n\
		result[0] = float4( \n\
						dot(m1.col0, row[0] ), \n\
						dot(m1.col0, row[1] ), \n\
						dot(m1.col0, row[2] ),\n\
						dot(m1.col0, row[3] ));\n\
		result[1] = float4( \n\
						dot(m1.col1, row[0] ), \n\
						dot(m1.col1, row[1] ), \n\
						dot(m1.col1, row[2] ),\n\
						dot(m1.col1, row[3] ));\n\
		result[2] = float4( \n\
						dot(m1.col2, row[0] ), \n\
						dot(m1.col2, row[1] ), \n\
						dot(m1.col2, row[2] ),\n\
						dot(m1.col2, row[3] ));\n\
		result[3] = float4( \n\
						dot(m1.col3, row[0] ), \n\
						dot(m1.col3, row[1] ), \n\
						dot(m1.col3, row[2] ),\n\
						dot(m1.col3, row[3] ));\n\
		\n\
		return result;\n\
	}\n\
	\n\
	float2x2 mul (float2x4 m1, float4x2 m2)\n\
	{\n\
		float2x2 result ;\n\
		\n\
		float4 row[2]; \n\
		getRows(m2, row);\n\
		\n\
		result[0] = float2( \n\
						dot(m1.col0, row[0] ), \n\
						dot(m1.col0, row[1] ));\n\
		result[1] = float2( \n\
						dot(m1.col1, row[0] ), \n\
						dot(m1.col1, row[1] ));\n\
						\n\
		return result;\n\
	}\n\
	\n\
	float4x3 mul (float4x2 m1, float2x3 m2)\n\
	{\n\
		float4x3 result ;\n\
		\n\
		float2 row[3]; \n\
		getRows(m2, row);\n\
		\n\
		result.col0 = float3( \n\
						dot(m1.col0, row[0] ), \n\
						dot(m1.col0, row[1] ), \n\
						dot(m1.col0, row[2] ));\n\
		result.col1 = float3( \n\
						dot(m1.col1, row[0] ), \n\
						dot(m1.col1, row[1] ), \n\
						dot(m1.col1, row[2] ));\n\
		result.col2 = float3( \n\
						dot(m1.col2, row[0] ), \n\
						dot(m1.col2, row[1] ), \n\
						dot(m1.col2, row[2] ));\n\
		result.col3 = float3( \n\
						dot(m1.col3, row[0] ), \n\
						dot(m1.col3, row[1] ), \n\
						dot(m1.col3, row[2] ));\n\
		\n\
		return result;\n\
	}\n\
	\n\
	float4x2 mul (float4x2 m1, float2x2 m2)\n\
	{\n\
		float4x2 result ;\n\
		\n\
		float2 row[2]; \n\
		getRows(m2, row);\n\
		\n\
		result.col0 = float2( \n\
						dot(m1.col0, row[0] ), \n\
						dot(m1.col0, row[1] ));\n\
		result.col1 = float2( \n\
						dot(m1.col1, row[0] ), \n\
						dot(m1.col1, row[1] ));\n\
		result.col2 = float2( \n\
						dot(m1.col2, row[0] ), \n\
						dot(m1.col2, row[1] ));\n\
		result.col3 = float2( \n\
						dot(m1.col3, row[0] ), \n\
						dot(m1.col3, row[1] ));\n\
		\n\
		return result;\n\
	}\n\
	\n\
	float4x3 mul (float4x3 m1, float3x3 m2)\n\
	{\n\
		float4x3 result ;\n\
		\n\
		float3 row[3]; \n\
		getRows(m2, row);\n\
		\n\
		result.col0 = float3( \n\
						dot(m1.col0, row[0] ), \n\
						dot(m1.col0, row[1] ),\n\
						dot(m1.col0, row[2] ));\n\
		result.col1 = float3( \n\
						dot(m1.col1, row[0] ), \n\
						dot(m1.col1, row[1] ),\n\
						dot(m1.col1, row[2] ));\n\
		result.col2 = float3( \n\
						dot(m1.col2, row[0] ), \n\
						dot(m1.col2, row[1] ),\n\
						dot(m1.col2, row[2] ));\n\
						\n\
		return result;\n\
	}\n\
	\n\
	float4x2 mul (float4x3 m1, float3x2 m2)\n\
	{\n\
		float4x2 result ;\n\
		\n\
		float3 row[2]; \n\
		getRows(m2, row);\n\
		\n\
		result.col0 = float2( \n\
						dot(m1.col0, row[0] ), \n\
						dot(m1.col0, row[1] ));\n\
		result.col1 = float2( \n\
						dot(m1.col1, row[0] ), \n\
						dot(m1.col1, row[1] ));\n\
		result.col2 = float2( \n\
						dot(m1.col2, row[0] ), \n\
						dot(m1.col2, row[1] ));\n\
		result.col3 = float2( \n\
						dot(m1.col3, row[0] ), \n\
						dot(m1.col3, row[1] ));\n\
						\n\
		return result;\n\
	}\n\
	\n\
	float3x4 mul (float3x4 m1, float4x4 m2)\n\
	{\n\
		float3x4 result ;\n\
		\n\
		float4 row[4]; \n\
		getRows(m2, row);\n\
		\n\
		result.col0 = float4( \n\
						dot(m1.col0, row[0] ), \n\
						dot(m1.col0, row[1] ),\n\
						dot(m1.col0, row[2] ),\n\
						dot(m1.col0, row[3] ));\n\
		result.col1 = float4( \n\
						dot(m1.col1, row[0] ), \n\
						dot(m1.col1, row[1] ),\n\
						dot(m1.col1, row[2] ),\n\
						dot(m1.col1, row[3] ));\n\
		result.col2 = float4( \n\
						dot(m1.col2, row[0] ), \n\
						dot(m1.col2, row[1] ),\n\
						dot(m1.col2, row[2] ),\n\
						dot(m1.col2, row[3] ));\n\
						\n\
		return result;\n\
	}\n\
	\n\
	float3x2 mul (float3x4 m1, float4x2 m2)\n\
	{\n\
		float3x2 result ;\n\
		\n\
		float4 row[2]; \n\
		getRows(m2, row);\n\
		\n\
		result.col0 = float2( \n\
						dot(m1.col0, row[0] ), \n\
						dot(m1.col0, row[1] ));\n\
		result.col1 = float2( \n\
						dot(m1.col1, row[0] ), \n\
						dot(m1.col1, row[1] ));\n\
		result.col2 = float2( \n\
						dot(m1.col2, row[0] ), \n\
						dot(m1.col2, row[1] ));\n\
						\n\
		return result;\n\
	}\n\
	\n\
	float2x4 mul (float2x3 m1, float3x4 m2)\n\
	{\n\
		float2x4 result ;\n\
		\n\
		float3 row[4]; \n\
		getRows(m2, row);\n\
		\n\
		result.col0 = float4( \n\
						dot(m1.col0, row[0] ), \n\
						dot(m1.col0, row[1] ),\n\
						dot(m1.col0, row[2] ),\n\
						dot(m1.col0, row[3] ));\n\
		result.col1 = float4( \n\
						dot(m1.col1, row[0] ), \n\
						dot(m1.col1, row[1] ),\n\
						dot(m1.col1, row[2] ),\n\
						dot(m1.col1, row[3] ));\n\
						\n\
		return result;\n\
	}\n\
	\n\
	float2x3 mul (float2x3 m1, float3x3 m2)\n\
	{\n\
		float2x3 result ;\n\
		\n\
		float3 row[3]; \n\
		getRows(m2, row);\n\
		\n\
		result.col0 = float3( \n\
						dot(m1.col0, row[0] ), \n\
						dot(m1.col0, row[1] ),\n\
						dot(m1.col0, row[2] ));\n\
		result.col1 = float3( \n\
						dot(m1.col1, row[0] ), \n\
						dot(m1.col1, row[1] ),\n\
						dot(m1.col1, row[2] ));\n\
						\n\
		return result;\n\
	}\n\
	\n\
	float3x4 mul (float3x2 m1, float2x4 m2)\n\
	{\n\
		float3x4 result ;\n\
		\n\
		float2 row[4]; \n\
		getRows(m2, row);\n\
		\n\
		result.col0 = float4( \n\
						dot(m1.col0, row[0] ), \n\
						dot(m1.col0, row[1] ),\n\
						dot(m1.col0, row[2] ),\n\
						dot(m1.col0, row[3] ));\n\
		result.col1 = float4( \n\
						dot(m1.col1, row[0] ), \n\
						dot(m1.col1, row[1] ),\n\
						dot(m1.col1, row[2] ),\n\
						dot(m1.col1, row[3] ));\n\
		result.col2 = float4( \n\
						dot(m1.col2, row[0] ), \n\
						dot(m1.col2, row[1] ),\n\
						dot(m1.col2, row[2] ),\n\
						dot(m1.col2, row[3] ));\n\
						\n\
		return result;\n\
	}\n\
	\n\
	float3x2 mul (float3x2 m1, float2x2 m2)\n\
	{\n\
		float3x2 result ;\n\
		\n\
		float2 row[2]; \n\
		getRows(m2, row);\n\
		\n\
		result.col0 = float2( \n\
						dot(m1.col0, row[0] ), \n\
						dot(m1.col0, row[1] ));\n\
		result.col1 = float2( \n\
						dot(m1.col1, row[0] ), \n\
						dot(m1.col1, row[1] ));\n\
		result.col2 = float2( \n\
						dot(m1.col2, row[0] ), \n\
						dot(m1.col2, row[1] ));\n\
						\n\
		return result;\n\
	}\n\
	\n\
	float2x4 mul (float2x4 m1, float4x4 m2)\n\
	{\n\
		float2x4 result ;\n\
		\n\
		float4 row[4]; \n\
		getRows(m2, row);\n\
		\n\
		result.col0 = float4( \n\
						dot(m1.col0, row[0] ), \n\
						dot(m1.col0, row[1] ),\n\
						dot(m1.col0, row[2] ),\n\
						dot(m1.col0, row[3] ));\n\
		result.col1 = float4( \n\
						dot(m1.col1, row[0] ), \n\
						dot(m1.col1, row[1] ),\n\
						dot(m1.col1, row[2] ),\n\
						dot(m1.col1, row[3] ));\n\
						\n\
		return result;\n\
	}\n\
	\n\
	float2x3 mul (float2x4 m1, float4x3 m2)\n\
	{\n\
		float2x3 result ;\n\
		\n\
		float4 row[3]; \n\
		getRows(m2, row);\n\
		\n\
		result.col0 = float3( \n\
						dot(m1.col0, row[0] ), \n\
						dot(m1.col0, row[1] ),\n\
						dot(m1.col0, row[2] ));\n\
		result.col1 = float3( \n\
						dot(m1.col1, row[0] ), \n\
						dot(m1.col1, row[1] ),\n\
						dot(m1.col1, row[2] ));\n\
						\n\
		return result;\n\
	}\n\
	\n\
	float4x3 mul (float4x4 m1, float4x3 m2)\n\
	{\n\
		float4x3 result ;\n\
		\n\
		float4 row[3]; \n\
		getRows(m2, row);\n\
		\n\
		result.col0 = float3( \n\
						dot(m1[0], row[0] ), \n\
						dot(m1[0], row[1] ),\n\
						dot(m1[0], row[2] ));\n\
		result.col1 = float3( \n\
						dot(m1[1], row[0] ), \n\
						dot(m1[1], row[1] ),\n\
						dot(m1[1], row[2] ));\n\
		result.col2 = float3( \n\
						dot(m1[2], row[0] ), \n\
						dot(m1[2], row[1] ),\n\
						dot(m1[2], row[2] ));\n\
		result.col3 = float3( \n\
						dot(m1[3], row[0] ), \n\
						dot(m1[3], row[1] ),\n\
						dot(m1[3], row[2] ));\n\
						\n\
		return result;\n\
	}\n\
	\n\
	float4x2 mul (float4x4 m1, float4x2 m2)\n\
	{\n\
		float4x2 result ;\n\
		\n\
		float4 row[2]; \n\
		getRows(m2, row);\n\
		\n\
		result.col0 = float2( \n\
						dot(m1[0], row[0] ), \n\
						dot(m1[0], row[1] ));\n\
		result.col1 = float2( \n\
						dot(m1[1], row[0] ), \n\
						dot(m1[1], row[1] ));\n\
		result.col2 = float2( \n\
						dot(m1[2], row[0] ), \n\
						dot(m1[2], row[1] ));\n\
		result.col3 = float2( \n\
						dot(m1[3], row[0] ), \n\
						dot(m1[3], row[1] ));\n\
						\n\
		return result;\n\
	}\n\
	\n\
	float3x4 mul (float3x3 m1, float3x4 m2)\n\
	{\n\
		float3x4 result ;\n\
		\n\
		float3 row[4]; \n\
		getRows(m2, row);\n\
		\n\
		result.col0 = float4( \n\
						dot(m1[0], row[0] ), \n\
						dot(m1[0], row[1] ),\n\
						dot(m1[0], row[2] ),\n\
						dot(m1[0], row[3] ));\n\
		result.col1 = float4( \n\
						dot(m1[1], row[0] ), \n\
						dot(m1[1], row[1] ),\n\
						dot(m1[1], row[2] ),\n\
						dot(m1[1], row[3] ));\n\
		result.col2 = float4( \n\
						dot(m1[2], row[0] ), \n\
						dot(m1[2], row[1] ),\n\
						dot(m1[2], row[2] ),\n\
						dot(m1[2], row[3] ));\n\
						\n\
		return result;\n\
	}\n\
	\n\
	float3x2 mul (float3x3 m1, float3x2 m2)\n\
	{\n\
		float3x2 result ;\n\
		\n\
		float3 row[2]; \n\
		getRows(m2, row);\n\
		\n\
		result.col0 = float2( \n\
						dot(m1[0], row[0] ), \n\
						dot(m1[0], row[1] ));\n\
		result.col1 = float2( \n\
						dot(m1[1], row[0] ), \n\
						dot(m1[1], row[1] ));\n\
		result.col2 = float2( \n\
						dot(m1[2], row[0] ), \n\
						dot(m1[2], row[1] ));\n\
						\n\
		return result;\n\
	}\n\
	float2x4 mul (float2x2 m1, float2x4 m2)\n\
	{\n\
		float2x4 result ;\n\
		\n\
		float2 row[4]; \n\
		getRows(m2, row);\n\
		\n\
		result.col0 = float4( \n\
						dot(m1[0], row[0] ), \n\
						dot(m1[0], row[1] ),\n\
						dot(m1[0], row[2] ),\n\
						dot(m1[0], row[3] ));\n\
		result.col1 = float4( \n\
						dot(m1[1], row[0] ), \n\
						dot(m1[1], row[1] ),\n\
						dot(m1[1], row[2] ),\n\
						dot(m1[1], row[3] ));\n\
						\n\
		return result;\n\
	}\n\
	float2x3 mul (float2x2 m1, float2x3 m2)\n\
	{\n\
		float2x3 result ;\n\
		\n\
		float2 row[3]; \n\
		getRows(m2, row);\n\
		\n\
		result.col0 = float3( \n\
						dot(m1[0], row[0] ), \n\
						dot(m1[0], row[1] ),\n\
						dot(m1[0], row[2] ));\n\
		result.col1 = float3( \n\
						dot(m1[1], row[0] ), \n\
						dot(m1[1], row[1] ),\n\
						dot(m1[1], row[2] ));\n\
						\n\
		return result;\n\
	}\n";
	}
	{
	fullSource += 
#if 1
		"#define half2x3 float2x3\n\
		#define half3x2 float3x2\n\
		#define half3x4 float3x4\n\
		#define half4x3 float4x3\n\
		#define half2x4 float2x4\n\
		#define half4x2 float4x2\n\
		\n\
		#define fixed2x3 float2x3\n\
		#define fixed3x2 float3x2\n\
		#define fixed3x4 float3x4\n\
		#define fixed4x3 float4x3\n\
		#define fixed2x4 float2x4\n\
		#define fixed4x2 float4x2\n\
		#line 1\n";
#else
	"//-----------------half--------------------\n\
	struct half2x3\n\
	{\n\
		half3 col0;\n\
		half3 col1;\n\
	};\n\
	\n\
	struct half3x2\n\
	{\n\
		half2 col0;\n\
		half2 col1;\n\
		half2 col2;\n\
	};\n\
	\n\
	struct half3x4\n\
	{\n\
		half4 col0;\n\
		half4 col1;\n\
		half4 col2;\n\
	};\n\
	\n\
	struct half4x3\n\
	{\n\
		half3 col0;\n\
		half3 col1;\n\
		half3 col2;\n\
		half3 col3;\n\
	};\n\
	\n\
	struct half2x4\n\
	{\n\
		half4 col0;\n\
		half4 col1;\n\
	};\n\
	\n\
	struct half4x2\n\
	{\n\
		half2 col0;\n\
		half2 col1;\n\
		half2 col2;\n\
		half2 col3;\n\
	};\n\
	\n\
	half2 mul(half2x3 m, half3 v)\n\
	{\n\
		half2 vec;\n\
		vec.x = dot(m.col0, v);\n\
		vec.y = dot(m.col1, v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	half3 mul(half3x2 m, half2 v)\n\
	{\n\
		half3 vec;\n\
		vec.x = dot(m.col0, v);\n\
		vec.y = dot(m.col1, v);\n\
		vec.z = dot(m.col2, v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	half3 mul(half3x4 m, half4 v)\n\
	{\n\
		half3 vec;\n\
		vec.x = dot(m.col0, v);\n\
		vec.y = dot(m.col1, v);\n\
		vec.z = dot(m.col2, v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	half4 mul(half4x3 m, half3 v)\n\
	{\n\
		half4 vec;\n\
		vec.x = dot(m.col0, v);\n\
		vec.y = dot(m.col1, v);\n\
		vec.z = dot(m.col2, v);\n\
		vec.w = dot(m.col3, v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	half2 mul(half2x4 m, half4 v)\n\
	{\n\
		half2 vec;\n\
		vec.x = dot(m.col0, v);\n\
		vec.y = dot(m.col1, v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	half4 mul(half4x2 m, half2 v)\n\
	{\n\
		half4 vec;\n\
		vec.x = dot(m.col0, v);\n\
		vec.y = dot(m.col1, v);\n\
		vec.z = dot(m.col2, v);\n\
		vec.w = dot(m.col3, v);\n\
		\n\
		return vec;\n\
	}\n\
	//---------scalar mul matrix---------------\n\
	half2x3 mul(half f, half2x3 m)\n\
	{\n\
		return half2x3(m.col0 * f, m.col1 * f);\n\
	}\n\
	half3x2 mul(half f, half3x2 m)\n\
	{\n\
		return half3x2(m.col0 * f, m.col1 * f, m.col2 * f);\n\
	}\n\
	half3x4 mul(half f, half3x4 m)\n\
	{\n\
		return half3x4(m.col0 * f, m.col1 * f, m.col2 * f);\n\
	}\n\
	half4x3 mul(half f, half4x3 m)\n\
	{\n\
		return half4x3(m.col0 * f, m.col1 * f, m.col2 * f, m.col3 * f);\n\
	}\n\
	half2x4 mul(half f, half2x4 m)\n\
	{\n\
		return half2x4(m.col0 * f, m.col1 * f);\n\
	}\n\
	half4x2 mul(half f, half4x2 m)\n\
	{\n\
		return half4x2(m.col0 * f, m.col1 * f, m.col2 * f, m.col3 * f);\n\
	}\n\
	//---------matrix mul scalar---------------\n\
	half2x3 mul(half2x3 m, half f)\n\
	{\n\
		return half2x3(m.col0 * f, m.col1 * f);\n\
	}\n\
	half3x2 mul(half3x2 m, half f)\n\
	{\n\
		return half3x2(m.col0 * f, m.col1 * f, m.col2 * f);\n\
	}\n\
	half3x4 mul(half3x4 m, half f)\n\
	{\n\
		return half3x4(m.col0 * f, m.col1 * f, m.col2 * f);\n\
	}\n\
	half4x3 mul(half4x3 m, half f)\n\
	{\n\
		return half4x3(m.col0 * f, m.col1 * f, m.col2 * f, m.col3 * f);\n\
	}\n\
	half2x4 mul(half2x4 m, half f)\n\
	{\n\
		return half2x4(m.col0 * f, m.col1 * f);\n\
	}\n\
	half4x2 mul(half4x2 m, half f)\n\
	{\n\
		return half4x2(m.col0 * f, m.col1 * f, m.col2 * f, m.col3 * f);\n\
	}\n\
	\n\
	half2x3 mulAssign(inout half2x3 m, half f)\n\
	{\n\
		m = half2x3(m.col0 * f, m.col1 * f); return m;\n\
	}\n\
	half3x2 mulAssign(inout half3x2 m, half f)\n\
	{\n\
		m = half3x2(m.col0 * f, m.col1 * f, m.col2 * f); return m;\n\
	}\n\
	half3x4 mulAssign(inout half3x4 m, half f)\n\
	{\n\
		m = half3x4(m.col0 * f, m.col1 * f, m.col2 * f); return m;\n\
	}\n\
	half4x3 mulAssign(inout half4x3 m, half f)\n\
	{\n\
		m = half4x3(m.col0 * f, m.col1 * f, m.col2 * f, m.col3 * f); return m;\n\
	}\n\
	half2x4 mulAssign(inout half2x4 m, half f)\n\
	{\n\
		m = half2x4(m.col0 * f, m.col1 * f); return m;\n\
	}\n\
	half4x2 mulAssign(inout half4x2 m, half f)\n\
	{\n\
		m = half4x2(m.col0 * f, m.col1 * f, m.col2 * f, m.col3 * f); return m;\n\
	}\n\
	//---------matrix add matrix---------------\n\
	half2x3 add(half2x3 m, half2x3 m2)\n\
	{\n\
		return half2x3(m.col0 + m2.col0, m.col1 + m2.col1);\n\
	}\n\
	half3x2 add(half3x2 m, half3x2 m2)\n\
	{\n\
		return half3x2(m.col0 + m2.col0, m.col1 + m2.col1, m.col2 + m2.col2);\n\
	}\n\
	half3x4 add(half3x4 m, half3x4 m2)\n\
	{\n\
		return half3x4(m.col0 + m2.col0, m.col1 + m2.col1, m.col2 + m2.col2);\n\
	}\n\
	half4x3 add(half4x3 m, half4x3 m2)\n\
	{\n\
		return half4x3(m.col0 + m2.col0, m.col1 + m2.col1, m.col2 + m2.col2, m.col3 + m2.col3);\n\
	}\n\
	half2x4 add(half2x4 m, half2x4 m2)\n\
	{\n\
		return half2x4(m.col0 + m2.col0, m.col1 + m2.col1);\n\
	}\n\
	half4x2 add(half4x2 m, half4x2 m2)\n\
	{\n\
		return half4x2(m.col0 + m2.col0, m.col1 + m2.col1, m.col2 + m2.col2, m.col3 + m2.col3);\n\
	}\n\
	\n\
	half2x3 addAssign(inout half2x3 m, half2x3 m2)\n\
	{\n\
		m = half2x3(m.col0 + m2.col0, m.col1 + m2.col1); return m;\n\
	}\n\
	half3x2 addAssign(inout half3x2 m, half3x2 m2)\n\
	{\n\
		m = half3x2(m.col0 + m2.col0, m.col1 + m2.col1, m.col2 + m2.col2); return m;\n\
	}\n\
	half3x4 addAssign(inout half3x4 m, half3x4 m2)\n\
	{\n\
		m = half3x4(m.col0 + m2.col0, m.col1 + m2.col1, m.col2 + m2.col2); return m;\n\
	}\n\
	half4x3 addAssign(inout half4x3 m, half4x3 m2)\n\
	{\n\
		m = half4x3(m.col0 + m2.col0, m.col1 + m2.col1, m.col2 + m2.col2, m.col3 + m2.col3); return m;\n\
	}\n\
	half2x4 addAssign(inout half2x4 m, half2x4 m2)\n\
	{\n\
		m = half2x4(m.col0 + m2.col0, m.col1 + m2.col1); return m;\n\
	}\n\
	half4x2 addAssign(inout half4x2 m, half4x2 m2)\n\
	{\n\
		m = half4x2(m.col0 + m2.col0, m.col1 + m2.col1, m.col2 + m2.col2, m.col3 + m2.col3); return m;\n\
	}\n\
	//---------vector mul matrix---------------\n\
	half2 mul(half3 v, half3x2 m)\n\
	{\n\
		half2 vec;\n\
		\n\
		half3 row0 = half3(m.col0.x, m.col1.x, m.col2.x);\n\
		half3 row1 = half3(m.col0.y, m.col1.y, m.col2.y);\n\
		\n\
		vec.x = dot(v, row0);\n\
		vec.y = dot(v, row1);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	half3 mul(half2 v, half2x3 m)\n\
	{\n\
		half3 vec;\n\
		\n\
		half2 row0 = half2(m.col0.x, m.col1.x);\n\
		half2 row1 = half2(m.col0.y, m.col1.y);\n\
		half2 row2 = half2(m.col0.z, m.col1.z);\n\
		\n\
		vec.x = dot(row0, v);\n\
		vec.y = dot(row1, v);\n\
		vec.z = dot(row2, v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	half3 mul(half4 v, half4x3 m)\n\
	{\n\
		half3 vec;\n\
		\n\
		half4 row0 = half4(m.col0.x, m.col1.x, m.col2.x, m.col3.x);\n\
		half4 row1 = half4(m.col0.y, m.col1.y, m.col2.y, m.col3.y);\n\
		half4 row2 = half4(m.col0.z, m.col1.z, m.col2.z, m.col3.z);\n\
		\n\
		vec.x = dot(row0, v);\n\
		vec.y = dot(row1, v);\n\
		vec.z = dot(row2, v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	half4 mul(half3 v, half3x4 m)\n\
	{\n\
		half4 vec;\n\
		\n\
		half3 row0 = half3(m.col0.x, m.col1.x, m.col2.x);\n\
		half3 row1 = half3(m.col0.y, m.col1.y, m.col2.y);\n\
		half3 row2 = half3(m.col0.z, m.col1.z, m.col2.z);\n\
		half3 row3 = half3(m.col0.w, m.col1.w, m.col2.w);\n\
		\n\
		vec.x = dot(row0, v);\n\
		vec.y = dot(row1, v);\n\
		vec.z = dot(row2, v);\n\
		vec.w = dot(row3, v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	half2 mul(half4 v, half4x2 m)\n\
	{\n\
		half2 vec;\n\
		\n\
		half4 row0 = half4(m.col0.x, m.col1.x, m.col2.x, m.col3.x);\n\
		half4 row1 = half4(m.col0.y, m.col1.y, m.col2.y, m.col3.y);\n\
		\n\
		vec.x = dot(row0, v);\n\
		vec.y = dot(row1, v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	half4 mul(half2 v, half2x4 m)\n\
	{\n\
		half4 vec;\n\
		\n\
		half2 row0 = half2(m.col0.x, m.col1.x);\n\
		half2 row1 = half2(m.col0.y, m.col1.y);\n\
		half2 row2 = half2(m.col0.z, m.col1.z);\n\
		half2 row3 = half2(m.col0.w, m.col1.w);\n\
		\n\
		vec.x = dot(row0, v);\n\
		vec.y = dot(row1, v);\n\
		vec.z = dot(row2, v);\n\
		vec.w = dot(row3, v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	//-----------------fixed--------------------\n\
	struct fixed2x3\n\
	{\n\
		fixed3 col0;\n\
		fixed3 col1;\n\
	};\n\
	\n\
	struct fixed3x2\n\
	{\n\
		fixed2 col0;\n\
		fixed2 col1;\n\
		fixed2 col2;\n\
	};\n\
	\n\
	struct fixed3x4\n\
	{\n\
		fixed4 col0;\n\
		fixed4 col1;\n\
		fixed4 col2;\n\
	};\n\
	\n\
	struct fixed4x3\n\
	{\n\
		fixed3 col0;\n\
		fixed3 col1;\n\
		fixed3 col2;\n\
		fixed3 col3;\n\
	};\n\
	\n\
	struct fixed2x4\n\
	{\n\
		fixed4 col0;\n\
		fixed4 col1;\n\
	};\n\
	\n\
	struct fixed4x2\n\
	{\n\
		fixed2 col0;\n\
		fixed2 col1;\n\
		fixed2 col2;\n\
		fixed2 col3;\n\
	};\n\
	//---------scalar mul matrix---------------\n\
	fixed2x3 mul(fixed f, fixed2x3 m)\n\
	{\n\
		return fixed2x3(m.col0 * f, m.col1 * f);\n\
	}\n\
	fixed3x2 mul(fixed f, fixed3x2 m)\n\
	{\n\
		return fixed3x2(m.col0 * f, m.col1 * f, m.col2 * f);\n\
	}\n\
	fixed3x4 mul(fixed f, fixed3x4 m)\n\
	{\n\
		return fixed3x4(m.col0 * f, m.col1 * f, m.col2 * f);\n\
	}\n\
	fixed4x3 mul(fixed f, fixed4x3 m)\n\
	{\n\
		return fixed4x3(m.col0 * f, m.col1 * f, m.col2 * f, m.col3 * f);\n\
	}\n\
	fixed2x4 mul(fixed f, fixed2x4 m)\n\
	{\n\
		return fixed2x4(m.col0 * f, m.col1 * f);\n\
	}\n\
	fixed4x2 mul(fixed f, fixed4x2 m)\n\
	{\n\
		return fixed4x2(m.col0 * f, m.col1 * f, m.col2 * f, m.col3 * f);\n\
	}\n\
	//---------matrix mul scalar---------------\n\
	fixed2x3 mul(fixed2x3 m, fixed f)\n\
	{\n\
		return fixed2x3(m.col0 * f, m.col1 * f);\n\
	}\n\
	fixed3x2 mul(fixed3x2 m, fixed f)\n\
	{\n\
		return fixed3x2(m.col0 * f, m.col1 * f, m.col2 * f);\n\
	}\n\
	fixed3x4 mul(fixed3x4 m, fixed f)\n\
	{\n\
		return fixed3x4(m.col0 * f, m.col1 * f, m.col2 * f);\n\
	}\n\
	fixed4x3 mul(fixed4x3 m, fixed f)\n\
	{\n\
		return fixed4x3(m.col0 * f, m.col1 * f, m.col2 * f, m.col3 * f);\n\
	}\n\
	fixed2x4 mul(fixed2x4 m, fixed f)\n\
	{\n\
		return fixed2x4(m.col0 * f, m.col1 * f);\n\
	}\n\
	fixed4x2 mul(fixed4x2 m, fixed f)\n\
	{\n\
		return fixed4x2(m.col0 * f, m.col1 * f, m.col2 * f, m.col3 * f);\n\
	}\n\
	\n\
	fixed2x3 mulAssign(inout fixed2x3 m, fixed f)\n\
	{\n\
		m = fixed2x3(m.col0 * f, m.col1 * f); return m;\n\
	}\n\
	fixed3x2 mulAssign(inout fixed3x2 m, fixed f)\n\
	{\n\
		m = fixed3x2(m.col0 * f, m.col1 * f, m.col2 * f); return m;\n\
	}\n\
	fixed3x4 mulAssign(inout fixed3x4 m, fixed f)\n\
	{\n\
		m = fixed3x4(m.col0 * f, m.col1 * f, m.col2 * f); return m;\n\
	}\n\
	fixed4x3 mulAssign(inout fixed4x3 m, fixed f)\n\
	{\n\
		m = fixed4x3(m.col0 * f, m.col1 * f, m.col2 * f, m.col3 * f); return m;\n\
	}\n\
	fixed2x4 mulAssign(inout fixed2x4 m, fixed f)\n\
	{\n\
		m = fixed2x4(m.col0 * f, m.col1 * f); return m;\n\
	}\n\
	fixed4x2 mulAssign(inout fixed4x2 m, fixed f)\n\
	{\n\
		m = fixed4x2(m.col0 * f, m.col1 * f, m.col2 * f, m.col3 * f); return m;\n\
	}\n\
	//---------matrix add matrix---------------\n\
	fixed2x3 add(fixed2x3 m, fixed2x3 m2)\n\
	{\n\
		return fixed2x3(m.col0 + m2.col0, m.col1 + m2.col1);\n\
	}\n\
	fixed3x2 add(fixed3x2 m, fixed3x2 m2)\n\
	{\n\
		return fixed3x2(m.col0 + m2.col0, m.col1 + m2.col1, m.col2 + m2.col2);\n\
	}\n\
	fixed3x4 add(fixed3x4 m, fixed3x4 m2)\n\
	{\n\
		return fixed3x4(m.col0 + m2.col0, m.col1 + m2.col1, m.col2 + m2.col2);\n\
	}\n\
	fixed4x3 add(fixed4x3 m, fixed4x3 m2)\n\
	{\n\
		return fixed4x3(m.col0 + m2.col0, m.col1 + m2.col1, m.col2 + m2.col2, m.col3 + m2.col3);\n\
	}\n\
	fixed2x4 add(fixed2x4 m, fixed2x4 m2)\n\
	{\n\
		return fixed2x4(m.col0 + m2.col0, m.col1 + m2.col1);\n\
	}\n\
	fixed4x2 add(fixed4x2 m, fixed4x2 m2)\n\
	{\n\
		return fixed4x2(m.col0 + m2.col0, m.col1 + m2.col1, m.col2 + m2.col2, m.col3 + m2.col3);\n\
	}\n\
	\n\
	fixed2x3 addAssign(inout fixed2x3 m, fixed2x3 m2)\n\
	{\n\
		m = fixed2x3(m.col0 + m2.col0, m.col1 + m2.col1); return m;\n\
	}\n\
	fixed3x2 addAssign(inout fixed3x2 m, fixed3x2 m2)\n\
	{\n\
		m = fixed3x2(m.col0 + m2.col0, m.col1 + m2.col1, m.col2 + m2.col2); return m;\n\
	}\n\
	fixed3x4 addAssign(inout fixed3x4 m, fixed3x4 m2)\n\
	{\n\
		m = fixed3x4(m.col0 + m2.col0, m.col1 + m2.col1, m.col2 + m2.col2); return m;\n\
	}\n\
	fixed4x3 addAssign(inout fixed4x3 m, fixed4x3 m2)\n\
	{\n\
		m = fixed4x3(m.col0 + m2.col0, m.col1 + m2.col1, m.col2 + m2.col2, m.col3 + m2.col3); return m;\n\
	}\n\
	fixed2x4 addAssign(inout fixed2x4 m, fixed2x4 m2)\n\
	{\n\
		m = fixed2x4(m.col0 + m2.col0, m.col1 + m2.col1); return m;\n\
	}\n\
	fixed4x2 addAssign(inout fixed4x2 m, fixed4x2 m2)\n\
	{\n\
		m = fixed4x2(m.col0 + m2.col0, m.col1 + m2.col1, m.col2 + m2.col2, m.col3 + m2.col3); return m;\n\
	}\n\
	//-----------matrix mul vector-------------------\n\
	fixed2 mul(fixed2x3 m, fixed3 v)\n\
	{\n\
		fixed2 vec;\n\
		vec.x = dot(m.col0, v);\n\
		vec.y = dot(m.col1, v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	fixed3 mul(fixed3x2 m, fixed2 v)\n\
	{\n\
		fixed3 vec;\n\
		vec.x = dot(m.col0, v);\n\
		vec.y = dot(m.col1, v);\n\
		vec.z = dot(m.col2, v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	fixed3 mul(fixed3x4 m, fixed4 v)\n\
	{\n\
		fixed3 vec;\n\
		vec.x = dot(m.col0, v);\n\
		vec.y = dot(m.col1, v);\n\
		vec.z = dot(m.col2, v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	fixed4 mul(fixed4x3 m, fixed3 v)\n\
	{\n\
		fixed4 vec;\n\
		vec.x = dot(m.col0, v);\n\
		vec.y = dot(m.col1, v);\n\
		vec.z = dot(m.col2, v);\n\
		vec.w = dot(m.col3, v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	fixed2 mul(fixed2x4 m, fixed4 v)\n\
	{\n\
		fixed2 vec;\n\
		vec.x = dot(m.col0, v);\n\
		vec.y = dot(m.col1, v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	fixed4 mul(fixed4x2 m, fixed2 v)\n\
	{\n\
		fixed4 vec;\n\
		vec.x = dot(m.col0, v);\n\
		vec.y = dot(m.col1, v);\n\
		vec.z = dot(m.col2, v);\n\
		vec.w = dot(m.col3, v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	//---------vector mul matrix---------------\n\
	fixed2 mul(fixed3 v, fixed3x2 m)\n\
	{\n\
		fixed2 vec;\n\
		\n\
		fixed3 row0 = fixed3(m.col0.x, m.col1.x, m.col2.x);\n\
		fixed3 row1 = fixed3(m.col0.y, m.col1.y, m.col2.y);\n\
		\n\
		vec.x = dot(v, row0);\n\
		vec.y = dot(v, row1);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	fixed3 mul(fixed2 v, fixed2x3 m)\n\
	{\n\
		fixed3 vec;\n\
		\n\
		fixed2 row0 = fixed2(m.col0.x, m.col1.x);\n\
		fixed2 row1 = fixed2(m.col0.y, m.col1.y);\n\
		fixed2 row2 = fixed2(m.col0.z, m.col1.z);\n\
		\n\
		vec.x = dot(row0, v);\n\
		vec.y = dot(row1, v);\n\
		vec.z = dot(row2, v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	fixed3 mul(fixed4 v, fixed4x3 m)\n\
	{\n\
		fixed3 vec;\n\
		\n\
		fixed4 row0 = fixed4(m.col0.x, m.col1.x, m.col2.x, m.col3.x);\n\
		fixed4 row1 = fixed4(m.col0.y, m.col1.y, m.col2.y, m.col3.y);\n\
		fixed4 row2 = fixed4(m.col0.z, m.col1.z, m.col2.z, m.col3.z);\n\
		\n\
		vec.x = dot(row0, v);\n\
		vec.y = dot(row1, v);\n\
		vec.z = dot(row2, v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	fixed4 mul(fixed3 v, fixed3x4 m)\n\
	{\n\
		fixed4 vec;\n\
		\n\
		fixed3 row0 = fixed3(m.col0.x, m.col1.x, m.col2.x);\n\
		fixed3 row1 = fixed3(m.col0.y, m.col1.y, m.col2.y);\n\
		fixed3 row2 = fixed3(m.col0.z, m.col1.z, m.col2.z);\n\
		fixed3 row3 = fixed3(m.col0.w, m.col1.w, m.col2.w);\n\
		\n\
		vec.x = dot(row0, v);\n\
		vec.y = dot(row1, v);\n\
		vec.z = dot(row2, v);\n\
		vec.w = dot(row3, v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	fixed2 mul(fixed4 v, fixed4x2 m)\n\
	{\n\
		fixed2 vec;\n\
		\n\
		fixed4 row0 = fixed4(m.col0.x, m.col1.x, m.col2.x, m.col3.x);\n\
		fixed4 row1 = fixed4(m.col0.y, m.col1.y, m.col2.y, m.col3.y);\n\
		\n\
		vec.x = dot(row0, v);\n\
		vec.y = dot(row1, v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	fixed4 mul(fixed2 v, fixed2x4 m)\n\
	{\n\
		fixed4 vec;\n\
		\n\
		fixed2 row0 = fixed2(m.col0.x, m.col1.x);\n\
		fixed2 row1 = fixed2(m.col0.y, m.col1.y);\n\
		fixed2 row2 = fixed2(m.col0.z, m.col1.z);\n\
		fixed2 row3 = fixed2(m.col0.w, m.col1.w);\n\
		\n\
		vec.x = dot(row0, v);\n\
		vec.y = dot(row1, v);\n\
		vec.z = dot(row2, v);\n\
		vec.w = dot(row3, v);\n\
		\n\
		return vec;\n\
	}\n\
	\n\
	#line 1\n";
#endif
	}
}


/// Initializize the symbol table
/// \param BuiltInStrings
///      Pointer to built in strings.
/// \param language
///      Shading language to initialize symbol table for
/// \param infoSink
///      Information sink (for errors/warnings)
/// \param symbolTables
///      Array of symbol tables (one for each language)
/// \param bUseGlobalSymbolTable
///      Whether to use the global symbol table or the per-language symbol table
/// \return
///      True if succesfully initialized, false otherwise
static bool InitializeSymbolTable( TBuiltInStrings* BuiltInStrings, EShLanguage language, TInfoSink& infoSink, 
                            TSymbolTable* symbolTables, const TString &cgProfile, bool bUseGlobalSymbolTable )
{
   TIntermediate intermediate(infoSink); 
   TSymbolTable* symbolTable;

   if ( bUseGlobalSymbolTable )
      symbolTable = symbolTables;
   else
      symbolTable = &symbolTables[language];

	//@TODO: for now, we use same global symbol table for all target language versions.
	// This is wrong and will have to be changed at some point.
	TParseContext parseContext(*symbolTable, intermediate, language, ETargetGLSL_ES_100, cgProfile, 0, infoSink);

   GlobalParseContext = &parseContext;

   setInitialState();

   assert(symbolTable->isEmpty() || symbolTable->atSharedBuiltInLevel());

   //
   // Parse the built-ins.  This should only happen once per
   // language symbol table.
   //
   // Push the symbol table to give it an initial scope.  This
   // push should not have a corresponding pop, so that built-ins
   // are preserved, and the test for an empty table fails.
   //

   symbolTable->push();

   //Initialize the Preprocessor
   int ret = InitPreprocessor();
   if (ret)
   {
      infoSink.info.message(EPrefixInternalError,  "Unable to intialize the Preprocessor");
      return false;
   }

   for (TBuiltInStrings::iterator i  = BuiltInStrings[parseContext.language].begin();
       i != BuiltInStrings[parseContext.language].end();
       ++i)
   {
      const char* builtInShaders = (*i).c_str();

      if (PaParseString(const_cast<char*>(builtInShaders), parseContext) != 0)
      {
         infoSink.info.message(EPrefixInternalError, "Unable to parse built-ins");
         return false;
      }
   }

   if (  !bUseGlobalSymbolTable )
   {
      IdentifyBuiltIns(parseContext.language, *symbolTable);
   }

   FinalizePreprocessor();

   return true;
}


/// Generate the built in symbol table
/// \param infoSink
///      Information sink (for errors/warnings)
/// \param symbolTables
///      Array of symbol tables (one for each language)
/// \param bUseGlobalSymbolTable
///      Whether to use the global symbol table or the per-language symbol table
/// \param language
///      Shading language to build symbol table for
/// \return
///      True if succesfully built, false otherwise
static bool GenerateBuiltInSymbolTable(TInfoSink& infoSink, TSymbolTable* symbolTables, EShLanguage language, const TString& cgProfile)
{
   TBuiltIns builtIns;

   if ( language != EShLangCount )
   {      
      InitializeSymbolTable(builtIns.getBuiltInStrings(), language, infoSink, symbolTables, cgProfile, true);
   }
   else
   {
      builtIns.initialize();
      InitializeSymbolTable(builtIns.getBuiltInStrings(), EShLangVertex, infoSink, symbolTables, cgProfile, false);
      InitializeSymbolTable(builtIns.getBuiltInStrings(), EShLangFragment, infoSink, symbolTables, cgProfile, false);
   }

   return true;
}



int C_DECL Hlsl2Glsl_Initialize(GlobalAllocateFunction alloc, GlobalFreeFunction free, void* user)
{
   TInfoSink infoSink;
   bool ret = true;

   SetGlobalAllocationAllocator(alloc, free, user);
	
   if (!InitProcess())
      return 0;

   // This method should be called once per process. If its called by multiple threads, then 
   // we need to have thread synchronization code around the initialization of per process
   // global pool allocator
   if (!PerProcessGPA)
   {
      TPoolAllocator *builtInPoolAllocator = new TPoolAllocator(true);
      builtInPoolAllocator->push();
      TPoolAllocator* gPoolAllocator = &GlobalPoolAllocator;
      SetGlobalPoolAllocatorPtr(builtInPoolAllocator);

      TSymbolTable symTables[EShLangCount];
      GenerateBuiltInSymbolTable(infoSink, symTables, EShLangCount, "");

      PerProcessGPA = new TPoolAllocator(true);
      PerProcessGPA->push();
      SetGlobalPoolAllocatorPtr(PerProcessGPA);

      SymbolTables[EShLangVertex].copyTable(symTables[EShLangVertex]);
      SymbolTables[EShLangFragment].copyTable(symTables[EShLangFragment]);

      SetGlobalPoolAllocatorPtr(gPoolAllocator);

      symTables[EShLangVertex].pop();
      symTables[EShLangFragment].pop();

      initializeHLSLSupportLibrary();

      builtInPoolAllocator->popAll();
      delete builtInPoolAllocator;        

   }

   return ret ? 1 : 0;
}

int C_DECL Hlsl2Glsl_Shutdown()
{
   return DetachProcess();
}

int C_DECL Hlsl2Glsl_Finalize()
{
   if (PerProcessGPA)
   {
      SymbolTables[EShLangVertex].pop();
      SymbolTables[EShLangFragment].pop();

      PerProcessGPA->popAll();
      delete PerProcessGPA;
      PerProcessGPA = NULL;
      finalizeHLSLSupportLibrary();
   }
   return 1;
}


ShHandle C_DECL Hlsl2Glsl_ConstructCompiler( const EShLanguage language )
{
   if (!InitThread())
      return 0;

   HlslCrossCompiler* compiler = new HlslCrossCompiler(language);
   return compiler;
}

void C_DECL Hlsl2Glsl_DestructCompiler( ShHandle handle )
{
   if (handle == 0)
      return;

   delete handle;
}


int C_DECL Hlsl2Glsl_Parse(
	const ShHandle handle,
	const char* shaderString,
	const char *cgProfile,
	ETargetVersion targetVersion,
	unsigned options)
{
   if (!InitThread())
      return 0;

   if (handle == 0)
      return 0;

   HlslCrossCompiler* compiler = handle;
   compiler->cgProfile = cgProfile != NULL? cgProfile : "";

   GlobalPoolAllocator.push();
   compiler->infoSink.info.erase();
   compiler->infoSink.debug.erase();

   if (!shaderString)
	   return 1;

   TIntermediate intermediate(compiler->infoSink);
   TSymbolTable symbolTable(SymbolTables[compiler->getLanguage()]);

   GenerateBuiltInSymbolTable(compiler->infoSink, &symbolTable, compiler->getLanguage(), compiler->cgProfile);

   TParseContext parseContext(symbolTable, intermediate, compiler->getLanguage(), targetVersion, compiler->cgProfile, options, compiler->infoSink);

   GlobalParseContext = &parseContext;

   setInitialState();

   InitPreprocessor();    
   //
   // Parse the application's shaders.  All the following symbol table
   // work will be throw-away, so push a new allocation scope that can
   // be thrown away, then push a scope for the current shader's globals.
   //
   bool success = true;

   symbolTable.push();
   if (!symbolTable.atGlobalLevel())
      parseContext.infoSink.info.message(EPrefixInternalError, "Wrong symbol table level");


   std::string fullSource ;
   //quick check if there is any reference to non square matrix
   if (strstr(shaderString, "float2x3") != NULL ||
	   strstr(shaderString, "float3x2") != NULL ||
	   strstr(shaderString, "float3x4") != NULL ||
	   strstr(shaderString, "float4x3") != NULL ||
	   strstr(shaderString, "float2x4") != NULL ||
	   strstr(shaderString, "float4x2") != NULL ||

	   strstr(shaderString, "half2x3") != NULL ||
	   strstr(shaderString, "half3x2") != NULL ||
	   strstr(shaderString, "half3x4") != NULL ||
	   strstr(shaderString, "half4x3") != NULL ||
	   strstr(shaderString, "half2x4") != NULL ||
	   strstr(shaderString, "half4x2") != NULL ||

	   
	   strstr(shaderString, "fixed2x3") != NULL ||
	   strstr(shaderString, "fixed3x2") != NULL ||
	   strstr(shaderString, "fixed3x4") != NULL ||
	   strstr(shaderString, "fixed4x3") != NULL ||
	   strstr(shaderString, "fixed2x4") != NULL ||
	   strstr(shaderString, "fixed4x2") != NULL 
	   )
	 //add support for non square matrix
		initFullSource(fullSource);

   fullSource += shaderString;

   int ret = PaParseString(const_cast<char*>(fullSource.c_str()), parseContext);
   if (ret)
      success = false;

   if (success && parseContext.treeRoot)
   {
		TIntermAggregate* aggRoot = parseContext.treeRoot->getAsAggregate();
		if (aggRoot && aggRoot->getOp() == EOpNull)
			aggRoot->setOperator(EOpSequence);

		if (options & ETranslateOpIntermediate)
			intermediate.outputTree(parseContext.treeRoot);

		compiler->TransformAST (parseContext.treeRoot);
		compiler->ProduceGLSL (&parseContext, targetVersion, options);
   }
   else if (!success)
   {
      parseContext.infoSink.info.prefix(EPrefixError);
      parseContext.infoSink.info << parseContext.numErrors << " compilation errors.  No code generated.\n\n";
      success = false;
	  if (options & ETranslateOpIntermediate)
         intermediate.outputTree(parseContext.treeRoot);
   }

   intermediate.remove(parseContext.treeRoot);

   //
   // Ensure symbol table is returned to the built-in level,
   // throwing away all but the built-ins.
   //
   while (! symbolTable.atSharedBuiltInLevel())
      symbolTable.pop();

   FinalizePreprocessor();
   //
   // Throw away all the temporary memory used by the compilation process.
   //
   GlobalPoolAllocator.pop();

   return success ? 1 : 0;
}


int C_DECL Hlsl2Glsl_Translate(
	const ShHandle handle,
	const char* entry,
	ETargetVersion targetVersion,
	unsigned options)
{
   if (handle == 0)
      return 0;

   HlslCrossCompiler* compiler = handle;
   compiler->infoSink.info.erase();
	if (!compiler->IsASTTransformed() || !compiler->IsGlslProduced())
	{
		compiler->infoSink.info.message(EPrefixError, "Shader does not have valid object code.");
		return 0;
	}

	bool ret = compiler->GetLinker()->link(compiler, entry, compiler->cgProfile.c_str(), targetVersion, options);

   return ret ? 1 : 0;
}


const char* C_DECL Hlsl2Glsl_GetShader( const ShHandle handle )
{
	if (!handle)
		return 0;
	return handle->GetLinker()->getShaderText();
}


const char* C_DECL Hlsl2Glsl_GetInfoLog( const ShHandle handle )
{
   if (!InitThread())
      return 0;
   if (handle == 0)
      return 0;
   HlslCrossCompiler* base = static_cast<HlslCrossCompiler*>(handle);
   TInfoSink* infoSink = &(base->getInfoSink());
   infoSink->info << infoSink->debug.c_str();
   return infoSink->info.c_str();
}


int C_DECL Hlsl2Glsl_GetUniformCount( const ShHandle handle )
{
	if (!handle)
		return 0;
   const HlslLinker *linker = handle->GetLinker();
   if (!linker)
      return 0;
   return linker->getUniformCount();
}


const ShUniformInfo* C_DECL Hlsl2Glsl_GetUniformInfo( const ShHandle handle )
{
	if (!handle)
		return 0;
   const HlslLinker *linker = handle->GetLinker();
   if (!linker)
      return 0;
   return linker->getUniformInfo();
}


int C_DECL Hlsl2Glsl_SetUserAttributeNames ( ShHandle handle, 
                                             const EAttribSemantic *pSemanticEnums, 
                                             const char *pSemanticNames[], 
                                             int nNumSemantics )
{
	if (!handle)
		return 0;
	HlslLinker* linker = handle->GetLinker();

   for (int i = 0; i < nNumSemantics; i++ )
   {
      bool bError = linker->setUserAttribName ( pSemanticEnums[i], pSemanticNames[i] );

      if ( bError == false )
         return false;
   }

   return true;
}


int C_DECL Hlsl2Glsl_UseUserVaryings ( ShHandle handle, bool bUseUserVaryings )
{
	if (!handle)
		return 0;
	HlslLinker* linker = handle->GetLinker();
   linker->setUseUserVaryings ( bUseUserVaryings );
   return 1;
}


static bool kVersionUsesPrecision[ETargetVersionCount] = {
	true,	// ES 1.00
	false,	// 1.10
	false,	// 1.20
};

bool C_DECL Hlsl2Glsl_VersionUsesPrecision (ETargetVersion version)
{
	assert (version >= 0 && version < ETargetVersionCount);
	return kVersionUsesPrecision[version];
}
