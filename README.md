# Wintile

Wintile is a tiling window manager for windows.

<img src="https://i.imgur.com/inFT801.png" width="500">  

### Features
* Simple keyboard manipulation
* Easily Customize config

```
Default Modkey : VK_NONCONVERT(MUHENKAN)
S-key          : Shift + key

.=================================================================.
| Command  | Behavior                     | Implementation Status |
|----------|------------------------------|-----------------------|
| j, k     | Move focus                   | DONE                  |
| S-j, S-k | Move window                  | DONE                  |
| h, l     | Move window (when floating   | x                     |
| S-h, S-l | Resize window                | x                     |
| 1~9      | Move virtual desktop         | DONE                  |
| S-1~9    | Move window to other desktop | DONE                  |
| m        | Maximize                     | DONE                  |
| S-Enter  | Open terminal                | DONE                  |
| r        | Launcher                     | x                     |
| S-d      | Close window                 | DONE                  |
| i        | Open browser                 | DONE                  |
| a        | Set as a main window         | DONE                  |
| Space    | Change tiling mode           | DONE                  |
| q        | Quit                         | DONE                  |
.=================================================================.
```

### Recommend
* Add below line to your .minttyrc (ver>=2.2.1)
```
ZoomFontWithWindow=no
```

