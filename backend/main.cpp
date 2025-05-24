#include "server.h"
#include "server.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
  std::cout << "Starting NextGig Backend Server" << std::endl;
  Server server;
  
  // Keep the server running
  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  
  return 0;
}