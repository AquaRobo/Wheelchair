# Smart Assisstance Wheelchair with Robotic Arm 
## Overview
SAWRA is a wheelchair with a robotic arm to help disabled to gain control of their life without dependency on other helpers
## Project Structure 
```
SAWRA/
│── controlWheelchair/          # Control side on Pi (Git submodule)
│── cvWheelchair/               # Computer vision models (Git submodule)
│── guiWheelchair/              # GUI (Git submodule)
│── .gitmodules                 # Defines the robot submodule
│── .gitignore                  # Ignored files
│── requirments.sh              # Required dependencies of the system
│── README.md                   # This file
```
## Setting Up the Project
### Clone the Repository with Submodule
Run the following command to ensure the control submodules are properly initialized:
```bash
git clone --recurse-submodules https://github.com/AquaRobo/Wheelchair.git
```
If you already cloned the repo but forgot the submodules, run:
```bash
git submodule update --init --recursive
```
