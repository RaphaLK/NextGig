# NextGig
 Job portal that connects freelancers with clients based on job skills and project requirements.

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
- **Ninja** build system (recommended)

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
4. The CMakeLists.txt file should handle paths for Firebase SDK binaries
   ```

### RUN STEPS (IMPORTANT)
Assuming the executables work without compilation needed (since this submission is zipped for a class project):
- Run the executable using the startBackend.sh and startFrontend.sh on different console instances
- Feel free to start either executable first, order doesn't matter as the FrontEnd Client establishes TCP connections to the backend as needed.
- DO NOT RUN b_startBackend.sh and b_startFrontend without dependencies installed, this builds the project and WILL cause errors.
- Otherwise, feel free to run b_start***.