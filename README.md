# Smart Assisstance Wheelchair with Robotic Arm 
## Overview
SAWRA is a voice activated wheelchair that navigates autonomously to various rooms and brings various objects using a robotic arm.
## Project Structure 
```
SAWRA/
│── controlWheelchair/          # Control side on micro-processor (Git submodule)
│── ComputerVision/             # Computer vision models (Git submodule)
│── wheelchair_speech/          # ML modules (Git submodule)
│── .gitmodules                 # Defines the robot submodule
│── .gitignore                  # Ignored files
│── requirments.txt             # Required dependencies of the system
│── README.md                   # This file
```
## Setting Up the Project
### Clone the Repository with Submodule
Run the following command to ensure the control submodules are properly initialized:
> `` git clone --recurse-submodules https://github.com/AquaRobo/Wheelchair.git ``

If you already cloned the repo but forgot the submodules, run:

> `` git submodule update --init --recursive ``

Install dependencies 

> `` pip install -r requirements.txt `` 


