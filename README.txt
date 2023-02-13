Super Mario 64 Coin Collector

TO COMPILE:
- This project depends on the irrKlang and FreeType libraries to compile. I did
  this by editing my Linker settings in Visual Studio, but to compile on
  another computer you will also have to link these libraries. (I could not
  figure out to how to modify the Makefile to auto-link these libraries.) In
  case you do not have the libraries installed, I have included their lib and
  dll files in the "library" sub-directory.

Features:
- Mouse camera control, with movement relative to the camera using WASD.
- Skybox
- Textured meshes throughout the world
- Lighting comes from the camera so everything appears bright and shiny from
  all angles
- Particles spawned whenever a coin is collected
- Text HUD to display how many coins have been collected
- Background music and a jingle when a coin is collected
