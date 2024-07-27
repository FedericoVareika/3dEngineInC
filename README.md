### Running program
Make sure SDL2 and SDL2_ttf are installed
```
brew install sdl2
brew install sdl2_ttf
```
To run the compiled engine run 
```make run``` 
or
```./engine```

Currently the makefile is not os-agnostic, so it should only work for m1's

### Controls
```WASD``` to move player\
Arrows to angle camera\
```P``` for normal view\
```O``` for depth view (currently too bright)\
```I``` for mesh view\
```U``` to enable/disable GUI

### Functionality
>SDL2 is only linked to draw pixel buffers to screen, the processing of those pixel buffers are done by the program\
>SDL2_ttf is only linked to draw the GUI to screen

This 3D engine is built around the basic graphics pipeline:\
```View Transform -> Triangle Clipping -> Projection Transform -> Viewport Transform -> Rasterization```
which is all done by the CPU, including rasterization.\
The shading is face normal shading, explaining its roughness.\
The meshes are loaded through a primitive .obj loader.



### Screenshots
Normal view
<img width="1272" alt="Engine_Normal_View" src="https://github.com/user-attachments/assets/aac2dc77-c689-4a6a-805d-174582447282">

Mesh view
<img width="1277" alt="Screenshot 2024-07-23 at 15 19 12" src="https://github.com/user-attachments/assets/238a5a19-3fcf-4c4b-9bec-bc260b944d20">

Depth view
<img width="1270" alt="Screenshot 2024-07-23 at 15 20 00" src="https://github.com/user-attachments/assets/37200b62-df92-47cf-8469-5b538bb1d157">
