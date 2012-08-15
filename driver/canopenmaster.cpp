#include <queue>
#include <map>
#include <inttypes.h>
#include <iostream>
#include <thread>
#include <chrono>
#include "canopenmaster.h"

namespace canopen {
  std::chrono::milliseconds controllerCycleDuration_msec(100);
  std::map<std::string, Chain*> chainMap;

  void homingCallback(std::string chainName) { 
    chainMap[chainName]->chainHoming();
  }
  
  void initCallback(std::string chainName) { 
    chainMap[chainName]->chainInit();
  }

  void initChainMap(std::string robotDescFilename) {

    // parse description file: // todo: make this more flexible: YAML etc.
    std::ifstream fin(robotDescFilename);
    std::string ll;

    std::vector<std::string> all_chainNames;
    std::vector< std::vector<std::string> > all_deviceNames;
    std::vector< std::vector<std::string> > all_CANbuses;
    std::vector< std::vector<uint16_t> > all_CANids;

    while (std::getline(fin, ll)) {
      std::istringstream x(ll);
      std::string chainName;
      std::vector<std::string> deviceNames;
      std::vector<std::string> CANbuses;
      std::vector<uint16_t> CANids;
      std::string temp;
      x >> chainName;
      while (x >> temp) {
	deviceNames.push_back(temp);
	x >> temp;
	CANbuses.push_back(temp);
	x >> temp;
	CANids.push_back(std::stoi(temp));
      }

      all_chainNames.push_back(chainName);
      all_deviceNames.push_back(deviceNames);
      all_CANbuses.push_back(CANbuses);
      all_CANids.push_back(CANids);
    }

    // construct chainMap:
    for (int i=0; i<all_chainNames.size(); i++) {
      chainMap[all_chainNames[i]] = new Chain(all_chainNames[i], all_deviceNames[i],
					      all_CANbuses[i], all_CANids[i]);
    }
  }

  void masterFunc() {
    // std::vector<std::thread> SDOthreadPool;
    // SDO calls block until return or timeout, hence the thread pool
    // todo: currently, SDO sending is directly to bus
    
    // send out to CAN bus at specified rate:
    auto tic = std::chrono::high_resolution_clock::now();
    while (true) {
      // send PDOs to CAN bus (if chain has sendPosActive_==true):
      for (auto chain : chainMap) {
	if (chain.second->sendPosActive_) {
	  std::cout << chain.first << " send." << std::endl; // todo: only for testing
	  // chainMap[chain.first].sendPos();
	}
	else // todo: only for debug, remove later
	  std::cout << chain.first << " NOSEND!" << std::endl;
      }
	
      std::cout << "master tic toc" << std::endl;
      std::cout << "master outgoing queue size: " << outgoingMsgQueue.size() << std::endl;
      
      // send sync: (always or only if specific condition is met?)
      // sendSync();

      while (std::chrono::high_resolution_clock::now() < tic + controllerCycleDuration_msec) {
	// fetch a message from the outgoingMsgQueue and send to CAN bus:
	// todo: move to thread pool; just sleep here!
	if (outgoingMsgQueue.size() > 0) {
	  outgoingMsgQueue.front().writeCAN(false, true); // todo: remove writeMode!
	  outgoingMsgQueue.pop();
	}
	std::this_thread::sleep_for(std::chrono::microseconds(10)); 
      }

      tic = std::chrono::high_resolution_clock::now();
    }

  }


  void initMasterThread() { // todo: chain descriptions as argument
    // using_master_thread = true;
    std::thread master_thread(masterFunc);
    master_thread.detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}



void initNonPDOSenderThread() {

  // todo: send nonPDOs from here

}

  
// std::queue<std::vector<uint32_t> > outgoingPosQueue;
// std::queue<std::vector<uint32_t> > incomingPosQueue;





/*
  int main() {
  
  std::vector<uint32_t> x {1,2,3};
  std::vector<uint32_t> y {1,2,3,4};

  outgoingPosQueue.push(x);
  outgoingPosQueue.push(y);


  while (outgoingPosQueue.size() > 0) {
  std::cout << "hi" << std::endl;
  std::vector<uint32_t> z = outgoingPosQueue.front();
  outgoingPosQueue.pop();
  std::cout << z.size() << std::endl;

    

  }

  }
  }
*/
