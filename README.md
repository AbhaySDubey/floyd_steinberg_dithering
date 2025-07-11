# Floyd-Steinber Dithering Algorithm
 - This project aims to showcase some basic image quantization and how the Floyd-Steinberg Dithering algorithm can be used to dare I say, **enhance image quality** of the quantized image.

---

## Introduction

 - [Dither is an intentionally applied form of noise used to randomize quantization error, preventing large-scale patterns such as color banding in images.](https://en.wikipedia.org/wiki/Dither) - from Wikipedia.

 - Dithering allows to enhance quality of quantized images in post-processing during transmission and is used extensively in processing of both digital audio and video data.

 - One of the earliest, and still one of the most popular, is the [Floyd–Steinberg dithering] (https://en.wikipedia.org/wiki/Dither#Algorithms), algorithm, which was developed in 1975. One of the strengths of this algorithm is that it minimizes visual artifacts through an error-diffusion process; error-diffusion algorithms typically produce images that more closely represent the original than simpler dithering algorithms.

 - The algorithm uses error diffusion, in which the residual quantization error of a pixel is pushed onto its neighbors, in the following form.

 ![Error Diffusion to neighboring pixels in Floyd-Steinberg Dithering](resources\erroDiffusionInFloydSteinberg.png "Error Diffusion to neighboring pixels in Floyd-Steinberg Dithering")

 - To read more, refer to the corresponding Wikipedia pages for: [Dithering](https://en.wikipedia.org/wiki/Dither), [Floyd-Steinberg Dithering](https://en.wikipedia.org/wiki/Floyd%E2%80%93Steinberg_dithering), [Quantization](https://en.wikipedia.org/wiki/Quantization)
 
 - A major problem with the Floyd-Steinberg dithering algorithm is the fact that its operations cannot be parallelized, i.e. you cannot operate on 2 or more pixels at a time concurrently, which does reduce the efficiency of the algorithm.

---

## Usage

 - ### Quantization and Dithering:
    - Use the ```'Q'``` key to toggle quantization of the image.
    - Use the ```'D'``` key to toggle dithering of the image.
    - ```(Note: Dithering and quantization depend on the number of bits dedicated to each of the color channels between 1 and 8, which can be adjusted as given below)```

 - ### Quantization Control:
    - Press number keys ```1``` - ```8``` to dynamically adjust quantization bit depth and see immediate results - from 1-bit black/white to 8-bit full color.

 - ### Color Palette System:
    - Toggle between general N-bit quantization and custom 5-color palette (Black, White, Yellow, Magenta, Cyan, you can change) using the ```'C'``` key.
    - To change the color palette, configure the ```nShades``` array in the ```Quantize_RGB_CustomPalette``` lambda.

 - ### Pan and Zoom Viewport:
    - Pan with ```left-click``` drag, zoom with ```mouse wheel```, and reset camera position using the ```'R'``` key.

 - ### Toggle grayscale:
    - Use the ```'G'``` key to toggle between grayscale and RGB image.

## Additional Notes
 - The project uses [Raylib](https://www.raylib.com/), so you may require to set it up beforehand. An easier workaround is to use the [w64-devkit](https://github.com/raysan5/raylib/wiki/Working-on-Windows#mingw-w64gcc) for easier and quicker setup.

 - Please note, that the **Loading** and **Unloading** of the images and textures may take some time, so if you're sure you've pressed the appropriate *hot-key*, rest back and wait for the processing to complete.