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
```bash
echo 'game.Players.LocalPlayer.Character.Humanoid.WalkSpeed = 48' > /tmp/spiral/pipe
```

## why is it crappy?
* linux only
* it needs scripts running in the game
* no custom functions
* no ui
* most functions are found with pattern matching at runtime which is slow
* (fixed, yay!) ~~if you try to execute a script after leaving a game and joining a new one, it crashes~~
