# Source the ROS 2 workspace install if it has been built.
# This adds generated message packages (e.g. interfaces) to PYTHONPATH
# so that type checkers and Python nodes can find them without a manual
# `source ros/install/setup.bash` in every terminal.
SETUP_SH="${PIXI_PROJECT_ROOT}/ros/install/setup.sh"
if [ -f "${SETUP_SH}" ]; then
    source "${SETUP_SH}"
fi
