#!/bin/bash

# 기본 빌드 디렉토리 및 타입 설정
BUILD_DIR=build
BUILD_TYPE=Debug

# 빌드 디렉토리 생성
mkdir -p $BUILD_DIR

# CMake 설정
cmake -S . -B $BUILD_DIR -DCMAKE_BUILD_TYPE=$BUILD_TYPE

# 빌드 수행
cmake --build $BUILD_DIR

# Visual Studio Code 디버그 설정 파일 생성
mkdir -p .vscode
cat <<EOL > .vscode/launch.json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "(gdb) Launch",
            "type": "cppdbg",
            "request": "launch",
            "program": "\${workspaceFolder}/${BUILD_DIR}/main",
            "args": [],
            "stopAtEntry": false,
            "cwd": "\${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ],
            "miDebuggerPath": "/usr/bin/gdb",
            "logging": {
                "moduleLoad": true,
                "trace": true
            }
        }
    ]
}
EOL

cat <<EOL > .vscode/tasks.json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "CMake Build",
            "type": "shell",
            "command": "cmake",
            "args": [
                "--build",
                "\${workspaceFolder}/${BUILD_DIR}"
            ],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": ["\$gcc"],
            "detail": "Generated task by CMake Build"
        }
    ]
}
EOL

echo "Build and debug setup complete. To build and debug in VS Code, use the provided tasks."
