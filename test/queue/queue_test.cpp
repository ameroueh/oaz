#include <iostream>
#include <thread>

#include "oaz/queue/queue.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


using namespace oaz::queue;
using namespace testing;
using namespace std;


TEST (InstantiationTest, Default) {
	 SafeQueue<int> q;
}

TEST (LockTest, Default) {
	 SafeQueue<int> q;
	 q.Lock();
}

TEST (UnlockTest, Default) {
	 SafeQueue<int> q;
	 q.Lock();
	 q.Unlock();
}

void addIntegersToQueue(SafeQueue<int>* q, int n) {
	for(int i=0; i!=n; ++i) {
		q->Lock();
		q->push(i);
		q->Unlock();
	}
}

void emptyQueue(SafeQueue<int>* q) {
	bool success = true;
	while(success) {
		q->Lock();
		if(!q->empty()) {
			q->pop();
		}
		else {
			success = false;
		}
		q->Unlock();
	}
}

TEST (ConcurrentUse, Default) {
	int counter=0;
	int n=10000;

	SafeQueue<int> q;
	std::thread first(addIntegersToQueue, &q, n);
	std::thread second(addIntegersToQueue, &q, n); 

	first.join();
	second.join();
	
	ASSERT_EQ(2*n, q.size());
	
	std::thread first_remove(emptyQueue, &q);
	std::thread second_remove(emptyQueue, &q);

	first_remove.join();
	second_remove.join();

	ASSERT_TRUE(q.empty());
}
