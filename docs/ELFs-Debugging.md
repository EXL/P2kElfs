ELFs Debugging Notes
====================

## Using MIDway application

1. Phone Actions: Settings => Java Settings => Java App Loader => Insert cable now => JAL link is active...
2. Phone USB Connection: Modem mode -- Motorola USB Modem, AT commands.
3. MIDway Actions: Set proper COM port of Motorola USB Modem device in "File => Settings..." then go to the "Debug Log" tab.

Debug function sample:

```c
PFprintf("KeyPress = %d.\n", event->data.key_pressed);
```

## Using P2KDataLogger application from Motorola PST suite

1. Phone Actions: PDS SEEM `SEEM_IN_FACTORY` (01C1_0001) must be in FF state (IN_FACTORY), not 00 (NON_FACTORY).
2. Phone USB Connection: Switch phone to P2K mode -- Motorola USB Device (Accessories Interface, Data Logging MCU Interface, Test Command Interface).
3. P2KDataLogger Actions: Check "Display => Filters => All Primitives/Messages" then choose "Display => Start Display".

```c
UtilLogStringData("KeyPress = %d.\n", event->data.key_pressed);
```
