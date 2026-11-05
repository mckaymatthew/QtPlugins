* The goal of this project is to provide deployable Qt6 plugins (as DLL on Win, SO on Linux) that integrate into an existing application.
* An example host application should be built. 
  * This application should let you load extensions from a directory.
  * It should be minimal so that the code can be copied into the real host application later.
* An example plugin should be built. 
  * The example plugin will be a tic tac toe game
  * It should use 3d rendering for the game board
* For initial work, this will be on a headless Linux box VM. 
  * On the linux box, install the tools you need to do this. 
* Later for interactive graphics work, you will be on a Mac M2.
  * On the Mac, always ask permission to install things.
* If possible, target Qt 6.4.2, or something in that same era of Qt. 
  * Don't compile Qt from source or anything, use avaiable binary distributions for both platforms, newer versions are OK but avoid going to major new ones.
* Later, we will develop a more complex 3D accelerated game. 
