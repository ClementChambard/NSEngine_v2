#!/bin/bash

SHADER_PATH="assets/shaders"

compile_shader_sub() {
  NAME="$SHADER_PATH/$1"
  TYPE=$2
  echo "  - $NAME.$TYPE.glsl -> $NAME.$TYPE.spv"
  glslc -fshader-stage=$TYPE $NAME.$TYPE.glsl -o $NAME.$TYPE.spv
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

# TODO: auto add shaders
compile_shader "Builtin" "MaterialShader"
compile_shader "Builtin" "UIShader"
# ...
