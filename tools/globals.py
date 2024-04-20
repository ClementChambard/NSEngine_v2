import os

# get the executable's path
NS_PATH = os.path.dirname(__file__)

# executable is in $NSENGINE_DEV_PATH/tools
NSENGINE_DEV_PATH = os.path.dirname(NS_PATH)

NSENGINE_BUILD_DIR = "build"
NSENGINE_BUILD_PATH = f"{NSENGINE_DEV_PATH}/{NSENGINE_BUILD_DIR}"

assert NS_PATH.endswith("/tools")
assert NS_PATH.startswith("/")
assert len(NSENGINE_DEV_PATH) > 2
