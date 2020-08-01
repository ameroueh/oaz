#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include<iostream>

#include "H5Cpp.h"

using namespace H5;

TEST (HDF5, Default) {
	const H5std_string  FILE_NAME( "test.h5" );
	const H5std_string  DATASET_NAME( "/Data/IntArray" );
	const int   NX = 5;                    // dataset dimensions
	const int   NY = 6;
	const int   RANK = 2;

	int i, j;
	int data[NX][NY];          // buffer for data to write
	for (j = 0; j < NX; j++)
		for (i = 0; i < NY; i++)
			data[j][i] = i + j;
      
	H5File file( FILE_NAME, H5F_ACC_TRUNC );

	hsize_t     dimsf[2];              // dataset dimensions
	dimsf[0] = NX;
	dimsf[1] = NY;
	DataSpace dataspace( RANK, dimsf );

	IntType datatype( PredType::NATIVE_INT );
        datatype.setOrder( H5T_ORDER_LE );

	file.createGroup("/Data");
	DataSet dataset = file.createDataSet( DATASET_NAME, datatype, dataspace );

	dataset.write( data, PredType::NATIVE_INT );
}
