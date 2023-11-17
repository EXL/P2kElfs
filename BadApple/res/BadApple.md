```bash
# Download video from YouTube.
yt-dlp -f 605 "https://youtu.be/FtutLA63Cp8"

# Rotate it to 90 degrees.
ffmpeg -i 1.mp4 -vf "transpose=1" 2.mp4

# Set 15 fps instead of 30 fps.
# ffmpeg -i 2.mp4 -vf "fps=15" 3.mp4

# Scale it to proper resolution
ffmpeg -i 3.mp4 -vf "scale=176:220" 4.mp4

# Unpack raw frames.
ffmpeg -r 1 -i 4.mp4 -r 1 'frames/%04d.bmp'

# Convert frames from 24bpp to 1bpp
find * -name '*.bmp' -exec bash -c 'echo "Converting {}..." && convert {} -monochrome bw/bw_{}' \;
```
