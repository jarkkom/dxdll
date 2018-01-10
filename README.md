# DXDLL

## History

Originally DXDLL started as a project to add trilinear texture filtering to Operation Flashpoint as original OFP only used bilinear filtering, which made ground textures look fairly ugly (did anisotropic filtering even exist?). In Windows, DLL files in same directory have historically precedence over system files, so it was relatively simple to create your own Direct3D 8 DLL which exported exact same functions as Microsoft's d8d3.dll, and just loaded real DLL and called original implementations. 

After initial implementation worked, we started thinking what else would be possible, and next steps I think were fairly simple like FPS counters or screenshot functionality, or just messing around like rendering all graphics in wireframe to see how engine worked. Then postprocessing functionality was added and in the end Kegetys took it to the completely different level by adding water reflections. And the rest is history.

This code is a snapshot that I found on my old hard drive, and I'm not 100% it is same version that was released publicly, or whether it even compiles anymore (I don't have Visual Studio 2003? 2005 or whatever on hand that was required to build it).

## Internals

### Startup

DllMain function does just basic housekeeping, loading configuration and loading real d3d8.dll from system directory. It also hooks up keyboard handler using SetWindowsHookEx to enable hotkeys.

DXDLL implements `Direct3DCreate8` function that calls original function to acquire COM interface, calls `VirtualProtectEx` to enable writing to DLL memory and overrides `CreateDevice` function in COM interfaces vtable with our own. If you're more interested in details of COM objects, see [Raymond Chen's blog post "The layout of a COM object"](https://blogs.msdn.microsoft.com/oldnewthing/20040205-00/?p=40733)

`NewCreateDevice` function first disables "pure device" flag, so we can query some Direct3D state variables from GPU driver. It slowed down 3D rendering somewhat but at that point OFP was already heavily CPU-limited and GPU was never running at full utilization. `CreateDevice` then calls real `CreateDevice` function and again overwrites COM vtable of `IDirect3DDevice8` object with our own objects.

### Reflections

To render reflections, we would have to know what part of game world was being rendered. End goal to was render all terrain, sky and objects to reflection buffer with reflection matrix derived from current camera position, while skipping UI elements and water itself. Amd when water was being rendered, we would overlay water  with our reflection buffer texture. 

To achieve that, DXDLL would track what part of game world was being rendered by hooking `UnlockRect` function used to load textures and calculating CRC32 checksum of texture being loaded. If checksum matched, we then knew what texture handle matched which texture.

When game would call `DrawIndexedPrimitive` to render some object, we could know whether to render that object again to reflection buffer. Unfortunataly game would always render water as one of first things in frame, so we would have to use reflection buffer from previous frame. This is reason why reflections are always lagging when turning fast.

Texture checksum were also being used to detect when NVG, map or tracers were being rendered and override textures or apply special pixel shaders or postprocessing.

### Postprocessing

With postprocessing enable, `DrawIndexedPrimitive` would render everything to texture target instead of display buffers. Then when `EndScene` was called, texture target would be rendered onscreen with couple of special pixel shaders that added blur and bloom effects.
