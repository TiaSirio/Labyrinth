#!/bin/bash

source ./../../configuration_files/versions_config.env

is_dev=false

# Check if the first argument is 'dev' or 'install'
if [ "$1" == "dev" ]; then
    is_dev=true
elif [ "$1" == "install" ]; then
    is_dev=false
else
    echo "Invalid argument. Defaulting to 'install'."
    is_dev=true
fi

# Install Vulkan tools
sudo apt install -y vulkan-tools="$VULKAN_VERSION"

# Install Vulkan development libraries
sudo apt install -y libvulkan-dev"$VULKAN_VERSION"

# Install Vulkan SPIR-V tools
sudo apt install -y spirv-tools

# Install GLFW development library
sudo apt install -y libglfw3

# Install GLM development library
sudo apt install -y libglm-dev

# Install glslang tools
sudo apt-get install -y glslang-tools

# Only dev
if [ "$is_dev" = true ]; then
    sudo apt install -y libglfw3-dev
    sudo apt install -y vulkan-validationlayers-dev
fi