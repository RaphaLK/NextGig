# NextGig
 Job portal that connects freelancers with clients based on job skills and project requirements. As of 2026, unsure how this will work since the C++ SDK of Firebase has been deprecated.

## Overview
The architecture has the following components:
- **Frontend**: Qt5 Desktop Application (C++)
- **Backend**: C++-based server instance that utilizes QTcpSockets (Runs on localhost:8080)
## Build Process
cd build
cmake ..
ninja
./frontend/NextGigFrontend

## Requirements
- Qt5
- Qt5Widgets
- Firebase C++ SDK **https://firebase.google.com/docs/cpp/setup?platform=ios**
- CMake
- Ninja
  
#### Windows
- **Visual Studio 2019/2022** with C++ development tools, OR
- **MinGW-w64** with GCC
- **Ninja** build system (It's slow otherwise)

#### macOS
- NOTICE: Unsure about MacOS compatibility.

#### Linux (Ubuntu/Debian)
- **Build essentials**:
  ```bash
  sudo apt-get update
  sudo apt-get install build-essential cmake ninja-build
  sudo apt-get install libsecret-1-dev libglib2.0-dev
  ```

### Setup for Firebase SDK

1. Download Firebase C++ SDK from [Firebase Console](https://firebase.google.com/docs/cpp/setup)
2. Extract to project root as `firebase_cpp_sdk/`
3. Ensure the directory structure matches:
   ```
   frontend/
   backend/
   src/
   build/
   firebase_cpp_sdk/
   ├── include/
   └── libs/
       ├── windows/x64/     # Windows libraries
       ├── macos/           # macOS libraries
       └── linux/x86_64/    # Linux libraries
4. The CMakeLists.txt file should handle paths for Firebase SDK binaries (On Linux, at least)

### RUN STEPS (IMPORTANT)
Binaries will run on x86 Linux Machines (Probably).
- Run the executable using the startBackend.sh and startFrontend.sh on different console instances (Assuming you successfully built the binaries.
- Feel free to start either executable first, order doesn't matter as the backend is just an RPC interface. No initial sync is required.

Otherwise, set up and run the b_* files (backend, then frontend) to build and run each binary

### Future Work
- If I can recall, async requests aren't handled properly via awaits and weak references. Instead, we use runtime checks and literal timers to sync things up. A whole refactor is needed. Additionally, the server.cc file is big piece of garbage due to the way Firebase works alongside C++ and just how it was organized initially.
