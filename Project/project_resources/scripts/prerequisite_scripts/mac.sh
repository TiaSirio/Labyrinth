#!/bin/bash
cd "$(dirname "$0")"

source ./../../configuration_files/versions_config.env

VULKAN_EXT_CHOICE_DIR=./../vulkan_scripts/vulkan_installer_mac.sh

IFW_INSTALL_DIR=./../../../libraries/IFW
rm -f "%ENV_FILE%"
rm -f "%MARKER_FILE%"

# Download and install Homebrew
echo "Downloading and installing Homebrew..."
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)" || { echo "Failed to install Homebrew"; exit 1; }

# Install libraries
echo "Installing libraries"
brew install python cmake glfw glm glslang curl gcc

echo "====================================="
echo "Do you want to install IFW as installer tool? (Y or N)"
echo "====================================="
read -p "Your choice: " choice_ifw

# Convert the input to lowercase for easier comparison
choice_ifw=$(echo "$choice_ifw" | tr '[:upper:]' '[:lower:]')

if [[ "$choice_ifw" == "y" ]]; then
    pip3 install --upgrade pip
    pip3 install aqtinstall

    # Install IFW
    echo Installing IFW...
    aqt install-tool --outputdir %IFW_INSTALL_DIR% ios desktop tools_ifw %IFW_VERSION%

    echo "Updating .env.cmake with the new downloaded file..."
    LINEIFW="set(IFW_PATH $(pwd)/../../../libraries/IFW)"
    FILEIFW="$(pwd)/../../../.env.cmake"
    DIRIFW="$(pwd)/../../../libraries/IFW"

    # Check if the directory is empty and the line is not already in the file
    if [ -z "$(ls -A "$DIRIFW" 2>/dev/null)" ]; then
        grep -qxF "$LINEIFW" "$FILEIFW" || echo "$LINEIFW" >> "$FILEIFW"
    else
        echo "Directory $DIRIFW is not empty. Operation skipped."
    fi
elif [[ "$choice_ifw" == "n" ]]; then
    echo "Skipping installation."
else
    echo "Invalid choice! Please enter Y or N."
    exit 1
fi

echo "====================================="
echo "Do you want to install Doxygen and Graphviz to create the documentation? (Y or N)"
echo "====================================="
read -p "Your choice: " choice_dox

# Convert the input to lowercase for easier comparison
choice_dox=$(echo "$choice_dox" | tr '[:upper:]' '[:lower:]')

if [[ "$choice_dox" == "y" ]]; then
    # Install Doxygen on macOS
    brew install doxygen
    # Install Graphviz on macOS
    brew install graphviz
elif [[ "$choice_dox" == "n" ]]; then
    echo "Skipping installation."
else
    echo "Invalid choice! Please enter Y or N."
    exit 1
fi

start /wait "%VULKAN_EXT_CHOICE_DIR%" dev

# Download and install tiny_obj_loader.h
echo "Downloading tiny_obj_loader.h..."
curl -L -o tiny_obj_loader.h https://raw.githubusercontent.com/tinyobjloader/tinyobjloader/master/tiny_obj_loader.h \
|| { echo "Failed to download tiny_obj_loader.h"; exit 1; } && \
mkdir -p ../../../libraries/TinyObjLoader \
&& mv tiny_obj_loader.h ../../../libraries/TinyObjLoader/ \
|| { echo "Failed to move tiny_obj_loader.h"; exit 1; }

# Update .env.cmake with the new downloaded file
echo "Updating .env.cmake with the new downloaded file..."
LINE="set(TINYOBJ_PATH $(pwd)/../../../libraries/TinyObjLoader)"
FILE="$(pwd)/../../../.env.cmake"
DIR="$(pwd)/../../../libraries/TinyObjLoader"

# Check if the directory is empty and the line is not already in the file
if [ -z "$(ls -A "$DIR" 2>/dev/null)" ]; then
    grep -qxF "$LINE" "$FILE" || echo "$LINE" >> "$FILE"
else
    echo "Directory $DIR is not empty. Operation skipped."
fi

echo "Prerequisite script run at $(date)" > "$MARKER_FILE"
echo "All prerequisites have been installed."

rm vulkan-sdk.dmg
read -p "Press [Enter] to continue..."