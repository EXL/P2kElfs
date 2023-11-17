Useful Notes
============

## Set executable flag to file on Windows using Git

```bash
$ git update-index --chmod=+x BadApple/res/bmp2fbm.py
```

## Calculate average/max/min FPS value

```bash
cat FPS.txt
FPS: 11.9
FPS: 12.0
FPS: 10.1
FPS: 10.8
FPS: 11.7
FPS: 11.7
FPS: 11.6
FPS: 10.1
FPS: 12.0
FPS: 12.0
FPS: 11.9
FPS: 12.0
FPS: 9.0
FPS: 11.9
FPS: 12.0
FPS: 10.6
FPS: 12.0
FPS: 10.7
FPS: 11.4
FPS: 10.9
FPS: 12.0
FPS: 11.9
FPS: 12.0
FPS: 11.9
FPS: 12.0
FPS: 11.3
FPS: 12.0
FPS: 10.7
FPS: 11.7
FPS: 8.5
FPS: 10.5
FPS: 12.0
FPS: 12.0

$ awk '{ total += $2; count++ } END { print total/count }' FPS.txt
11.3576

$ awk 'BEGIN {max = 0} {if ($2>max) max=$2} END {print max}' FPS.txt
12.0

$ awk 'BEGIN {min = 99} {if ($2<min) min=$2} END {print min}' FPS.txt
8.5
```
