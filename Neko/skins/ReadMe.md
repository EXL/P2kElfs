Neko Skins
==========

## Animation map (25 frames/sprites)

1. Idle #1: sleeping frame.
2. Movement: start step frame.
3. Movement: jump frame.
4. Movement: end step frame.
5. Idle #2: transition from movement to idle frame.
6. Idle #2: frame #1.
7. Idle #2: frame #2.
8. Idle #2: frame #3.
9. Idle #2: frame #4.
10. Idle #3: frame #1.
11. Idle #3: frame #2.
12. Idle #3: frame #3.
13. Idle #4: frame #1.
14. Idle #4: frame #2.
15. Idle #4: frame #3.
16. Idle #4: frame #4.
17. Idle #4: frame #5.
18. Missed call: frame #1.
19. Missed call: frame #2.
20. Missed call: frame #3.
21. New message: frame #1.
22. New message: frame #2.
23. New message: frame #3.
24. New message: frame #4.
25. New message: frame #5.

## Convertion "bmp2ani" tool

All sprites are limited to any 16 colors from RGB565 color space, most tested with 15 colors. Max tested sprite resolution is 40x40 pixels. The characters' face should be directed to the left side.

### Dependencies

```bash
python -m pip install --upgrade pip
python -m pip install --upgrade Pillow
```

### Usage

```bash
# 1. Convert ANI-image to BMP-image.
./bmp2ani.py Sheep.ani

# 2. Convert BMP-image to ANI-image.
./bmp2ani.py Sheep.bmp

# 3. Convert BMP 4 bpp image to BMP 8 bpp image.
./bmp2ani.py Sheep.bm4
```

## Skin Authors

* **Neco-Arc**: Type-Moon, French-Bread, ECOLE, Random Talking Bush, etc.

    [Additional Information](https://www.spriters-resource.com/playstation_2/mbaa/sheet/28104/).

* **Kitty**: Dazz, GenericNPC, MapleStory game developers.

    [Additional Information](https://www.spriters-resource.com/pc_computer/maplestory/sheet/21897/).

* **Sheep**: baat & voyager, original idea and sprites Village Center, Inc.

    [Additional Information #1](https://forum.motofan.ru/index.php?s=&showtopic=165504&view=findpost&p=1371628).
    [Additional Information #2](http://web.archive.org/web/20050204192403/http://www.villagecenter.co.jp/english/poe.html).

* **PepeD**: garych.

    [Additional Information](https://knowyourmeme.com/memes/peped).
