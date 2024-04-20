import os
import subprocess

from globals import NSENGINE_BUILD_PATH, NSENGINE_DEV_PATH


def _compile_all():
    try:
        os.chdir(NSENGINE_BUILD_PATH)
    except:
        print("\033[31mNOT CONFIGURED\033[0m")
        exit(1)
    result = subprocess.run(["cmake", ".."]).returncode
    if result != 0:
        print("\033[31mCMAKE ERROR\033[0m")
        exit(1)
    result = subprocess.run(["ninja"]).returncode
    if result != 0:
        print("\033[31mCOMPILING ERROR\033[0m")
        exit(1)
    else:
        print("\033[33mCOMPILING SUCCESS\033[0m")

    os.chdir(NSENGINE_DEV_PATH)
    result = subprocess.run(["./tools/post-build.sh"]).returncode
    if result != 0:
        print("\033[31mPOSTBUILD ERROR\033[0m")
        exit(1)
    else:
        print("\033[33mPOSTBUILD SUCCESS\033[0m")


def configure(args):
    os.mkdir(NSENGINE_BUILD_PATH)
    os.chdir(NSENGINE_BUILD_PATH)
    subprocess.run(["cmake", "-GNinja", ".."])


def git(args):
    print("not implemented")


def build(args):
    _compile_all()


def edit(args):
    os.chdir(NSENGINE_DEV_PATH)
    subprocess.run(["nvim"])


def run(args):
    _compile_all()
    os.chdir(NSENGINE_BUILD_PATH)
    subprocess.run(["testbed/testbed"])


def debug(args):
    _compile_all()
    os.chdir(NSENGINE_BUILD_PATH)
    subprocess.run(["gdb", "testbed/testbed"])


def test(args):
    _compile_all()
    os.chdir(NSENGINE_BUILD_PATH)
    subprocess.run(["tests/tests"])


def clean(args):
    if args.all:
        subprocess.run(["rm", "-rf", NSENGINE_BUILD_PATH])
    else:
        os.chdir(NSENGINE_BUILD_PATH)
        subprocess.run(["cmake", "--build", ".", "--", "clean"])
