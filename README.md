## Goodboy Advance
Hi, this is the source for [_Goodboy Advance_](https://hotpengu.itch.io/goodboyadvance), a small GBA game made in 72 hours for Ludum Dare 43, by [me](https://twitter.com/exelotl) and [hot_pengu](https://twitter.com/hot_pengu).

Much has changed since this code was written. We learned a lot, and we're working on a full-size version of the game called _Goodboy Galaxy_!

Follow us for updates :)
<sup>(ok follow Rik for updates or me for RT'd puyo fanart and memes)</sup>


### Learning

See [this post](https://itch.io/post/633221) for a quick overview of what we used to make the game. The code was born from the ashes of a Super Crate Box [clone](https://twitter.com/exelotl/status/737086384868102145) that I was working on many years ago.   

Here are some things that this codebase does poorly, which we've fixed in the remake and/or you may want to do differently if you're aiming to make something bigger than a jam game:

* All declarations are in 1 header file. This was convenient for the jam but wouldn't cut it for a larger project. (Actually for Galaxy we rewrote everything in [Nim](https://nim-lang.org/) which has a decent module system instead of header files)

* All tile/map/palette locations are hardcoded or allocated by simple counters. Instead, you might want to use allocators such as the ones presented in [Gameboy Advance Resource Management](https://www.gamasutra.com/view/feature/131491/gameboy_advance_resource_management.php)

* Each type of entity gets its own statically allocated array (consequently there's a maximum of 20 breakable blocks in a level, 8 bullets alive at a time, etc.). This doesn't scale well: if the current level doesn't have any breakables, that array of 20 breakables is still taking up RAM for no good reason.
  * Consider putting all entities in 1 array and using some mechanism such as dispatch tables to give different behaviours to different entities.
  * On modern hardware, dynamic dispatch can be considered a thing to avoid in the interest of cache-friendliness, but on the GBA it turns out that _chasing pointers isn't particularly slow_ compared to the speed of the processor itself.

* The wrapping level mechanic introduces some nasty bugs. When the player goes beyond the right side of the level, they are teleported to the left side. This can cause objects to vanish or appear out of nowhere. It can also lead to getting stuck in walls or ceiling depending on placement. For the jam, we designed the levels to minimise the problem. In Galaxy, we fixed it with more thorough checking and a proper spawn system (which ties into the '1 array of entities' mentioned above).

* Streaming larger level maps (bigger than 2x2 screenblocks) - this code works but it's not that good. All 4 screenblocks are copied every frame even if nothing changed. Probably need to be smarter about that...

* Too much DMA - the game occasionally crashes on screen transitions on real hardware. I suspect this might be due to me using DMA copies to load absolutely everything. As Tonc says, ["don't wear it out"](https://www.coranac.com/tonc/text/dma.htm#ssec-func-use). I'd recommend using memcpy32 for most things instead. (_If somebody knows more about this, I'd really like to know!_)

* Tooling: We have a script that converts Tiled level data into C code. Besides that, we used grit for all our image data, including the level maps themselves. In Galaxy we realised we need more control, so we ditched grit and ended up making a lot more scripts that generate all kinds of data from various config files. It takes time to get this stuff working, but it makes adding content to the game much quicker and less error-prone! tl;dr Don't be afraid to write code that spits out more code.

* Polish: The game has some niceties such as muzzle flash / impact fx / screenshake, but the position-locked camera and the static background let things down a little. In Galaxy we use HBlank DMA to get some gorgeous [multi-layer parallax](https://twitter.com/hot_pengu/status/1142904127594401797) from just a single background.
