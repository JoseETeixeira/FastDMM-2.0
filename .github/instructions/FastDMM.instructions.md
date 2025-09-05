---
applyTo: '**'
---
# FastDMM Instructions
This repository contains FastDMM, a tool designed to facilitate the creation and manipulation of DMM files for BYOND game development. FastDMM aims to streamline the process of editing map files, making it easier for developers to manage their game environments.
## Features
- **Efficient DMM Editing**: FastDMM provides a user-friendly interface for editing DMM files, allowing developers to quickly make changes to their maps.
- **Batch Processing**: The tool supports batch processing of multiple DMM files, saving time when working on large projects.
- **Integration with BYOND**: FastDMM is designed to work seamlessly with BYOND, ensuring compatibility and ease of use.
- **Customizable Settings**: Users can customize various settings to tailor the tool to their specific needs.
## Running
To run FastDMM, follow these steps:
1. Set the JAVA_HOME and env as follows: `$env:JAVA_HOME="C:\Program Files\Eclipse Adoptium\jdk-8.0.462.8-hotspot"; $env:Path="$env:JAVA_HOME\bin;" + $env:Path; java -version`
2. Navigate to the directory where FastDMM is located.
3. Execute `.\gradlew run --no-daemon --stacktrace` to start the application.

## Testing
Always test your changes by compiling and running the application. Ensure that all functionalities work as expected and that there are no errors during execution.

## Building
1. Set the JAVA_HOME and env as follows: `$env:JAVA_HOME="C:\Program Files\Eclipse Adoptium\jdk-8.0.462.8-hotspot"; $env:Path="$env:JAVA_HOME\bin;" + $env:Path; java -version`
2. Navigate to the directory where FastDMM is located.
3. Execute `.\gradlew fatCapsule` to build the application.