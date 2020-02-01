#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/tensor.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include<iostream>

using namespace tensorflow;
using namespace tensorflow::ops;

TEST (TensorFlowTest, Default) {
	Scope root = Scope::NewRootScope();
	auto A = Const(root, { {3.f, 2.f}, {-1.f, 0.f} });
	auto b = Const(root, { {3.f, 5.f} });
	auto v = MatMul(root.WithOpName("v"), A, b, MatMul::TransposeB(true));
	std::vector<Tensor> output;
	ClientSession session(root);
	TF_CHECK_OK(session.Run({v}, &output));
	auto mat = output[0].matrix<float>();
	ASSERT_EQ(19, mat(0, 0));
	ASSERT_EQ(-3, mat(0, 1));
}

TEST (DirectTensorAccess, Default) {
	Scope root = Scope::NewRootScope();
	Tensor A(DT_FLOAT, TensorShape({1}));
	float *pA = A.flat<float>().data();
	*pA = 0;
	auto vec0 = A.vec<float>();
	ASSERT_EQ(0, vec0(0)); 
	*pA = 1;
	auto vec1 = A.vec<float>();
	ASSERT_EQ(1, vec1(0)); 
}

TEST (DirectTensorAccess, MiniNN) {
	Scope root = Scope::NewRootScope();

	Tensor Data(DT_FLOAT, TensorShape({1, 2}));
	auto DataView = Data.tensor<float, 2>();

	auto Input = Placeholder(root, DT_FLOAT);
	auto LayerWeights0 = Const(root, { {1.f, 2.f}, {3.f, 4.f} }); 
	auto LayerBiases0 = Const(root, { {5.f, 6.f} });
	auto mul0_output = MatMul(root.WithOpName("Mul0"), Input, LayerWeights0);
	auto add0_output = Add(root.WithOpName("Add0"), mul0_output, LayerBiases0);

	std::vector<Tensor> output;

	ClientSession session(root);

	for(int i=0; i!=2; ++i) {
		for(int j=0; j!=2; ++j) {
			DataView(0, 0) = (float) i;
			DataView(0, 1) = (float) j;
			session.Run({{Input, Data}}, {add0_output}, &output);
			auto mat = output[0].matrix<float>();
			ASSERT_EQ((float) (i + 3*j + 5), mat(0, 0));
			ASSERT_EQ((float) (2*i + 4*j + 6), mat(0, 1));
		}
	}
}


