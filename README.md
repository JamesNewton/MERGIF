
## Minimal Embedded Remote Graphics Interface Format

Embedded systems often need a small display. The programming required to support that 
distracts from the main focus of the project. IO pins are also costly. 
The simplest possible method of adding a display is desirable. 
Having a low cost separate processor to manage the display with an easy to load 
firmware and a simple command set seems like a good idea. 
A single serial tx pin (and rx if a touch screen is used) should be enough. 
Instead of using binary commands and supplying a library of code in every language, 
a simple human readable text command format can be used.

The general format is a series of commands which are just the first letter 
e.g. L for Line, with lower case attributes (e.g. c for Color) followed by values. 
Default attributes can be set with an uppercase letter, 
and remain in effect until the end of the line (carriage return aka cr, decimal 13) 
or until overridden. 

It ends up a bit like a g-code. But postfix at least for now. e.g. not `X10`, but `10x`. 

# Commands

| Commands | Attributes | Description |
|---|---|---|
| `Z`ero  | | Clear the display, erase all groups and areas |
| `O`     |  x y diameter color | A small filled in circle |
| `P`oint |   x y P \[x y P ...\] | Just a way to save a list of points | 
| `L`ine  |  color width x y \[x y ...\] | A series of lines from saved 'P'oints (which are cleared) |
| `R`ect  |  x y width height | A filled in rectangle (use Path for outlines) |
| `A`rc   |  x y diameter begin end color | An arc or non-filled circle | 
| `T`ext  |  x y color height | Text. The characters are placed between the T and the attribues |
| `I`d    | num | Starts a group by setting the ID number |
| `G`roup | | Just ends the group started by `I` | 
| `M`ap   |   pixel data | See below|
| `S`pline? | | Future feature |
| `F`rame?  | |  " | 

# Attributes 

Use upper case to set default, lower to set value for command

| Attributes |  |
|---|---|
| `x` | 0 to display width |
| `y` | 0 to display length |
| `w`idth | + integer |
| `h`eight | + integer |
| `d`iameter | 0 to minimum of display width and height |
| `c`olor | RGB three values, default radix is hex so FFF is white,  |
| `b`egin | Starting arc degrees 0-360 |
| `e`nd | Ending arc degrees 0-360 |
| `#` | set radix to hex for this number | 

# Examples:

`D x100 y100 CFFF L w2 x200 y100`

Draw a 2 pixel line from 100,100 to 200,100 colored white. 
It starts with a Dot at x100y100 and a color setting of FFF (RGB all fully on)
Then it continues with a Line which is 2 pixels wide, to x200y100
Note that the color setting continues from the dot to the line, 
and the dot doesn't require a radius because it defaults to zero.
Dots can be filled circles, where Arcs (with no Begin or End) are outlined circles.

`1i 10x 20y 40h 50w #f800C R`

Sets up a clickable filled rectangle. 
It's clickable because it's in a Group which starts with the Id of 1. 
The Rect goes from 0, 20 and is 200 wide and 80 hight, and a red fill Color. 

A click in that area sends a "1" back to the host. 

Multiple areas can be part of a single group.

A live example with test code (no command interpreter yet) is available at:
https://wokwi.com/projects/446578045170487297


## Future

Map (aka Mat ala OpenCV) is for transfering pixel data. 
Combined with the radix setting (#) to change base and we can do things like 
a binary map from 10,20 for a zero with a dot in the middle. 
Most commands send with a cr, this one needs an empty line, e.g. cr cr.
```
#2Mx10y20
00110000
01001000
10000100
10110100
10000100
01001000
00110000
```
which is the same as 
`#16Mx10y20 30 48 84 B4 84 48 30`

Radix 1 is a special binary format using `#` and ` ` to be a clearer image than 0 and 1. 

```
#2Mx10y20
  ##  
 #  # 
#    #
# ## #
#    #
 #  # 
  ##  
```

The code, if there is any interest in developing it, would be the following:
- an interpreter for the commands which make calls to a graphics library like Adafruit, or LVGL
- a lookup table for areas in a group which translates touchscreen clicks into group IDs


## Notes

Unused letters:
J 
K
N
O
Q
U
V

Used letters:
ABCDEFGHILMPRSTWYXZ