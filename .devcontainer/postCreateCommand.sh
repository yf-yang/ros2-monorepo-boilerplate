set -e

git config --global --add safe.directory $(pwd)
echo 'eval "$(pixi completion --shell zsh)"' >> /home/vscode/.zshrc
sudo chown vscode:vscode /dc/pixi-cache
sudo chown vscode:vscode .pixi
