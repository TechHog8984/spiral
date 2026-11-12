# spiral
spiral is a crappy Luau script executor for Vortex targetting Linux operating systems

## build
cmake.

## run
run Vortex and then ./inject.sh. wait for '\[spiral] initialization finished.' to be printed and then join a game.

## execute scripts
write script content to /tmp/spiral/pipe:
```bash
echo 'print(game.Players.LocalPlayer.Name)' > /tmp/spiral/pipe
```
