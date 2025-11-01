#!/bin/bash

# Set Vulkan version from second argument
source ./../../configuration_files/versions_config.env

# Define Vulkan SDK URL and directory
VULKAN_SDK_URL="https://sdk.lunarg.com/sdk/download/mac/vulkan-sdk.dmg"
VULKAN_SDK_DIR="$HOME/VulkanSDK/$VULKAN_VERSION"

# Default configuration file
configFile="../../configuration_files/vulkan_installer_config_install.ini"

# Check the input parameter for "dev" or "install"
if [ "$1" == "dev" ]; then
    configFile="../../configuration_files/vulkan_installer_config_dev.ini"
elif [ "$1" == "install" ]; then
    configFile="../../configuration_files/vulkan_installer_config_install.ini"
else
    echo "Invalid input provided. Defaulting to 'install'."
    configFile="../../configuration_files/vulkan_installer_config_install.ini"
fi

# Initialize the final parameters string
installParams=""

# Read the configuration file line by line
while IFS="=" read -r key value; do
    # Remove comments after the hash (#)
    value=$(echo "$value" | cut -d '#' -f 1)
    # Check if the value is true
    if [ "${value,,}" == "true" ]; then
        installParams="$installParams $key"
    fi
done < "$configFile"

# Output the composed parameters string
#echo "$installParams"

# Check if Vulkan SDK directory exists and is not empty
if [ -d "$VULKAN_SDK_DIR" ] && [ "$(ls -A "$VULKAN_SDK_DIR")" ]; then
    echo "Vulkan SDK directory already exists and is not empty, skipping download and extraction."
else
    echo "Downloading Vulkan SDK..."
    curl -L -o "$INSTALL_DIR/vulkan-sdk.dmg" "$VULKAN_SDK_URL"

    echo "Installing Vulkan SDK..."
    wine "$INSTALL_DIR/vulkan-sdk.dmg" --accept-licenses --default-answer --confirm-command install $output_vulkan_ext
    
    echo "Deleting Vulkan SDK installer..."
    rm "$INSTALL_DIR/vulkan-sdk.dmg"
fi