#include "boost/multi_array.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "stdint.h"
#include "tensorflow/c/c_api.h"
#include "tensorflow/c/eager/c_api.h"

TEST(TensorFlowTest, Default) {
  TF_Status* status = TF_NewStatus();
  TFE_ContextOptions* opts = TFE_NewContextOptions();
  TFE_Context* ctx = TFE_NewContext(opts, status);
  TFE_DeleteContextOptions(opts);
  TFE_DeleteContext(ctx);
  TF_DeleteStatus(status);
}

TEST(TensorFlowTest, MatrixMultiplication) {
  TF_DataType data_type = TF_FLOAT;
  int num_dims = 2;
  size_t len = 10000 * 10000 * sizeof(float);
  std::array<int64_t, 2> dimensions = {10000, 10000};
  int num_ret_vals = 1;
  std::array<TFE_TensorHandle*, 1> ret_vals;

  TF_Tensor* tensor1 =
      TF_AllocateTensor(data_type, dimensions.data(), num_dims, len);

  TF_Tensor* tensor2 =
      TF_AllocateTensor(data_type, dimensions.data(), num_dims, len);
  boost::multi_array_ref<float, 2> matrix1((float*)TF_TensorData(tensor1),
                                           boost::extents[10000][10000]);
  boost::multi_array_ref<float, 2> matrix2((float*)TF_TensorData(tensor2),
                                           boost::extents[10000][10000]);

  TF_Status* status = TF_NewStatus();
  TFE_ContextOptions* opts = TFE_NewContextOptions();
  TFE_Context* ctx = TFE_NewContext(opts, status);
  TFE_DeleteContextOptions(opts);

  TFE_TensorHandle* handle1 = TFE_NewTensorHandle(tensor1, status);
  TFE_TensorHandle* handle2 = TFE_NewTensorHandle(tensor2, status);

  TFE_Op* matrix_mul = TFE_NewOp(ctx, "MatMul", status);
  TFE_OpAddInput(matrix_mul, handle1, status);
  TFE_OpAddInput(matrix_mul, handle2, status);
  /* TFE_OpSetAttrType(matrix_mul, "T", TFE_TensorHandleDataType(handle1)); */

  TFE_Execute(matrix_mul, ret_vals.data(), &num_ret_vals, status);

  TFE_DeleteTensorHandle(handle1);
  TFE_DeleteTensorHandle(handle2);
  TFE_DeleteTensorHandle(ret_vals[0]);

  TFE_DeleteContext(ctx);
  TF_DeleteStatus(status);

  TF_DeleteTensor(tensor1);
  TF_DeleteTensor(tensor2);
}
