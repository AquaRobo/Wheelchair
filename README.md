# ASV
## Overview
ASV is the complete system of autonomous vehicle that cleans oceans from debris. 
## Project Structure 
```
ASV/
│── control/          # Control side on Pi (Git submodule)
│── cv/               # Computer vision models (Git submodule)
│── gui/              # GUI (Git submodule)
│── .gitmodules       # Defines the robot submodule
│── .gitignore        # Ignored files
│── requirments.sh    # Required dependencies of the system
│── README.md         # This file
```
## Setting Up the Project
### Clone the Repository with Submodule
Run the following command to ensure the control submodules are properly initialized:
```bash
git clone --recurse-submodules https://github.com/AquaRono/ASV.git
```
If you already cloned the repo but forgot the submodules, run:
```bash
git submodule update --init --recursive
```