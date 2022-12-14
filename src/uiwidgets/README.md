

uiwidgets -- Mini UI widget library
===================================

A UI widget library for drawing on an LCD in Arduino.

Depends on the `Seeed_Arduino_LCD` or `TFT_eSPI` library for the underlying
display control.

* https://github.com/Seeed-Studio/Seeed\_Arduino\_LCD/
* https://github.com/Bodmer/TFT\_eSPI

Compiling
=========

`make all install`

Depends on my Arduino Makefile (https://github.com/kimballa/arduino-makefile).

Layout
======

The layout of widgets is based on a nesting system. The top-most visible object is always a
`Screen`. You can have multiple `Screens`, but only one will show at a time. The Screen can hold one
element, which may be something that visibly displays, like a `TextLabel`, or it could be a `Rows`
or `Cols`; these can be configured to have N items across or down (which may be more `Rows` or
`Cols`). Each object has an x, y, width, and height.  As containing objects are moved by layout
changes, they cascade the position and size offsets to their children.

Each UIWidget knows its absolute coordinates, so if a small element of the screen is invalidated
due to changed data, a hierarchical sub-portion of the screen can be redrawn.

Widgets
=======

Screen
------
The top-most container; exactly one Screen can be shown at a time. No porcelain by default.

Panel
-----
A container for another item. No porcelain by default. Can have background color or borders enabled.

Rows
----
A collection of N objects that will be displayed stacked on top of one another. Each row expands to
the full width of its container. The `Rows` can have a fixed per-row height, or "flex" to have each
row show directly below the required height of its predecessor.

Each row can have an optional border above/below.

Cols
----
A collection of N objects that will be displayed left-to-right. Each column expands to
the full height of its container. The `Cols` can have a fixed per-col width, or "flex" to have each
col show directly to the right of the required width of its predecessor.

Each col can have an optional border to the left or right.

Button (TODO)
------
A selectable button with text in it.

Label
--------
Displays some text.

StrLabel
--------
Subclass of `Label`. Displays a string.

IntLabel
--------
Subclass of `Label`. Displays an integer.

UIntLabel
--------
Subclass of `Label`. Displays an unsigned integer.

FloatLabel
--------
Subclass of `Label`. Displays a float.

Menu (TODO)
----
A collection of text lines that can each be selected.
(TODO: Does this take over the whole screen?)

VScroll (TODO)
-------
A container that holds a variable number of items, more than can be shown on the screen.
A vertically-oriented scrollbar is on the right side; this can be scrolled to carousel which
elements are rendered to the screen.


