Michael Georgariou
Assignment 4

To control the camera:
- Controls are non-inverted. This is intentional, as I am more used to
  controlling non-inverted games. Also note that I did not use the scroll
  callback for movement, as I wanted the controls to be constant (like in
  most video games). This is also why the cursor snaps to the center forever.
- Moving the mouse up/down adjusts pitch, and moving the mouse left/right
  adjusts yaw. Pitch is limited to 80 degrees in both directions.
- "W" moves forward, "S" moves backward, "A" moves left and "D" moves right
  (all relative to the current gaze).
- "Left shift" moves down and "Space" moves up (to make the controls feel more
  like Minecraft!)

Skybox:
- The skybox is simply a picture of outside. It's quite pretty, and it gives a
  very homely feel to the scene :)

Textured mesh:
- There is a sphere in the sky that is textured to look like a mini globe.

Lighting/Models:
- There is a point light at x=0, y=10, z=-5. This is what illuminates the
  scene.
- All non-textured models use materials similarly to in previous projects --
  textured models are also shaded using Blinn Phong (with the texture color
  replaced the MatDif, MatSpec, and MatAmb)
- The bunny and Mario both have no normal data in their obj file, so this is
  being computed manually.

World Cconstruction:
- The models are all meant to represent a scene of Mario and Peach basically
  worshipping the bunny, as they recognize the people who created their models
  likely used the Stanford Bunny to test models before using theirs.
- The three "character" models (the bunny, Mario, and Peach) are all colored as
  a shiny blue plastic, to show off how the lighting works on them clearly.
- The bottom plane is colored as a flat grey (meant to look like concrete).
- The globe up top is intenionally very shiny (I wanted it to look like one of
  those laminated globes my dad has.

