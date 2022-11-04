# LIV SDK v1.6 for Unreal

## Plugin Installation

1. If not already present, create a directory called `Plugins` in your Unreal project's directory.  
![Ensure a directory named Plugins is present in your UE4 project directory](Resources/Docs/Liv-Project-Directory.png)
2. Copy this directory into the Plugins directory, it should be named 'LIV'. If you have a blueprint only project (no C++ modules) then you can now just open your project as usual and skip to step 6.  
![Copy this directory into the Plugins directory](Resources/Docs/Liv-Plugin-Directory.png)
3. Generate project files for your project (right click your [Project-Name].uproject file > Generate Visual Studio project files)  
![Right click your uproject file and generate project files](Resources/Docs/Liv-Generate-Project-Files.png)
4. Open the Visual Studio solution ([Project-Name].uproject) and make sure you can see the LIV plugin in the solution explorer.  
![Ensure the LIV plugin is present in Solution Explorer](Resources/Docs/Liv-Solution-Explorer.png)
5. Build and run your project to launch the editor. You can just press F5 or click the button in the screenshot below.  
![Press F5 or click this button to build and launch UE4](Resources/Docs/Liv-Debugger-Button.png)
6. You can check the LIV plugin is installed correctly by navigating to Edit > Plugins.  
![Access plugins window via Edit > Plugins](Resources/Docs/Liv-Access-Plugins.png)
7. Check you see the LIV plugin which will typically be at the bottom of the lefthand list under `Project`.  
![Access plugins window via Edit > Plugins](Resources/Docs/Liv-Plugin-Window.png)

## Integrating LIV

### Quick Integration

If you downloaded the plugin from the [LIV Dev Portal](https://dev.liv.tv/) then your project will already be linked. If not however follow the prompt in editor or click on the LIV icon in the toolbar to get set up.

LIV works out of the box without configuration but for more advanced customisation see "Advanced Integration" below.

### Advanced Integration

The LIV plugin provides a variety of rendering methods for capturing your gameplay. To change the method used you can select a different class for "Capture Method" in the LIV Plugin Settings. You can access this from Edit > Project Settings, in the LIV section under the Plugins header.

![Access Project Settings via Edit > Project Settings](Resources/Docs/Liv-Project-Settings.png)

The default capture method is the new `ULivCaptureSingle` as it is more performant and doesn't require the global clip plane. It also doesn't suffer from issues that arise from multiple captures. 

If you want transparent objects in the foreground to render over the top of the player / avatar then then you must use a global clip plane based capture method.

Below we will outline some details of the capture methods LIV provides.

#### ULivCaptureSingle

New rendering method available from v1.6.0. This leverages scene view extensions and Render Dependency Graph (RDG) to capture more efficiently. With this method only one scene render is issued and passes are added in a scene view extension to separate the foreground and background. However this does not support foreground transparency.

#### ULivCaptureMulti (WIP)
This is a new work in progress rendering method that supports foreground transparency but requires the global clip plane to be enabled. It issues two scene captures and combines them in a scene view extension. We are awaiting a [pull request](https://github.com/EpicGames/UnrealEngine/pull/9009) for this method to work properly so it is not recommended just yet. However if you were using `ULivCaptureGlobalClipPlanePostProcess` then you may find this a bit more efficient. 

#### ULivCaptureCombo (WIP)
This method hasn't yet been implemented and is reserved for future work so please do not use this.

#### ULivCaptureGlobalClipPlaneNoPostProcess

This method captures your gameplay twice each frame for the foreground and the background. It requires support for the global clip plane in `Project Settings`. The captured images do not have post processing and this method requires 3 render targets.

#### ULivCaptureGlobalClipPlanePostProcess

This method requires capturing your gameplay three times each frame to provide post processed images with the correct masks to LIV. This method also requires support for the global clip plane. We require 4 render targets for this method.

#### ULivCaptureMeshClipPlaneNoPostProcess

This method uses a mesh instead of the global clip plane to separate foreground and background and requires capturing twice each frame. 4 render targets are used in this method and the images will not using post processing.

#### ULivCaptureMeshClipPlanePostProcess

This method also uses a mesh instead of the global clip plane and does support post processing but requires 3 captures each frame. Overally we also require 5 render targets for this method.

### Global Clip Plane

Some of LIV's capture method require the Global Clip Plane to be enabled. Unless you are already utilising the Global Clip Plane then it is best to use the methods that do not require it as there is a [performance impact](https://docs.unrealengine.com/en-US/BuildingWorlds/LightingAndShadows/PlanarReflections/index.html) from enabling it.

If you wish to enable support for the global clip plane it can be found in `Project Settings` and requires restarting the editor.  

![Enabling support for the Global Clip Plane in Project Settings](Resources/Docs/Liv-Enable-Global-Clip-Plane.png)

### Show and Hide Actors and Components

By getting a reference to the Liv Local Player Subsystem via a Player Controller you can show and hide actors and components. A typical place to put the calls would be within the Begin Play and End Play events of your pawn. Make sure to check the validity of the subsystem!

![Blueprint that hides an actor and a component.](Resources/Docs/Liv-Show-Hide.png)

## Testing

**Note**: These instructions are the minimum you need to check if your integration worked.

**We suggest testing with a packaged build of your application!**

- Download the [LIV App]( https://liv.tv/download#liv-app ).
- Run it, install the Virtual Camera driver.
- In LIV, launch the compositor.
- Run your project.
- Using "Manual" capture mode, pick your project from the list.

If all has gone well, you'll see your application in LIV's output! 

## Known Issues

### UE-86191

We are aware of a performance bug ([UE-86191](https://issues.unrealengine.com/issue/UE-86191)) affecting UE4.23 onwards, if this issue affects you or your customers we recomend voting on the issue.

Adding the following to your `DefaultEngine.ini` may help mitigate the performance issues:

```
[/Script/Engine.RendererSettings]
r.SceneRenderTargetResizeMethod=2
r.SceneRenderTargetResizeMethodForceOverride=True
```

### Auto Exposure / Eye Adaptation

Auto exposure / Eye Adaptation currently can cause the foreground and background to have different luminance. Unreal uses the average luminance of the frame to calculate exposure. As the foreground will almost always contain a lot more black than the background this means the exposure values are different which causes a mismatch in luminance on the output frame. 

In general a lot of VR developers will "disable" Auto-Exposure at the project level. However this actually isn't enough to prevent the issue as it will still adjust the image based on its luminance. To properly disable it `Metering Mode` must be set to manual, `Exposure Compensation` to zero, and `Apply Physical Camera Exposure` should be disabled. Unfortunately this tends to change the look of the scene dramatically so it is not a great solution.

Currently there's no way without engine modification to use the same eye adaptation texture on multiple scene captures so that the same luminance can be applied. We have a [pull request](https://github.com/EpicGames/UnrealEngine/pull/9009) open that exposes the eye adaptation in scene view extensions. This lets us copy the eye adaptation texture from the background and apply it to the foreground which results in the same luminance being applied.

### Bloom

Bloom can cause minor mismatches between the foreground and the background. This is because in the foreground there is generally a fair amount of black. We'll work on solving this problem but it will likely require another pull request like the one mentioned for eye adaptation above however the mismatch is usually very subtle.

## Further Help & Feedback

We want to hear your ideas, plans, and what you're going to end up doing with the SDK - if you've got an exotic idea that you might need a hand with - reach out. We're just as enthusiastic as you <3

We're always available through our , so please pop in if you need help. If you're not familiar with Discord, it's a popular gaming-oriented instant messenger & VOIP client.

Alternately, you can email us through techteam@liv.tv!

### Support Links

* [Developer Portal](https://dev.liv.tv/)
* [User Support](https://help.liv.tv/hc)
* [Discord Community](http://liv.chat)
* [Contact Form](https://help.liv.tv/hc/requests/new)
