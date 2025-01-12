Input Events
============

The Input System is deprecated! It is being replaced by a Lua scripting
engine.

Introduction
------------

The Input System is actually a large number of things all bunched into
one.

Primarily it is about giving the user control of what button does what
and when. There is a new concept called Input Mode - this is a the mode
the GUI is in for input. For example, you can click on the info boxes
and you are now in "infobox" mode. Clicking on the map is called
"default". But it doesn’t stop there, you can create a new mode called
anything you like. This may not mean much - but wait till you combine it
with the rest of the features...

Input is not restricted to hardware buttons any more. You can map all
your hardware buttons (currently support for APP1 to APP6, Left, Right,
Up, Down and Enter, although I believe we can do some more) but also any
key code at all. This feature allows those with a built in keyboard to
use any key to map to any function in XCS. Where it comes into real
advantage is in external keyboards. There are a number of bluetooth
devices out there (eg: http://shop.brando.com.hk/btgamepad.php) which
can map each of their buttons to any key code - that key code can then
be mapped to any feature in XCS. You can then add to the hardware
buttons the buttons available to you on external devices. Other inputs
(eg: Serial) are also being looked at - and support is in the code for
that extension.

We are striving towards a platform which is not only easier to use and
more intuitive, but also faster and easier to use in flight as well. As
such, another new feature as part of input is the concept of Button
Labels. Combined with the modes mentioned above, you can create any
arbitrary set of functions to map to any number of buttons. Think about
it like creating a tree, or a multiple level menu.

This produces two benefits that I know will be appreciated by people
with limited inputs. The first is that you can create menus, where by
you press one button to get to the next level (eg: pressing on APP1
brings up AutoZoom, Pan Mode, Full screen on the other buttons. Press
APP1 again and it goes to Terrain, Marker and Auto MacCready. Press APP1
again and the menu is gone) - but more importantly for those with touch
screens and limited buttons, each of these labels can optionally be
assigned a key and you can touch the button area as if it was a button.
This means that we can actually control on a touch screen model the
entire system without buttons - press an area of the screen and the
buttons pop up, click through - change options and more.

The combined features of labels, configurable buttons (including from
external hardware), hierarchical menus (for lack of a better name),
touch screen buttons has allowed us to configure XCS - without recompile
- for an enormous range of hardware, and personal preference. And all
configurable as plane text, simple files. There is no need for a file,
the defaults internally will probably be a combination of a 4 button
bottom system with one button always shown on screen for no/few button
display.

The screen layout - location of the labels - is also totally
configurable - allowing us to vary the layout of buttons depending on
the type of organiser or desired look and feel.

There is a great unexpected benefit in the development of the input
system.

We can execute any number of events attached to an input with only 2
extra lines of code. This worked perfectly. So now we have a basic macro
system, allowing many more events to be attached to a single input
event.

But it doesn’t stop there, this has lead to some more excellent
developments. The idea of Glide Computer Events things like "Maximum
Altitude Reached". Currently we play a sound effect for that. But you
may choose to play a sound, bring up a message box and write to the log
file.

One nice feature of XCS is the ability to change things such as Zoom and
North when Circling. Now you can do so much more. You could choose to
point North, Zoom to 1.0 (rather than a relative change), Turn on Vario
Sounds, Start a timer. When switching back to Cruise mode, you can bring
up the stats box for 30 seconds. The options are limited by your
imagination.

This is also contributing to a major reduction in complex code. We can
move out these complex tests into one centrally, easier to manage
system, reducing bugs and improving maintainability.

Another side benefits of these Macros is User Defined Flight Modes. One
idea was a button which switched to Zoom 1.0, Pan ON, Pan Move to Next
Waypoint. Basically the ability to jump and see the next waypoint. And
in the previous we can change the Input Mode to "ViewWaypoint" - at
which point you can redefine the same button to switch back to your
original settings.

The flexibility of this system comes with only one small price. We can’t
provide an interface within XCS to fully customise all of these near
infinitely variable possibilities. However I believe that is unnecessary
anyway, you are not likely to change these sort of features very often,
and definitely not on the field. That does not mean you can’t, you can
of course edit the plane(sic) text file to change functions.

What this really means is that we can have people in the project helping
and contributing to the customising of XCS, without having to change the
code. This, especially on an open source project is fantastic as it
nicely separates the user interface changes from the highly reliable
part of the code. It also involves people who can develop new interfaces
and functions that are expert gliders but not necessarily programmers.

For information on file formats see :file:`Data/Input/default.xci` and
the web site documentation.

Defaults and Files
------------------

The file in the source :file:`Data/input/default.xci` is used to
generate automatically the C code necessary for the default
configuration. However it is in the exact same format as can be read
in by XCS and therefore can be used literally as a template for a more
complicated file.

When you create your own file, you will need to select it as the Input
File within XCSoar Menu/Config/System/Look/Language,Input/Events. Choose
the custom file you would have previously created, and then restart XCS.

File format
-----------

The file is plain text, with key=value pairs and a blank line to
indicate the end of a record::

 mode=default
 type=key
 data=APP1
 event=StatusMessage My favorite settings are done
 event=ScreenModes full
 event=Sounds on
 event=Zoom 1.0
 event=Pan off
 label=My Prefs
 location=1

The record above demonstrates remapping the first hardware key on your
organiser to change Pan to off, Zoom to 1.0 Sounds on, ScreenModes full,
and then a status message to tell you it is done.

Lines are terminated by the stanard DOS newline which is CRLF (Carrage
Return then Line Feed). Records are terminated by an extra new line.

Event order
-----------

Until further work is done on processing, events are actually done in
reverse order - also known as RPN. This is because the events work on
the stack principle. Each one is pushed onto the stack for execution,
and then executed by popping back off the stack. This has reduced
complexity of the code base.

When writing input events, have a look where you put the StatusMessage
and make sure that it is at the top, not the bottom (if you have one).

Event list
----------

.. list-table::
 :widths: 20 80
 :header-rows: 1

 * - Event
   - Description
 * - ``MainMenu``
   -
 * - ``MarkLocation``
   - Mark a location.
 * - ``Mode M``
   - Set the screen mode.
 * - ``Pan [P]``
   - Control pan mode. Possible arguments: ``on`` (enable pan),
     ``off`` (disable pan), ``up``, ``down``, ``left``, ``right``
 * - ``PlaySound S``
   - Play the specified sound.
 * - ``SnailTrail S``
   - Change snail trail setting. Possible arguments: ``off``,
     ``short``, ``long``, ``show``.
 * - ``ScreenModes M``
   - Set the screen mode. Possible arguments: ``normal``, ``auxilary``,
     ``toggleauxiliary``, ``full``, ``togglefull``, ``toggle``.
 * - ``Sounds S``
   - Change vario sounds. Possible arguments: ``toggle``, ``on``,
     ``off``, ``show``.
 * - ``StatusMessage MSG``
   - Display the specified status message.
 * - ``Zoom Z``
   - Everything about zoom of map. Possible arguments: ``auto
     toogle``, ``auto on``, ``auto off``, ``auto show``, ``in``,
     ``out``, ``+``, ``++``, ``-``, ``–-``.

Modes
-----

XCSoar now has the concept of Modes. These are an arbitrary string that
associates with where and what XCS is doing.

Note: a mode entry in a record can have multiple entries by using a
space between eg: "infobox menu1 menu2"

List of known modes
~~~~~~~~~~~~~~~~~~~

- ``default``: Really map mode, where you mostly are.
- ``infobox``: An info box has been selected on the screen.
- ``*``: Any other arbitrary string.

Mode precedence has been tricky, so instead of solving the problem it is
being worked around. XCS will choose to set a global variable to specify
what mode it thinks it is in. This can then be used by the input code to
decide what to do. This mode could get out of sink with the real world,
and careful checking will be required, but at this stage it seems like
the only sensible option.

The code will review first if an entry exists in the current mode, and
then in the default mode. This allows you to do one of the following
example: Define a default action for button "A" to be "Zoom In" but make
that button increase Bugs value in infobox mode only. You can do this by
making an "default" and a "infobox" entry. You can also put an entry in
for Button "A" for every mode and have complete control.

Special Modes - eg: the level of a menu (Think File vs Edit, vs Tools vs
Help)

have special modes, such as the level of the menu you are at. You press
one button, then another set become available (like pressing menu and
seeing Settings etc). This will be very useful in non-touch screen
models. The menu configuration can then be read from this same file and
configured, allowing any number of levels and any number of
combinations.

The only hard part is what mode to go back to. We need a "Calculate Live
Mode" function - which can be called to calculate the real live mode
(eg: finalglide vs curse) rather than the temporary mode such as Menu,
Special Menu Level, Warning etc.

The label and location values are examples of what can be done here to
allow input button labels to be displayed. What needs to be considered
is a simple way of mapping the locations and the size. In some models it
may be that buttons are 4 across the top of the screen, where as others
it is 3 or 2 or even 6. So both size and location needs to be
considered.

The label itself will go through gettext to allow language translations.

Keys
----

The key type can have the following possible values:

- ``APP1-APP6``: Hardware key on pocket pc
- ``F1-F12``: Standard function keys
- ``LEFT, RIGHT, UP, DOWN, RETURN, ESCAPE, MENU, TAB``: Mapped to arrow 
  keys - joystick on organisers
- ``A-Z, 0-9``: and other possible keyboard buttons (case is ignored)

Android only:

- ``BUTTON_R1, BUTTON_R2, BUTTON_L1, BUTTON_L2, BUTTON_A, BUTTON_B, 
  BUTTON_C, BUTTON_X, BUTTON_Y, BUTTON_Z, MEDIA_NEXT, MEDIA_PREVIOUS, 
  MEDIA_PLAY_PAUSE, VOLUME_UP, VOLUME_DOWN``: intended for Bluetooth
  keypads, media controllers

Windows only:

- ``F13-F20``: intended for the Triadis-RemoteStick, as well as for
  expanded keyboards

XXX Review... Input Types

Types:

hardware These are the standard hardware buttons on normal organisers.
Usually these are APP1..6.

keyboard Normal characters on the keyboard (a-z etc)

nmea A sentence received via NMEA stream (either)

virtual Virtual buttons are a new idea, allowing multiple buttons to be
created on screen. These buttons can then be optionally mapped to
physical buttons or to a spot on the screen (probably transparent
buttons over the map).

Modifiers

It is a long term goal of this project to allow modifiers for keys. This
could include one of the following possibilities:

-  Combination presses (although not supported on many devices)

-  Double Click

-  Long Click

Modifiers such as the above will not be supported in the first release.

.. list-table::
 :widths: 20 80
 :header-rows: 1

 * - Functions/Events
   - what it does
 * - AutoZoom
   - on, off, toggle
 * - FullScreen
   - on, off, toggle
 * - SnailTrail
   - on, off, long, toggle
 * - VarioSound
   - on, off
 * - Marker
   - optional text to add
 * - MenuButton
   - on, off, toggle
 * - Menu
   - open, close, toggle
 * - MenuEntry
   - task, b+b, abortresume, abore, resume, pressure logger, settings, status, analysis,
     exit, cancel

     NOTE: Some of the above may be separate functions
 * - Settings
   - (each setting, bring up to that point)
 * - Bugs
   - add, subtract, 0-100
 * - Ballast
   - add, subtract, 0-100
 * - Zoom
   - add, subtract, 0-nn (set value)
 * - Wind
   - up, down, 0-nn (set value, left, right, "n","ne","e","se","s","sw","w","nw"...
 * - MacCready
   - add, subtract, 0-nn (set value)
 * - WaypointNext
   - "String" to specific waypoint eg: WayPointNext "home"
 * - WayPoint???
   - "reverse" -
     reverse, from last passed back to start (ie: from here to home) "drop
     next" - drop the next "restore" - restore all - from start of flight but
     XXX This needs more thought flight "startstop", "start", "stop",
     "release" Start/Stop of flight - Can be automatic, but pressing will
     override automatic part.
 * - release
   - marks the point of release from tow

Glide Computer Events
---------------------

These are automatically triggered events. They work in exactly the same
way, but instead of the user pressing a key, the glide computer triggers
the events.

A simple example is moving from Cruise to Climb mode. We want to zoom
in, change our track up to north up and switch to full screen. You may
also choose to drop a marker with the words "entered thermal". The
choicese are up to your imaginations - the GCE (Glide Computer Events)
allow you to control what happens.

These are represented as ``type=gce`` and ``data=*`` - as listed
below.

``COMMPORT_RESTART``
   The comm port is restarted.

``FLIGHTMODE_CLIMB``
   The flight mode has switched to "climb".

``FLIGHTMODE_CRUISE``
   The flight mode has switched to "cruise".

``FLIGHTMODE_FINALGLIDE``
   The flight mode has switched to "final glide".

``GPS_CONNECTION_WAIT``
   Waiting for the GPS connection.

``GPS_FIX_WAIT``
   Waiting for a valid GPS fix.

``HEIGHT_MAX``
   Maximum height reached for this trip.

``LANDING``
   You are at landing.

``STARTUP_REAL``
   First message - this happens at startup of the real XCS.

``STARTUP_SIMULATOR``
   Startup first message. This happens during simulator mode.

``TAKEOFF``
   You have taken off.

``AIRSPACE_NEAR``
   The aircraft has approached an airspace for which warnings are
   enabled.

``AIRSPACE_ENTER``
   The aircraft has entered an airspace for which warnings are
   enabled.
