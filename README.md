
## Minimal Embedded Remote Graphics Interface Format

Embedded systems often need a small display. The programming required to support that 
distracts from the main focus of the project. IO pins are also costly. 
The simplest possible method of adding a display is desirable. 
Having a low cost separate processor to manage the display with an easy to load 
firmware and a simple command set seems like a good idea. 
A single serial tx pin (and rx if a touch screen is used) should be enough. 
Instead of using binary commands and supplying a library of code in every language, 
a simple human readable text command format can be used.

The general format is a series of commands which are just a number and the first letter 
off a command or attribute. e.g. L for Line, c or C for Color.

It ends up a bit like a g-code. But postfix at least for now. e.g. not `X10`, but `10x`. 

# Commands

| Commands | Attributes | Description |
|---|---|---|
| `Z`ero  | | Clear the display, erase all groups and areas |
| `I`d    | num | Specify a group by setting the ID number |
| `O`     |  x y diameter color | A filled in circle |
| `P`oint |   x y P \[x y P ...\] | Just a way to save a list of points | 
| `L`ine  |  color width x y \[x y ...\] | A series of lines from saved 'P'oints (which are cleared) |
| `R`ect  |  x y width height | A filled in rectangle (use Path for outlines) |
| `A`rc   |  x y diameter begin end color | An arc or non-filled circle | 
| `T`ext  |  x y color height | Text. The characters are placed between the T and the attribues |
| `M`ap   |   pixel data | See below|

# Attributes 

| Attributes |  |
|---|---|
| `x` | 0 to display width |
| `y` | 0 to display length |
| `w`idth | + integer |
| `h`eight | + integer |
| `d`iameter | 0 to minimum of display width and height |
| `c`olor | RGB 565 value, use # for hex but then uppercase C to end |
| `b`egin | Starting arc degrees 0-360 |
| `e`nd | Ending arc degrees 0-360 |
| `#` | set radix (default hex) for this number | 
| `S`ize? | Future features |
| `F`ont?  |  " | 

# Examples:

`100x 100y P 200x 100y P #ffffC L `

Draw a line from 100,100 to 200,100 colored white. 
It starts with a Point at 100,100 and another at 200x100 
with a color setting of ffff (RGB all fully on) which is in hex (#)
and then with all that set up, we can draw the Line.

`1i 10x 20y 40h 50w #f800C R`

Sets up a clickable filled rectangle. 
It's clickable because it's in a Group which starts with the Id of 1. 
The Rect goes from 10, 20 and is 50 wide and 40 hight, and a red fill Color. 

A click in that area sends a "1" back to the host. 

Multiple objects can be part of a single group, just set the same id for each one.

`2i 100x 35y 25d #07e0C O`

A 25 pixel diameter green filled in circle at 100,35 in group 2.

A live example with test code (no command interpreter yet) is available at:
https://wokwi.com/projects/446578045170487297


## Future

### Text / Font

The original fonts via GFX are a bit sad, but they have been expanded of late.
'd' could be re-used as direction. 's' for size.

### Arc

Arcs are not supported by the GFX library, so a series of lines or pixels would
need to be drawn to support that. 

### Map (bitmaps)

Map (aka Mat ala OpenCV) is for transfering pixel data. 
Combined with the radix setting (#) to change base and we can do things like 
a binary map from 10,20 for a zero with a dot in the middle. 
Most commands send with a cr, this one needs an empty line, e.g. cr cr.
```
10x20y2#
00110000
01001000
10000100
10110100
10000100
01001000
00110000
M
```
which is the same as 
`10x20y #30 48 84 B4 84 48 30 M`

Radix 1 is a special binary format using `#` and ` ` to be a clearer image than 0 and 1. 

```
10x20y1#
  ##  
 #  # 
#    #
# ## #
#    #
 #  # 
  ##  
M
```

The code, if there is any interest in developing it, would be the following:
- an interpreter for the commands which make calls to a graphics library like Adafruit, or LVGL
- a lookup table for areas in a group which translates touchscreen clicks into group IDs


## Notes

Unused letters:
G
J 
K
N
Q
U
V

Used letters:
ABCDEFGHILMOPRSTWYXZ
