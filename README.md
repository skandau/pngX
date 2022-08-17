
# pngX - fast, lossless image compression smaller than PNG size

Single-file MIT licensed 


## Example Usage

- [pngx.cpp](https://github.com/skandau/pngx.cpp)
converts between png <> pngx

## Why?

Compared to stb_image and stb_image_write pngx offers 20x-50x faster encoding,
3x-4x faster decoding . It's also simple and
fits in about 300 lines of C. Better than fpng, lodepng, stbi and qoi.

## Results
fish.png 447419 bytes  
pngx     444373 bytes
fpng     628592 bytes
lodepng  551966 bytes
stbi     823675 bytes
qoi      1132407 bytes

dice.png 226933 bytes
pngx     226353 bytes
fpng     301694 bytes
lodepng  261855 bytes
stbi     349827 bytes
qoi      519653 bytes   

kodim04.png 637432 bytes
pngx        633442 bytes
fpng        706439 bytes
lodepng     658090 bytes
stbi        902181 bytes
qoi         717534 bytes
   
