# TO DO #

## Rasterized mode ##

Some ideas about using a rasterized mode to speed up rendering

### Preliminary thoughts ###

Current calculations per pixel: ~ res * lambda

* res is the filter resolution (number of sampling points)
* lambda is the average number of grains, depending on the pixel luminance.

With res = 1024 and rad = 0.025, we’re in the 100k–1M range.

For grains of this size or above, we should be able to simplify the model by rasterization, because we can put res out of the equation and replace it with a fixed constant.

#### Grid ####

We setup a grid of NxN points per pixel (with N = 16, 32, 64... to be decided).

Each point is black or white and is represented by a single bit, which greatly reduces the amount of memory to handle.

Each new grain can be printed into the grid by a simple logical OR.

We can precompute the grains (the number of precomputations depends on the size of the grains and their variety)

One can also use an octagonal model for the shape of the grain (or even a square model if the grains are small enough) and fill the bits on the fly.

#### Downsampling using binary/boolean representation ####

For sigma = 0, it's easy, just count the 1 bits covering the pixel.
* From SSE4.2 there is `_mm_popcnt_u32`
* In vectorized form, see the paper “Faster Population Counts Using AVX2 Instructions.”
* With AVX512-BITALG there are the `_mmNNN_popcnt_epiMM` instructions.

For sigma > 0, we should be able to use lookup tables, of small size if possible (8 bits → 16 bits or float), one table per group of N points in the pixels covered by the kernel.
At the end, we just have to sum up the results.

Question about the layout: should we store the points line by line as part of a large image, or group them by pixels?

* Find a terminology for the grid and its points.

### Rendering ###

1. Each time a pixel is rendered, all neighboring pixels may be updated due to the overlapping grains.
2. To do the downsampling, all pixels potentially affected by the downsampling must have been rendered (which is identical to the current reference implementation). It is therefore not necessary that they are all complete.
3. But because of 1., it is necessary to increase the cache size, compared to the reference implementation.

### Layout ###

For N = 32, each pixel takes 128 bytes, that is 240 KiB per 1920 pix wide whole line.

* If we want to keep the data locality (for the cache), it is better to store the grid as individual pixels.
* However, in practical terms, it is simpler to make continuous horizontal lines, to keep line data contiguous. This would make it easier to render the grains (easier horizontal clipping/splitting)

Maybe we should do a mix of both?
For the pixel rendering, we can set up a buffer that covers all the pixels potentially covered by the grains of this pixel.
Other pixels data are copied from the main cache before rendering, and then updated afterwards.

Is it really worth it?
First we need to test with simple grids of single pixels.

### Resolution ###

* N is the grid resolution
* r is the grain radius (in pixels)
* Grain area is `pi * r^2`.
* Size (diameter) of a square grain in whole points:
```
d = round (sqrt (pi * (r * N)^2))
  = round (sqrt (pi) * r * N)
```
 
So, for N = 32:

* r = 0.100 pixels, size d = 5.67 → 6 points.
* r = 0.025 pixels, size d = 1.42 → 1 point.

We can use slightly rectangular grains too (w = h + {0 or 1}), to gain a bit more accuracy.

Given the significant difference between the actual value and the rounding, it is essential to take this error into account and update the grain radius passed to the statistical generator.
Therefore we get:
```
r' = d / (N * sqrt (pi))
```

The calculations are more complex with variable-size grains. Or we can just target the average radius.

### SIMD use ###

If the grids are stored per pixel, we can use SIMD to fill multiple rows at once.
With N = 32 again, we can fill 4 rows at a time (128-bit SIMD), so any grain of diameter ≤ 4 pix requires only one instruction (if not split across several pixels).
It may be worth templating the algo to handle 1–4 instructions without any loop, if the loop overhead is not too much less than the grain setup time.

There are some precalculations to do when generating the grains (integer coordinates, grain size, various masks…), they can be done in SIMD (GenGrain::build_cell)

It is possible to do the convolution in sigma = 0 mode with AVX2.
When sigma > 0, it seems difficult to use SIMD (too much lookup)

### Cache rationale ###

Currently the cache automatically calculates the distribution of the grains when a cell (pixel) is requested for the first time.
In principle, one can leave this operation as it is, and defer the rasterization of the grains.

Ideally, it should be half reversed.
We store the rasterization bitmap instead of the grain distribution.
When a cell is requested for the first time, its bitmap is reset.

There are two passes, which can obviously be done simultaneously, offseted:

1. Rendering the grains of a particular pixel. This is where the distribution is calculated, and then the raster rendering can be done. Other neighboring cells can be called to add the grain overflows.
2. Downsampling, when the area in the bounding box of the filter core has been fully rendered (1st pass).

Question (regarding the cache use): is it more relevant to do both passes separately (line by line for example), or as close as possible (pixel level)?
