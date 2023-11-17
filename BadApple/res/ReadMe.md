Frame Bitmap Map files
======================

## Header

```c
typedef struct {
	UINT16 width;
	UINT16 height;
	UINT16 frames;
	UINT16 max_compressed_size;
	UINT16 bpp;
	UINT16 frame_size;
} FBM_HEADER_T;
```

## Encoding recipe

```bash
# Download original "Bad Apple!!" video from YouTube without sound.
yt-dlp -f 605 "https://youtu.be/FtutLA63Cp8"

# Rotate it to 90° clockwise.
ffmpeg -i input.mp4 -vf "transpose=1" output.mp4

# Set 15 FPS instead of 30 FPS (optional for low FPS devices).
ffmpeg -i input.mp4 -vf "fps=15" output.mp4

# Scale it to proper resolution without aspect ratio savings.
ffmpeg -i input.mp4 -vf "scale=128:160" output.mp4
ffmpeg -i input.mp4 -vf "scale=176:220" output.mp4

# Unpack raw bitmap frames.
mkdir frames
ffmpeg -r 1 -i input.mp4 -r 1 'frames/%04d.bmp'

# Convert frames from 24bpp to 1bpp using ImageMagick convert tool.
mkdir bw
cd frames
find * -name '*.bmp' -exec bash -c 'echo "Converting {}..." && convert {} -monochrome ../bw/bw_{}' \;

# Create compressed FBM file from frames.
./bmp2fbm.py bw
```
