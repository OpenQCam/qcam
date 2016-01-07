#pragma once

#include <string>
#include <vector>
#include <stdio.h>
#include "../scv/scv.h"

using namespace scv;


//extern void convTri(const Matrix &input, Matrix &output);
//
extern void convTri_fast(const Matrix &input, Matrix &output);
//
extern void convTri_c(const Matrix &input, Matrix &output);
//
//extern void convTri_s2u(const Matrix &input, Matrix &output);

extern void convTri_total_M(const Matrix &input, Matrix &output, const short* const offset);

extern void pushdata(const Matrix &input, Matrix &output, const int* const channel);

extern void savetxt(const char* filename, Matrix& mat);



