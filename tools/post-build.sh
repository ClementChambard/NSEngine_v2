#!/bin/bash

mkdir -p .build/assets/

SHADER_INPUT_PATH="assets/shaders"
SHADER_OUTPUT_PATH=".build/assets/shaders"

compile_shader_sub() {
  NAME=$1
  TYPE=$2
  echo "  - $SHADER_INPUT_PATH/$NAME.$TYPE.glsl -> $SHADER_OUTPUT_PATH/$NAME.$TYPE.spv"
  glslc -fshader-stage=$TYPE $SHADER_INPUT_PATH/$NAME.$TYPE.glsl -o $SHADER_OUTPUT_PATH/$NAME.$TYPE.spv
  if [ $? -ne 0 ]; then
    echo "Error: $?"
    exit 1
  fi
}

compile_shader() {
  echo "-- Compiling $1 $2"
  SHADER_BASE=$1
  SHADER_NAME=$2
  # TODO: add support for different shader stages
  compile_shader_sub "$SHADER_BASE.$SHADER_NAME" vert
  compile_shader_sub "$SHADER_BASE.$SHADER_NAME" frag
}



# compile all shaders
echo "Compiling shaders..."

mkdir -p $SHADER_OUTPUT_PATH

compile_shader "Builtin" "MaterialShader"
# ...


# copy assets
echo "Copying assets..."
echo "cp -R \"assets\" \".build\""
cp -R "assets" ".build"
