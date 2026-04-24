set -e

USER_ROOT="${HOME:-$USERPROFILE}"

mkdir -p "$USER_ROOT/.ssh"
mkdir -p "$USER_ROOT/.codex"
mkdir -p "$USER_ROOT/.claude"
mkdir -p "$USER_ROOT/.factory"