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
# Download original "Bad Apple!!" video from YouTube.
yt-dlp "https://youtu.be/FtutLA63Cp8" -o BadApple.webm

# Rotate it to 90° clockwise.
ffmpeg -i BadApple.webm -vf "transpose=1" BadApple_90d.webm

# Set 15 FPS instead of 30 FPS (optional for low FPS devices).
ffmpeg -i BadApple_90d.webm -vf "fps=15" BadApple_90d_15f.webm

# Scale it to proper resolution without aspect ratio savings.
ffmpeg -i BadApple_90d.webm -vf "scale=128:160" BadApple_90d_160p.webm
ffmpeg -i BadApple_90d.webm -vf "scale=176:220" BadApple_90d_220p.webm

# Unpack raw bitmap frames.
mkdir frames
ffmpeg -i BadApple_90d_220p.webm 'frames/%04d.bmp'

# Convert frames from 24bpp to 1bpp using ImageMagick convert tool.
mkdir bw
cd frames
find * -name '*.bmp' -exec bash -c 'echo "24bpp => 1bpp: {}..." && convert {} -threshold 50% -depth 1 -monochrome ../bw/bw_{}' \;

# Create compressed FBM file from frames.
cd ..
./bmp2fbm.py bw
```

## Encoding recipe (60fps)

```bash
# Download 4K @ 60 FPS version of "Bad Apple!!" video from YouTube.
yt-dlp "https://youtu.be/UsIAaRLUI9s" -o BadApple_4K_60f.webm

# Rotate it to 90° clockwise.
ffmpeg -i BadApple_4K_60f.webm -vf "transpose=1" BadApple_90d.webm
ffmpeg -i BadApple_4K_60f.webm -vf "transpose=1" -ss "00:00:40.00" -to "00:00:56.00" BadApple_90d.webm

# Scale it to proper resolution without aspect ratio savings.
ffmpeg -i BadApple_90d.webm -vf "scale=128:160" BadApple_90d_160p.webm
ffmpeg -i BadApple_90d.webm -vf "scale=176:220" BadApple_90d_220p.webm

# Unpack raw bitmap frames.
mkdir frames
ffmpeg -i BadApple_90d_220p.webm 'frames/%04d.bmp'

# Convert frames from 24bpp to 1bpp using ImageMagick convert tool.
mkdir bw
cd frames
find * -name '*.bmp' -exec bash -c 'echo "24bpp => 1bpp: {}..." && convert {} -threshold 50% -depth 1 -monochrome ../bw/bw_{}' \;

# Create compressed FBM file from frames.
# Set `compress = False` in `bmp2fbm.py` for 60 FPS.
cd ..
./bmp2fbm.py bw
```
