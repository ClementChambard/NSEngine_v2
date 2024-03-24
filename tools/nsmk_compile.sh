nsmk_compile_all() {
  cd $NSENGINE_BUILD_DIR > /dev/null 2> /dev/null
  if [ $? -ne 0 ]; then
    echo -e "\e[31mNOT CONFIGURED\e[0m"
    exit 1
  fi
  cmake .. &&
  ninja

  if [ $? -ne 0 ]; then
    echo -e "\e[31mCOMPILING ERROR\e[0m"
    return 1
  else
    echo -e "\e[33mCOMPILING SUCCESS\e[0m"
  fi

  cd .. &&
  ./tools/post-build.sh

  if [ $? -ne 0 ]; then
    echo -e "\e[31mPOSTBUILD ERROR\e[0m"
    return 1
  else
    echo -e "\e[33mPOSTBUILD SUCCESS\e[0m"
  fi

  return 0
}
