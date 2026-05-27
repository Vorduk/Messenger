# Messenger

## Build Instructions

### Windows

1. **Install OpenSSL**
Download and install OpenSSL for Windows (e.g., from slproweb.com). Make sure the bin directory is added to your system PATH.

2. **Build the project**
Run the build script: build-cmake-windows.bat

3. **Generate SSL keys (server only)**
Run the SSL generation script: generateSSL.bat

4. **Copy assets**
Place the fonts folder into the directory containing the .exe files.

### Linux

1. **Install OpenSSL**
sudo apt update && sudo apt install openssl

2. **Build the project**
Use the commands from build-linux.txt (execute them line by line).

3. **Generate SSL keys (server only)**
Run the script: chmod +x generateSSL_linux.bat && ./generateSSL_linux.bat

4. **Copy assets**
Place the fonts folder into the directory containing the executable files.