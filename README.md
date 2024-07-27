### Running program
Make sure SDL2 and SDL2_ttf are installed
```
brew install sdl2
brew install sdl2_ttf
```
To run and compile the engine run 
```bash build.sh {object name} {texture name}``` 
or to just run the compiled engine run
```./engine {object name} {texture name}```
where the object name must be present in ```assets/objects``` and the texture must be present in ```assets/sprites```.\
If no texture name is provided, then the rendered object will be textureless.\
If no object name is provided, then the Tree object will be rendered textureless.\

Currently the makefile is not os-agnostic, so it should only work for arm macs.

#### Recommended to see capabilities
The map of Spyro 1 'Artisans Hub':
```
./engine ArtisansHub High
```


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
The meshes are loaded through a primitive .obj loader. This loader is capable of loading vertices, vertex normals, texture coordinates, and n-sided faces.\
Textures are loaded



### Screenshots
##### Normal view
<img width="1278" alt="Artisans Hub with High texture" src="https://github.com/user-attachments/assets/6c4af7ed-15eb-4f7d-a218-0d98d33e8f95">
<img width="1268" alt="Screenshot 2024-07-27 at 20 14 46" src="https://github.com/user-attachments/assets/60b2ca4b-e98e-44de-b595-2b7ca49521b0">
<img width="1271" alt="Airboat" src="https://github.com/user-attachments/assets/d32f7c7b-8e54-49c8-adda-bad56d620f33">

#### Mesh view
<img width="1278" alt="Artisans Hub mesh" src="https://github.com/user-attachments/assets/7dea43a9-9fb3-4672-a48c-64815d454ac2">
<img width="1268" alt="Airboat Mesh" src="https://github.com/user-attachments/assets/2bd3d3e5-60de-4708-8218-214305b18349">
<img width="1269" alt="Screenshot 2024-07-27 at 20 05 44" src="https://github.com/user-attachments/assets/1fe1ae87-59c0-4f3f-b027-673027c7d507">

#### Depth view
<img width="1274" alt="Screenshot 2024-07-27 at 20 13 20" src="https://github.com/user-attachments/assets/25a8a7eb-852d-4dbb-a333-8a30748873bb">
<img width="1270" alt="Screenshot 2024-07-23 at 15 20 00" src="https://github.com/user-attachments/assets/37200b62-df92-47cf-8469-5b538bb1d157">

