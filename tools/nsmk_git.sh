GIT_DIR=$NSMK_PATH/..
GIT_PUSH_ALL_TARGETS="engine assets testbed tests tools CMakeLists.txt README.md"

nsmk_cmd_git_push_all() {
  echo "git add $GIT_PUSH_ALL_TARGETS" &&
  git add $GIT_PUSH_ALL_TARGETS &&
  echo "git commit -m \"$1\"" &&
  git commit -m "$1" &&
  echo "git push" &&
  git push
  return $?
}

nsmk_cmd_git() {
  cd $GIT_DIR
  if [ $# -lt 1 ]; then
    echo "Usage: nsmk git <command> ..."
    return 1
  fi
  if [ "$1" = "pushall" ]; then
    if [ $# -ne 2 ]; then
      echo "invalid number of arguments: $#"
      echo "Usage: nsmk git pushall <commit_message>"
      return 1
    fi
    nsmk_cmd_git_push_all "$2"
    return $?
  else
    echo "invalid command: $1"
    return 1
  fi
  return 0
}
