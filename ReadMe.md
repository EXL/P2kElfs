ENUM APPS like

GUI app
Deamon app
GUI + Deaemon app

## Debug notes

```c
/*
 * Send log to MIDway application.
 * USB Connection: USB Modem Mode, AT commands.
 * Phone Actions: Settings => Java Settings => Java App Loader => Insert USB cable now => JAL Connection. //TODO:
 * MIDway Actions:
 *
 * PFprintf("KeyPress = %d.\n", event->data.key_pressed);
 *
 * Send log to P2KDataLogger application.
 * USB Connection: P2K Mode.
 * Phone Actions: SEEM_IN_FACTORY (01C1_0001) must be in FF (IN_FACTORY), not 00 (NON_FACTORY).
 * P2KDataLogger application:
 *
 * UtilLogStringData("KeyPress = %d.\n", event->data.key_pressed);
 */
```
