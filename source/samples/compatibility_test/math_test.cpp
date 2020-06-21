//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "math_test.h"
//----------------------------------------------------------------------------//
MathTestUnit::MathTestUnit() : TestUnit("MATH")
{
	tasks.Add(ut::MakeUnique<MatrixTask>());
	tasks.Add(ut::MakeUnique<QuaternionTask>());
}

//----------------------------------------------------------------------------//
MatrixTask::MatrixTask() : TestTask("Matrix")
{ }

void MatrixTask::Execute()
{
	ut::Matrix<3, 3> matrix3x3_0 = ut::Matrix<3, 3>::MakeIdentity();
	
	// identity
	if (matrix3x3_0 != ut::Matrix<3, 3>(1, 0, 0,
	                                    0, 1, 0,
	                                    0, 0, 1))
	{
		report += "FAIL: Invalid identity matrix";
		failed_test_counter.Increment();
		return;
	}

	// negation
	matrix3x3_0 = -matrix3x3_0;
	if (matrix3x3_0 != ut::Matrix<3, 3>(-1,  0,  0,
	                                     0, -1,  0,
	                                     0,  0, -1))
	{
		report += "FAIL: Invalid negation operator";
		failed_test_counter.Increment();
		return;
	}

	// copy constructor
	ut::Matrix<3, 3> matrix3x3_1(matrix3x3_0);
	if (matrix3x3_1 != ut::Matrix<3, 3>(-1,  0,  0,
	                                     0, -1,  0,
	                                     0,  0, -1))
	{
		report += "FAIL: Invalid copy constructor";
		failed_test_counter.Increment();
		return;
	}

	// addition assignment operator
	matrix3x3_1 += matrix3x3_0;
	if (matrix3x3_1 != ut::Matrix<3, 3>(-2,  0,  0,
	                                     0, -2,  0,
	                                     0,  0, -2))
	{
		report += "FAIL: Invalid addition assignment operator";
		failed_test_counter.Increment();
		return;
	}

	// assignment and multiplication operator
	matrix3x3_0 = ut::Matrix<3, 3>( 1, 2, -3,
	                                4, 5,  6,
	                               -7, 8,  9);
	matrix3x3_1 = ut::Matrix<3, 3>( 9,  1, 3,
	                                2, -4, 6,
	                               -1,  7, 5);
	matrix3x3_1 = matrix3x3_1 * matrix3x3_0;
	if (matrix3x3_1 != ut::Matrix<3, 3>(-8,  47, 6,
	                                    -56, 32, 24,
	                                    -8,  73, 90))
	{
		report += ut::String("FAIL: Invalid multiplication operator (3x3*3x3)");
		failed_test_counter.Increment();
		return;
	}

	// multiplication of matrices with different size
	ut::Matrix<3, 5> matrix3x5_0 = ut::Matrix<3, 5>(0,  1,  2,  3,  4,
	                                                5,  6,  7,  8,  9,
		                                            10, 11, 12, 13, 14);
	ut::Matrix<3, 5> matrix3x5_1 = matrix3x3_0.Multiply(matrix3x5_0);
	if (matrix3x5_1 != ut::Matrix<3, 5>(-20, -20, -20, -20, -20,
	                                     85, 100, 115, 130, 145,
	                                    130, 140, 150, 160, 170))
	{
		report += ut::String("FAIL: Invalid multiplication operator (3x3*3x5)");
		failed_test_counter.Increment();
		return;
	}

	// multiplication of vector and matrix
	ut::Vector<3> vec3_0(11, 22, 33);
	vec3_0 = vec3_0 * matrix3x3_0;
	if (vec3_0 != ut::Vector<3>(-132, 396, 396))
	{
		report += ut::String("FAIL: Invalid multiplication operator (1x3*3x3)");
		failed_test_counter.Increment();
		return;
	}

	// inversion
	ut::Optional< ut::Matrix<3, 3> > inverse_result = matrix3x3_0.Invert();
	if (!inverse_result)
	{
		report += ut::String("FAIL: Invalid invertible matrix (degenerate)");
		failed_test_counter.Increment();
		return;
	}
	if (matrix3x3_0 * inverse_result.Get() != ut::Matrix<3, 3>(1, 0, 0,
	                                                           0, 1, 0,
	                                                           0, 0, 1))
	{
		report += ut::String("FAIL: Invalid invertible matrix");
		failed_test_counter.Increment();
		return;
	}

	// check degenerate matrix
	matrix3x3_0 = ut::Matrix<3, 3>(0, 2, 0,
	                               4, 0, 6,
	                               0, 8, 0);
	inverse_result = matrix3x3_0.Invert();
	if (inverse_result)
	{
		report += ut::String("FAIL: Degenerate matrix wasn't detected");
		failed_test_counter.Increment();
		return;
	}

	// vector named elements
	vec3_0.X() = 11.0f;
	vec3_0.Y() = 13.0f;
	vec3_0.Z() = 15.0f;

	// dot product
	ut::Vector<3> vec3_1(1, 2, 3);
	float dot_product = vec3_0.Dot(vec3_1);
	if (!ut::Equal(dot_product, 82.0f))
	{
		report += ut::String("FAIL: Invalid dot product.");
		failed_test_counter.Increment();
		return;
	}

	// length
	if (!ut::Equal(vec3_0.Length(), 22.6936114f))
	{
		report += ut::String("FAIL: Invalid length.");
		failed_test_counter.Increment();
		return;
	}

	// cross product
	ut::Vector<3> vec3_2 = vec3_0.Cross(vec3_1);
	if (vec3_2 != ut::Vector<3>(9,-18,9))
	{
		report += ut::String("FAIL: Invalid cross product.");
		failed_test_counter.Increment();
		return;
	}

	// normalize
	vec3_2 = vec3_2.Normalize();
	if (!ut::Equal(vec3_2.Length(), 1.0f))
	{
		report += ut::String("FAIL: Invalid normalization.");
		failed_test_counter.Increment();
		return;
	}

	// different tags assignment
	ut::Matrix<1, 4> m1x4(0.0f);
	ut::Vector<4> vec4_tag(0.0f);
	vec4_tag = m1x4;

	report += "Success";
}

//----------------------------------------------------------------------------//
QuaternionTask::QuaternionTask() : TestTask("Quaternion")
{ }

void QuaternionTask::Execute()
{
	ut::Quaternion<float> q_0 = ut::Quaternion<float>::MakeFromAngleAndAxis(45.0f, ut::Vector<3>(1, 0, 0));
	ut::Quaternion<float> q_1 = ut::Quaternion<float>::MakeFromAngles(ut::Vector<3>(45, 30, 60));
	q_0 *= q_1;
	q_0 = q_0 * q_1;

	bool b = q_0 != q_1;

	q_0 = q_0.Invert();
	
	q_0 = q_0.Normalize();

	ut::Vector<3> vec3(0, 1, 0);
	q_0 = q_0 * vec3;
	vec3 = q_0.Rotate(vec3);

	ut::Matrix<3, 3> m3x3 = q_0.ToTransform<3>();
	ut::Matrix<4, 4> m4x4 = q_0.ToTransform<4>();

	float angle = q_0.GetAngle();
	vec3 = q_0.GetAxis();

	ut::Quaternion<double> q_d = ut::Quaternion<double>::MakeFromAngles(ut::Vector<3, double>(45, 30, 60));
	q_d = q_d * q_d;

	report += "Success";
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//