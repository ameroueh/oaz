#ifndef __NEURAL_NETWORK_H__
#define __NEURAL_NETWORK_H__

#include <string>

#include "tensorflow/cc/client/client_session.h"

namespace oaz::nn {
	
	void loadModel(tensorflow::ClientSession*, std::string, std::string);
}
#endif // __NEURAL_NETWORK_H__
